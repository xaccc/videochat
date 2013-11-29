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
    : m_isOpenPlayer(0), szRTMPUrl(NULL), pSpeexCodec(NULL), pAudioOutput(NULL), pH264Decodec(NULL), pVideoRender(NULL)
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
    if (pVideoRender)
        delete pVideoRender;

    pVideoRender = new VideoRender();
    pVideoRender->set_view(width, height);
    pVideoRender->set_frame(pH264Decodec->getPicture(), &pH264Decodec->getPictureLock());
}

int VideoChat::Init()
{
    pSpeexCodec = new SpeexCodec();
    pAudioOutput = new AudioOutput();
    pH264Decodec = new H264Decodec();

    return 0;
}

void VideoChat::Release()
{
    delete pSpeexCodec;
    delete pAudioOutput;
    delete pH264Decodec;

    if (pVideoRender)
        delete pVideoRender;
}

int VideoChat::Play(const char* url)
{
    if (m_isOpenPlayer > 0) return -1;
    m_isOpenPlayer = 1;
    szRTMPUrl = new char[strlen(url) + 2];
    memset(szRTMPUrl, 0, strlen(url) + 2);
    strcpy(szRTMPUrl, url);
    pthread_create(&thread_play, &thread_attr, &_play, (void*)this);
}

int VideoChat::StopPlay()
{
    if (m_isOpenPlayer == 0) return -1;

    m_isOpenPlayer = 0;
    int iRet = pthread_join(thread_play, NULL);

    return iRet;
}

void* VideoChat::_play(void* pVideoChat)
{
    VideoChat* pThis = (VideoChat*)pVideoChat;

    int audio_frame_size = pThis->pSpeexCodec->audio_frame_size();
    short *audio_buffer = audio_frame_size > 0 ? new short[audio_frame_size] : NULL;

    //int result_code = mkdir("/data/data/cn.videochat.MainActivity/files/", 0770);
#ifdef DEBUG
    FILE* fd = fopen("/sdcard/video_data.yuv", "wb");
    if (!fd) {
        LOGI("ERROR: create debug decode video file failed!!!");
    }
#endif

    do {
        pThis->pRtmp = RTMP_Alloc();
        RTMP_Init(pThis->pRtmp);
        LOGI("Play RTMP_Init %s\n", pThis->szRTMPUrl);
        if (!RTMP_SetupURL(pThis->pRtmp, (char*)pThis->szRTMPUrl)) {
            LOGI("Play RTMP_SetupURL error\n");
            break; // error
        }

        if (!RTMP_Connect(pThis->pRtmp, NULL) || !RTMP_ConnectStream(pThis->pRtmp, 0)) {
            LOGI("Play RTMP_Connect or RTMP_ConnectStream error\n");
            break; // error
        }

        LOGI("RTMP_Connected\n");

        RTMPPacket rtmp_pakt = { 0 };
        while(pThis->m_isOpenPlayer > 0) {

            RTMP_GetNextMediaPacket(pThis->pRtmp, &rtmp_pakt);

            if (!rtmp_pakt.m_nBodySize)
                continue;

            if (rtmp_pakt.m_packetType == RTMP_PACKET_TYPE_AUDIO) {
                // 处理音频数据包
                LOGI( "AudioPacket: Head=0x%X\n", rtmp_pakt.m_body[0]);
                char* data = rtmp_pakt.m_body + 1;
                int offset = 0;
                int data_size = rtmp_pakt.m_nBodySize - 1;
                while(offset < data_size) {
                    // decode data+offset, data_size - offset
                    pThis->pSpeexCodec->decode(data, data_size, audio_buffer);
                    // play decode ouput_buffer
                    pThis->pAudioOutput->play(audio_buffer, audio_frame_size);
                    offset += data_size;
                }
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
                        pThis->pVideoRender->set_size(
                            pThis->pH264Decodec->getWidth(),
                            pThis->pH264Decodec->getHeight());
                        pThis->pVideoRender->ready2render();
                    }

#ifdef DEBUG
                    // debug data
                    if (got_picture && fd) {
                        AVFrame *frame = pThis->pH264Decodec->getPicture();
                        uint32_t width = pThis->pH264Decodec->getWidth();
                        uint32_t height = pThis->pH264Decodec->getHeight();

                        for(int lines = 0; lines < height; lines ++)
                            fwrite(frame->data[0]+frame->linesize[0]*lines, 1, width, fd);
                        for(int lines = 0; lines < height/2; lines ++)
                            fwrite(frame->data[1]+frame->linesize[1]*lines, 1, width/2, fd);
                        for(int lines = 0; lines < height/2; lines ++)
                            fwrite(frame->data[2]+frame->linesize[2]*lines, 1, width/2, fd);

                        fflush(fd);
                    } else if (!fd){
                        LOGI("ERROR: don't open video log file!!!");
                    }
#endif // DEBUG
                } else {
                    LOGI( "AVC: CodecId != 7" );
                }

            } else if (rtmp_pakt.m_packetType == RTMP_PACKET_TYPE_INFO) {
                // 处理信息包
            } else if (rtmp_pakt.m_packetType == RTMP_PACKET_TYPE_FLASH_VIDEO) {
                // 其他Flash数据
            }

            RTMPPacket_Free(&rtmp_pakt);
        }

    }while(0);

#ifdef DEBUG
    if (fd) fclose(fd);
#endif

    delete[] audio_buffer;

    if (RTMP_IsConnected(pThis->pRtmp)) {
        RTMP_Close(pThis->pRtmp);
    }
    RTMP_Free(pThis->pRtmp);
    pThis->m_isOpenPlayer = 0;

    return 0;
}
