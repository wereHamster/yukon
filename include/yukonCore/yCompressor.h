
#ifndef __YUKON_COMPRESSOR_H__
#define __YUKON_COMPRESSOR_H__

#include <yukonCore/yCore.h>

extern void yCompressorInit();

typedef struct DecodeTable {
	uint8_t shift[256];
	unsigned char* table_pointers[32];
	unsigned char table_data[129*25];
} DecodeTable;

extern uint32_t huffEncodeTable[256];
extern DecodeTable huffDecodeTable;

extern uint32_t *huffCompress(uint32_t *dst, uint8_t *src, uint8_t *end, uint32_t *tbl);
extern uint8_t *huffDecompress(uint8_t *dst, uint32_t *src, uint8_t *end, DecodeTable *tbl);

extern uint32_t *chuffCompress(uint32_t *dst, uint8_t *src, uint8_t *end, uint32_t *tbl);

#endif /* __YUKON_COMPRESSOR_H__ */
