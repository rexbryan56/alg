
#ifndef __ZLIB__
#define __ZLIB__

#include <stdint.h>
#include "deflate.h"
#include "adler32.h"

void generate_zlib_header (struct deflate_state * z);
void writeout_adler32 (struct deflate_state * a, uint32_t adler);

#endif
