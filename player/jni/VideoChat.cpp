/**
 *
 * Copyright (C) 2013 Daijingjing (jjyyis@qq.com)
 *
 * Licensed under the GPLv2 license. See 'COPYING' for more details.
 *
 */
 
#include "VideoChat.h"

#define NALU_TYPE_SLICE     1
#define NALU_TYPE_DPA       2
#define NALU_TYPE_DPB       3
#define NALU_TYPE_DPC       4
#define NALU_TYPE_IDR       5
#define NALU_TYPE_SEI       6
#define NALU_TYPE_SPS       7
#define NALU_TYPE_PPS       8
#define NALU_TYPE_AUD       9   //访问分隔符
#define NALU_TYPE_EOSEQ     10
#define NALU_TYPE_EOSTREAM  11
#define NALU_TYPE_FILL      12

static const char* get_nalu_type_name(uint8_t t) {
    switch(t&0x1f) {
    case NALU_TYPE_SLICE   :   return "SLICE   ";  break;
    case NALU_TYPE_DPA     :   return "DPA     ";  break;
    case NALU_TYPE_DPB     :   return "DPB     ";  break;
    case NALU_TYPE_DPC     :   return "DPC     ";  break;
    case NALU_TYPE_IDR     :   return "IDR     ";  break;
    case NALU_TYPE_SEI     :   return "SEI     ";  break;
    case NALU_TYPE_SPS     :   return "SPS     ";  break;
    case NALU_TYPE_PPS     :   return "PPS     ";  break;
    case NALU_TYPE_AUD     :   return "AUD     ";  break;
    case NALU_TYPE_EOSEQ   :   return "EOSEQ   ";  break;
    case NALU_TYPE_EOSTREAM:   return "EOSTREAM";  break;
    case NALU_TYPE_FILL    :   return "FILL    ";  break;
    }

    return "UNKNOWN TYPE";
}

static void DumpBuffer(uint8_t* buffer, uint size)
{
    int offset = 0;
    int index = 0;
    while(offset < size && offset < 128) {
        LOGI("[DumpBuffer](%d - %02d): %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", size, index++,
            buffer[offset + 0], buffer[offset + 1], buffer[offset + 2], buffer[offset + 3],
            buffer[offset + 4], buffer[offset + 5], buffer[offset + 6], buffer[offset + 7],
            buffer[offset + 8], buffer[offset + 9], buffer[offset + 10], buffer[offset + 11],
            buffer[offset + 12], buffer[offset + 13], buffer[offset + 14], buffer[offset + 15]);

        offset += 16;
    }
}


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
    
    // disable perceptual enhancer
    //int enh=0;
    //speex_decoder_ctl(dec_state, SPEEX_SET_ENH, &enh);
}

SpeexCodec::~SpeexCodec()
{
    speex_bits_destroy(&dbits);
    speex_decoder_destroy(dec_state);
}

int SpeexCodec::decode(char* data, int data_size, short* output_buffer)
{
    LOGI( "[SpeexCodec::decode] packet size=%d\n", data_size);
    speex_bits_read_from(&dbits, data, data_size);
    speex_decode_int(dec_state, &dbits, output_buffer);
}


///////////////////////////////////////////////////////////////////////////////
//
// class H264Decodec 实现
//
///////////////////////////////////////////////////////////////////////////////
H264Decodec::H264Decodec()
    : lengthSizeMinusOne(0), dec_buffer(NULL), dec_buffer_size(0)
{
    codec_context = avcodec_alloc_context();
    avcodec_open(codec_context); 
    picture = avcodec_alloc_frame();
}

H264Decodec::~H264Decodec()
{
	if(codec_context)
	{
		decode_end(codec_context);
	    free(codec_context->priv_data);

		free(codec_context);
		codec_context = NULL;
	}

	if(picture)
    {
		free(picture);
		picture = NULL;
	}

    if (dec_buffer) {
        delete[] dec_buffer;
    }
}

int H264Decodec::decode(uint8_t* rtmp_video_buf, uint32_t buf_size, int* got_picture)
{
    // VideoTagHeader (5 bytes)
    uint8_t frame_type = (rtmp_video_buf[0] >> 4) & 0x0f;
    uint8_t codec_id = rtmp_video_buf[0] & 0x0f;
    uint8_t packet_type = *(rtmp_video_buf+1);
    uint32_t ts = AMF_DecodeInt24((const char *)rtmp_video_buf+2); /* composition time */
    int32_t cts = (ts+0xff800000)^0xff800000;

    LOGI( "AVC/DEC: frame_type=%d, codec_id=%d, packet_type=%d, packet_size=%d\n", frame_type, codec_id, packet_type, buf_size);
    //DumpBuffer(rtmp_video_buf, buf_size);

    if (packet_type == 0) /* Sequence header */
    {
        return decodeSequenceHeader(rtmp_video_buf + 5, buf_size - 5, got_picture);
    }
    else if (packet_type == 1) /* NALU */
    {
        return decodeNALU(rtmp_video_buf + 5, buf_size - 5, got_picture);
    }

    return -1;
}

