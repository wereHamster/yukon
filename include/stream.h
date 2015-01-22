
#ifndef __YUKON_STREAM__
#define __YUKON_STREAM__

#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <seom/stream.h>

struct yukonBuffer {
	pthread_mutex_t mutex;
	pthread_cond_t cond;

	unsigned long size, head, tail;
	struct seomPacket *array[0];
};

struct yukonStream {
	int fileDescriptor;
	seomStream *stream;
	pthread_t multiplexerThread;
	struct yukonBuffer *buffer;
};

struct yukonBuffer *yukonBufferCreate(unsigned long size);
void yukonBufferPut(struct yukonBuffer *buffer, struct seomPacket *packet);
struct seomPacket *yukonBufferGet(struct yukonBuffer *buffer);
unsigned long yukonBufferCount(struct yukonBuffer *buffer);
void yukonBufferDestroy(struct yukonBuffer *buffer);

struct yukonStream *yukonStreamCreate(const char *spec, unsigned long size);
unsigned long yukonStreamStatus(struct yukonStream *stream);
void yukonStreamPut(struct yukonStream *stream, struct seomPacket *packet);
void yukonStreamDestroy(struct yukonStream *stream);

#endif /* __YUKON_STREAM__ */
