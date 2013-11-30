#ifndef __VIDEODECODER_H__
#define __VIDEODECODER_H__



extern "C" {

#ifndef   UINT64_C
#define   UINT64_C(value) __CONCAT(value,ULL)
#endif

#include "libavcodec/avcodec.h"
#include "libpostproc/postprocess.h"  

} // extern "C" 


#include "utils.h"



//
// video codec
//
class H264Decodec
{
public:
    H264Decodec();
    ~H264Decodec();

    int decode(uint8_t* rtmp_video_buf, uint32_t buf_size, int* got_picture);
    AVFrame* getFrame(void){ return picture; }
    Mutex& getFrameLock(void){return _picture_lock;};

    uint32_t getWidth() { return codec_context->width; }
    uint32_t getHeight() { return codec_context->height; }

private:
    int decodeSequenceHeader(uint8_t* buffer, uint32_t buf_size, int* got_picture);
    int decodeNALU(uint8_t* buffer, uint32_t buf_size, int* got_picture);
    
    int final_decode(uint8_t* buffer, uint32_t buf_size, int* got_picture);
    int final_decode_header(uint8_t* sequenceParameterSet, uint32_t sequenceParameterSetLength, 
                            uint8_t* pictureParameterSet, uint32_t pictureParameterSetLength, int* got_picture);

private:
    uint8_t lengthSizeMinusOne;
    AVCodecContext *codec_context;
    AVCodec *_codec;
    AVFrame *picture;
    uint8_t *dec_buffer;
    uint32_t dec_buffer_size;

    Mutex _picture_lock;

#ifdef DEBUG
    FILE* decoder_fd;
#endif
};



#endif  //__VIDEODECODER_H__