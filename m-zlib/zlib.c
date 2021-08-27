/*The code in this file creates the zlib header as
 *  specified in the RFC 1950
 */
#include <stdint.h>
#include "zlib.h"

/*(CMF*256)+FLG must be a multiple of 31
 * FCHECK 11100 in binary
 * FDICT is not set, so is 0
 * FDLEVEL 10 or (2)
 */

#define CMF 120
#define FLG 156 
void generate_zlib_header (struct deflate_state * z)
{
	/*Write CMF*/
	z->outbuf[z->outbuf_index++] = CMF;
	/*Write FLG*/
	z->outbuf[z->outbuf_index++] = FLG;
}

/*This code is not portable as it is. i.e won't run properly on
 * big-endian machines. But should be easy to modify*/
void writeout_adler32 (struct deflate_state * a, uint32_t adler)
{
	/*Write adler32 checksum*/
	uint8_t * ptr = (uint8_t *) &adler;
	uint8_t i = sizeof(adler);
	while(i)
		a->outbuf[a->outbuf_index++] = ptr[--i];

}
