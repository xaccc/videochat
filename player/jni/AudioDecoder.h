#ifndef __AUDIODECODER_H__
#define __AUDIODECODER_H__

extern "C" {
#include <speex/speex.h>
#include <speex/speex_header.h>
#include <speex/speex_jitter.h>
} // extern "C"

#include "utils.h"


#define USE_SPEEX_JITTER 1

#if USE_SPEEX_JITTER

typedef struct SpeexJitter {
   SpeexBits current_packet;         /**< Current Speex packet */
   int valid_bits;                   /**< True if Speex bits are valid */
   JitterBuffer *packets;            /**< Generic jitter buffer state */
   void *dec;                        /**< Pointer to Speex decoder */
   spx_int32_t frame_size;           /**< Frame size of Speex decoder */
} SpeexJitter;

void speex_jitter_init(SpeexJitter *jitter, void *decoder, int sampling_rate);
/** Destroy jitter buffer */
void speex_jitter_destroy(SpeexJitter *jitter);
/** Put one packet into the jitter buffer */
void speex_jitter_put(SpeexJitter *jitter, char *packet, int len, int timestamp);
/** Get one packet from the jitter buffer */
void speex_jitter_get(SpeexJitter *jitter, spx_int16_t *out, int *start_offset);
/** Get pointer timestamp of jitter buffer */
int speex_jitter_get_pointer_timestamp(SpeexJitter *jitter);

#endif //USE_SPEEX_JITTER


//
// audio codec
//
class SpeexCodec
{
public:
    SpeexCodec();
    ~SpeexCodec();

    int decode(uint32_t ts, char* data, int data_size, short** output_buffer);

    int output_buffer_size() { return dec_frame_size * sizeof(short); }
    int audio_frame_size() { return dec_frame_size; }

private:
    int dec_frame_size;
    void *dec_state;

#if USE_SPEEX_JITTER
    SpeexJitter speexJitter;
#else
    SpeexBits dbits;
#endif //USE_SPEEX_JITTER
};


#endif  //__AUDIODECODER_H__
