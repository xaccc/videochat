#include "VideoChat.h"
#include "json.h"
extern "C"{
#include <librtmp/http.h>
#include <unistd.h>
}

///////////////////////////////////////////////////////////////////////////////
//
// class VideoChat
//
///////////////////////////////////////////////////////////////////////////////

#define MAX_RTMPURL_SIZE    4096

#define USE_BUFFER 0


static void DumpBuffer(uint8_t* buffer, uint size)
{
    int offset = 0;
    int index = 0;
    while(offset < size && offset < 128) {
        LOGI("[DumpBuffer](%d - %02d): %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", size, index++,
            buffer[offset + 0], buffer[offset + 1], buffer[offset + 2], buffer[offset + 3],
            buffer[offset + 4], buffer[offset + 5], buffer[offset + 6], buffer[offset + 7],
            buffer[offset + 8], buffer[offset + 9], buffer[offset + 10], buffer[offset + 11],
            buffer[offset + 12], buffer[offset + 13], buffer[offset + 14], buffer[offset + 15]);

        offset += 16;
    }
}



VideoChat::VideoChat()
    : m_playing(false)
    , m_paused(false)
//    , pRtmp(NULL)
    , szUrl(NULL)
    , szRTMPUrl(NULL)
    , pSpeexCodec(NULL)
    , pAudioOutput(NULL)
    , pH264Decodec(NULL)
    , pVideoRender(NULL)
	, data_buffer(2)
{
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

    szUrl = new char[MAX_RTMPURL_SIZE]; szUrl[0] = '\0';
    szRTMPUrl = new char[MAX_RTMPURL_SIZE]; szRTMPUrl[0] = '\0';
}

VideoChat::~VideoChat()
{
    Release();
    if (szUrl) delete[] szUrl;
    if (szRTMPUrl) delete[] szRTMPUrl;
}


int VideoChat::Init()
{
    return 0;
}

void VideoChat::Release()
{
    StopPlay();
}

int VideoChat::Play(const char* url)
{
    AutoLock autoLock(handleLock);
    
    if (m_playing) return -1;
    m_playing = true;

    memset(szUrl, 0, MAX_RTMPURL_SIZE);
    strcpy(szUrl, url);
    
//    pthread_attr_t attr;
//    pthread_attr_init(&attr);
//    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    wait_stop.Lock();
    pthread_create(&thread_network, NULL, &_network, (void*)this);
    pthread_create(&thread_player, NULL, &_player, (void*)this);

    return 0;
}

void VideoChat::PausePlayer(bool paused)
{
//    AutoLock autoLock(handleLock);
//    AutoLock lock(renderLock);
//    m_paused = paused;
//    if (pAudioOutput) pAudioOutput->pause(paused);
//    if (pVideoRender) pVideoRender->pause(paused);
}

int VideoChat::StopPlay()
{
    AutoLock autoLock(handleLock);
    
    if (!m_playing) return -1;
    m_playing = false;
    
    wait_stop.Unlock();
    pthread_join(thread_network, NULL);
    pthread_join(thread_player, NULL);

    return 0;
}

//
// {
//     "Method":"liveUrl",
//     "UID":"user1",
//     "Host":"183.203.16.207",
//     "Port":8100,
//     "Application":"live",
//     "Session":"24c9e15e52afc47c225b757e7bee1f9d"
// }
size_t VideoChat::convert_UID_to_RTMP_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
    LOGI("[HTTP_get] enter convert_UID_to_RTMP_callback!");
    LOGI("[HTTP_get] size:%d, data: %s", size, (char*)ptr);

    VideoChat* pThis = (VideoChat*)stream;
    json_value* data_ptr = json_parse((char*)ptr, strlen((char*)ptr));

    if (data_ptr) {
        json_value& data = *data_ptr;

        memset(pThis->szRTMPUrl, 0, MAX_RTMPURL_SIZE);

        const char* host = (const char*)data["Host"];
        if (host && strlen(host) > 0) {
        	snprintf(pThis->szRTMPUrl, MAX_RTMPURL_SIZE, "rtmp://%s:%d/%s/%s live=1 buffer=3000 timeout=10",
        		host,
                (int)(json_int_t)data["Port"],
                (const char*)data["Application"],
                (const char*)data["Session"]);
        }
        LOGI("[HTTP_get] json_parse: %s", pThis->szRTMPUrl);
        // relase
        json_value_free(data_ptr);
    } else {
        LOGI("[HTTP_get] json_parse error");
    }
    return 0;
}

