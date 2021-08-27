#include <stdint.h>
#include "gzlib.h"

#define ID1 31 /*ID1 and ID2 are fixed*/
#define ID2 139
#define CM 8
#define FLG 8 /*00001000 IN BITS*/
#define XFL 2
#define OS 3 /*unix*/

#define MTIME_SIZE 4 /*size of memory space allocated for member mtime in rfc 1952*/
#define CRC_WTH 4
#define ISIZE_WTH 4

void write_gzip_member_header (struct deflate_state * gz, struct filedata *Filedata)
{
	/*write ID1*/
	gz->outbuf[gz->outbuf_index++] = ID1;
	/*write ID2*/
	gz->outbuf[gz->outbuf_index++] = ID2;
	/*write CM*/
	gz->outbuf[gz->outbuf_index++] = CM;
	/*write FLG: if filename == null set all bits to zero*/
	gz->outbuf[gz->outbuf_index++] = (Filedata->filename) ? FLG : 0;
	
	/*write mtime: potentially non-portable code here*/
	uint32_t tmp = Filedata->mtime;
	uint8_t * mtime = (uint8_t *) &tmp;
	for (int i = 0; i < MTIME_SIZE; i++)
		gz->outbuf[gz->outbuf_index++] = mtime[i];
		
	/*write XFL*/
	gz->outbuf[gz->outbuf_index++] = XFL;
	
	/*write OS*/
	gz->outbuf[gz->outbuf_index++] = OS;
	
	/*if filename not null*/
	if (Filedata->filename)
	{
		while (*Filedata->filename)
			gz->outbuf[gz->outbuf_index++] = *(Filedata->filename++);
			
		gz->outbuf[gz->outbuf_index++] = 0;
	}
}

/*write crc and input size( mod 2^32) to buffer
* This code is not portatble*/
void write_gzip_trailer (struct deflate_state * gz, uint32_t crc, uint64_t Isize)
{
	int i;
	Isize %= 4294967296;
	uint8_t * p_crc = (uint8_t *) &crc;
	uint8_t * p_isize = (uint8_t *) &Isize;
	
	for (i = 0; i < CRC_WTH; i++)
		gz->outbuf[gz->outbuf_index++] = p_crc[i];
	
	for (i = 0; i < ISIZE_WTH; i++)
		gz->outbuf[gz->outbuf_index++] = p_isize[i];
			
}
