
#include <yukonCore/yCore.h>

yBuffer *yBufferCreate(uint64_t elementSize, uint64_t elementCount)
{
	yBuffer *buffer = malloc(sizeof(yBuffer) + elementCount * (elementSize + sizeof(void *)));
	if (buffer == 0) {
		return 0;
	}

	pthread_mutex_init(&buffer->bufferMutex, NULL);
	pthread_cond_init(&buffer->bufferCond, NULL);

	buffer->elementSize = elementSize;
	buffer->elementCount = elementCount;

	buffer->bufferHead = 0;
	buffer->bufferTail = 0;

	buffer->bufferCount = 0;

	for (uint64_t bufferIndex = 0; bufferIndex < elementCount; ++bufferIndex) {
		buffer->bufferArray[bufferIndex] = (void *)buffer + sizeof(yBuffer) + elementCount * sizeof(void *) + bufferIndex * elementSize;
	}

	return buffer;
}

void yBufferDestroy(yBuffer *buffer)
{
	pthread_mutex_destroy(&buffer->bufferMutex);
	pthread_cond_destroy(&buffer->bufferCond);

	free(buffer);
}

void *yBufferHead(yBuffer *buffer)
{
	void *retVal = 0;

	pthread_mutex_lock(&buffer->bufferMutex);

	while (buffer->bufferCount == buffer->elementCount) {
		pthread_cond_wait(&buffer->bufferCond, &buffer->bufferMutex);
	}

	//printf("%s: bufferCount: %llu, bufferHead: %llu, bufferTail: %llu\n", __func__, buffer->bufferCount, buffer->bufferHead, buffer->bufferTail);

	retVal = buffer->bufferArray[buffer->bufferHead];

	pthread_mutex_unlock(&buffer->bufferMutex);

	return retVal;
}


void yBufferHeadAdvance(yBuffer *buffer)
{
	pthread_mutex_lock(&buffer->bufferMutex);

	buffer->bufferHead = (buffer->bufferHead + 1) % buffer->elementCount;
	++buffer->bufferCount;

	//printf("%s: bufferCount: %llu, bufferHead: %llu, bufferTail: %llu\n", __func__, buffer->bufferCount, buffer->bufferHead, buffer->bufferTail);

	pthread_mutex_unlock(&buffer->bufferMutex);

	pthread_cond_broadcast(&buffer->bufferCond);
}

void *yBufferTail(yBuffer *buffer)
{
	void *retVal = 0;

	pthread_mutex_lock(&buffer->bufferMutex);

	while (buffer->bufferCount == 0) {
		pthread_cond_wait(&buffer->bufferCond, &buffer->bufferMutex);
	}

	//printf("%s: bufferCount: %llu, bufferHead: %llu, bufferTail: %llu\n", __func__, buffer->bufferCount, buffer->bufferHead, buffer->bufferTail);

	retVal = buffer->bufferArray[buffer->bufferTail];

	pthread_mutex_unlock(&buffer->bufferMutex);

	return retVal;
}

void yBufferTailAdvance(yBuffer *buffer)
{
	pthread_mutex_lock(&buffer->bufferMutex);

	while (buffer->bufferCount == 0) {
		pthread_cond_wait(&buffer->bufferCond, &buffer->bufferMutex);
	}
	
	buffer->bufferTail = (buffer->bufferTail + 1) % buffer->elementCount;
	--buffer->bufferCount;

	//printf("%s: bufferCount: %llu, bufferHead: %llu, bufferTail: %llu\n", __func__, buffer->bufferCount, buffer->bufferHead, buffer->bufferTail);

	pthread_mutex_unlock(&buffer->bufferMutex);

	pthread_cond_broadcast(&buffer->bufferCond);
}

uint64_t yBufferStatus(yBuffer *buffer)
{
	return buffer->elementCount - buffer->bufferCount;
}

