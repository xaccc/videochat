#include "VideoDecoder.h"
#include <librtmp/rtmp.h>

//#undef LOGI
//#define LOGI(...)


///////////////////////////////////////////////////////////////////////////////
//
// class H264Decodec 实现
//
///////////////////////////////////////////////////////////////////////////////


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


static void log_callback(void* ptr, int level,const char* fmt,va_list vl)
{
   // LOGI(fmt, vl);
}

H264Decodec::H264Decodec()
    : lengthSizeMinusOne(0), h264stream_buffer(NULL), h264stream_buffer_size(0)
{
#ifdef	USEFFMPEG
    av_log_set_callback(log_callback);
    avcodec_register_all();

    _codec = avcodec_find_decoder(CODEC_ID_H264);
    codec_context = avcodec_alloc_context3(_codec);

    // codec_context->width  = 320;
    // codec_context->height = 240;
    // codec_context->pix_fmt = PIX_FMT_YUV420P;

    avcodec_open2(codec_context, _codec, NULL);
#else // custom h264dec
    codec_context = avcodec_alloc_context();
    avcodec_open(codec_context);
#endif
}

H264Decodec::~H264Decodec()
{
    if(codec_context)
    {
#ifdef	USEFFMPEG
        avcodec_close(codec_context);
        av_free(codec_context);
        codec_context = NULL;
#else // custom h264dec
    decode_end(codec_context);
    free(codec_context->priv_data);
    free(codec_context);
    codec_context = NULL;
#endif
    }

    if (h264stream_buffer) {
        delete[] h264stream_buffer;
    }
}

int H264Decodec::decode(uint8_t* rtmp_video_buf, uint32_t buf_size, AVFrame* picture, int* got_picture)
{
    // VideoTagHeader (5 bytes)
    uint8_t frame_type = (rtmp_video_buf[0] >> 4) & 0x0f;
    uint8_t codec_id = rtmp_video_buf[0] & 0x0f;
    uint8_t packet_type = *(rtmp_video_buf+1);
    uint32_t ts = AMF_DecodeInt24((const char *)rtmp_video_buf+2); /* composition time */
    int32_t cts = (ts+0xff800000)^0xff800000;

    LOGI( "AVC/DEC: frame_type=%d, codec_id=%d, packet_type=%d, packet_size=%d\n", frame_type, codec_id, packet_type, buf_size);

    if (packet_type == 0) /* Sequence header */
    {
        return decodeSequenceHeader(rtmp_video_buf + 5, buf_size - 5, picture, got_picture);
    }
    else if (packet_type == 1) /* NALU */
    {
        return decodeNALU(rtmp_video_buf + 5, buf_size - 5, picture, got_picture);
    }

    return -1;
}

int H264Decodec::decodeSequenceHeader(uint8_t* buffer, uint32_t buf_size, AVFrame* picture, int* got_picture)
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
    uint32_t sequenceParameterSetLength = 0;
    uint8_t* sequenceParameterSet       = NULL;
    uint32_t pictureParameterSetLength  = 0;
    uint8_t* pictureParameterSet        = NULL;

    uint32_t numOfSequenceParameterSets = buffer[consumed_bytes] & 0x1F; consumed_bytes++;
    for(int i = 0; i<numOfSequenceParameterSets && consumed_bytes < buf_size; i++) {
        sequenceParameterSetLength = AMF_DecodeInt16((const char *)buffer+consumed_bytes); consumed_bytes+=2;
        sequenceParameterSet       = buffer+consumed_bytes; consumed_bytes += sequenceParameterSetLength;
    }

    uint32_t numOfPictureParameterSets  = buffer[consumed_bytes] & 0x1F; consumed_bytes++;
    for(int i = 0; i < numOfPictureParameterSets && consumed_bytes < buf_size; i++) {
        pictureParameterSetLength  = AMF_DecodeInt16((const char *)buffer+consumed_bytes); consumed_bytes+=2;
        pictureParameterSet        = buffer+consumed_bytes; consumed_bytes += pictureParameterSetLength;
    }

    LOGI("[H264Decodec::decodeSequenceHeader] SPS Length=%d, NALU-Type: %s, PPS Length=%d, NALU-Type: %s\n",
        sequenceParameterSetLength, get_nalu_type_name(sequenceParameterSet[0]),
        pictureParameterSetLength, get_nalu_type_name(pictureParameterSet[0]));

    final_decode_header(
        sequenceParameterSet, sequenceParameterSetLength,
        pictureParameterSet, pictureParameterSetLength, picture, got_picture);
}

