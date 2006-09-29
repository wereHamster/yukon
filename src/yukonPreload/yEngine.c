
#include <yukonPreload/yEngine.h>

static yEngine *engineRegistry[64];

static char doCapture;

static void scaleFrameFast(uint32_t *inBuffer, uint32_t *outBuffer, int width, int height)
{
	uint32_t *src = inBuffer;
	uint32_t *dest = outBuffer;

	uint32_t *endLine = inBuffer + width;
	uint32_t *endBuffer = inBuffer + width * height;

	__asm__ __volatile__ ("pxor %mm7, %mm7");

	while (src != endBuffer) {
		while (src != endLine) {
			
			__asm__ __volatile__ (
				"movd      %1,    %%mm0     \n\t"
				"punpcklbw %%mm7, %%mm0     \n\t"
				"movd      %2,    %%mm1     \n\t"
				"punpcklbw %%mm7, %%mm1		\n\t"
				"movd      %3,    %%mm2     \n\t"
				"punpcklbw %%mm7, %%mm2     \n\t"
				"movd      %4,    %%mm3     \n\t"
				"punpcklbw %%mm7, %%mm3     \n\t"
				
				"paddusw   %%mm1, %%mm0     \n\t"
				"paddusw   %%mm2, %%mm0     \n\t"
				"paddusw   %%mm3, %%mm0     \n\t"
				
				"psrlw     $2,    %%mm0     \n\t"
				
				"packuswb  %%mm7, %%mm0     \n\t"
				"movd      %%mm0, %0        \n\t"
				
				: "=m" (*dest)
				: "m" (*(src+0)),
				  "m" (*(src+1)),
				  "m" (*(src+0+width)),
				  "m" (*(src+1+width))
			);

			src += 2;
			dest++;
		}
		
		src += width;
		endLine = src + width;
	}
}

static void resampleFrameFast(uint32_t *inBuffer, uint8_t *outBuffer[3], int width, int height)
{
	uint32_t *src = inBuffer;
	
	uint8_t *destY = outBuffer[0];
	uint8_t *destU = outBuffer[1];
	uint8_t *destV = outBuffer[2];
	
	uint32_t *endLine = inBuffer + width;
	uint32_t *endBuffer = inBuffer + width * height;

	
	while (src != endBuffer) {
		while (src != endLine) {
									
			__asm__ __volatile__ (
				"pxor %%mm7, %%mm7			\n\t"
				
				"movd %0, %%mm1				\n\t"
				"punpcklbw %%mm7, %%mm1		\n\t"
				
				"movd %1, %%mm2				\n\t"
				"punpcklbw %%mm7, %%mm2		\n\t"
				
				"movd %2, %%mm3				\n\t"
				"punpcklbw %%mm7, %%mm3		\n\t"
				
				"movd %3, %%mm4				\n\t"
				"punpcklbw %%mm7, %%mm4		\n\t"
				
				"movq %%mm1, %%mm5			\n\t"
				"paddw %%mm2, %%mm5			\n\t"
				"paddw %%mm3, %%mm5			\n\t"
				"paddw %%mm4, %%mm5			\n\t"
				
				:
				: "m" (*(src+0)),
				  "m" (*(src+1)),
				  "m" (*(src+0+width)),
				  "m" (*(src+1+width))
			);
			
			static const uint64_t yMul = 0x0000004200810019;
			__asm__ __volatile__ (
				"movq %4, %%mm7				\n\t"
				
				"pmaddwd %%mm7, %%mm1		\n\t"
				"movq %%mm1, %%mm0			\n\t"
				"psrlq $32, %%mm0			\n\t"
				"paddd %%mm0, %%mm1			\n\t"
				"movd %%mm1, %%eax			\n\t"
				"shr $8, %%eax				\n\t"
				"add $16, %%eax				\n\t"
				"mov %%al, %0				\n\t"
				
				"pmaddwd %%mm7, %%mm2		\n\t"
				"movq %%mm2, %%mm0			\n\t"
				"psrlq $32, %%mm0			\n\t"
				"paddd %%mm0, %%mm2			\n\t"
				"movd %%mm2, %%eax			\n\t"
				"shr $8, %%eax				\n\t"
				"add $16, %%eax				\n\t"
				"mov %%al, %1				\n\t"
				
				"pmaddwd %%mm7, %%mm3		\n\t"
				"movq %%mm3, %%mm0			\n\t"
				"psrlq $32, %%mm0			\n\t"
				"paddd %%mm0, %%mm3			\n\t"
				"movd %%mm3, %%eax			\n\t"
				"shr $8, %%eax				\n\t"
				"add $16, %%eax				\n\t"
				"mov %%al, %2				\n\t"
				
				"pmaddwd %%mm7, %%mm4		\n\t"
				"movq %%mm4, %%mm0			\n\t"
				"psrlq $32, %%mm0			\n\t"
				"paddd %%mm0, %%mm4			\n\t"
				"movd %%mm4, %%eax			\n\t"
				"shr $8, %%eax				\n\t"
				"add $16, %%eax				\n\t"
				"mov %%al, %3				\n\t"
				
				: "=m" (*(destY+0)),
				  "=m" (*(destY+1)),
				  "=m" (*(destY+0+width)),
				  "=m" (*(destY+1+width))
				: "m" (yMul)
				: "%eax"
			);

			static const uint64_t uMul = 0x0000ffdaffb60070;
			__asm__ __volatile__ (
				"movq %1, %%mm7				\n\t"
				"movq %%mm5, %%mm6			\n\t"
				"pmaddwd %%mm7, %%mm5		\n\t"
				"movq %%mm5, %%mm0			\n\t"
				"psrlq $32, %%mm0			\n\t"
				"paddd %%mm0, %%mm5			\n\t"
				"movd %%mm5, %%eax			\n\t"
				"shr $10, %%eax				\n\t"
				"add $128, %%eax			\n\t"
				"mov %%al, %0				\n\t"
				
				: "=m" (*destU)
				: "m" (uMul)
				: "%eax"
			);
			
			static const uint64_t vMul = 0x00000070ffa2ffee;
			__asm__ __volatile__ (
				"movq %1, %%mm7				\n\t"
				"pmaddwd %%mm7, %%mm6		\n\t"
				"movq %%mm6, %%mm0			\n\t"
				"psrlq $32, %%mm0			\n\t"
				"paddd %%mm0, %%mm6			\n\t"
				"movd %%mm6, %%eax			\n\t"
				"shr $10, %%eax				\n\t"
				"add $128, %%eax			\n\t"
				"mov %%al, %0				\n\t"
				
				: "=m" (*destV)
				: "m" (vMul)
				: "%eax"
			);
			
			src += 2;
			destY += 2;
			
			++destU;
			++destV;
		}
		
		src += width;
		destY += width;
		endLine = src + width;
	}
	
	__asm__ __volatile__ ("emms");
}

