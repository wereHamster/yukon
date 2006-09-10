
#ifndef __YUKON_PRELOAD_ENGINE_H__
#define __YUKON_PRELOAD_ENGINE_H__

#include <yukonCore/yCore.h>

typedef struct _yEngine yEngine;
typedef struct _yEngineBuffer yEngineBuffer;

void yEngineEvent(Display *dpy, XEvent *event);

void yEngineCapture(Display *dpy, GLXDrawable drawable);

struct _yEngine {
	Display *dpy;
	GLXDrawable drawable;

	pthread_t engineThread;

	struct {
		struct {
			yBuffer *videoBuffer;
		} video;
	} dataBuffers;

	struct {
		struct {
			struct {
				uint64_t width;
				uint64_t height;
			} drawableSize;
			double captureInterval;
			long downScale;
			uint64_t offset[2];
		} video;
	} staticInfo;

	struct {
		struct {
			double captureInterval;
			double captureDelay;
			yTime lastCapture;
			double engineInterval;
		} video;
	} captureStatistics;

	struct {
		struct {
			int outputFile;
		} video;
	} outputStreams;
	
	int statFile;
	
	pthread_mutex_t engineMutex;
};

struct _yEngineBuffer {
	yTime timeStamp;
	char bufferData[0];
};


#endif /* __YUKON_PRELOAD_ENGINE_H__ */
