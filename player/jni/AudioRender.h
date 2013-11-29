#ifndef __AUDIORENDER_H__
#define __AUDIORENDER_H__


// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "utils.h"


// 一秒钟，16kHz，单声道，16-bit signed little endian，帧大小
#define AUDIO_FRAMES_SIZE 16000


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


#endif  //__AUDIORENDER_H__