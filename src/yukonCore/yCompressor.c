
#include <yukonCore/yCore.h>

uint32_t huffEncodeTable[256];
DecodeTable huffDecodeTable;

static void p32(uint32_t val)
{
	for (int i=31; i>=0; --i) {
		printf("%s", (val & (1 << i)) ? "1" : "0");
	}
	printf("\n");
}

static const unsigned char grad_yuv[] = {
  34,35,36,37,38,103,168,9,8,10,11,11,10,12,8,237,14,9,15,11,
  16,8,241,242,19,8,212,85,54,23,10,54,119,54,55,54,119,182,55,54,
  53,54,117,244,51,52,147,178,177,16,8,15,9,14,10,13,9,12,9,11,
  9,10,11,9,8,168,103,38,37,36,34,

  66,36,38,40,42,44,45,46,80,81,82,115,148,21,10,54,151,58,56,90,
  57,26,162,57,90,56,57,58,56,58,57,58,57,152,55,54,55,86,117,52,
  53,116,53,52,53,148,147,82,49,48,79,45,44,41,39,37,35,34,

  66,36,38,40,41,42,76,45,78,47,112,113,82,115,52,53,55,86,55,54,
  55,54,23,193,54,53,54,53,52,53,51,84,51,50,51,82,49,112,47,78,
  45,44,43,42,41,40,37,35,34, 0
};

#define max(a,b) ( a > b ? a : b);

static const unsigned char* DecompressHuffmanTable(const unsigned char* hufftable, unsigned char* dst) {
  int i=0;
  do {
    int val = *hufftable & 31; // lower 5 bits
    int repeat = *hufftable++ >> 5; // upper 3 bits
    if (!repeat)
      repeat = *hufftable++;
      
    while (repeat--) {
      dst[i++] = val;
    }
  } while (i<256);
  
  return hufftable;
}

static const unsigned char* InitializeShiftAddTables(const unsigned char* hufftable, unsigned char* shift, unsigned* add_shifted, uint32_t *tbl) {
//printf("\n");
  hufftable = DecompressHuffmanTable(hufftable, shift);
  int min_already_processed = 32;
  uint32_t bits = 0;
  do {
    int max_not_processed=0;
    for (int i=0; i<256; ++i) {
      if (shift[i] < min_already_processed && shift[i] > max_not_processed)
        max_not_processed = shift[i];
    }
    int bit = 1<<(32-max_not_processed);
    assert(!(bits & (bit-1)));
    for (int j=0; j<256; ++j) {
      if (shift[j] == max_not_processed) {
        add_shifted[j] = bits;
        bits += bit;
      }
    }
    min_already_processed = max_not_processed;
  } while (bits&0xFFFFFFFF);
  
  for (int j=0; j<256; ++j) {
      int bad = add_shifted[j] & shift[j];
      if (bad) {
          printf("index %d is bad:\n", j);
          p32(bad);
          p32(add_shifted[j]); p32(shift[j]);
      }
      tbl[j] = add_shifted[j] + (shift[j] & 0xff);
      
      uint8_t s = tbl[j];
      uint32_t ui32 = tbl[j];
      uint8_t c = ui32;
   //   assert(s == shift[j]);
   //   assert(c == s);
   //   assert(tbl[0][j] == s);
   //   assert(encode1_add_shifted[j] == (tbl[j] & ~31));
      //p32(tbl[0][0], encode1_shift[0]);
    }
  return hufftable;
}

static const unsigned char* InitializeEncodeTables(const unsigned char* hufftable) {
	uint8_t shift[256];
	uint32_t add[256];
	hufftable = InitializeShiftAddTables(hufftable, shift, add, huffEncodeTable);
//	hufftable = InitializeShiftAddTables(hufftable, shift, add, huffEncodeTable);
//	hufftable = InitializeShiftAddTables(hufftable, shift, add, huffEncodeTable);
	return hufftable;
}


const unsigned char* InitializeDecodeTable(const unsigned char* hufftable, DecodeTable* decode_table) {
  unsigned add_shifted[256];
  unsigned empty[256];
  uint8_t *shift = decode_table->shift;
  hufftable = InitializeShiftAddTables(hufftable, shift, add_shifted, empty);
  char code_lengths[256];
  char code_firstbits[256];
  char table_lengths[32];
  memset(table_lengths,-1,32);
  int all_zero_code=-1;
  for (int i=0; i<256; ++i) {
    if (add_shifted[i]) {
      for (int firstbit=31; firstbit>=0; firstbit--) {
        if (add_shifted[i] & (1<<firstbit)) {
          code_firstbits[i] = firstbit;
          int length = shift[i] - (32-firstbit);
          code_lengths[i] = length;
          table_lengths[firstbit] = max(table_lengths[firstbit], length);
          break;
        }
      }
    } else {
      all_zero_code = i;
    }
  }
  unsigned char* p = decode_table->table_data;
  *p++ = 31;
  *p++ = all_zero_code;
  for (int j=0; j<32; ++j) {
    if (table_lengths[j] == -1) {
      decode_table->table_pointers[j] = decode_table->table_data;
    } else {
      decode_table->table_pointers[j] = p;
      *p++ = j-table_lengths[j];
      p += 1<<table_lengths[j];
    }
  }
  for (int k=0; k<256; ++k) {
    if (add_shifted[k]) {
      int firstbit = code_firstbits[k];
      int val = add_shifted[k] - (1<<firstbit); // clear firstbit
      unsigned char* table = decode_table->table_pointers[firstbit];
      memset(&table[1+(val>>table[0])],
        k, 1<<(table_lengths[firstbit]-code_lengths[k]));
    }
  }
  return hufftable;
}

