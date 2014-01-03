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
    , pRtmp(NULL)
    , szUrl(NULL)
    , szRTMPUrl(NULL)
    , pSpeexCodec(NULL)
    , pAudioOutput(NULL)
    , pH264Decodec(NULL)
    , pVideoRender(NULL)
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

    pSpeexCodec = new SpeexCodec();
    pAudioOutput = new AudioOutput();
    pH264Decodec = new H264Decodec();

    memset(szUrl, 0, MAX_RTMPURL_SIZE);
    strcpy(szUrl, url);
    
//    pthread_attr_t attr;
//    pthread_attr_init(&attr);
//    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&thread_play, NULL, &_play, (void*)this);
}

void VideoChat::PausePlayer(bool paused)
{
    AutoLock autoLock(handleLock);
    AutoLock lock(renderLock);
    m_paused = paused;
    wait_mutex.Unlock();
    if (pAudioOutput) pAudioOutput->pause(paused);
    if (pVideoRender) pVideoRender->pause(paused);
}

int VideoChat::StopPlay()
{
    AutoLock autoLock(handleLock);
    
    if (!m_playing) return -1;
    m_playing = false;
    
    wait_mutex.Unlock();

    pthread_join(thread_play, NULL);
    
    SAFE_DELETE(pSpeexCodec);  LOGI("release SpeexCodec");
    SAFE_DELETE(pAudioOutput); LOGI("release AudioOutput");
    SAFE_DELETE(pH264Decodec); LOGI("release H264Decodec");

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
        	snprintf(pThis->szRTMPUrl, MAX_RTMPURL_SIZE, "rtmp://%s:%d/%s/%s live=1",
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

void* VideoChat::_play(void* pVideoChat)
{
	JNIEnv *env;

    VideoChat* pThis = (VideoChat*)pVideoChat;

    if (((JavaVM*)pThis->m_jVM)->AttachCurrentThread(&env, NULL) < 0)
    	LOGE("jvm->AttachCurrentThread failed!");

    jclass clazz = env->GetObjectClass(pThis->m_jObject);
    jmethodID jEventMethodId = env->GetMethodID(clazz, "onJniEvent", "(I)V"); //jmethodID
    LOGI("env->GetMethodID = 0x%08X", (int)jEventMethodId);
    LOGI("Play thread begin...");

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


	pthread_cond_t cond;
	//Mutex wait_mutex;
	pthread_cond_init(&cond, NULL);

	struct timespec timeout;

    //
    // while do connect media server
    //
    while(pThis->m_playing)
    {
        env->CallVoidMethod((jobject)pThis->m_jObject, jEventMethodId, 1);

        if (pThis->m_paused) {
        	clock_gettime(CLOCK_REALTIME, &timeout);
        	timeout.tv_sec += 1;
        	pthread_cond_timedwait(&cond, pThis->wait_mutex.mutex(), &timeout);
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
    		pthread_cond_timedwait(&cond, pThis->wait_mutex.mutex(), &timeout);
            continue; // connect faild!
        }

        pThis->pRtmp = my_rtmp_connect(pThis->szRTMPUrl);
        if (pThis->pRtmp == NULL) {
        	env->CallVoidMethod((jobject)pThis->m_jObject, jEventMethodId, -2);

        	clock_gettime(CLOCK_REALTIME, &timeout);
        	timeout.tv_sec += 1;
    		pthread_cond_timedwait(&cond, pThis->wait_mutex.mutex(), &timeout);
    		continue; // connect faild!
        }

        uint32_t timestamp = 0;
        uint32_t process_ts = 0;

        RTMPPacket rtmp_pakt;
        bool bFirstPacket = true;
        while(pThis->m_playing)
        {
        	if (pThis->m_paused) {
            	clock_gettime(CLOCK_REALTIME, &timeout);
            	timeout.tv_sec += 1;
        		pthread_cond_timedwait(&cond, pThis->wait_mutex.mutex(), &timeout);

        		break; // paused to reconnect
        	}

            RTMPPacket_Reset(&rtmp_pakt);
            
            //
            if(!RTMP_GetNextMediaPacket(pThis->pRtmp, &rtmp_pakt)) {
            	env->CallVoidMethod((jobject)pThis->m_jObject, jEventMethodId, -3);
                break; // recv error
            }

            // slow
            //env->CallVoidMethod((jobject)pThis->m_jObject, jEventMethodId, 0);
            
            if (rtmp_pakt.m_body == NULL) {
            	env->CallVoidMethod((jobject)pThis->m_jObject, jEventMethodId, -3);
                break; // reconnect
            }

            if (timestamp == 0) {
            	// reset timestamp
            	timestamp = rtmp_pakt.m_nTimeStamp;
            	process_ts = time(0);
            }

            if (bFirstPacket){
            	bFirstPacket = false;
            	env->CallVoidMethod((jobject)pThis->m_jObject, jEventMethodId, 3);
            }
/*
            if (rtmp_pakt.m_nTimeStamp - timestamp > 30000){ // drop packet for pass 3 seconds
            	RTMPPacket_Free(&rtmp_pakt);
            	LOGE("Drop packet...(TS:%d)",rtmp_pakt.m_nTimeStamp);
            	if (time(0) - process_ts > 60000){
            		// reset packet timestamp
            		timestamp = rtmp_pakt.m_nTimeStamp;
            		LOGE("reset packet timestamp...(TS:%d)",rtmp_pakt.m_nTimeStamp);
            	}
            	continue;
            }
*/
            timestamp = rtmp_pakt.m_nTimeStamp;
            process_ts = time(0);

            if (rtmp_pakt.m_packetType == RTMP_PACKET_TYPE_AUDIO && 0xB2 == rtmp_pakt.m_body[0])
            {
                // Speex Voice Data
                int dec_audio_count = pThis->pSpeexCodec->decode(rtmp_pakt.m_body + 1, rtmp_pakt.m_nBodySize - 1, audio_buffer);
                pThis->pAudioOutput->play(audio_buffer[0], audio_frame_size*dec_audio_count);

            }
            else if (rtmp_pakt.m_packetType == RTMP_PACKET_TYPE_VIDEO && 7 == (rtmp_pakt.m_body[0] & 0x0f))
            {
                // H264 Video Data
                int got_picture = 0;
                pThis->pH264Decodec->decode((uint8_t*)rtmp_pakt.m_body, rtmp_pakt.m_nBodySize, picture, &got_picture);

                AutoLock lock(pThis->renderLock);
                if (got_picture && pThis->pVideoRender) {
                    // draw buffer
                    pThis->pVideoRender->setFrame( picture,
                        pThis->pH264Decodec->getWidth(),
                        pThis->pH264Decodec->getHeight());
                }

            }
            else if (rtmp_pakt.m_packetType == RTMP_PACKET_TYPE_INFO)
            {
                // data
            	env->CallVoidMethod((jobject)pThis->m_jObject, jEventMethodId, -4);
            }
            else if (rtmp_pakt.m_packetType == RTMP_PACKET_TYPE_FLASH_VIDEO)
            {
                // Flash data
            	env->CallVoidMethod((jobject)pThis->m_jObject, jEventMethodId, -4);
            }
            else
            {
            	env->CallVoidMethod((jobject)pThis->m_jObject, jEventMethodId, -4);
            }

            RTMPPacket_Free(&rtmp_pakt);
        }

        //
        // free rtmp
        //
        if (RTMP_IsConnected(pThis->pRtmp)) {
            RTMP_Close(pThis->pRtmp);
        }

        RTMP_Free(pThis->pRtmp);
        pThis->pRtmp = NULL;

    }

    {
    	AutoLock lock(pThis->renderLock);
    	if (pThis->pVideoRender)
    		pThis->pVideoRender->clearFrame();
    }

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
    env->CallVoidMethod((jobject)pThis->m_jObject, jEventMethodId, 4);
    //env->DeleteGlobalRef( pThis->m_jObject );
    ((JavaVM*)pThis->m_jVM)->DetachCurrentThread();
    LOGI("Play thread exit...");
    return 0;
}
