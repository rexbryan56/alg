
#ifndef __GZLIB__
#define __GZLIB__
#include <stdint.h>
#include "zlib.h"



#define GZIP_EXT ".gz"

struct filedata
{
	uint64_t fsize;
	uint32_t mtime;
	char * filename;
};

void write_gzip_member_header (struct deflate_state * gz, struct filedata *Filedata);
void write_gzip_trailer (struct deflate_state * gz, uint32_t crc, uint64_t Isize);

#endif