const unsigned char* InitializeDecodeTables(const unsigned char* hufftable) {
  hufftable = InitializeDecodeTable(hufftable, &huffDecodeTable);
//  hufftable = InitializeDecodeTable(hufftable, &huffDecodeTable);
//  hufftable = InitializeDecodeTable(hufftable, &huffDecodeTable);
  return hufftable;
}

static void test()
{
	static uint8_t src[128];
	static uint8_t cpy[128];
	strcpy(src, "asdfasdfasdfasdfasdfasdf");
	cpy[0] = src[0];
	for (int j = 1; j < 128; ++j) {
		cpy[j] = src[j] - src[j-1];
	}
	
	uint32_t dst[1024];
	uint32_t *d = dst;
	uint32_t *end = huffCompress(dst, cpy, cpy+128, huffEncodeTable);
	
	printf("%012d\n", end - d);
	p32(ntohl(dst[0]));
	p32(ntohl(dst[1]));
	p32(ntohl(dst[2]));
	p32(ntohl(dst[3]));
	uint32_t *cend = chuffCompress(dst, cpy, cpy+128, huffEncodeTable);
	printf("%012d\n", cend - d);
	p32(ntohl(dst[0]));
	p32(ntohl(dst[1]));
	p32(ntohl(dst[2]));
	p32(ntohl(dst[3]));
	
	printf("%012d\n", cend - d);
	uint8_t data[1024];
	uint8_t *dend = huffDecompress(data, dst, data+128, &huffDecodeTable);
	for (int j = 1; j < 128; ++j) {
		data[j] = data[j] + data[j-1];
	}
	printf("orig: %s\n", src);
	printf("data: %s\n", data);
}

void yCompressorInit()
{
	InitializeEncodeTables(grad_yuv);
	InitializeDecodeTables(grad_yuv);
	
	//test();
	//exit(0);
}

uint32_t *chuffCompress(uint32_t *dst, uint8_t *src, uint8_t *end, uint32_t *tbl)
{
	int bits = 32;
	int epos = 0;
	uint64_t qword = 0;
	while (src < end) {
		uint8_t sym = *src++;
		uint32_t code = tbl[sym];
		int len = code & 0xff;
		qword &= 0xffffffff00000000;
		qword |= code;
		if (bits - len <= 0) {
			qword <<= bits;
			dst[epos++] = htonl(qword >> 32);
			len -= bits;
			bits = 32;
		}
		//printf("%d/%d\n", bits, len);
		qword <<= len;
		bits -= len;
		//_p32(byte);
	}
	
	if (bits > 0) {
		qword <<= bits;
		dst[epos++] = htonl(qword >> 32);
	}
	
	//printf("%012d - %012d\n", epos*4, size);
	return dst + epos;
}

#ifndef __x86_64__
uint8_t *huffDecompress(uint8_t *dst, uint32_t *src, uint8_t *end, DecodeTable *tbl)
{
	int bpos=0;
	int opos = 0;
	uint32_t *src32 = src;
	for (;;) {
	    int vpos = bpos % 32;
		uint32_t val = ntohl(src32[bpos/32]);
		uint32_t val2 = ntohl(src32[bpos/32+1]);
		if (vpos) {
			val <<= vpos;
			val += val2 >> (32-vpos);
		}
		
		int firstbit = 0;
		val |= 1;
		for (int i=31; i >= 0; --i) {
			if (val & (1 << i)) {
				firstbit=i;
				break;
			}
		}
		//printf("msb: %d\n", firstbit);
		
		val -= (1 << firstbit);
		
		char *table = tbl->table_pointers[firstbit];
		//printf("length: %d\n", 32-table[0]);
		
		val >>= table[0];
		//_p32(val);
		
		uint8_t sym = table[1+val];
		//printf("symbol: 0x%02x\n", sym);
		dst[opos++] = sym;
		bpos += tbl->shift[sym];
		if (dst+opos >= end) {
			break;
		}
	}
}
#endif
