#include "AudioDecoder.h"




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


