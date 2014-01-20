#ifndef __AUDIORENDER_H__
#define __AUDIORENDER_H__


// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "utils.h"


// 1秒钟buffer，16kHz，单声道，16-bit signed little endian，帧大小
#define AUDIO_FRAMES_SIZE 8000


//
// audio play
//
class AudioOutput 
{
public:
    AudioOutput();
    ~AudioOutput();

    int play(short* data, int dataSize);
    void pause(bool paused);


private:
    SLObjectItf engineObject;
    SLEngineItf engineEngine;

    SLObjectItf outputMixObject;
    SLObjectItf bqPlayerObject;

    SLPlayItf bqPlayerPlay;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;

    static void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context);
    static void* _player(void *context);

    pthread_t thread_play;

    Mutex m_lock;
    
    RingBuffer<char> ringbuffer;

    bool m_paused;
    bool m_running;
};


#endif  //__AUDIORENDER_H__
