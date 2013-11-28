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

//
//Shader.frag文件内容
//
static const char* FRAG_SHADER =
    "varying lowp vec2 tc;                          \n"
    "uniform sampler2D SamplerY;                    \n"
    "uniform sampler2D SamplerU;                    \n"
    "uniform sampler2D SamplerV;                    \n"
    "void main(void)                                \n"
    "{                                              \n"
        "mediump vec3 yuv;                          \n"
        "lowp vec3 rgb;                             \n"
        "yuv.x = texture2D(SamplerY, tc).r;         \n"
        "yuv.y = texture2D(SamplerU, tc).r - 0.5;   \n"
        "yuv.z = texture2D(SamplerV, tc).r - 0.5;   \n"
        "rgb = mat3( 1, 1, 1,                       \n"
                    "0, -0.39465, 2.03211,          \n"
                    "1.13983, -0.58060, 0) * yuv;   \n"
        "gl_FragColor = vec4(rgb, 1);               \n"
    "}                                              \n";

//
// Shader.vert文件内容
//
static const char* VERTEX_SHADER =
      "attribute vec4 vPosition;    \n"
      "attribute vec2 a_texCoord;	\n"
      "varying vec2 tc;             \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = vPosition;  \n"
      "	  tc = a_texCoord;	        \n"
      "}                            \n";


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