bool VideoChat::get_rtmp_url(VideoChat* pThis)
{
	/*
    int test = 7;
    if (pThis->szUrl[test++] != '1' ||
        pThis->szUrl[test++] != '2' ||
        pThis->szUrl[test++] != '2' ||
        pThis->szUrl[test++] != '.' ||
        pThis->szUrl[test++] != '0' ||
        pThis->szUrl[test++] != '.' ||
        pThis->szUrl[test++] != '6' ||
        pThis->szUrl[test++] != '7' ||
        pThis->szUrl[test++] != '.' ||
        pThis->szUrl[test++] != '1' ||
        pThis->szUrl[test++] != '8' ||
        pThis->szUrl[test++] != '0') { // http://122.0.67.180
        LOGI("Media Platform Address Invalid!");
        return false;
    }
    */
    HTTP_ctx http_c = {0};
    http_c.date = (char*)"\0";
    http_c.data = (void*)pThis;
    if (HTTPRES_OK == HTTP_get(&http_c, pThis->szUrl, convert_UID_to_RTMP_callback)) {
        return true;
    } else {
        // TODO: retry ???
        LOGI("[HTTP_get] failed!");
    }

    return false;
}

static RTMP* my_rtmp_connect(const char* rtmpurl)
{
    RTMP* pRtmp;
    
    pRtmp = RTMP_Alloc();
    RTMP_Init(pRtmp);
    LOGI("RMPT Connect: %s\n", rtmpurl);
    if (!RTMP_SetupURL(pRtmp, (char*)rtmpurl)) {
        LOGE("Play RTMP_SetupURL error\n");
        return NULL;
    }

    if (!RTMP_Connect(pRtmp, NULL) || !RTMP_ConnectStream(pRtmp, 0)) {
        LOGI("RTMP_Connect or RTMP_ConnectStream error\n");
        RTMP_Free(pRtmp);
        pRtmp = NULL;
        return NULL;
    }
    
    LOGI("RTMP_Connected\n");

    return pRtmp;
}