// median(L,T,L+T-LT)
static uint8_t median(uint8_t v1, uint8_t v2, uint8_t v3)
{
	return ( v1 + v2 + v3 ) / 3;
}

static void writePlane(yEngine *engine, uint8_t *plane, int width, int height)
{
	static uint32_t huffBuffer[1280*1024*8];
	static uint8_t tmp[1280*1024*8];
	
#define src(x,y) ( plane[(y)*width+(x)] )
	
	tmp[0] = plane[0];
	for (int x = 1; x < width; ++x) {
		tmp[x] = src(x,0) - median(src(x-1,0), 0, src(x-1,0));
	}
	
	for (int y = 1; y < height; ++y) {
		tmp[y * width] = src(0,y) - median(0, src(0,y-1), src(0,y-1));
		for (int x = 1; x < width; ++x) {
			tmp[y * width + x] = src(x,y) - median(src(x-1,y), src(x,y-1), src(x-1,y)+src(x,y-1)-src(x-1,y-1));
		}
	}
#undef src
	
	uint32_t *start = huffBuffer;
	uint32_t *end = huffCompress(huffBuffer, tmp, tmp + width * height, huffEncodeTable);
	uint64_t size = (end - start) * 4;
	write(engine->outputStreams.video.outputFile, &size, sizeof(size));
	write(engine->outputStreams.video.outputFile, huffBuffer, size);
}

