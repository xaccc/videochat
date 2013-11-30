#ifndef __AUDIODECODER_H__
#define __AUDIODECODER_H__

extern "C" {
#include <speex/speex.h>
#include <speex/speex_header.h>
} // extern "C"

#include "utils.h"




//
// audio codec
//
class SpeexCodec
{
public:
    SpeexCodec();
    ~SpeexCodec();

    int decode(char* data, int data_size, short** output_buffer);

    int output_buffer_size() { return dec_frame_size * sizeof(short); }
    int audio_frame_size() { return dec_frame_size; }

private:
    int dec_frame_size;
    void *dec_state;
    SpeexBits dbits;
};



#endif  //__AUDIODECODER_H__