CC = gcc
CFLAGS = -Wall -pedantic -D_FORTIFY_SOURCE=2 -Ddebug# -g #-Dtest
SOURCE = deflate.c main.c zlib.c adler.c
ZOBJECTS = deflate.o zlib.o adler32.o
GZOBJ = deflate.o crc32.o gzlib.o
.PHONY:all test clean

all:clean m-zlib m-gzip

deflate.o: deflate.c deflate.h
	$(CC) $(CFLAGS) -c deflate.c

adler32.o: adler32.c adler32.h
	$(CC) $(CFLAGS) -c adler32.c

crc32.o: crc32.c crc32.h
	$(CC) $(CFLAGS) -c crc32.c

zlib.o: zlib.c zlib.h deflate.h
	$(CC) $(CFLAGS) -c zlib.c

gzlib.o: gzlib.c gzlib.h
	$(CC) $(CFLAGS) -c gzlib.c

m-zlib: $(ZOBJECTS) main.c
	$(CC) $(CFLAGS) main.c $(ZOBJECTS) -o $@

m-gzip: $(GZOBJ) m-gzip.c
	$(CC) $(CFLAGS) m-gzip.c $(GZOBJ) -o $@

test: m-zlib
	./m-zlib test/result  && ./puff/puff -2 -w test/test.bin
	
clean: 
	$(RM) $(ZOBJECTS) $(GZOBJ) m-gzip m-zlib test/de.txt test/test.bin