static void *yEngineThreadCallback(void *data)
{
	yEngine *engine = data;
	
	printf("yEngine thread started\n");

	int srcWidth = engine->staticInfo.video.drawableSize.width;
	int srcHeight = engine->staticInfo.video.drawableSize.height;
	
	int destWidth = engine->staticInfo.video.downScale ? srcWidth / 2 : srcWidth;
	int destHeight = engine->staticInfo.video.downScale ? srcHeight / 2 : srcHeight;
	
	printf("yEngine: size: %d:%d => %d:%d\n", srcWidth, srcHeight, destWidth, destHeight);
	
	uint32_t *scaledFrame = malloc(destWidth * destHeight * 4);
	
	uint8_t *yuvPlanes[3];
	yuvPlanes[0] = malloc(destWidth * destHeight);
	yuvPlanes[1] = malloc(destWidth * destHeight / 4);
	yuvPlanes[2] = malloc(destWidth * destHeight / 4);
	
	yTime timeStart;
	yTime timeStop;
	yTime timeElapsed;

	while (1) {
		yEngineBuffer *videoBuffer = yBufferTail(engine->dataBuffers.video.videoBuffer);
		if (engine->dpy == 0) {
			break;
		}
				
		yTimeGet(&timeStart);
		
		if (engine->staticInfo.video.downScale == 0) {
			memcpy(scaledFrame, &videoBuffer->bufferData[0], srcWidth * srcHeight * 4);
		} else {
			scaleFrameFast((uint32_t *)&videoBuffer->bufferData[0], scaledFrame, srcWidth, srcHeight);
		}

		uint64_t tStamp = videoBuffer->timeStamp.sec * 1000000 + videoBuffer->timeStamp.usec;

		yBufferTailAdvance(engine->dataBuffers.video.videoBuffer);
		
		resampleFrameFast((uint32_t *)scaledFrame, yuvPlanes, destWidth, destHeight);
		
		write(engine->outputStreams.video.outputFile, &tStamp, sizeof(tStamp));
		writePlane(engine, yuvPlanes[0], destWidth, destHeight);
		writePlane(engine, yuvPlanes[1], destWidth / 2, destHeight / 2);
		writePlane(engine, yuvPlanes[2], destWidth / 2, destHeight / 2);
		
		yTimeGet(&timeStop);
		yTimeSub(&timeStop, &timeStart, &timeElapsed);
		
		double tElapsed = (double)(timeElapsed.sec * 1000000 + timeElapsed.usec);
		
		pthread_mutex_lock(&engine->engineMutex);
		double eInterval = engine->captureStatistics.video.engineInterval;
		if (eInterval == engine->staticInfo.video.captureInterval) {
			engine->captureStatistics.video.engineInterval = tElapsed;
			engine->captureStatistics.video.captureInterval = tElapsed;
		} else {
			engine->captureStatistics.video.engineInterval = eInterval * 0.9 + tElapsed * 0.1;
		}
		pthread_mutex_unlock(&engine->engineMutex);
	}
	
	free(yuvPlanes[0]);
	free(yuvPlanes[1]);
	free(yuvPlanes[2]);
}

static yEngine *yEngineCreate()
{
	for (int registryIndex = 0; registryIndex < 64; ++registryIndex) {
		if (engineRegistry[registryIndex] == 0) {
			engineRegistry[registryIndex] = malloc(sizeof(yEngine));

			return engineRegistry[registryIndex];
		}
	}

	return 0;
}

static void yEngineDestroy(yEngine *engine)
{
	for (int registryIndex = 0; registryIndex < 64; ++registryIndex) {
		if (engineRegistry[registryIndex] == engine) {
			engineRegistry[registryIndex] = 0;

			free(engine);
		}
	}
}


static yEngine *yEngineLocate(Display *dpy, GLXDrawable drawable)
{
	for (int registryIndex = 0; registryIndex < 64; ++registryIndex) {
		if (engineRegistry[registryIndex] &&
			engineRegistry[registryIndex]->dpy &&
			DefaultRootWindow(engineRegistry[registryIndex]->dpy) == DefaultRootWindow(dpy) &&
			engineRegistry[registryIndex]->drawable == drawable) {
			return engineRegistry[registryIndex];
		}
	}

	return 0;
}


