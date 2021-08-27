#ifndef _DEFLATE_B
#define _DEFLATE_B

#include <stdint.h>

#define INBUF_LEN 4000 //block size of 4Kb seems gives us better compression efficiency
#define OUTBUF_LEN (INBUF_LEN*1.3)

struct deflate_state
{
	/*const*/ uint8_t *inbuf; //input buffer
	uint64_t inbuf_len;
	uint64_t inbuf_index; // the element being worked on
	uint8_t *outbuf;
	uint64_t outbuf_len;
	uint64_t outbuf_index; // the element being worked on
	uint8_t holdbuf;
	uint16_t bitwrtn; // no of bits written into an element of the outbuf
	uint64_t bitcnt; //Total no of bits written into buffer
	_Bool lastblk; //as name implies
	_Bool nocomp; // true if data is not to be compressed
	int state; //true if there is still data to be read into input buffer(inbuf)
};


int deflate (struct deflate_state * w);

#endif
