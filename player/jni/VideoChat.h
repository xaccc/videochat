#if !defined(__VIDEOCHAT_H__)
#define __VIDEOCHAT_H__





#ifdef __cplusplus
extern "C" {
#endif

#include <librtmp/rtmp.h>
#include <librtmp/log.h>

//#include <libx264/x264.h>
//#include <libx264/x264_config.h>


#ifdef __cplusplus
}
#endif


#include "utils.h"

#include "AudioDecoder.h"
#include "AudioRender.h"

#include "VideoDecoder.h"
#include "VideoRender.h"



class fuckBuffer
{
public:
	class fuckBufferOne
	{
	public:
		fuckBufferOne() : next(NULL),prev(NULL)  {
			memset(&rtmp_packet, 0, sizeof(rtmp_packet));
			RTMPPacket_Reset(&rtmp_packet);
		}
		~fuckBufferOne() {
			RTMPPacket_Free(&rtmp_packet);
		}

		RTMPPacket rtmp_packet;

		fuckBufferOne* next;
		fuckBufferOne* prev;
	};

public:
	fuckBuffer(const uint32_t hold_sec) : m_hold(hold_sec*1000) {
		m_buffers = NULL;
		m_bufferSize = 0;
		m_beginTS = 0;
		m_endTS = 0;
		m_buffered = false;
	}
	~fuckBuffer(){
		m_lock.Lock();
		fuckBufferOne* last = m_buffers;
		while(last){
			fuckBufferOne* one = last;
			last = last->next;
			delete one;
		}
	}

	void put(fuckBufferOne* one){
		m_lock.Lock();

		// append to link
		if (m_buffers) {
			m_beginTS = m_endTS = m_buffers->rtmp_packet.m_nTimeStamp;
			fuckBufferOne* last = m_buffers;
			while(last->next){
				m_beginTS = min(m_beginTS, last->rtmp_packet.m_nTimeStamp);
				m_endTS = max(m_endTS, last->rtmp_packet.m_nTimeStamp);
				last = last->next;
			}
			last->next = one;
			one->prev = last;

			m_beginTS = min(m_beginTS, one->rtmp_packet.m_nTimeStamp);
			m_endTS = max(m_endTS, one->rtmp_packet.m_nTimeStamp);
		} else {
			m_buffers = one;
			m_beginTS = m_endTS = one->rtmp_packet.m_nTimeStamp;
		}

		if (!m_buffered && m_endTS - m_beginTS >= m_hold) {
			m_buffered = true; // buffer is full
			LOGE("Buffer is Full!!!");
		}
		m_bufferSize += one->rtmp_packet.m_nBodySize;

		m_lock.Unlock();
		LOGI("buffer size: %d", m_bufferSize);

		if (m_bufferSize > 1024*1024) // release old buffer over 1MB
		{
			LOGE("Buffer Over 1MB, to release...");
			fuckBufferOne* one = get();
			if (one) delete one;
		}
	}

	fuckBufferOne* get()
	{
		AutoLock lock(m_lock);
		if (!m_buffered)
			return NULL;

		fuckBufferOne *minBuffer,*minBufferPrev,*last;
		minBuffer = minBufferPrev = last = m_buffers;
		while(last){
			if (minBuffer->rtmp_packet.m_nTimeStamp > last->rtmp_packet.m_nTimeStamp){
				minBuffer = last;
			}
			last = last->next;
		}

		// remove from link
		if (minBuffer->prev)
			minBuffer->prev->next = minBuffer->next;
		if (minBuffer->next)
			minBuffer->next->prev = minBuffer->prev;

		if (m_buffers == minBuffer)
			m_buffers = minBuffer->next;

		//LOGE("[FUCK_BUFFER] got one buffer timestamp: %9d, size:%9d", minBuffer->timestamp, minBuffer->buffer_size);
		if (m_buffers == NULL) {
			m_buffered = false; // start to buffer
			LOGE("Buffer is Empty!!!");
		}

		minBuffer->rtmp_packet.m_nTimeStamp;

		m_bufferSize -= minBuffer->rtmp_packet.m_nBodySize;

		return minBuffer;
	}

private:
	fuckBufferOne* m_buffers;
	uint32_t       m_bufferSize;
	uint32_t       m_hold;

	uint32_t       m_beginTS;
	uint32_t       m_endTS;

	bool 		   m_buffered;
	Mutex          m_lock;
};

//
// video chat
//
class VideoChat
{
public:
    VideoChat();
    ~VideoChat();

    int Init();
    void Release();
    void setVideoRender(VideoRender* _pVideoRender) { AutoLock lock(renderLock); pVideoRender = _pVideoRender; }

    int Play(const char* szUrl);
    int StopPlay();
    void PausePlayer(bool paused);

private:
    static void* _network(void* pVideoChat);
    static void* _player(void* pVideoChat);

    static size_t convert_UID_to_RTMP_callback(void *ptr, size_t size, size_t nmemb, void *stream);
    static bool get_rtmp_url(VideoChat* pThis);
    
private:
    SpeexCodec* pSpeexCodec;
    AudioOutput* pAudioOutput;
    H264Decodec* pH264Decodec;
    VideoRender* pVideoRender;

    //RTMP* pRtmp;

    char* szUrl;
    char* szRTMPUrl;

    pthread_t thread_player;
    pthread_t thread_network;

    pthread_attr_t thread_attr;

    Mutex handleLock;
    Mutex renderLock;
    Mutex wait_stop;
    
    bool m_playing;
    bool m_paused;

    fuckBuffer data_buffer;
    uint32_t   m_firstTimestamp;
    bool       m_bFirstPacket;

public:
    JavaVM* m_jVM;
    jobject m_jObject;
};


#endif //__VIDEOCHAT_H__