static void yEngineStart(Display *dpy, GLXDrawable drawable)
{
	Window rootWindow;
	int unused;
	int width, height;
	struct sockaddr_in addr;

	XGetGeometry(dpy, drawable, &rootWindow, &unused, &unused, &width, &height, &unused, &unused);

	struct timeval currentTime;
	gettimeofday(&currentTime, 0);

	yEngine *engine = yEngineLocate(dpy, drawable);
	if (engine) {
		return;
	}
	
	engine = yEngineCreate(dpy, drawable);
	
	uint64_t insets[4];
	yConfigInsets(insets);
	
	printf("yEngineStart(): %p - 0x%08x\n", dpy, drawable);
	
	printf("yEngineStart(): %p, insets: %llu:%llu:%llu:%llu\n", engine, insets[0], insets[1], insets[2], insets[3]);
	
	width = width - insets[1] - insets[3];
	height = height - insets[0] - insets[2];
	
	char scale[64];
	yConfigScale(scale);
	engine->staticInfo.video.downScale = strcmp(scale, "full");
	printf("scale: %s\n", scale);
	
	if (engine->staticInfo.video.downScale) {
		width &= ~(3);
		height &= ~(3);
	} else {
		width &= ~(1);
		height &= ~(1);
	}
	
	engine->staticInfo.video.offset[0] = insets[3];
	engine->staticInfo.video.offset[1] = insets[2];
	
	char server[256];
	yConfigServer(server);
	unsigned int serverPort;
	char serverAddr[64];
	int success = sscanf(server, "%s %u", serverAddr, &serverPort);
	if (success == 0) {
		yEngineDestroy(engine);
		return;
	}
	
	printf("server address is: %s:%d\n", serverAddr, serverPort);
	
	int fdSocket = socket(AF_INET, SOCK_STREAM, 0);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(serverPort);
	addr.sin_addr.s_addr = inet_addr(serverAddr);

	if (connect(fdSocket, &addr, sizeof(addr)) == 0) {
		engine->outputStreams.video.outputFile = fdSocket;
		printf("connection to server established\n");
	} else {
		close(fdSocket);
		yEngineDestroy(engine);
		perror("failed to connect to the server");
		return;
	}
	
	engine->dpy = dpy;
	engine->drawable = drawable;

	engine->dataBuffers.video.videoBuffer = yBufferCreate(sizeof(yEngineBuffer) + width * height * 4, 16);	
	
	engine->staticInfo.video.drawableSize.width = width;
	engine->staticInfo.video.drawableSize.height = height;

	yConfigInterval(&engine->staticInfo.video.captureInterval);
	
	engine->captureStatistics.video.captureInterval = engine->staticInfo.video.captureInterval;
	engine->captureStatistics.video.engineInterval = engine->staticInfo.video.captureInterval;
	engine->captureStatistics.video.captureDelay = 0.0;

	yTimeGet(&engine->captureStatistics.video.lastCapture);
	
	pthread_create(&engine->engineThread, NULL, yEngineThreadCallback, engine);

	uint64_t streamWidth = (uint64_t) engine->staticInfo.video.downScale ? width / 2 : width;
	uint64_t streamHeight = (uint64_t) engine->staticInfo.video.downScale ? height / 2 : height;
	write(engine->outputStreams.video.outputFile, &streamWidth, sizeof(streamWidth));
	write(engine->outputStreams.video.outputFile, &streamHeight, sizeof(streamHeight));

	printf("yEngineStart(): %p, capturing %llu:%llu\n", engine, streamWidth, streamHeight);
	
	//engine->statFile = open("/home/tomc/afc.data", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	
	pthread_mutex_init(&engine->engineMutex, NULL);
	
	return;
}

static void yEngineStop(Display *dpy, GLXDrawable drawable)
{
	yEngine *engine = yEngineLocate(dpy, drawable);
	if (engine == 0) {
		return;
	}

	printf("yEngineStop(): %p\n", engine);

	do {
	} while (yBufferStatus(engine->dataBuffers.video.videoBuffer) < 16);
	
	engine->dpy = 0;
	yBufferHeadAdvance(engine->dataBuffers.video.videoBuffer);
	pthread_join(engine->engineThread, NULL);

	yBufferDestroy(engine->dataBuffers.video.videoBuffer);

	close(engine->outputStreams.video.outputFile);
	pthread_mutex_destroy(&engine->engineMutex);

	yEngineDestroy(engine);
}


static Time eventTime;
void yEngineEvent(Display *dpy, XEvent *event)
{
	static char hotkey[64];
	
	yConfigHotkey(hotkey);
	KeyCode keycode = XKeysymToKeycode(dpy, XStringToKeysym(hotkey));
	
	if (keycode == 0) {
		return;
	}
	
	switch(event->type) {
	case KeyPress:
		if (event->xkey.keycode == keycode) {
			if (event->xkey.time == eventTime) {
				return;
			}
			
			eventTime = event->xkey.time;
			
			if (engineRegistry[0]) {
				printf("yEngineEvent: end\n");
				doCapture = 0;
				yEngineStop(engineRegistry[0]->dpy, engineRegistry[0]->drawable);
			} else {
				printf("yEngineEvent: start\n");
				doCapture = 1;
			}
  		}
		break;
	default:
		break;
	}
}

