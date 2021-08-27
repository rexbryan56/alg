#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "deflate.h"
#include "zlib.h"
#include "adler32.h"



void displaybits (unsigned int value, unsigned int n_bits)
{
	unsigned int displaymask = 1 << (n_bits-1);

	printf ("%02x = ",value);

	for (unsigned int c = 1; c <= n_bits; c++)
	{
		putchar(value & displaymask ? '1' : '0');
		value <<= 1;

		if (c % 8 == 0)
			putchar(' ');
	}
	putchar(' ');
}


int main (int argc, char **argv)
{
	if (argc < 2 || argc > 2)
	{	
		fprintf(stderr,"invalid operand\n");
		return 1;
	}

	int state;
	FILE * infile = NULL;
	if (!(infile = fopen(argv[1],"rb")))
	{
		fprintf(stderr,"could not open file : %s\n", argv[1]);
                return 1;
	}
	
	FILE * outfile = NULL;
	if (!(outfile = fopen ("test/test.bin","wb")))
	{
		fprintf(stderr,"could not open output file test.bin\n");
		return 1;
	}
	
	uint8_t * src = calloc(INBUF_LEN, sizeof(uint8_t));
	uint8_t * outbuf = calloc(OUTBUF_LEN, sizeof(uint8_t));
	if (!src || !outbuf)
        {
                fprintf(stderr,"could not allocate memory\n");
                return 1;
	}
	
	struct deflate_state d = {
	.inbuf = src,
	.outbuf = outbuf,//((uint8_t [OUTBUF_LEN]) {0}),
	.outbuf_len = OUTBUF_LEN,
	.nocomp = false //whether to use compression
	};
	
	uint64_t adler = 0;
	generate_zlib_header(&d);

	do {
		size_t q = 0;
		while ((state = fread(&src[q], sizeof(src[q]), 1, infile)) == 1)
		{	
			q++;
			if (q == INBUF_LEN)
				break;		
		}

		d.inbuf_len = q;
		d.lastblk = (state == 1 ) ? false : true;
		deflate(&d);
		adler = fast_adler32(adler, d.inbuf, q);
		//d.outbuf_index = (state == 1) ? d.outbuf_index : d.outbuf_index+1;
		
		if (state != 1)
		{
			writeout_adler32 (&d, adler);
			fprintf(stderr, "adler32 = %lu \n",adler);
		}
		fwrite(d.outbuf, sizeof(uint8_t), d.outbuf_index, outfile);
		d.inbuf_index = 0;
		d.outbuf_index = 0;
		memset(d.outbuf, 0, d.outbuf_len);
	} while(state == 1);
	
	fclose(infile);
	fclose(outfile);
	free(src);
	free(outbuf);
}