int H264Decodec::decodeNALU(uint8_t* buffer, uint32_t buf_size, AVFrame* picture, int* got_picture)
{
    // NALU length                  4 bytes (lengthSizeMinusOne!!!!!)
    // NALU data                    = NALU length
    uint32_t consumed_bytes = 0;
    while(consumed_bytes < buf_size) {
        int nalu_size = 4; // default 4 bytes

        switch(lengthSizeMinusOne) {
        case 1: nalu_size = buffer[consumed_bytes];break;
        case 2: nalu_size = AMF_DecodeInt16((const char *)buffer+consumed_bytes);break;
        case 3: nalu_size = AMF_DecodeInt24((const char *)buffer+consumed_bytes);break;
        case 4: nalu_size = AMF_DecodeInt32((const char *)buffer+consumed_bytes); break;
        }
        consumed_bytes += lengthSizeMinusOne;

        LOGI("[H264Decodec::decodeNALU] NALU-Length = %d, NALU-Type: (%d) %s\n", nalu_size, buffer[consumed_bytes]&0x1f, get_nalu_type_name(buffer[consumed_bytes]));

        final_decode(buffer + consumed_bytes, nalu_size, picture, got_picture);

        consumed_bytes += nalu_size;
    }
}

int H264Decodec::final_decode_header(
    uint8_t* sequenceParameterSet, uint32_t sequenceParameterSetLength,
    uint8_t* pictureParameterSet, uint32_t pictureParameterSetLength, AVFrame* picture, int* got_picture) {

    int final_size = sequenceParameterSetLength + pictureParameterSetLength + 8;

    while (final_size > h264stream_buffer_size) {
        h264stream_buffer_size += 1024*10; // up 10k
        delete[] h264stream_buffer;
        h264stream_buffer = new uint8_t[h264stream_buffer_size];
    }

    h264stream_buffer[0] = 0; h264stream_buffer[sequenceParameterSetLength + 4 + 0] = 0;
    h264stream_buffer[1] = 0; h264stream_buffer[sequenceParameterSetLength + 4 + 1] = 0;
    h264stream_buffer[2] = 0; h264stream_buffer[sequenceParameterSetLength + 4 + 2] = 0;
    h264stream_buffer[3] = 1; h264stream_buffer[sequenceParameterSetLength + 4 + 3] = 1;
    memcpy(h264stream_buffer + 4, sequenceParameterSet, sequenceParameterSetLength);
    memcpy(h264stream_buffer + sequenceParameterSetLength + 8, pictureParameterSet, pictureParameterSetLength);

#ifdef USEFFMPEG
    AVPacket packet = {0};
    av_init_packet(&packet);
    av_packet_from_data(&packet, h264stream_buffer, final_size);

    return avcodec_decode_video2(codec_context, picture, got_picture, &packet);
#else
    decode_frame(codec_context, picture, got_picture, h264stream_buffer, sequenceParameterSetLength + 4);
    decode_frame(codec_context, picture, got_picture, h264stream_buffer + sequenceParameterSetLength + 4, pictureParameterSetLength + 4);

    return 0;
#endif
}

int H264Decodec::final_decode(uint8_t* buffer, uint32_t buf_size, AVFrame* picture, int* got_picture) {

    int final_size = buf_size + 4;

    while (final_size > h264stream_buffer_size) {
        h264stream_buffer_size += 1024*10; // up 10k
        delete[] h264stream_buffer;
        h264stream_buffer = new uint8_t[h264stream_buffer_size];
    }

    h264stream_buffer[0] = 0;
    h264stream_buffer[1] = 0;
    h264stream_buffer[2] = 0;
    h264stream_buffer[3] = 1;
    memcpy(h264stream_buffer + 4, buffer, buf_size);

#ifdef USEFFMPEG
    AVPacket packet = {0};
    av_init_packet(&packet);
    av_packet_from_data(&packet, h264stream_buffer, final_size);

    return avcodec_decode_video2(codec_context, picture, got_picture, &packet);
#else
    return decode_frame(codec_context, picture, got_picture, h264stream_buffer, final_size);
#endif
}
