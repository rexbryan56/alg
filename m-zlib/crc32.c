#include <stdint.h>
#include "crc32.h"

/*This code was taken of rfc 1952*/

/*Make the table for fast crc
* The table is computed only once
*/

uint64_t crc_table[256];
uint8_t crc_table_computed = 0;

void make_crc_table(void)
{
	uint64_t c;
	uint32_t n, k;

	for (n = 0; n < 256; n++)
	{
		c = (uint64_t) n;
		for (k = 0; k < 8; k++)
		{
			if (c & 1)
			{
				c = 0xedb88320L ^ (c >> 1);
			}
			else
			{
				c = c >> 1;
			}
		}
		crc_table[n] = c;
	}
	crc_table_computed = 1;
}

/*crc should be intialised to 0*/
uint64_t crc32(uint64_t crc, uint8_t *buf, uint64_t len)
{
	uint64_t c = crc ^ 0xffffffffL;
	uint64_t n;

	if (!crc_table_computed)
		make_crc_table();
	for (n = 0;n < len; n++)
	{
		c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
	}
	return c ^ 0xffffffffL;
}
