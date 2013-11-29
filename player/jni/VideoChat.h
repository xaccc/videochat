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
    void InitRender(int width, int height);
    void RenderFrame() { if (pVideoRender) pVideoRender->render_frame(); }

    int Play(const char* szRTMPUrl);
    int StopPlay();
    void PausePlayer(bool paused);

private:
    static void* _play(void* pVideoChat);
    
private:
    SpeexCodec* pSpeexCodec;
    AudioOutput* pAudioOutput;
    H264Decodec* pH264Decodec;
    VideoRender* pVideoRender;

    RTMP* pRtmp;

    char* szRTMPUrl;

    pthread_t thread_play;
    pthread_attr_t thread_attr;
    
    bool m_playing;
    bool m_paused;
};


#endif //__VIDEOCHAT_H__
