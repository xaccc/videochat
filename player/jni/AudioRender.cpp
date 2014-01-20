#include "AudioRender.h"
#include <errno.h>

#undef LOGI
#define LOGI(...)


///////////////////////////////////////////////////////////////////////////////
//
// class AudioOutput 实现
//
///////////////////////////////////////////////////////////////////////////////

AudioOutput::AudioOutput()
    : m_paused(false), m_running(true), ringbuffer(1024*1024)
{
	SLresult result; // SL_RESULT_SUCCESS


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
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);
    // set the player's state to playing
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);

    //pthread_create(&thread_play, NULL, &_player, (void*)this);
}

AudioOutput::~AudioOutput()
{
    pause(true);
    
    m_running = false;
    //pthread_join(thread_play, NULL);

    (*bqPlayerObject)->Destroy(bqPlayerObject);
    (*outputMixObject)->Destroy(outputMixObject);
    (*engineObject)->Destroy(engineObject);
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

// this callback handler is called every time a buffer finishes playing
void AudioOutput::bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
	AudioOutput* pThis = (AudioOutput*)context;
	pThis->m_lock.Unlock();
}

void* AudioOutput::_player(void *context)
{
	AudioOutput* pThis = (AudioOutput*)context;

	short audioBuffer[AUDIO_FRAMES_SIZE];
	struct timespec timeout;

	pthread_cond_t cond;
	pthread_cond_init(&cond, NULL);


	bool bFirst = true;
	while(pThis->m_running) {
    	clock_gettime(CLOCK_REALTIME, &timeout);
    	timeout.tv_nsec += 1000;
		pthread_cond_timedwait(&cond, pThis->m_lock.mutex(), &timeout);

		if (!bFirst)
			pThis->m_lock.Lock();

		if (pThis->ringbuffer.pop((char*)audioBuffer, AUDIO_FRAMES_SIZE*sizeof(short))) {
			(*pThis->bqPlayerBufferQueue)->Enqueue(pThis->bqPlayerBufferQueue,
					audioBuffer, AUDIO_FRAMES_SIZE * sizeof(short));
		} else {
			pThis->m_lock.Unlock();
		}
	}

	pthread_cond_destroy(&cond);

	return 0;
}

int AudioOutput::play(short* data, int dataSize)
{
    if (m_paused) return -1;
    
#if USE_RINGBUFFER
    if (!ringbuffer.push((char*)data, dataSize*sizeof(short))) {
    	LOGE("Audio Buffer is Full!!!");
    }
#else
	(*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, data, dataSize * sizeof(short));
	m_lock.Lock();
#endif


    return 0;
}

