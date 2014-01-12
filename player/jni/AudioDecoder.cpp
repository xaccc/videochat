#include "AudioDecoder.h"




///////////////////////////////////////////////////////////////////////////////
//
// class SpeexCodec 实现
//
///////////////////////////////////////////////////////////////////////////////

SpeexCodec::SpeexCodec() : dec_state (NULL)
{
    reset();
}

SpeexCodec::~SpeexCodec()
{
	if (dec_state)
		speex_decoder_destroy(dec_state);
	dec_state = NULL;
}

void SpeexCodec::reset(void) {
	if (dec_state != NULL)
		speex_decoder_destroy(dec_state);

	decode_frames = 0;
    dec_state = speex_decoder_init(&speex_wb_mode);
    speex_decoder_ctl(dec_state, SPEEX_GET_FRAME_SIZE, &dec_frame_size);

    // disable perceptual enhancer
    int enh=1;
    speex_decoder_ctl(dec_state, SPEEX_SET_ENH, &enh);
}

int SpeexCodec::decode(char* data, int data_size, short** output_buffer)
{
    //if (output_buffer == NULL || output_buffer[0] == NULL)
    //    return -1;
	if (decode_frames > 100)
		reset();

    SpeexBits dbits;
    speex_bits_init(&dbits);
    speex_bits_set_bit_buffer(&dbits, data, data_size);
    
    int frames_count = 0;
    while(speex_bits_remaining(&dbits) > 0 && frames_count < 100) {
        if (speex_decode_int(dec_state, &dbits, output_buffer[frames_count]))
            break; // error
            
        frames_count++;
    }
    
    speex_bits_destroy(&dbits);

    decode_frames += frames_count;

    return frames_count;
}


