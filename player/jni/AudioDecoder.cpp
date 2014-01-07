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
    // int enh=0;
    // speex_decoder_ctl(dec_state, SPEEX_SET_ENH, &enh);

    dec_frame = (char*)malloc(output_buffer_size());

    jitter = jitter_buffer_init(5); //???
    jitter_buffer_reset(jitter);

}

SpeexCodec::~SpeexCodec()
{
    speex_bits_destroy(&dbits);
    speex_decoder_destroy(dec_state);
    jitter_buffer_destroy(jitter);
    free(dec_frame);
}


#ifdef USE_JITTER

int SpeexCodec::decode(uint32_t ts, char* data, int data_size, short** output_buffer)
{
    if (output_buffer == NULL || output_buffer[0] == NULL)
        return -1;

    speex_bits_set_bit_buffer(&dbits, data, data_size);
    
    int frames_count = 0;
    while(speex_bits_remaining(&dbits) > 0 && frames_count < 100) {
        if (speex_decode_int(dec_state, &dbits, (short *)dec_frame))
            break; // error
            
		JitterBufferPacket packet;
		packet.data = dec_frame;
		packet.len = output_buffer_size();
		packet.timestamp = ts;
		packet.span = 5; //???
		packet.sequence = 0;
		packet.user_data = 0;
		jitter_buffer_put(jitter, &packet);

        frames_count++;
    }
    
    JitterBufferPacket outpacket;

    int start_offset;
    outpacket.data = dec_frame;
    outpacket.len = output_buffer_size();
    if(JITTER_BUFFER_OK != jitter_buffer_get(jitter, &outpacket, 5, &start_offset)) {
    	return 0;
    }

    jitter_buffer_tick(jitter);
    memcpy(output_buffer[0], dec_frame, output_buffer_size());

    return 1;
}

#else // USE_JITTER

int SpeexCodec::decode(uint32_t ts, char* data, int data_size, short** output_buffer)
{
    if (output_buffer == NULL || output_buffer[0] == NULL)
        return 0;

    speex_bits_set_bit_buffer(&dbits, data, data_size);

    int frames_count = 0;
    while(speex_bits_remaining(&dbits) > 0 && frames_count < 100) {
        if (speex_decode_int(dec_state, &dbits, (short *)output_buffer[frames_count]))
            break; // error

        frames_count++;
    }

    return frames_count;
}

#endif // USE_JITTER
