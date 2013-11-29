#include "AudioRender.h"



///////////////////////////////////////////////////////////////////////////////
//
// class AudioOutput 实现
//
///////////////////////////////////////////////////////////////////////////////

AudioOutput::AudioOutput()
    : playerBufferIndex(0)
{
    playerBuffer = new short[AUDIO_FRAMES_SIZE*2];

    SLresult result;

    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);
    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    assert(SL_RESULT_SUCCESS == result);

    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids1[1] = { SL_IID_ENVIRONMENTALREVERB };
    const SLboolean req1[1] = { SL_BOOLEAN_FALSE };
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids1, req1);
    assert(SL_RESULT_SUCCESS == result);

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);


    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2 };
    SLDataFormat_PCM format_pcm = { SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_16, SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16, SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN };
    SLDataSource audioSrc = { &loc_bufq, &format_pcm };

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = { SL_DATALOCATOR_OUTPUTMIX, outputMixObject };
    SLDataSink audioSnk = { &loc_outmix, NULL };

    // create audio player
    const SLInterfaceID ids2[3] = { SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, /*SL_IID_MUTESOLO,*/SL_IID_VOLUME };
    const SLboolean req2[3] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, /*SL_BOOLEAN_TRUE,*/SL_BOOLEAN_TRUE };
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk, 3, ids2, req2);
    assert(SL_RESULT_SUCCESS == result);

    // realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    assert(SL_RESULT_SUCCESS == result);

    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);

    // register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, playerCallback, this);
    assert(SL_RESULT_SUCCESS == result);

    // get the effect send interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_EFFECTSEND, &bqPlayerEffectSend);
    assert(SL_RESULT_SUCCESS == result);

#if 0
    // mute/solo is not supported for sources that are known to be mono, as this is
    // get the mute/solo interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_MUTESOLO, &bqPlayerMuteSolo);
    assert(SL_RESULT_SUCCESS == result);
#endif

    // get the volume interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);
    assert(SL_RESULT_SUCCESS == result);

    // set the player's state to playing
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
}

AudioOutput::~AudioOutput()
{
    (*bqPlayerObject)->Destroy(bqPlayerObject);
    delete[] playerBuffer;
}

int AudioOutput::play(short* data, int dataSize)
{
   // LOGI("putAudioQueue");
    memcpy(playerBuffer + playerBufferIndex, data, dataSize * sizeof(short));
    playerBufferIndex += dataSize;
    LOGI("[AudioOutput::play] playerBufferIndex=%d, All=%d", playerBufferIndex, AUDIO_FRAMES_SIZE);

    if(playerBufferIndex >= AUDIO_FRAMES_SIZE)
    {
        SLresult result;
        result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, playerBuffer, playerBufferIndex * sizeof(short));
        playerBufferIndex = 0;
        LOGI("Enqueue player buffer");
    }
}

void AudioOutput::playerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    AudioOutput* pThis = (AudioOutput*)context;

    LOGI("VideoChat::playerCallback\n");
}


