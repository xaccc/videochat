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
    static void* _play(void* pVideoChat);
    static size_t convert_UID_to_RTMP_callback(void *ptr, size_t size, size_t nmemb, void *stream);
    static bool get_rtmp_url(VideoChat* pThis);
    
private:
    SpeexCodec* pSpeexCodec;
    AudioOutput* pAudioOutput;
    H264Decodec* pH264Decodec;
    VideoRender* pVideoRender;

    RTMP* pRtmp;

    char* szUrl;
    char* szRTMPUrl;

    pthread_t thread_play;
    pthread_attr_t thread_attr;

    Mutex handleLock;
    Mutex renderLock;
    Mutex wait_mutex;
    
    bool m_playing;
    bool m_paused;

public:
    JavaVM* m_jVM;
    jobject m_jObject;
};


#endif //__VIDEOCHAT_H__
