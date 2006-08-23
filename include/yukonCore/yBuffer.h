
#ifndef __YUKON_CORE_BUFFER_H__
#define __YUKON_CORE_BUFFER_H__

#include <yukonCore/yCore.h>

typedef struct _yBuffer yBuffer;

struct _yBuffer
{
	pthread_mutex_t bufferMutex;
	pthread_cond_t bufferCond;

	uint64_t elementSize;
	uint64_t elementCount;

	uint64_t bufferHead;
	uint64_t bufferTail;

	uint64_t bufferCount;

	void *bufferArray[0];
};

yBuffer *yBufferCreate(uint64_t elementSize, uint64_t elementCount);
void yBufferDestroy(yBuffer *buffer);

void *yBufferHead(yBuffer *buffer);
void yBufferHeadAdvance(yBuffer *buffer);
void *yBufferTail(yBuffer *buffer);
void yBufferTailAdvance(yBuffer *buffer);

uint64_t yBufferStatus(yBuffer *buffer);


#endif /* __YUKON_CORE_BUFFER_H__ */
