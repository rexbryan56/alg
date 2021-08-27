
#ifndef __CRC32__
#define __CRC32__

#include <stdint.h>

void make_crc_table(void);
uint64_t crc32(uint64_t crc, uint8_t *buf, uint64_t len);

#endif