int H264Decodec::decodeSequenceHeader(uint8_t* buffer, uint32_t buf_size, int* got_picture)
{
    uint32_t out_size = 0;
    uint32_t consumed_bytes = 4; // skip configurationVersion/AVCProfileIndication/profile_compatibility/AVCLevelIndication

    // configurationVersion         1 bytes
    // AVCProfileIndication         1 bytes
    // profile_compatibility        1 bytes
    // AVCLevelIndication           1 bytes
    // lengthSizeMinusOne           1 bytes NALU Length's Size(bytes) &0x03 + 1
    // numOfSequenceParameterSets   1 bytes (0x1F mask)
    // sequenceParameterSetLength   2 bytes
    // SPS                          = sequenceParameterSetLength
    // numOfPictureParameterSets    1 bytes
    // pictureParameterSetLength    2 bytes
    // PPS                          = pictureParameterSetLength
    lengthSizeMinusOne = (buffer[consumed_bytes] & 0x03) + 1; consumed_bytes++;
    LOGI("[H264Decodec::decodeSequenceHeader] lengthSizeMinusOne=%d\n", lengthSizeMinusOne);

    uint32_t numOfSequenceParameterSets = buffer[consumed_bytes] & 0x1F; consumed_bytes++;
    for(int i = 0; i<numOfSequenceParameterSets; i++) {
        uint32_t sequenceParameterSetLength = AMF_DecodeInt16((const char *)buffer+consumed_bytes); consumed_bytes+=2;
        uint8_t* sequenceParameterSet       = buffer+consumed_bytes; consumed_bytes += sequenceParameterSetLength;
    
        LOGI("[H264Decodec::decodeSequenceHeader] SPS Length=%d, NALU-Type: %s\n", sequenceParameterSetLength, get_nalu_type_name(sequenceParameterSet[0]));
        decode_frame(codec_context, picture, got_picture, 
            (unsigned char*)buildDecodeNALUbuffer(sequenceParameterSet, sequenceParameterSetLength, &out_size), out_size);
    }

    uint32_t numOfPictureParameterSets  = buffer[consumed_bytes] & 0x1F; consumed_bytes++;
    for(int i = 0; i < numOfPictureParameterSets; i++) {
        uint32_t pictureParameterSetLength  = AMF_DecodeInt16((const char *)buffer+consumed_bytes); consumed_bytes+=2;
        uint8_t* pictureParameterSet        = buffer+consumed_bytes; consumed_bytes += pictureParameterSetLength;
    
        LOGI("[H264Decodec::decodeSequenceHeader] PPS Length=%d, NALU-Type: %s\n", pictureParameterSetLength, get_nalu_type_name(pictureParameterSet[0]));
        decode_frame(codec_context, picture, got_picture, 
            (unsigned char*)buildDecodeNALUbuffer(pictureParameterSet, pictureParameterSetLength, &out_size), out_size);
    }
}

int H264Decodec::decodeNALU(uint8_t* buffer, uint32_t buf_size, int* got_picture)
{
    // NALU length                  4 bytes (lengthSizeMinusOne!!!!!)
    // NALU data                    = NALU length
    uint32_t consumed_bytes = 0;
    while(consumed_bytes < buf_size) {
        int nalu_size = 4;

        switch(lengthSizeMinusOne) {
        case 1: nalu_size = buffer[consumed_bytes];break;
        case 2: nalu_size = AMF_DecodeInt16((const char *)buffer+consumed_bytes);break;
        case 3: nalu_size = AMF_DecodeInt24((const char *)buffer+consumed_bytes);break;
        case 4: nalu_size = AMF_DecodeInt32((const char *)buffer+consumed_bytes); break;
        }
        consumed_bytes += lengthSizeMinusOne;

        //LOGI("[H264Decodec::decodeNALU] NALU-Length = %d, NALU-Type: (%d) %s\n", nalu_size, buffer[consumed_bytes]&0x1f, get_nalu_type_name(buffer[consumed_bytes]));

        uint32_t out_size = 0;
        decode_frame(codec_context, picture, got_picture, 
            (unsigned char*)buildDecodeNALUbuffer(buffer + consumed_bytes, nalu_size, &out_size), out_size);

        if (*got_picture > 0) {
            //LOGI("[H264Decodec::decodeNALU] got-picture!");
            LOGI("[H264Decodec::decodeNALU] got-picture: true, picture-size: %dx%d", codec_context->width, codec_context->height);
        }
        consumed_bytes += nalu_size;
    }
}

uint8_t* H264Decodec::buildDecodeNALUbuffer(uint8_t* buffer, uint32_t buf_size, uint32_t *out_buf_size) {
    *out_buf_size = buf_size + 3;

    if (*out_buf_size > dec_buffer_size) {
        dec_buffer_size += 1024*10; // up 10k
        delete[] dec_buffer;
        dec_buffer = new uint8_t[dec_buffer_size];
    }

    dec_buffer[0] = 0x00;
    dec_buffer[1] = 0x00;
    dec_buffer[2] = 0x01;
    memcpy(dec_buffer + 3, buffer, buf_size);

    return dec_buffer;
}

