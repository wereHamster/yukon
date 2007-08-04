
#ifndef __YUKON_H__
#define __YUKON_H__

#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <linux/limits.h>

#include <pthread.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>

void logMessage(unsigned long level, const char *fmt, ...);
void updateConfiguration(void);

typedef struct {
	unsigned long logLevel;
	KeySym hotkey;

	unsigned int scale;
	double fps;
	char output[PATH_MAX];
} yukonGlobalData;

extern yukonGlobalData yukonGlobal;

struct packet {
	uint8_t type;
	uint64_t time;
	uint64_t size;
};

struct buffer {
	pthread_mutex_t mutex;
	pthread_cond_t cond;

	unsigned long size, head, tail;
	struct packet *array[0];
};

unsigned long coreRunning(void);
void coreStop(void);
void captureVideo(void);
void setupEngine(Display *dpy, GLXDrawable drawable);

void multiplexerPut(struct packet *packet);
void copyFrame(void *dst[3], struct packet *packet);
void *streamThreadCallback(void *data);
void *audioThreadCallback(void *data);

void setupAudio(void);
void stopAudio(void);

void streamPutVideo(struct packet *packet);
void streamPutAudio(struct packet *packet);
			
void readFramebuffer(unsigned int width, unsigned int height, void *data);

void frameResample(void *buf, unsigned long w, unsigned long h);
void frameConvert(void *dst[3], void *src, unsigned long w, unsigned long h);

struct packet *packetCreate(unsigned char type, unsigned long size);
void *packetPayload(struct packet *packet);
void packetDestroy(struct packet *packet);

struct buffer *bufferCreate(unsigned long size);
void bufferPut(struct buffer *buffer, struct packet *packet);
struct packet *bufferGet(struct buffer *buffer);
unsigned long bufferCount(struct buffer *buffer);
void bufferDestroy(struct buffer *buffer);


#endif /* __YUKON_H__ */
