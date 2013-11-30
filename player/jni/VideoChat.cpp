#include "VideoChat.h"


///////////////////////////////////////////////////////////////////////////////
//
// class VideoChat 实现
//
///////////////////////////////////////////////////////////////////////////////


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
    , szRTMPUrl(NULL)
    , pSpeexCodec(NULL)
    , pAudioOutput(NULL)
    , pH264Decodec(NULL)
    , pVideoRender(NULL)
{
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
}

VideoChat::~VideoChat()
{
    if (szRTMPUrl) delete[] szRTMPUrl;
}

void VideoChat::InitRender(int width, int height)
{
    SAFE_DELETE(pVideoRender);

    pVideoRender = new VideoRender();
    pVideoRender->set_view(width, height);
}

int VideoChat::Init()
{
    Release();
    
    pSpeexCodec = new SpeexCodec();
    pAudioOutput = new AudioOutput();
    pH264Decodec = new H264Decodec();

    return 0;
}

void VideoChat::Release()
{
    SAFE_DELETE(pSpeexCodec);
    SAFE_DELETE(pAudioOutput);
    SAFE_DELETE(pH264Decodec);
    SAFE_DELETE(pVideoRender);
}

int VideoChat::Play(const char* url)
{
    if (m_playing) return -1;
    m_playing = true;
    
    szRTMPUrl = new char[strlen(url) + 2];
    memset(szRTMPUrl, 0, strlen(url) + 2);
    strcpy(szRTMPUrl, url);
    pthread_create(&thread_play, &thread_attr, &_play, (void*)this);
}

void VideoChat::PausePlayer(bool paused)
{
    m_paused = paused;
    pAudioOutput->pause(paused);
    pVideoRender->pause(paused);
}

int VideoChat::StopPlay()
{
    if (!m_playing) return -1;

    m_playing = false;
    int iRet = pthread_join(thread_play, NULL);

    return iRet;
}

void* VideoChat::_play(void* pVideoChat)
{
    VideoChat* pThis = (VideoChat*)pVideoChat;

    const int max_audio_buffer_size = 100;
    
    int audio_frame_size = pThis->pSpeexCodec->audio_frame_size();
    short *audio_buffer[max_audio_buffer_size] = {0};
    audio_buffer[0] = new short[audio_frame_size*max_audio_buffer_size];
    
    for(int i=1; i<max_audio_buffer_size; i++)
        audio_buffer[i] = audio_buffer[i-1] + audio_frame_size;

    //int result_code = mkdir("/data/data/cn.videochat.MainActivity/files/", 0770);
#ifdef DEBUG
    FILE* fd = fopen("/sdcard/video_data.yuv", "wb");
    if (!fd) {
        LOGI("ERROR: create debug decode video file failed!!!");
    }
#endif

    do {
        bool connected = false;
        
        if (pThis->pRtmp) {
            if (RTMP_IsConnected(pThis->pRtmp)) {
                RTMP_Close(pThis->pRtmp);
            }
            
            RTMP_Free(pThis->pRtmp);
            pThis->pRtmp = NULL;
        }

        pThis->pRtmp = RTMP_Alloc();
        RTMP_Init(pThis->pRtmp);
        LOGI("Play RTMP_Init %s\n", pThis->szRTMPUrl);
        if (!RTMP_SetupURL(pThis->pRtmp, (char*)pThis->szRTMPUrl)) {
            LOGE("Play RTMP_SetupURL error\n");
            break; // error
        }

        if (!RTMP_Connect(pThis->pRtmp, NULL) || !RTMP_ConnectStream(pThis->pRtmp, 0)) {
            LOGI("Play RTMP_Connect or RTMP_ConnectStream error\n");
            RTMP_Free(pThis->pRtmp);
            pThis->pRtmp = NULL;
            continue; // reconnection
        }

        connected = true;
        LOGI("RTMP_Connected\n");

        RTMPPacket rtmp_pakt = { 0 };
        
        while(pThis->m_playing && connected) {

            RTMP_GetNextMediaPacket(pThis->pRtmp, &rtmp_pakt);

            if (!RTMP_IsConnected(pThis->pRtmp) || !rtmp_pakt.m_nBodySize) {
                connected = false;
                continue; // post reconnection
            }

            if (rtmp_pakt.m_packetType == RTMP_PACKET_TYPE_AUDIO && 0xB2 == rtmp_pakt.m_body[0]) {
                // 处理音频数据包 & Speex Encode
                char* data = rtmp_pakt.m_body + 1;
                int data_size = rtmp_pakt.m_nBodySize - 1;
                int dec_audio_count = pThis->pSpeexCodec->decode(data, data_size, audio_buffer);
                // play decode ouput_buffer
                pThis->pAudioOutput->play(audio_buffer[0], audio_frame_size*dec_audio_count);
                LOGI("[PARSE_AUDIO] dec_audio_count:%d, data_size:%d ", dec_audio_count, data_size);
                
            } else if (rtmp_pakt.m_packetType == RTMP_PACKET_TYPE_VIDEO) {
                // 处理视频数据包
                /* H264 fix: */
                char *packetBody = rtmp_pakt.m_body;
                int packetBodySize = rtmp_pakt.m_nBodySize;

                uint8_t CodecId = packetBody[0] & 0x0f;
                if(CodecId == 7) { /* CodecId = H264 */
                    int got_picture = 0;
                    pThis->pH264Decodec->decode((uint8_t*)packetBody, packetBodySize, &got_picture);

                    if (got_picture && pThis->pVideoRender)
                    {
                        // pThis->pVideoRender->set_size(
                            // pThis->pH264Decodec->getWidth(),
                            // pThis->pH264Decodec->getHeight());
                        //pThis->pVideoRender->ready2render();
                        pThis->pVideoRender->set_frame(
                            pThis->pH264Decodec->getFrame(),
                            pThis->pH264Decodec->getWidth(),
                            pThis->pH264Decodec->getHeight());
                    }

                } else {
                    LOGE( "[AVC]Video Codec don't H.264" );
                }

            } else if (rtmp_pakt.m_packetType == RTMP_PACKET_TYPE_INFO) {
                // 处理信息包
            } else if (rtmp_pakt.m_packetType == RTMP_PACKET_TYPE_FLASH_VIDEO) {
                // 其他Flash数据
            }

            RTMPPacket_Free(&rtmp_pakt);
            
        }

        if (RTMP_IsConnected(pThis->pRtmp)) {
            RTMP_Close(pThis->pRtmp);
        }
        
        RTMP_Free(pThis->pRtmp);
        pThis->pRtmp = NULL;
        
    }while(pThis->m_playing);

#ifdef DEBUG
    if (fd) fclose(fd);
#endif

    if (audio_buffer[0]) delete[] audio_buffer[0];

    pThis->m_playing = false;

    return 0;
}