void yEngineCapture(Display *dpy, GLXDrawable drawable)
{
	yTime timeCurrent;
	yTime timeElapsed;

	Window rootWindow;
	int unused;
	int width;
	int height;
	
	//printf("drawable: %lx\n", drawable);

	yEngine *engine = yEngineLocate(dpy, drawable);
	if (engine == 0) {
		if (engineRegistry[0]) {
			/* only allow one engine to be running at any time */
			return;
		}
		
		if (doCapture) {
			yEngineStart(dpy, drawable);
			engine = yEngineLocate(dpy, drawable);
			if (engine == 0) {
				doCapture = 0;
				return;
			}
		} else {
			return;
		}
	}

	width = engine->staticInfo.video.drawableSize.width;
	height = engine->staticInfo.video.drawableSize.height;
	
	/*XGetGeometry(dpy, drawable, &rootWindow, &unused, &unused, &width, &height, &unused, &unused);
	if (engine->staticInfo.video.drawableSize.width != width || engine->staticInfo.video.drawableSize.height != height) {
		printf("geometry changed, stoping engine\n");
		yEngineStop(dpy, drawable);

		return;
	}*/

	uint64_t bufferStatus = yBufferStatus(engine->dataBuffers.video.videoBuffer);
	
	pthread_mutex_lock(&engine->engineMutex);
	double eInterval = engine->captureStatistics.video.engineInterval;
	pthread_mutex_unlock(&engine->engineMutex);
	
	double cInterval = engine->captureStatistics.video.captureInterval;
	int64_t bStatus = 8 - bufferStatus;
	double iCorrection = ( ( eInterval + bStatus * 100 ) - cInterval ) * 0.1;
	engine->captureStatistics.video.captureInterval = cInterval * 0.99 + ( cInterval + iCorrection ) * 0.01;
	if (engine->captureStatistics.video.captureInterval < engine->staticInfo.video.captureInterval) {
		engine->captureStatistics.video.captureInterval = engine->staticInfo.video.captureInterval;
	}

	yTimeGet(&timeCurrent);
	yTimeSub(&timeCurrent, &engine->captureStatistics.video.lastCapture, &timeElapsed);
	engine->captureStatistics.video.lastCapture = timeCurrent;
	
	double tElapsed = (double)(timeElapsed.sec * 1000000 + timeElapsed.usec);
	double tDelay = engine->captureStatistics.video.captureDelay - tElapsed;
	
	//static char buf[1024];
	//int ret = sprintf(buf, "%llu %f %f %f %f\n", bufferStatus, eInterval, engine->captureStatistics.video.captureInterval, iCorrection, tElapsed);
	//write(engine->statFile, buf, ret);

	double delayMargin = engine->captureStatistics.video.captureInterval / 10.0;
	if (tDelay < delayMargin) {
		if (bufferStatus) {
			yEngineBuffer *videoBuffer = yBufferHead(engine->dataBuffers.video.videoBuffer);

			videoBuffer->timeStamp = timeCurrent;
			uint64_t x = engine->staticInfo.video.offset[0];
			uint64_t y = engine->staticInfo.video.offset[1];
			glReadPixels(x, y, width, height, GL_BGRA, GL_UNSIGNED_BYTE, &videoBuffer->bufferData[0]);

			yBufferHeadAdvance(engine->dataBuffers.video.videoBuffer);
		//	printf("timeElapsed.usec: %llu\n", timeElapsed.usec);

			if (tDelay < 0) { // frame too late
				if (engine->captureStatistics.video.captureInterval + tDelay < 0.0) { // lag? drop frame(s) and return to normal frame interval
					engine->captureStatistics.video.captureDelay = engine->captureStatistics.video.captureInterval;
				} else { // frame too late, adjust capture interval
					engine->captureStatistics.video.captureDelay = engine->captureStatistics.video.captureInterval + tDelay;
				}
			} else { // frame too early, adjust capture interval
				engine->captureStatistics.video.captureDelay = engine->captureStatistics.video.captureInterval + tDelay;
			}
		} else { // encoder too slow, try next frame
			if (tDelay < 0) { // we are already too late
				engine->captureStatistics.video.captureDelay = 0;
			} else { // we get another chance to capture in time
				engine->captureStatistics.video.captureDelay = tDelay;
			}
		}
	} else { // normal update
		engine->captureStatistics.video.captureDelay = tDelay;
	}

//	printf("bufferStatus: %llu, captureInterval: %llu, captureDelay: %llu\n\n", bufferStatus, engine->captureStatistics.video.captureInterval, engine->captureStatistics.video.captureDelay);
}