static void checkGlError(const char* op)
{
    GLint error;
    for (error = glGetError(); error; error = glGetError())
    {
        LOGI("[OpenGL ERROR]::after %s() glError (0x%x)\n", op, error);
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
// class VideoRender 实现
//
///////////////////////////////////////////////////////////////////////////////
VideoRender::VideoRender()
    : m_width(0), m_height(0), m_viewport_width(0), m_viewport_height(0)
{
    simpleProgram = buildProgram(VERTEX_SHADER, FRAG_SHADER);
    glUseProgram(simpleProgram);
    glGenTextures(1, &m_texYId);
    glGenTextures(1, &m_texUId);
    glGenTextures(1, &m_texVId);
}

VideoRender::~VideoRender()
{
}

GLuint VideoRender::bindTexture(GLuint texture, const char *buffer, GLuint w , GLuint h)
{
//  GLuint texture;
//  glGenTextures ( 1, &texture );
    checkGlError("glGenTextures");
    glBindTexture ( GL_TEXTURE_2D, texture );
    checkGlError("glBindTexture");
    glTexImage2D ( GL_TEXTURE_2D, 0, GL_LUMINANCE, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer);
    checkGlError("glTexImage2D");
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    checkGlError("glTexParameteri");
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    checkGlError("glTexParameteri");
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    checkGlError("glTexParameteri");
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    checkGlError("glTexParameteri");
    //glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

GLuint VideoRender::buildShader(const char* source, GLenum shaderType)
{
    GLuint shaderHandle = glCreateShader(shaderType);

    if (shaderHandle)
    {
        glShaderSource(shaderHandle, 1, &source, 0);
        glCompileShader(shaderHandle);

        GLint compiled = 0;
        glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compiled);
        if (!compiled)
        {
            GLint infoLen = 0;
            glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen)
            {
                char* buf = (char*) malloc(infoLen);
                if (buf)
                {
                    glGetShaderInfoLog(shaderHandle, infoLen, NULL, buf);
                    LOGI("[OpenGL ERROR]::Could not compile shader %d:\n%s\n", shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shaderHandle);
                shaderHandle = 0;
            }
        }else{
            LOGI("[VideoRender::buildShader] shader compiled!\n");
        }
    }

    return shaderHandle;
}

GLuint VideoRender::buildProgram(const char* vertexShaderSource, const char* fragmentShaderSource)
{
    GLuint vertexShader = buildShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = buildShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    GLuint programHandle = glCreateProgram();

    if (programHandle)
    {
        glAttachShader(programHandle, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(programHandle, fragmentShader);
        checkGlError("glAttachShader");
        glLinkProgram(programHandle);

        GLint linkStatus = GL_FALSE;
        glGetProgramiv(programHandle, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(programHandle, bufLength, NULL, buf);
                    LOGI("[OpenGL ERROR]::Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(programHandle);
            programHandle = 0;
        }else{
            LOGI("[VideoRender::buildProgram] program linked!\n");
        }
    }

    return programHandle;
}

#if 1
// Galaxy Nexus 4.2.2
static GLfloat gl_squareVertices[] = {
    -1.0f, -1.0f, 1.0f, -1.0f,
    -1.0f,  1.0f, 1.0f,  1.0f
};

static GLfloat gl_coordVertices[] = {
    0.0f, 1.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 0.0f
};
#else
// HUAWEIG510-0010 4.1.1
static GLfloat gl_squareVertices[] = {
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 1.0f, 1.0f,
};

static GLfloat gl_coordVertices[] = {
    -1.0f,  1.0f, 1.0f,  1.0f,
    -1.0f, -1.0f, 1.0f, -1.0f,
};
#endif

void VideoRender::render_frame()
{
    // render
    AutoLock lock_frame(*m_frame_lock);

    uint32_t& width = m_width;
    uint32_t& height = m_height;
    LOGI("[VideoRender::render_frame] size=%dx%d, viewport=%dx%d", width, height, m_viewport_width, m_viewport_height);
    if (width == 0 || height == 0)
        return;

    glViewport(0, 0, m_viewport_width, m_viewport_height);
    bindTexture(m_texYId, (const char*)m_frame->data[0], width, height);
    bindTexture(m_texUId, (const char*)m_frame->data[1], width/2, height/2);
    bindTexture(m_texVId, (const char*)m_frame->data[2], width/2, height/2);



    // clear background
    glClearColor(0.5f, 0.5f, 0.5f, 1);
    checkGlError("glClearColor");
    glClear(GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");


    //PRINTF("setsampler %d %d %d", g_texYId, g_texUId, g_texVId);
    GLint tex_y = glGetUniformLocation(simpleProgram, "SamplerY");
    checkGlError("glGetUniformLocation");
    GLint tex_u = glGetUniformLocation(simpleProgram, "SamplerU");
    checkGlError("glGetUniformLocation");
    GLint tex_v = glGetUniformLocation(simpleProgram, "SamplerV");
    checkGlError("glGetUniformLocation");


    glBindAttribLocation(simpleProgram, ATTRIB_VERTEX, "vPosition");
    checkGlError("glBindAttribLocation");
    glBindAttribLocation(simpleProgram, ATTRIB_TEXTURE, "a_texCoord");
    checkGlError("glBindAttribLocation");

    glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, gl_squareVertices);
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(ATTRIB_VERTEX);
    checkGlError("glEnableVertexAttribArray");

    glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, 0, 0, gl_coordVertices);
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(ATTRIB_TEXTURE);
    checkGlError("glEnableVertexAttribArray");

    glActiveTexture(GL_TEXTURE0);
    checkGlError("glActiveTexture");
    glBindTexture(GL_TEXTURE_2D, m_texYId);
    checkGlError("glBindTexture");
    glUniform1i(tex_y, 0);
    checkGlError("glUniform1i");

    glActiveTexture(GL_TEXTURE1);
    checkGlError("glActiveTexture");
    glBindTexture(GL_TEXTURE_2D, m_texUId);
    checkGlError("glBindTexture");
    glUniform1i(tex_u, 1);
    checkGlError("glUniform1i");

    glActiveTexture(GL_TEXTURE2);
    checkGlError("glActiveTexture");
    glBindTexture(GL_TEXTURE_2D, m_texVId);
    checkGlError("glBindTexture");
    glUniform1i(tex_v, 2);
    checkGlError("glUniform1i");

    //glEnable(GL_TEXTURE_2D);
    //checkGlError("glEnable");
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    checkGlError("glDrawArrays");
}



///////////////////////////////////////////////////////////////////////////////
//
// class H264Decodec 实现
//
///////////////////////////////////////////////////////////////////////////////
static void log_callback(void* ptr, int level,const char* fmt,va_list vl)  
{      
   LOGI(fmt, vl);
}

H264Decodec::H264Decodec()
    : lengthSizeMinusOne(0), dec_buffer(NULL), dec_buffer_size(0)
{
    av_log_set_callback(log_callback);
    avcodec_register_all();

    _codec = avcodec_find_decoder(CODEC_ID_H264);
    codec_context = avcodec_alloc_context3(_codec);
    avcodec_open2(codec_context, _codec, NULL);

    picture = avcodec_alloc_frame();


    // output debug data
#ifdef DEBUG
    decoder_fd = fopen("/sdcard/video_data.264", "wb");

    if (!decoder_fd) {
        LOGI("ERROR: create debug decode video file failed!!!");
    }
#endif
}

H264Decodec::~H264Decodec()
{
#ifdef DEBUG
    if (decoder_fd)
        fclose(decoder_fd);
#endif

    if(codec_context)
    {
        avcodec_close(codec_context);
        codec_context = NULL;
    }

    if(picture)
    {
        av_frame_free(&picture);
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

    AutoLock lock_frame(_picture_lock);
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

    // trac SPS PPS
    uint32_t sequenceParameterSetLength = NULL;
    uint8_t* sequenceParameterSet       = NULL;
    uint32_t pictureParameterSetLength  = NULL;
    uint8_t* pictureParameterSet        = NULL;

    uint32_t numOfSequenceParameterSets = buffer[consumed_bytes] & 0x1F; consumed_bytes++;
    for(int i = 0; i<numOfSequenceParameterSets; i++) {
        sequenceParameterSetLength = AMF_DecodeInt16((const char *)buffer+consumed_bytes); consumed_bytes+=2;
        sequenceParameterSet       = buffer+consumed_bytes; consumed_bytes += sequenceParameterSetLength;
        LOGI("[H264Decodec::decodeSequenceHeader] SPS Length=%d, NALU-Type: %s\n", sequenceParameterSetLength, get_nalu_type_name(sequenceParameterSet[0]));
    }

    uint32_t numOfPictureParameterSets  = buffer[consumed_bytes] & 0x1F; consumed_bytes++;
    for(int i = 0; i < numOfPictureParameterSets; i++) {
        pictureParameterSetLength  = AMF_DecodeInt16((const char *)buffer+consumed_bytes); consumed_bytes+=2;
        pictureParameterSet        = buffer+consumed_bytes; consumed_bytes += pictureParameterSetLength;
        LOGI("[H264Decodec::decodeSequenceHeader] PPS Length=%d, NALU-Type: %s\n", pictureParameterSetLength, get_nalu_type_name(pictureParameterSet[0]));
    }
    
    final_decode_header(
        sequenceParameterSet, sequenceParameterSetLength, 
        pictureParameterSet, pictureParameterSetLength, got_picture);
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

        final_decode(buffer + consumed_bytes, nalu_size, got_picture);

        if (*got_picture > 0) {
            LOGI("[H264Decodec::decodeNALU] got-picture: true, picture-size: %dx%d", codec_context->width, codec_context->height);
        }
        consumed_bytes += nalu_size;
    }
}

int H264Decodec::final_decode_header(
    uint8_t* sequenceParameterSet, uint32_t sequenceParameterSetLength, 
    uint8_t* pictureParameterSet, uint32_t pictureParameterSetLength, int* got_picture) {

    AVPacket packet = {0};
    int final_size = sequenceParameterSetLength + pictureParameterSetLength + 8;

    while (final_size > dec_buffer_size) {
        dec_buffer_size += 1024*10; // up 10k
        delete[] dec_buffer;
        dec_buffer = new uint8_t[dec_buffer_size];
    }

    dec_buffer[0] = 0; dec_buffer[sequenceParameterSetLength + 4 + 0] = 0; 
    dec_buffer[1] = 0; dec_buffer[sequenceParameterSetLength + 4 + 1] = 0;
    dec_buffer[2] = 0; dec_buffer[sequenceParameterSetLength + 4 + 2] = 0;
    dec_buffer[3] = 1; dec_buffer[sequenceParameterSetLength + 4 + 3] = 1;
    memcpy(dec_buffer + 4, sequenceParameterSet, sequenceParameterSetLength);
    memcpy(dec_buffer + sequenceParameterSetLength + 8, pictureParameterSet, pictureParameterSetLength);

#ifdef DEBUG
    fwrite(dec_buffer, 1, final_size, decoder_fd);
    fflush(decoder_fd);
#endif

    av_init_packet(&packet);
    av_packet_from_data(&packet, dec_buffer, final_size);

    return avcodec_decode_video2(codec_context, picture, got_picture, &packet);
}

int H264Decodec::final_decode(uint8_t* buffer, uint32_t buf_size, int* got_picture) {

    AVPacket packet = {0};
    int final_size = buf_size + 4;

    while (final_size > dec_buffer_size) {
        dec_buffer_size += 1024*10; // up 10k
        delete[] dec_buffer;
        dec_buffer = new uint8_t[dec_buffer_size];
    }

    dec_buffer[0] = 0;
    dec_buffer[1] = 0;
    dec_buffer[2] = 0;
    dec_buffer[3] = 1;
    memcpy(dec_buffer + 4, buffer, buf_size);

#ifdef DEBUG
    fwrite(dec_buffer, 1, final_size, decoder_fd);
    fflush(decoder_fd);
#endif

    av_init_packet(&packet);
    av_packet_from_data(&packet, dec_buffer, final_size);

    return avcodec_decode_video2(codec_context, picture, got_picture, &packet);
}

///////////////////////////////////////////////////////////////////////////////
//
// class VideoChat 实现
//
///////////////////////////////////////////////////////////////////////////////

VideoChat::VideoChat()
    : m_isOpenPlayer(0), szRTMPUrl(NULL), pSpeexCodec(NULL), pAudioOutput(NULL), pH264Decodec(NULL), pVideoRender(NULL)
{
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
}

VideoChat::~VideoChat()
{
    if (szRTMPUrl) delete[] szRTMPUrl;
}

void VideoChat::InitRender(int width, int height)
{
    if (pVideoRender)
        delete pVideoRender;

    pVideoRender = new VideoRender();
    pVideoRender->set_view(width, height);
    pVideoRender->set_frame(pH264Decodec->getPicture(), &pH264Decodec->getPictureLock());
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

    if (pVideoRender)
        delete pVideoRender;
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

    //int result_code = mkdir("/data/data/cn.videochat.MainActivity/files/", 0770);
#ifdef DEBUG
    FILE* fd = fopen("/sdcard/video_data.yuv", "wb");
    if (!fd) {
        LOGI("ERROR: create debug decode video file failed!!!");
    }
#endif

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

                    if (got_picture && pThis->pVideoRender) 
                    {
                        pThis->pVideoRender->set_size(
                            pThis->pH264Decodec->getWidth(),
                            pThis->pH264Decodec->getHeight());
                    }

#ifdef DEBUG
                    // debug data
                    if (got_picture && fd) {
                        AVFrame *frame = pThis->pH264Decodec->getPicture();
                        uint32_t width = pThis->pH264Decodec->getWidth();
                        uint32_t height = pThis->pH264Decodec->getHeight();

                        fwrite(frame->data[0], 1, width * height, fd);
                        fwrite(frame->data[1], 1, width * height / 4, fd);
                        fwrite(frame->data[2], 1, width * height / 4, fd);

                        fflush(fd);
                    } else if (!fd){
                        LOGI("ERROR: don't open video log file!!!");
                    }
#endif // DEBUG
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

#ifdef DEBUG
    if (fd) fclose(fd);
#endif

    delete[] audio_buffer;

    if (RTMP_IsConnected(pThis->pRtmp)) {
        RTMP_Close(pThis->pRtmp);
    }
    RTMP_Free(pThis->pRtmp);
    pThis->m_isOpenPlayer = 0;

    return 0;
}
