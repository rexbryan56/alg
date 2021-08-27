#ifndef __ADLER32__
#define __ADLER32__

#include <stdint.h>
uint64_t fast_adler32(uint64_t adler, uint8_t *buf, uint64_t len);

#endif
