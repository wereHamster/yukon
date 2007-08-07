
#ifndef __YUKON_H__
#define __YUKON_H__

#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE

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

#include <stream.h>

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

struct yukonEngine {
	struct yukonStream *stream;
	unsigned long size[2];

	pthread_mutex_t audioMutex;
	unsigned long audioRunning;
	pthread_t audioThread;
};

struct yukonEngine *yukonEngineCreate(const char *spec, unsigned long scale, unsigned long size[2]);
void yukonEngineCapture(struct yukonEngine *engine);
struct yukonEngine *yukonEngineDestroy(struct yukonEngine *engine);

void *audioThreadCallback(void *data);

#endif /* __YUKON_H__ */
