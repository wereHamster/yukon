
#include <yukon.h>

/* shared variables */
static pthread_t streamThread, audioThread;
static struct buffer *streamBuffer, *audioBuffer;
int fd;

/* video */
static unsigned int size[2];

void setupEngine(Display *dpy, GLXDrawable drawable)
{
	Window root;
	unsigned int unused;
	XGetGeometry(dpy, drawable, &root, (int *)&unused, (int *)&unused, &size[0], &size[1], &unused, &unused);

	streamBuffer = bufferCreate(16);
	audioBuffer = bufferCreate(3);

	setupAudio();

	pthread_create(&streamThread, NULL, streamThreadCallback, streamBuffer);
	pthread_create(&audioThread, NULL, audioThreadCallback, audioBuffer);

	fd = open("/tmp/yukon.output", O_WRONLY | O_CREAT | O_TRUNC, 0664);

	logMessage(4, "setupEngine() done\n");
}

unsigned long coreRunning(void)
{
	return streamBuffer != 0;
}

void coreStop(void)
{
	multiplexerPut(NULL);
	pthread_join(streamThread, NULL);
	free(streamBuffer);
	streamBuffer = NULL;
	close(fd);
}

void multiplexerPut(struct packet *packet)
{
	bufferPut(streamBuffer, packet);
}

void captureVideo(void)
{
	/*if (bufferCount(streamBuffer) > 14)
		return;

	struct packet *packet = packetCreate(0x01, size[0] * size[1] * 4);
	if (packet == NULL)
		return;

	readFramebuffer(size[0], size[1], packetPayload(packet));
	multiplexerPut(packet);*/
}

void copyFrame(void *dst[3], struct packet *packet)
{
    unsigned long tmp[2] = { size[0], size[1] };
    unsigned int scale = yukonGlobal.scale;
   
    while (scale--) {
    	frameResample(packetPayload(packet), tmp[0], tmp[1]);
    	tmp[0] >>= 1; tmp[1] >>= 1;
	};

	frameConvert(dst, packetPayload(packet), tmp[0], tmp[1]);
}

void streamPutVideo(struct packet *packet)
{
	static char tmp[1920 * 1200 * 4 + 8000];
	static void *buf[3] = { tmp, tmp + 1920 * 1200, tmp + 1920 * 1200 * 3 / 2 };

	copyFrame(buf, packet);
	packet->size = size[0] * size[1] * 3 / 2;
	write(fd, packet, sizeof(struct packet));
	write(fd, tmp, size[0] * size[1] * 3 / 2);
}

void streamPutAudio(struct packet *packet)
{
	write(fd, packet, sizeof(struct packet));
	write(fd, packetPayload(packet), packet->size);
}
