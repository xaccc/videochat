#include "AudioRender.h"


#undef LOGI
#define LOGI(...)

#define USE_JITTER 0
#define USE_THREAD 0

#if USE_JITTER
#define USE_THREAD 1
#endif

///////////////////////////////////////////////////////////////////////////////
//
// class AudioOutput 实现
//
///////////////////////////////////////////////////////////////////////////////

AudioOutput::AudioOutput()
    : playerBufferIndex(0), m_paused(false), m_shutdown(false)
{
    SLresult result; // SL_RESULT_SUCCESS

    playerBuffer = new short[AUDIO_FRAMES_SIZE*2];

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
    // set the player's state to playing
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);

#if USE_JITTER
    jitterBuffer = jitter_buffer_init(AUDIO_FRAMES_SIZE);
#endif //USE_JITTER

#if USE_THREAD
    waitShutdown.Lock();
    pthread_create(&thread_play, NULL, &_play, (void*)this);
#endif //USE_THREAD
}

AudioOutput::~AudioOutput()
{
    pause(true);

#if USE_THREAD
    m_shutdown = true;
    waitShutdown.Unlock();
    pthread_join(thread_play, NULL);
#endif //USE_THREAD
    
#if USE_JITTER
    jitter_buffer_destroy(jitterBuffer);
#endif //USE_JITTER

    (*bqPlayerObject)->Destroy(bqPlayerObject);
    (*outputMixObject)->Destroy(outputMixObject);
    (*engineObject)->Destroy(engineObject);
    
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

int AudioOutput::play(uint32_t ts, short* data, int dataSize)
{
	AutoLock autoLock(handleLock);

#if USE_JITTER
	JitterBufferPacket packet;
	packet.data = (char*)data;
	packet.len = dataSize*sizeof(short);
	packet.timestamp = ts;
	packet.span = dataSize*1000/AUDIO_FRAMES_SIZE; //???
	packet.sequence = 0;
	packet.user_data = 0;
	jitter_buffer_put(jitterBuffer, &packet);
#else // USE_JITTER
#if USE_THREAD
	int size = min(AUDIO_FRAMES_SIZE*2-playerBufferIndex, dataSize);
	memcpy(playerBuffer + playerBufferIndex, data, size*sizeof(short));
	playerBufferIndex += size;
#else //USE_THREAD
   int last = 0;

	while (dataSize > 0) {
		last = min(AUDIO_FRAMES_SIZE - playerBufferIndex, dataSize);

		memcpy(playerBuffer + playerBufferIndex, data, last * sizeof(short));
		playerBufferIndex += last;
		dataSize -= last;

		if(playerBufferIndex == AUDIO_FRAMES_SIZE) {
			(*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, playerBuffer, playerBufferIndex * sizeof(short));
			playerBufferIndex = 0;
		}
	}
#endif //USE_THREAD
#endif // USE_JITTER

	return 0;
}

void* AudioOutput::_play(void* pAudioOutput)
{
#if USE_THREAD
	AudioOutput* pThis = (AudioOutput*)pAudioOutput;

	pthread_cond_t cond;
	pthread_cond_init(&cond, NULL);

	struct timespec justNow;
    int start_offset = 2000;
    int timestamp = 0;

    JitterBufferPacket outpacket;
    char* buffer = (char*)malloc(AUDIO_FRAMES_SIZE*sizeof(short));
    int prev_ts = 0;

	while(!pThis->m_shutdown)
	{
    	if (!pThis->m_paused) {
			AutoLock autoLock(pThis->handleLock);
#if USE_JITTER
			outpacket.data = buffer;
		    outpacket.len = AUDIO_FRAMES_SIZE*sizeof(short);

		    int ts = jitter_buffer_get_pointer_timestamp(pThis->jitterBuffer);
		    if(JITTER_BUFFER_OK == jitter_buffer_get(pThis->jitterBuffer, &outpacket, AUDIO_FRAMES_SIZE, &start_offset)) {
				(*pThis->bqPlayerBufferQueue)->Enqueue(pThis->bqPlayerBufferQueue, outpacket.data, outpacket.len);
				jitter_buffer_tick(pThis->jitterBuffer);
				LOGE("[buffer] pts:%7d, ts:%7d, dlt:%7d, size:%d", prev_ts, ts, ts - prev_ts,outpacket.len);
				prev_ts = ts;
			}
#else // USE_JITTER
		    if (AUDIO_FRAMES_SIZE == pThis->playerBufferIndex) {
		    	(*pThis->bqPlayerBufferQueue)->Enqueue(pThis->bqPlayerBufferQueue, pThis->playerBuffer, pThis->playerBufferIndex*sizeof(short));
		    	pThis->playerBufferIndex = 0;
		    }
#endif // USE_JITTER
		}

    	clock_gettime(CLOCK_REALTIME, &justNow);
    	//justNow.tv_sec += 1;
    	justNow.tv_nsec += 100000; // sleep
    	pthread_cond_timedwait(&cond, pThis->waitShutdown.mutex(), &justNow);
	}

	free(buffer);
	pthread_cond_destroy(&cond);
#endif //USE_THREAD

	return 0;
}
