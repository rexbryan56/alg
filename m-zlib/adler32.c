/*
*My implementation of the adler32 checksum as contained in the RFC 1950
*Should work properly for len <= 500000000*/
#include <stdint.h>
#include <assert.h>
#include "adler32.h"

#define BASE 65521

uint64_t fast_adler32(uint64_t adler, uint8_t *buf, uint64_t len)
{
	assert(len <= 500000000);
	adler = (adler == 0) ? 1 : adler;
	uint64_t s1 = adler & 0xffff;
	uint64_t s2 = (adler >> 16) & 0xffff;
	
	for (uint64_t i = 0; i < len; i++)
	{
		s1 = (s1 + buf[i]);
		s2 = (s2 + s1);
	}
	s1 %= BASE;
	s2 %= BASE;
	
	return (s2 << 16) | s1;
}

