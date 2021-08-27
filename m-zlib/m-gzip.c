#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "deflate.h"
#include "crc32.h"
#include "gzlib.h"

#define nextfile() do {\
			argc--;\
			argv++;\
			/*continue;*/\
		   } while (0);\


/*returns negative no if file does not exist or is not a regular file i.e a directory*/
int checkargs (char * arg, struct filedata * FileInfo)
{
	struct stat sbuf;
	int ret;

	ret = stat(arg, &sbuf); // so confused right here
	if (ret)
	{	
		fprintf(stderr, "'%s' does not exist. --ignored\n", arg);
		return -1;
	}
		
	if (!S_ISREG(sbuf.st_mode))
	{
		if (S_ISDIR(sbuf.st_mode))
			fprintf(stderr, "'%s' is a directory! --ignored\n", arg);
		else if (S_ISLNK(sbuf.st_mode))
			fprintf(stderr,"'%s' is a symbolic link! --ignored\n", arg);
		else
			fprintf(stderr,"'%s' is not a regular file. --ignored\n", arg);
		
		return -2;	
	}	
		
		FileInfo->fsize = sbuf.st_size;
		FileInfo->mtime = sbuf.st_mtime;
	return 0;
}

void usage (void)
{
	fprintf(stderr, "Usage: m-gzip [FILE]\n\t compress FILEs\n");
}
 
int main (int argc, char **argv)
{
	if (argc < 2 || argc > 2)
	{
		usage();
		return 1;
	}
			
	uint8_t inbuf[INBUF_LEN] = {0};
	uint8_t *outbuf = calloc(OUTBUF_LEN, sizeof(uint8_t));
	

	char *ret;
	struct stat fbuf;
	struct filedata fdata = {0};
	FILE *outfile = NULL, *infile = NULL;
	char outname[FILENAME_MAX] = {0};
	argv++;
	

	if (checkargs(*argv, &fdata))
	{
		//nextfile();
		return 1;
	}
		
	if ((ret = strrchr(*argv, '/')))
	{
		strcpy(outname, ++ret);
		strcat(outname, GZIP_EXT);
		fdata.filename = ret;
	}
	else
	{
		strcpy(outname, *argv);
		strcat(outname, GZIP_EXT);
		fdata.filename = *argv;
	}	

	if (stat(outname, &fbuf) != -1)
	{
		fprintf(stderr, "The file %s exists: would you like to overwrite? "
			"'y' for yes and 'n' for no > ", outname);
		if (getchar() != 'y')
		{
			fprintf(stderr, "file not overwritten\n --skipping\n");
			//nextfile();
			return 1;
		}
	}
	

	fprintf(stderr, "output file name is %s \n",outname);
	if ((infile = fopen(*argv, "rb")) == NULL)
	{
		fprintf(stderr, "Could not access %s \n", *argv);
		return 1;
	}
	if ((outfile = fopen(outname, "wb")) == NULL)
	{
		fprintf(stderr, "Could not write to disk\n");
		return 1;
	}
		
	uint64_t crc = 0;
		
	struct deflate_state gz = {
	.inbuf = inbuf,
	.outbuf = outbuf,
	.outbuf_len = OUTBUF_LEN,
	.nocomp = false
	};
	fprintf(stderr, "mtime %0x\n",fdata.mtime);
	fprintf(stderr, "fname %s\n",fdata.filename);
	write_gzip_member_header(&gz, &fdata);
		
	do {
		int i = 0;
		while ((gz.state = fread(&gz.inbuf[i], sizeof(uint8_t), 1, infile)) == 1)
		{
			i++;
			if (i == INBUF_LEN)
				break;
		}
		gz.inbuf_len = i;
		gz.lastblk = (gz.state == 1) ? false : true;
		deflate(&gz);
		crc = crc32(crc, gz.inbuf, gz.inbuf_len);
		//puts("llls1");
		
		if (gz.state != 1)
		{
			fprintf(stderr, "crc32 is %0lx\n",crc);
			write_gzip_trailer(&gz, crc, fdata.fsize);
			puts("llls3");
		}
		
		fwrite(gz.outbuf, sizeof(uint8_t), gz.outbuf_index, outfile);
		gz.inbuf_index = 0;
		gz.outbuf_index = 0;
		memset(gz.outbuf, 0, gz.outbuf_len);
	} while (gz.state == 1);
		
	fclose(infile);
	fclose(outfile);
	free(outbuf);
}