void* VideoChat::_network(void* pVideoChat)
{
    VideoChat* pThis = (VideoChat*)pVideoChat;

	JNIEnv *env;
    if (((JavaVM*)pThis->m_jVM)->AttachCurrentThread(&env, NULL) < 0)
    	LOGE("jvm->AttachCurrentThread failed!");

    jclass clazz = env->GetObjectClass(pThis->m_jObject);
    jmethodID jEventMethodId = env->GetMethodID(clazz, "onJniEvent", "(I)V"); //jmethodID
    LOGI("env->GetMethodID = 0x%08X", (int)jEventMethodId);
    LOGI("Network thread begin...");

	//Mutex wait_mutex;
	pthread_cond_t cond;
	pthread_cond_init(&cond, NULL);

	struct timespec timeout;

    //
    // while do connect media server
    //
    while(pThis->m_playing)
    {
    	pThis->m_bFirstPacket = true;
        env->CallVoidMethod((jobject)pThis->m_jObject, jEventMethodId, 1);
        LOGI("Fetch Media Server Address ...");

        if (pThis->m_paused) {
        	clock_gettime(CLOCK_REALTIME, &timeout);
        	timeout.tv_sec += 1;
        	pthread_cond_timedwait(&cond, pThis->wait_stop.mutex(), &timeout);
    		continue;
    	}

    	//
        // RTMP url from Service API
        //
        if (!get_rtmp_url(pThis) || strlen(pThis->szRTMPUrl) == 0) {
            LOGI("Fetch Media Server Address Failed!");
            env->CallVoidMethod((jobject)pThis->m_jObject, jEventMethodId, -1);

        	clock_gettime(CLOCK_REALTIME, &timeout);
        	timeout.tv_sec += 1;
    		pthread_cond_timedwait(&cond, pThis->wait_stop.mutex(), &timeout);
            continue; // connect faild!
        }

        LOGI("start to Connect RTMP URL...");

        RTMP* pRtmp = my_rtmp_connect(pThis->szRTMPUrl);
        if (pRtmp == NULL) {
        	env->CallVoidMethod((jobject)pThis->m_jObject, jEventMethodId, -2);

        	clock_gettime(CLOCK_REALTIME, &timeout);
        	timeout.tv_sec += 1;
    		pthread_cond_timedwait(&cond, pThis->wait_stop.mutex(), &timeout);
    		continue; // connect faild!
        }

        bool bFirstPacket = true;
        while(pThis->m_playing)
        {
        	if (pThis->m_paused) {
            	clock_gettime(CLOCK_REALTIME, &timeout);
            	timeout.tv_sec += 1;
        		pthread_cond_timedwait(&cond, pThis->wait_stop.mutex(), &timeout);

        		break; // paused to reconnect
        	}


        	fuckBuffer::fuckBufferOne* bufferOne = new fuckBuffer::fuckBufferOne();
            if(!RTMP_GetNextMediaPacket(pRtmp, &bufferOne->rtmp_packet)) {
            	env->CallVoidMethod((jobject)pThis->m_jObject, jEventMethodId, -3);
            	LOGE("recv packet error!!!");
                break; // recv error
            }

            if (bufferOne->rtmp_packet.m_packetType == RTMP_PACKET_TYPE_AUDIO ||
				bufferOne->rtmp_packet.m_packetType == RTMP_PACKET_TYPE_VIDEO)
			{
				LOGI("recv packet size:%6d, TS:%7d, A:%d V:%d",
						bufferOne->rtmp_packet.m_nBodySize,
						bufferOne->rtmp_packet.m_nTimeStamp,
						bufferOne->rtmp_packet.m_packetType == RTMP_PACKET_TYPE_AUDIO,
						bufferOne->rtmp_packet.m_packetType == RTMP_PACKET_TYPE_VIDEO);
				pThis->data_buffer.put(bufferOne);
			}
        }

        //
        // free rtmp
        //
        if (RTMP_IsConnected(pRtmp)) {
            RTMP_Close(pRtmp);
        }

        RTMP_Free(pRtmp);
        pRtmp = NULL;

    }
    pthread_cond_destroy(&cond);
    
    ((JavaVM*)pThis->m_jVM)->DetachCurrentThread();
    LOGE("Network thread exit...");
    return 0;
}