///////////////////////////////////////////////////////////////////////////////
//
// class VideoChat 实现
//
///////////////////////////////////////////////////////////////////////////////

VideoChat::VideoChat()
    : m_isOpenPlayer(0), szRTMPUrl(NULL), pSpeexCodec(NULL), pAudioOutput(NULL), pH264Decodec(NULL)
{
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
}

VideoChat::~VideoChat()
{
    if (szRTMPUrl) delete[] szRTMPUrl;
}

int VideoChat::Init()
{
    pSpeexCodec = new SpeexCodec();
    pAudioOutput = new AudioOutput();
    pH264Decodec = new H264Decodec();

    return 0;
}

void VideoChat::Release()
{
    delete pSpeexCodec;
    delete pAudioOutput;
    delete pH264Decodec;

}

int VideoChat::Play(const char* url)
{
    if (m_isOpenPlayer > 0) return -1;
    m_isOpenPlayer = 1;
    szRTMPUrl = new char[strlen(url) + 2];
    memset(szRTMPUrl, 0, strlen(url) + 2);
    strcpy(szRTMPUrl, url);
    pthread_create(&thread_play, &thread_attr, &_play, (void*)this);
}

int VideoChat::StopPlay()
{
    if (m_isOpenPlayer == 0) return -1;

    m_isOpenPlayer = 0;
    int iRet = pthread_join(thread_play, NULL);

    return iRet;
}

void* VideoChat::_play(void* pVideoChat)
{
    VideoChat* pThis = (VideoChat*)pVideoChat;

    int audio_frame_size = pThis->pSpeexCodec->audio_frame_size();
    short *audio_buffer = audio_frame_size > 0 ? new short[audio_frame_size] : NULL;

    do {
        pThis->pRtmp = RTMP_Alloc();
        RTMP_Init(pThis->pRtmp);
        LOGI("Play RTMP_Init %s\n", pThis->szRTMPUrl);
        if (!RTMP_SetupURL(pThis->pRtmp, (char*)pThis->szRTMPUrl)) {
            LOGI("Play RTMP_SetupURL error\n");
            break; // error
        }

        if (!RTMP_Connect(pThis->pRtmp, NULL) || !RTMP_ConnectStream(pThis->pRtmp, 0)) {
            LOGI("Play RTMP_Connect or RTMP_ConnectStream error\n");
            break; // error
        }

        LOGI("RTMP_Connected\n");

        RTMPPacket rtmp_pakt = { 0 };
        while(pThis->m_isOpenPlayer > 0) {

            RTMP_GetNextMediaPacket(pThis->pRtmp, &rtmp_pakt);

            if (!rtmp_pakt.m_nBodySize)
                continue;

            if (rtmp_pakt.m_packetType == RTMP_PACKET_TYPE_AUDIO) {
                // 处理音频数据包
                LOGI( "AudioPacket: Head=0x%X\n", rtmp_pakt.m_body[0]);
                char* data = rtmp_pakt.m_body + 1;
                int offset = 0;
                int data_size = rtmp_pakt.m_nBodySize - 1;
                while(offset < data_size) {
                    // decode data+offset, data_size - offset
                    pThis->pSpeexCodec->decode(data, data_size, audio_buffer);
                    // play decode ouput_buffer
                    pThis->pAudioOutput->play(audio_buffer, audio_frame_size);
                    offset += data_size;
                }
            } else if (rtmp_pakt.m_packetType == RTMP_PACKET_TYPE_VIDEO) {
                // 处理视频数据包
	            /* H264 fix: */
                char *packetBody = rtmp_pakt.m_body;
                int packetBodySize = rtmp_pakt.m_nBodySize;

	            uint8_t CodecId = packetBody[0] & 0x0f;
	            if(CodecId == 7) { /* CodecId = H264 */
                    int got_picture = 0;
                    pThis->pH264Decodec->decode((uint8_t*)packetBody, packetBodySize, &got_picture);
                    if (got_picture) 
                    {
                        // show picture
                        // pThis->pH264Decodec->getPicture();
                		//DisplayYUV_16((unsigned int*)Pixel, picture->data[0], picture->data[1], picture->data[2], 
                        //              c->width, c->height, picture->linesize[0], picture->linesize[1], iWidth);	
                    }
                } else {
                    LOGI( "AVC: CodecId != 7" );
                }

            } else if (rtmp_pakt.m_packetType == RTMP_PACKET_TYPE_INFO) {
                // 处理信息包
            } else if (rtmp_pakt.m_packetType == RTMP_PACKET_TYPE_FLASH_VIDEO) {
                // 其他Flash数据
            }

            RTMPPacket_Free(&rtmp_pakt);
        }

    }while(0);

    delete[] audio_buffer;

    if (RTMP_IsConnected(pThis->pRtmp)) {
        RTMP_Close(pThis->pRtmp);
    }
    RTMP_Free(pThis->pRtmp);
    pThis->m_isOpenPlayer = 0;

    return 0;
}



