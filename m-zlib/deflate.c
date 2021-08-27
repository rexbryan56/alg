/*I wrote this program to help me understand the deflate algorithm. It does not aim to be fast,
 * nor does it do error checking. But it's accurate */


#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "deflate.h"

#define bool _Bool

#ifdef debug
static FILE * log;
#endif

/*#define MSB 1 //write from most significant bit
#define LSB 0 //write from least significant bit*/

#define MAXBITS 15 // max no of bits in code
#define MAXLITCODES 256 // no of fixed literal codes
#define MAXLENCODES 29 // max no of fixed length codes
#define MAXDCODES 30 // max no of distance codes

#define BFINAL_W 1 //bit width for header bit
#define BTYPE_W 2  //bit width for header bit
#define NO_COMPR 0
#define COMP_FIXED 1
#define END_OF_BLK 256


#define HASHTABLE_SIZE 11701 //prime far from power of 2 to reduce collision
#define SLIDING_WIN_SIZE 32768

static const uint16_t lens[29] = { /* Size base for length codes 257..285 */
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
static const uint16_t lext[29] = { /* Extra bits for length codes 257..285 */
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
static const uint16_t dists[30] = { /* Offset base for distance codes 0..29 */
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577};
static const uint16_t dext[30] = { /* Extra bits for distance codes 0..29 */
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
        12, 12, 13, 13};

#define END_OF_BLK_CODE 0
#define END_OF_BLK_LEN 7

static const struct {
    uint16_t code;
    uint8_t codelen;
}litrec[] = {
    {0X00C, 8},{0X08C, 8},{0X04C, 8},{0X0CC, 8},{0X02C, 8},{0X0AC, 8},{0X06C, 8},{0X0EC, 8},
    {0X01C, 8},{0X09C, 8},{0X05C, 8},{0X0DC, 8},{0X03C, 8},{0X0BC, 8},{0X07C, 8},{0X0FC, 8},
    {0X002, 8},{0X082, 8},{0X042, 8},{0X0C2, 8},{0X022, 8},{0X0A2, 8},{0X062, 8},{0X0E2, 8},
    {0X012, 8},{0X092, 8},{0X052, 8},{0X0D2, 8},{0X032, 8},{0X0B2, 8},{0X072, 8},{0X0F2, 8},
    {0X00A, 8},{0X08A, 8},{0X04A, 8},{0X0CA, 8},{0X02A, 8},{0X0AA, 8},{0X06A, 8},{0X0EA, 8},
    {0X01A, 8},{0X09A, 8},{0X05A, 8},{0X0DA, 8},{0X03A, 8},{0X0BA, 8},{0X07A, 8},{0X0FA, 8},
    {0X006, 8},{0X086, 8},{0X046, 8},{0X0C6, 8},{0X026, 8},{0X0A6, 8},{0X066, 8},{0X0E6, 8},
    {0X016, 8},{0X096, 8},{0X056, 8},{0X0D6, 8},{0X036, 8},{0X0B6, 8},{0X076, 8},{0X0F6, 8},
    {0X00E, 8},{0X08E, 8},{0X04E, 8},{0X0CE, 8},{0X02E, 8},{0X0AE, 8},{0X06E, 8},{0X0EE, 8},
    {0X01E, 8},{0X09E, 8},{0X05E, 8},{0X0DE, 8},{0X03E, 8},{0X0BE, 8},{0X07E, 8},{0X0FE, 8},
    {0X001, 8},{0X081, 8},{0X041, 8},{0X0C1, 8},{0X021, 8},{0X0A1, 8},{0X061, 8},{0X0E1, 8},
    {0X011, 8},{0X091, 8},{0X051, 8},{0X0D1, 8},{0X031, 8},{0X0B1, 8},{0X071, 8},{0X0F1, 8},
    {0X009, 8},{0X089, 8},{0X049, 8},{0X0C9, 8},{0X029, 8},{0X0A9, 8},{0X069, 8},{0X0E9, 8},
    {0X019, 8},{0X099, 8},{0X059, 8},{0X0D9, 8},{0X039, 8},{0X0B9, 8},{0X079, 8},{0X0F9, 8},
    {0X005, 8},{0X085, 8},{0X045, 8},{0X0C5, 8},{0X025, 8},{0X0A5, 8},{0X065, 8},{0X0E5, 8},
    {0X015, 8},{0X095, 8},{0X055, 8},{0X0D5, 8},{0X035, 8},{0X0B5, 8},{0X075, 8},{0X0F5, 8},
    {0X00D, 8},{0X08D, 8},{0X04D, 8},{0X0CD, 8},{0X02D, 8},{0X0AD, 8},{0X06D, 8},{0X0ED, 8},
    {0X01D, 8},{0X09D, 8},{0X05D, 8},{0X0DD, 8},{0X03D, 8},{0X0BD, 8},{0X07D, 8},{0X0FD, 8},
    {0X013, 9},{0X113, 9},{0X093, 9},{0X193, 9},{0X053, 9},{0X153, 9},{0X0D3, 9},{0X1D3, 9},
    {0X033, 9},{0X133, 9},{0X0B3, 9},{0X1B3, 9},{0X073, 9},{0X173, 9},{0X0F3, 9},{0X1F3, 9},
    {0X00B, 9},{0X10B, 9},{0X08B, 9},{0X18B, 9},{0X04B, 9},{0X14B, 9},{0X0CB, 9},{0X1CB, 9},
    {0X02B, 9},{0X12B, 9},{0X0AB, 9},{0X1AB, 9},{0X06B, 9},{0X16B, 9},{0X0EB, 9},{0X1EB, 9},
    {0X01B, 9},{0X11B, 9},{0X09B, 9},{0X19B, 9},{0X05B, 9},{0X15B, 9},{0X0DB, 9},{0X1DB, 9},
    {0X03B, 9},{0X13B, 9},{0X0BB, 9},{0X1BB, 9},{0X07B, 9},{0X17B, 9},{0X0FB, 9},{0X1FB, 9},
    {0X007, 9},{0X107, 9},{0X087, 9},{0X187, 9},{0X047, 9},{0X147, 9},{0X0C7, 9},{0X1C7, 9},
    {0X027, 9},{0X127, 9},{0X0A7, 9},{0X1A7, 9},{0X067, 9},{0X167, 9},{0X0E7, 9},{0X1E7, 9},
    {0X017, 9},{0X117, 9},{0X097, 9},{0X197, 9},{0X057, 9},{0X157, 9},{0X0D7, 9},{0X1D7, 9},
    {0X037, 9},{0X137, 9},{0X0B7, 9},{0X1B7, 9},{0X077, 9},{0X177, 9},{0X0F7, 9},{0X1F7, 9},
    {0X00F, 9},{0X10F, 9},{0X08F, 9},{0X18F, 9},{0X04F, 9},{0X14F, 9},{0X0CF, 9},{0X1CF, 9},
    {0X02F, 9},{0X12F, 9},{0X0AF, 9},{0X1AF, 9},{0X06F, 9},{0X16F, 9},{0X0EF, 9},{0X1EF, 9},
    {0X01F, 9},{0X11F, 9},{0X09F, 9},{0X19F, 9},{0X05F, 9},{0X15F, 9},{0X0DF, 9},{0X1DF, 9},
    {0X03F, 9},{0X13F, 9},{0X0BF, 9},{0X1BF, 9},{0X07F, 9},{0X17F, 9},{0X0FF, 9},{0X1FF, 9}
};

struct code_s
{
    uint16_t code;
    uint8_t codelen;
    uint8_t extrabit;
    uint16_t min;
    uint16_t max;
};

static const struct code_s lenrec[] = {
    {0X040, 7, 0, 3, 3},
    {0X020, 7, 0, 4, 4},
    {0X060, 7, 0, 5, 5},
    {0X010, 7, 0, 6, 6},
    {0X050, 7, 0, 7, 7},
    {0X030, 7, 0, 8, 8},
    {0X070, 7, 0, 9, 9},
    {0X008, 7, 0, 10, 10},
    {0X048, 7, 1, 11, 12},
    {0X028, 7, 1, 13, 14},
    {0X068, 7, 1, 15, 16},
    {0X018, 7, 1, 17, 18},
    {0X058, 7, 2, 19, 22},
    {0X038, 7, 2, 23, 26},
    {0X078, 7, 2, 27, 30},
    {0X004, 7, 2, 31, 34},
    {0X044, 7, 3, 35, 42},
    {0X024, 7, 3, 43, 50},
    {0X064, 7, 3, 51, 58},
    {0X014, 7, 3, 59, 66},
    {0X054, 7, 4, 67, 82},
    {0X034, 7, 4, 83, 98},
    {0X074, 7, 4, 99, 114},
    {0X003, 8, 4, 115, 130},
    {0X083, 8, 5, 131, 162},
    {0X043, 8, 5, 163, 194},
    {0X0C3, 8, 5, 195, 226},
    {0X023, 8, 5, 227, 257},
    {0X0A3, 8, 0, 258, 258}
};

static const struct code_s distrec[] = {
    {0X000, 5, 0, 1, 1},
    {0X010, 5, 0, 2, 2},
    {0X008, 5, 0, 3, 3},
    {0X018, 5, 0, 4, 4},
    {0X004, 5, 1, 5, 6},
    {0X014, 5, 1, 7, 8},
    {0X00C, 5, 2, 9, 12},
    {0X01C, 5, 2, 13, 16},
    {0X002, 5, 3, 17, 24},
    {0X012, 5, 3, 25, 32},
    {0X00A, 5, 4, 33, 48},
    {0X01A, 5, 4, 49, 64},
    {0X006, 5, 5, 65, 96},
    {0X016, 5, 5, 97, 128},
    {0X00E, 5, 6, 129, 192},
    {0X01E, 5, 6, 193, 256},
    {0X001, 5, 7, 257, 384},
    {0X011, 5, 7, 385, 512},
    {0X009, 5, 8, 513, 768},
    {0X019, 5, 8, 769, 1024},
    {0X005, 5, 9, 1025, 1536},
    {0X015, 5, 9, 1537, 2048},
    {0X00D, 5, 10, 2049, 3072},
    {0X01D, 5, 10, 3073, 4096},
    {0X003, 5, 11, 4097, 6144},
    {0X013, 5, 11, 6145, 8192},
    {0X00B, 5, 12, 8193, 12288},
    {0X01B, 5, 12, 12289, 16384},
    {0X007, 5, 13, 16385, 24576},
    {0X017, 5, 13, 24577, 32768}
};

struct h_node
{
    uint32_t index;
    uint32_t key;
    struct h_node * next;
};

struct hash_table
{
    uint64_t nlen; //no of nodes in chained list
    struct h_node * top;
};

struct lzXX
{
    uint16_t len;
    uint32_t dist;
};


unsigned int reverseBits(unsigned int num, int n_bits) 
{ 
	//unsigned int NO_OF_BITS = sizeof(num) * 8; 
	unsigned int reverse_num = 0, i, temp; 

	for (i = 0; i < n_bits; i++) 
	{ 
		temp = (num & (1 << i)); 
		if(temp) 
			reverse_num |= (1 << ((n_bits - 1) - i)); 
	} 

	return reverse_num; 
} 


void rr (void)
{
    int i = 0;
    for (i = 0; i < MAXLITCODES; i++)
    {
        fprintf(log, "{0X%03X, %i},", reverseBits(litrec[i].code, litrec[i].codelen),
                     litrec[i].codelen);
        if ((i + 1) % 8 == 0)
        fprintf(log,"\n");
    }
    
    fprintf(log,"\n\n");
    
    for (i = 0; i < MAXLENCODES; i++)
        fprintf(log, "{0X%03X, %i, %i, %i, %i},\n", reverseBits(lenrec[i].code, lenrec[i].codelen),
            lenrec[i].codelen, lenrec[i].extrabit, lenrec[i].min, lenrec[i].max);
    
    fprintf(log,"\n\n");
    
    for (i = 0; i < MAXDCODES; i++)
    {
        fprintf(log, "{0X%03X, %i, %i, %i, %i},\n",reverseBits(distrec[i].code, distrec[i].codelen),
            distrec[i].codelen, distrec[i].extrabit, distrec[i].min, distrec[i].max);
    }
    fprintf(log,"\n\n");

}

/*if MSB = true store starting with most significant bit first else store from least significant bit
*I used 16bits to store the counter, so we've only got 65535 before trouble
*also you can only store 16 bits at a time*/
static void writebit (uint64_t num, uint8_t nbits, struct deflate_state * w)
{
#ifdef debug
fprintf(log,"write %i in bits of length %i\n", num, nbits);
#endif
    //assert(nbits != 0);
/*
    do {
        w->bitwrtn = 8 - w->bitcnt;
        w->outbuf[w->outbuf_index] |= (num << w->bitcnt);
        if (nbits < w->bitwrtn)
        {
            w->bitcnt += nbits;
            return;
        }
        else if (nbits == w->bitwrtn)
        {
            w->outbuf_index++;
            w->bitcnt = 0;
            return;
        }
        
        w->outbuf_index++;
        nbits -= w->bitwrtn;
        num >>= w->bitwrtn;
        w->outbuf[w->outbuf_index] |= num;
        if (nbits < 8)
        {
            w->bitcnt = nbits;
            return;
        }
        else
        {
            w->bitcnt = 0;
            nbits -= 8;
            w->outbuf_index++;
        }
        
    } while (nbits);
 */
    uint16_t i = 0;
    while (i != nbits)
    {
		if (((1 << i) & num))
			w->outbuf[w->outbuf_index] |= (1 << w->bitwrtn);

		w->bitwrtn++;
		i++;
	
		if (8 == w->bitwrtn)
		{
			w->outbuf_index++;
			w->bitwrtn = 0;
		}
		
	}      
}

static int encode_no_compr (struct deflate_state * w)
{
#define MAX_BLCK_SIZE 65535
    size_t i = 0,j = 0;
    while (i < w->inbuf_len)
    {
        bool lstblk = ((i+MAX_BLCK_SIZE) < w->inbuf_len) ? 0 : w->lastblk;
        uint32_t blcksize = (w->inbuf_len <= MAX_BLCK_SIZE) ? w->inbuf_len : 
                    ((i + MAX_BLCK_SIZE) < w->inbuf_len) ? MAX_BLCK_SIZE :
                    (w->inbuf_len - i);
                    
            writebit(lstblk, BFINAL_W, w);
            writebit(NO_COMPR, BTYPE_W, w);
            
            w->outbuf_index++;
            w->bitcnt = 0;
            writebit((uint16_t)blcksize, 16, w);
            writebit((uint16_t)~blcksize, 16, w);
        do {    
            w->outbuf[w->outbuf_index++] = w->inbuf[w->inbuf_index++];
            i++; j++;
        
        } while (i < w->inbuf_len && j < blcksize);
        j = 0;
    }
#undef MAX_BLCK_SIZE
    return 0;
}

#define h_f(key) ((key) % HASHTABLE_SIZE)

static void insert_node (struct hash_table * Table, uint32_t key, uint64_t cur_index)
{
    uint32_t h_index = h_f (key);

    struct h_node * temp = malloc(sizeof(struct h_node));
    if (!temp)
    {
        fprintf(stderr,"could not allocate memory");
        exit(EXIT_FAILURE);
    }
    temp->index = cur_index;
    temp->key = key;
    temp->next = NULL;

    if (!Table[h_index].top)
        Table[h_index].top = temp;
    else
    {
        struct h_node * hold = Table[h_index].top; //we don't need this variable 'hold'
        temp->next = hold;
        Table[h_index].top = temp;
    }
}

/*Return -7 if key was not found in the hash table*/
static int lzXX (struct hash_table * Table, struct deflate_state * w, struct lzXX * ret)
{
#define BASE 256
    int pos = w->inbuf_index;
    int last_pos = pos - 1;
    uint32_t key = (w->inbuf[pos] << 16) | (w->inbuf[pos+1] << 8) | (w->inbuf[pos+2]);
    uint32_t h_index = h_f(key);

#ifdef debug
    fprintf(log,"current index is %i\n",pos);
    fprintf(log,"key %i generated from %i %i %i\n", key, w->inbuf[pos], w->inbuf[pos+1],
            w->inbuf[pos+2]);
#endif

    if (!Table[h_index].top)
    {
#ifdef debug
        fprintf (log,"key not found in table : inserting key\n");
#endif
               insert_node (Table, key, pos);
        return -7;
    }
    else
    {
        struct h_node * temp = Table[h_index].top;
            while (temp)
            {
            if (key == temp->key)     // search for key
                break;
            temp = temp->next;
            }

        if ((!temp) || (((last_pos - temp->index)+2) > SLIDING_WIN_SIZE)) // check this code
        {
#ifdef debug
            fprintf(log,"%s\n",temp?"match not found in sliding window":"key not"  
            "found in hash chain");
#endif
            insert_node (Table, key, pos);
            return -7;
        }

        size_t i = 0;
        while ((w->inbuf[i+pos] == w->inbuf[i+temp->index]) && ((pos+i) < w->inbuf_len)) //search for longest match : also rewrite code to remove possibilities of hidden bugs
        {
            i++;
        }
#ifdef debug
        fprintf(log,"length of match %li\n",i);
#endif
        if (i < 3 || i > 258)
            return -7;
        ret->len = i; //+ 1 until i understand what is going on here;
        ret->dist = (last_pos - temp->index) + 1;
#ifdef debug
        fprintf(log, "match distance %i\n",ret->dist);
#endif
    }
    return 0;

#undef BASE
}

static void free_hash_chain (struct hash_table * Table)
{
    for (size_t i = 0;i < HASHTABLE_SIZE;i++)
        while (Table[i].top)
        {
            struct h_node * temp = Table[i].top;
            Table[i].top = Table[i].top->next;
            free(temp);
        }
}

static int encode_fixed_huff (struct deflate_state * w)
{
    struct hash_table Table[HASHTABLE_SIZE] = {0};
    struct lzXX range = {0};
    
    writebit(w->lastblk, BFINAL_W, w); //write block header
    writebit( COMP_FIXED, BTYPE_W, w); //fixed huff

    do
    {

#ifdef debug    
        fprintf (log,"%li lit %i code %i length %i \n",w->inbuf_index,w->inbuf[w->inbuf_index]
        , litrec[w->inbuf[w->inbuf_index]].code,litrec[w->inbuf[w->inbuf_index]].codelen);
#endif
        if ((w->inbuf_len - (w->inbuf_index+1)) < 3 || lzXX(Table, w, &range) == -7)
        {    writebit(litrec[w->inbuf[w->inbuf_index]].code, 
                    litrec[w->inbuf[w->inbuf_index]].codelen, w);
            w->inbuf_index++;
        }
        else
        {
            uint16_t i = 0, ext = 0;
            while (i < MAXLENCODES)
            {
                if (range.len >= lenrec[i].min && range.len <= lenrec[i].max)
                {
                    writebit(lenrec[i].code, lenrec[i].codelen, w);
                    if(lenrec[i].extrabit)
                    {
                        ext = range.len - lenrec[i].min;
                        writebit(ext, lenrec[i].extrabit, w);
                    }
                    break;
                }
                i++;
            }

            for (i = 0; i < MAXDCODES; i++)
            {
                if (range.dist >= distrec[i].min && range.dist <= distrec[i].max)
                {
                    writebit(distrec[i].code, distrec[i].codelen, w);
                    if (distrec[i].extrabit)
                    {
                        ext = range.dist - distrec[i].min;
                        writebit(ext, distrec[i].extrabit, w);
                    }
                    break;
                }
            }

            w->inbuf_index += range.len; //advance index
        }
    }
    while (w->inbuf_index < w->inbuf_len);
    
    writebit(END_OF_BLK_CODE, END_OF_BLK_LEN, w);
    free_hash_chain(Table);    
    
    return 0;
    
}

int deflate (struct deflate_state * w)
{
#ifdef debug
    if ((log = fopen("test/log","a")) == NULL)
    {
        fprintf(stderr,"could not access file");
        exit(1);
    }        
#endif
    rr();
    int ret = 0;
    
    if (w->bitcnt != 0)
        w->outbuf[w->outbuf_index] = w->holdbuf;
    if(w->nocomp)
        ret = encode_no_compr(w);
    else
    {
        ret = encode_fixed_huff(w);
        if (w->bitcnt != 0)
            w->holdbuf = w->outbuf[w->outbuf_index];
        if (w->lastblk )// && w->bitcnt != 0)
            w->outbuf_index++;
    }
#ifdef debug
    fclose(log);
#endif
    return ret;
}
