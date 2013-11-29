#if !defined(__VIDEOCHAT_H__)
#define __VIDEOCHAT_H__

#include <stdio.h>
#include <stdlib.h>
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
#include <semaphore.h>

// for surface view
#include <android/bitmap.h>
//#include <android/surface.h>
//#include <gui/Surface.h>
//#include <ui/Region.h>
//#include <utils/RefBase.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>



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

#ifndef   UINT64_C
#define   UINT64_C(value) __CONCAT(value,ULL)
#endif

#include "libavcodec/avcodec.h"
#include "libpostproc/postprocess.h"  

#ifdef __cplusplus
}
#endif


//#define DEBUG

#define LOG_TAG "VideoChat.NDK"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))

// 一秒钟，16kHz，单声道，16-bit signed little endian，帧大小
#define AUDIO_FRAMES_SIZE 16000


class AudioOutput;
class SpeexCodec;
class VideoChat;


//
// Thead utils
//
class Mutex
{
public:
    Mutex()     { pthread_mutex_init( &_mutex, NULL ); }
    ~Mutex()    { pthread_mutex_destroy( &_mutex );    }

    void Lock() const   { pthread_mutex_lock( &_mutex );       }
    void Unlock() const { pthread_mutex_unlock( &_mutex );     }

private:
    mutable pthread_mutex_t _mutex;
};
class AutoLock
{
public:
    AutoLock(Mutex& mutex) : _clsMutex(mutex) { _clsMutex.Lock();   }
    ~AutoLock()                               { _clsMutex.Unlock(); }

    Mutex& _clsMutex;
};
class Semaphore
{
public:
    Semaphore() { sem_init(&_semaphore, 0, 0); }
    ~Semaphore() { sem_destroy(&_semaphore); }
    
    void Post() { sem_post(&_semaphore); }
    void Wait() { sem_wait(&_semaphore); }
    bool tryWait() { return 0 == sem_trywait(&_semaphore); };
    
    sem_t _semaphore;
};




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
    VideoRender();
    ~VideoRender();

    void set_view(int width, int height) { m_viewport_width = width; m_viewport_height = height; }
    void set_size(uint32_t width, uint32_t height) { m_width = width; m_height = height; }
    void set_frame(AVFrame* frame, Mutex* frame_lock) { m_frame = frame; m_frame_lock = frame_lock; };
    void render_frame();
    //int setSurface(JNIEnv *env, jobject jsurface, jint version);
    //android::Surface* getNativeSurface(JNIEnv* env, jobject jsurface, jint version);
    
    void ready2render(){m_sem.Post();}

private:
    enum {
        ATTRIB_VERTEX,
        ATTRIB_TEXTURE,
    };

    GLuint bindTexture(GLuint texture, const char *buffer, GLuint w , GLuint h);
    GLuint buildShader(const char* source, GLenum shaderType);
    GLuint buildProgram(const char* vertexShaderSource, const char* fragmentShaderSource);

private:
    GLuint m_texYId;
    GLuint m_texUId;
    GLuint m_texVId;
    GLuint simpleProgram;
    
    uint8_t* m_image_buffer;
    uint32_t m_image_buffer_size;
    
    AVFrame* m_frame;
    Mutex*   m_frame_lock;
    
    Semaphore m_sem;
    
    pp_context* pp_c;
    pp_mode*    pp_m;

    uint32_t m_width;
    uint32_t m_height;

    int m_viewport_width;
    int m_viewport_height;
};


//
// video codec
//
class H264Decodec
{
public:
    H264Decodec();
    ~H264Decodec();

    int decode(uint8_t* rtmp_video_buf, uint32_t buf_size, int* got_picture);
    AVFrame* getPicture(void){ return picture; }
    Mutex& getPictureLock(void){return _picture_lock;};

    uint32_t getWidth() { return codec_context->width; }
    uint32_t getHeight() { return codec_context->height; }

private:
    int decodeSequenceHeader(uint8_t* buffer, uint32_t buf_size, int* got_picture);
    int decodeNALU(uint8_t* buffer, uint32_t buf_size, int* got_picture);
    
    int final_decode(uint8_t* buffer, uint32_t buf_size, int* got_picture);
    int final_decode_header(uint8_t* sequenceParameterSet, uint32_t sequenceParameterSetLength, 
                            uint8_t* pictureParameterSet, uint32_t pictureParameterSetLength, int* got_picture);

private:
    uint8_t lengthSizeMinusOne;
    AVCodecContext *codec_context;
    AVCodec *_codec;
    AVFrame *picture;
    uint8_t *dec_buffer;
    uint32_t dec_buffer_size;

    Mutex _picture_lock;

#ifdef DEBUG
    FILE* decoder_fd;
#endif
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
    void InitRender(int width, int height);
    void RenderFrame() { if (pVideoRender) pVideoRender->render_frame(); }

    int Play(const char* szRTMPUrl);
    int StopPlay();

private:
    static void* _play(void* pVideoChat);
    
private:
    SpeexCodec* pSpeexCodec;
    AudioOutput* pAudioOutput;
    H264Decodec* pH264Decodec;
    VideoRender* pVideoRender;

    RTMP* pRtmp;
    int m_isOpenPlayer;

    char* szRTMPUrl;

    pthread_t thread_play;
    pthread_attr_t thread_attr;
};


#endif //__VIDEOCHAT_H__