/**
 *
 * Copyright (C) 2013 Daijingjing (jjyyis@qq.com)
 *
 * Licensed under the GPLv2 license. See 'COPYING' for more details.
 *
 */
 
#include "VideoChat.h"



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
    LOGI("playerBufferIndex %d   all:%d", playerBufferIndex, AUDIO_FRAMES_SIZE);

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


///////////////////////////////////////////////////////////////////////////////
//
// class SpeexCodec 实现
//
///////////////////////////////////////////////////////////////////////////////

SpeexCodec::SpeexCodec()
{
    speex_bits_init(&dbits);
    dec_state = speex_decoder_init(&speex_wb_mode);
    speex_decoder_ctl(dec_state, SPEEX_GET_FRAME_SIZE, &dec_frame_size);
}

SpeexCodec::~SpeexCodec()
{
}

int SpeexCodec::decode(char* data, int data_size, short* output_buffer)
{
    speex_bits_read_from(&dbits, data, data_size);
    speex_decode_int(dec_state, &dbits, output_buffer);
}

///////////////////////////////////////////////////////////////////////////////
//
// class VideoChat 实现
//
///////////////////////////////////////////////////////////////////////////////

VideoChat::VideoChat()
    : m_isOpenPlayer(0)
{
}

VideoChat::~VideoChat()
{
}

int VideoChat::Init()
{
    return 0;
}

void VideoChat::Release()
{
}

int VideoChat::Play(char* szRTMPUrl)
{
    if (m_isOpenPlayer > 0) return -1;
    pRtmp = RTMP_Alloc();
    RTMP_Init(pRtmp);
    LOGI("Play RTMP_Init %s\n", szRTMPUrl);
    if (!RTMP_SetupURL(pRtmp, szRTMPUrl)) {
        LOGI("Play RTMP_SetupURL error\n");
        RTMP_Free(pRtmp);
        return -1;
    }

    if (!RTMP_Connect(pRtmp, NULL) || !RTMP_ConnectStream(pRtmp, 0)) {
        LOGI("Play RTMP_Connect or RTMP_ConnectStream error\n");
        RTMP_Free(pRtmp);
        return -1;
    }

    LOGI("RTMP_Connected\n");

    m_isOpenPlayer = 1;
    pthread_create(&thread_play, NULL, &_play, (void*)this);
}

int VideoChat::StopPlay()
{
    if (m_isOpenPlayer == 0) return -1;

    m_isOpenPlayer = 0;
    int iRet = pthread_join(thread_play, NULL);

    if (RTMP_IsConnected(pRtmp)) {
        RTMP_Close(pRtmp);
    }
    RTMP_Free(pRtmp);

    return iRet;
}

void* VideoChat::_play(void* pVideoChat)
{
    VideoChat* pThis = (VideoChat*)pVideoChat;

    RTMPPacket rtmp_pakt = { 0 };
    while(pThis->m_isOpenPlayer > 0) {
        RTMP_ReadPacket(pThis->pRtmp, &rtmp_pakt);
        if (!rtmp_pakt.m_nBodySize)
            continue;

        if (rtmp_pakt.m_packetType == RTMP_PACKET_TYPE_AUDIO) {
            // 处理音频数据包
            char* data = rtmp_pakt.m_body + 1;
            int offset = 0;
            int data_size = rtmp_pakt.m_nBodySize - 1;
            while(offset < data_size) {
                // decode data+offset, data_size - offset
                // play decode ouput_buffer
                offset += data_size;
            }
        } else if (rtmp_pakt.m_packetType == RTMP_PACKET_TYPE_VIDEO) {
            // 处理视频数据包
        } else if (rtmp_pakt.m_packetType == RTMP_PACKET_TYPE_INFO) {
            // 处理信息包
        } else if (rtmp_pakt.m_packetType == RTMP_PACKET_TYPE_FLASH_VIDEO) {
            // 其他Flash数据
        }

        LOGI( "rtmp_pakt size:%d  type:%d\n", rtmp_pakt.m_nBodySize, rtmp_pakt.m_packetType);
        RTMPPacket_Free(&rtmp_pakt);
    }

    return 0;
}



