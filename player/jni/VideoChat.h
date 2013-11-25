#if !defined(__VIDEOCHAT_H__)
#define __VIDEOCHAT_H__

#include <assert.h>
#include <string.h>

// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

// for native asset manager
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <android/log.h>
#include <pthread.h>


#ifdef __cplusplus
extern "C" {
#endif

#include <librtmp/rtmp.h>
#include <speex/speex.h>
#include <speex/speex_header.h>

#include <librtmp/rtmp.h>
#include <librtmp/log.h>

//#include <libx264/x264.h>
//#include <libx264/x264_config.h>

#ifdef __cplusplus
}
#endif

#include "common.h"
#include "avcodec.h"



#define LOG_TAG "VideoChat.NDK"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))

// 一秒钟，16kHz，单声道，16-bit signed little endian，帧大小
#define AUDIO_FRAMES_SIZE 16000


class AudioOutput;
class SpeexCodec;
class VideoChat;

//
// audio play
//
class AudioOutput 
{
public:
    AudioOutput();
    ~AudioOutput();

    int play(short* data, int dataSize);

private:
    static void playerCallback(SLAndroidSimpleBufferQueueItf bq, void *context);

private:
    SLObjectItf engineObject;
    SLEngineItf engineEngine;

    SLObjectItf outputMixObject;
    SLObjectItf bqPlayerObject;

    SLPlayItf bqPlayerPlay;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
    SLEffectSendItf bqPlayerEffectSend;
    SLMuteSoloItf bqPlayerMuteSolo;
    SLVolumeItf bqPlayerVolume;

    // audio buffer
    short* playerBuffer;
    int playerBufferIndex;
};

//
// audio codec
//
class SpeexCodec
{
public:
    SpeexCodec();
    ~SpeexCodec();

    int decode(char* data, int data_size, short* output_buffer);

    int output_buffer_size() { return dec_frame_size * sizeof(short); }
    int audio_frame_size() { return dec_frame_size; }

private:
    int dec_frame_size;
    void *dec_state;
    SpeexBits dbits;
};

//
//
//
class VideoRender
{
public:
private:
};


//
// video codec
//
class H264Decodec
{
public:
private:
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

    int Play(const char* szRTMPUrl);
    int StopPlay();

private:
    static void* _play(void* pVideoChat);

private:
    SpeexCodec* pSpeexCodec;
    AudioOutput* pAudioOutput;
    RTMP* pRtmp;
    int m_isOpenPlayer;

    char* szRTMPUrl;

    pthread_t thread_play;
    pthread_attr_t thread_attr;

    AVCodecContext *codec_context;
};


#endif //__VIDEOCHAT_H__