void* VideoChat::_player(void* pVideoChat)
{
    VideoChat* pThis = (VideoChat*)pVideoChat;

	JNIEnv *env;
    if (((JavaVM*)pThis->m_jVM)->AttachCurrentThread(&env, NULL) < 0)
    	LOGE("jvm->AttachCurrentThread failed!");

    jclass clazz = env->GetObjectClass(pThis->m_jObject);
    jmethodID jEventMethodId = env->GetMethodID(clazz, "onJniEvent", "(I)V"); //jmethodID
    LOGI("Play thread begin...");

    //
    // init module(audio video decode)
    //
    pThis->pSpeexCodec = new SpeexCodec();
    pThis->pAudioOutput = new AudioOutput();
    pThis->pH264Decodec = new H264Decodec();
    LOGI("modules initialed!");

    //
    // alloc audio&video buffer
    //
    AVFrame *picture = avcodec_alloc_frame();
    const int max_audio_buffer_size = 100;

    int audio_frame_size = pThis->pSpeexCodec->audio_frame_size();
    short *audio_buffer[max_audio_buffer_size] = {0};
    audio_buffer[0] = new short[audio_frame_size*max_audio_buffer_size];
    for(int i=1; i<max_audio_buffer_size; i++)
        audio_buffer[i] = audio_buffer[i-1] + audio_frame_size;
    LOGI("buffer initialed!");


	//Mutex wait_mutex;
	pthread_cond_t cond;
	pthread_cond_init(&cond, NULL);
	struct timespec timeout, fristTime, justTime;

	LOGI("Do loop play...");
	while(pThis->m_playing) {
    	fuckBuffer::fuckBufferOne *bufferOne = NULL;

    	bufferOne = pThis->data_buffer.get();

		if (bufferOne == NULL) {
			clock_gettime(CLOCK_REALTIME, &timeout);
			timeout.tv_nsec += 1000;
			pthread_cond_timedwait(&cond, pThis->wait_stop.mutex(), &timeout);
			continue;
		}
		LOGI("data_buffer.get A:%d, V:%d ts:%d",
				bufferOne->rtmp_packet.m_packetType == RTMP_PACKET_TYPE_AUDIO,
				bufferOne->rtmp_packet.m_packetType == RTMP_PACKET_TYPE_VIDEO,
				bufferOne->rtmp_packet.m_nTimeStamp);

		//LOGI("got_one_buf ts:%d", got_one_buf->timestamp);

		if (pThis->m_bFirstPacket){
			pThis->m_bFirstPacket = false;
			env->CallVoidMethod((jobject)pThis->m_jObject, jEventMethodId, 3);
			clock_gettime(CLOCK_REALTIME, &fristTime);
			pThis->m_firstTimestamp = bufferOne->rtmp_packet.m_nTimeStamp;
		}

		uint32_t delta;
		clock_gettime(CLOCK_REALTIME, &justTime);
		delta = (justTime.tv_sec - fristTime.tv_sec) * 1000 + justTime.tv_nsec / 1000000 - fristTime.tv_nsec / 1000000;

		if (delta < (bufferOne->rtmp_packet.m_nTimeStamp - pThis->m_firstTimestamp))
		{
			justTime.tv_nsec += ((bufferOne->rtmp_packet.m_nTimeStamp - pThis->m_firstTimestamp) - delta) * 1000000;
			pthread_cond_timedwait(&cond, NULL, &justTime);
		}

		if (bufferOne->rtmp_packet.m_packetType == RTMP_PACKET_TYPE_AUDIO && 0xB2 == bufferOne->rtmp_packet.m_body[0])
		{
			LOGI("got_one_audio_packet ts:%d", bufferOne->rtmp_packet.m_nTimeStamp);
			int dec_audio_count = pThis->pSpeexCodec->decode(bufferOne->rtmp_packet.m_nTimeStamp, bufferOne->rtmp_packet.m_body + 1, bufferOne->rtmp_packet.m_nBodySize - 1, audio_buffer);
			if (dec_audio_count > 0)
				pThis->pAudioOutput->play(bufferOne->rtmp_packet.m_nTimeStamp, audio_buffer[0], audio_frame_size*dec_audio_count);
		}
		else if (bufferOne->rtmp_packet.m_packetType == RTMP_PACKET_TYPE_VIDEO && 7 == (bufferOne->rtmp_packet.m_body[0] & 0x0f))
		{
			LOGI("got_one_video_packet ts:%d", bufferOne->rtmp_packet.m_nTimeStamp);
			// H264 Video Data
			int got_picture = 0;
			pThis->pH264Decodec->decode((uint8_t*)bufferOne->rtmp_packet.m_body, bufferOne->rtmp_packet.m_nBodySize, picture, &got_picture);

			AutoLock lock(pThis->renderLock);
			if (got_picture && pThis->pVideoRender) {
				// draw buffer
				pThis->pVideoRender->setFrame( picture,
					pThis->pH264Decodec->getWidth(),
					pThis->pH264Decodec->getHeight());
			}
		}
		else
		{
			env->CallVoidMethod((jobject)pThis->m_jObject, jEventMethodId, -4);
		}

		delete bufferOne;
    }

    env->CallVoidMethod((jobject)pThis->m_jObject, jEventMethodId, 4);

	pThis->renderLock.Lock();
	if (pThis->pVideoRender)
		pThis->pVideoRender->clearFrame();
	pThis->renderLock.Unlock();

	pthread_cond_destroy(&cond);

    //
    // free audio&video buffer
    //
    if (audio_buffer[0]) delete[] audio_buffer[0];
#if USEFFMPEG
    if (picture) avcodec_free_frame(&picture);
#else
    if (picture) av_free(picture);
#endif

    SAFE_DELETE(pThis->pSpeexCodec);  LOGI("release SpeexCodec");
    SAFE_DELETE(pThis->pAudioOutput); LOGI("release AudioOutput");
    SAFE_DELETE(pThis->pH264Decodec); LOGI("release H264Decodec");

    ((JavaVM*)pThis->m_jVM)->DetachCurrentThread();
    LOGE("Play thread exit...");
    return 0;
}

