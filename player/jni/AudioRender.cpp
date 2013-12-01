#include "AudioRender.h"



///////////////////////////////////////////////////////////////////////////////
//
// class AudioOutput 实现
//
///////////////////////////////////////////////////////////////////////////////

AudioOutput::AudioOutput()
    : playerBufferIndex(0), m_paused(false)
{
    SLresult result; // SL_RESULT_SUCCESS

    playerBuffer = new short[AUDIO_FRAMES_SIZE];

    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    // create output mix, with environmental reverb specified as a non-required interface    
    const SLInterfaceID ids1[1] = { SL_IID_ENVIRONMENTALREVERB };
    const SLboolean req1[1] = { SL_BOOLEAN_FALSE };

    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids1, req1);
    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2 };
    SLDataFormat_PCM format_pcm = { SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_16, SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16, SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN };
    SLDataSource audioSrc = { &loc_bufq, &format_pcm };

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = { SL_DATALOCATOR_OUTPUTMIX, outputMixObject };
    SLDataSink audioSnk = { &loc_outmix, NULL };

    // create audio player
    const SLInterfaceID ids2[3] = { SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME };
    const SLboolean req2[3] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk, 3, ids2, req2);
    // realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue);
    // register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, playerCallback, this);
    // get the effect send interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_EFFECTSEND, &bqPlayerEffectSend);
    // get the volume interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);
    // set the player's state to playing
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
}

AudioOutput::~AudioOutput()
{
    (*bqPlayerObject)->Destroy(bqPlayerObject);
    delete[] playerBuffer;
}

void AudioOutput::pause(bool paused)
{
    m_paused = paused;
    if (paused) {
        (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_STOPPED );
        (*bqPlayerBufferQueue)->Clear(bqPlayerBufferQueue);
    } else {
        (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING );
    }
}

int AudioOutput::play(short* data, int dataSize)
{
    if (dataSize > AUDIO_FRAMES_SIZE/5)
    {
        // 立即render , 测试过，立即播放抖动很厉害，没法听，也是由于dataSize过小导致的
        (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, data, dataSize * sizeof(short));
        LOGI("[AudioOutput::play] just Enqueue.");
    } else {
        // 缓存buffer 1s
        int last = 0;
        SLresult result;
        
        if (m_paused) return -1;
        
        while (dataSize > 0) {
            last = min(AUDIO_FRAMES_SIZE - playerBufferIndex, dataSize);

            memcpy(playerBuffer + playerBufferIndex, data, last * sizeof(short));
            playerBufferIndex += last;
            dataSize -= last;

            if(playerBufferIndex == AUDIO_FRAMES_SIZE) {
                (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, playerBuffer, playerBufferIndex * sizeof(short));
                playerBufferIndex = 0;
                LOGI("[AudioOutput::play] PlayerBufferQueue Enqueue.");
            }
        }
    }
}

void AudioOutput::playerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    // AudioOutput* pThis = (AudioOutput*)context;
}


