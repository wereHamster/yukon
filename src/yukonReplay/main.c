
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE

#include <yukonCore/yCore.h>
#include <X11/Xatom.h>

extern void yv12_to_rgba_c (
	uint8_t * x_ptr,
	int x_stride,
	uint8_t * y_src,
	uint8_t * v_src,
	uint8_t * u_src,
	int y_stride,
	int uv_stride,
	int width,
	int height,
	int vflip
);

static uint8_t *yuvPlanes[3];
static uint64_t yuvPlanesSizes[3];
static uint8_t *rgbFrame;

static Display *dpy = NULL;
static Window win = 0;

static Bool WaitFor__MapNotify(Display * d, XEvent * e, char *arg)
{
	return (e->type == MapNotify) && (e->xmap.window == (Window) arg);
}

static Bool WaitFor__ConfigureNotify(Display * d, XEvent * e, char *arg)
{
	return (e->type == ConfigureNotify) && (e->xconfigure.window == (Window) arg);
}

static void glCaptureCreateWindow(int width, int height)
{
	dpy = XOpenDisplay(NULL);

	GLXFBConfig *fbc;
	XVisualInfo *vi;
	Colormap cmap;
	XSetWindowAttributes swa;
	GLXContext cx;
	XEvent event;
	int nElements;

	int attr[] = { GLX_DOUBLEBUFFER, False, None };

	fbc = glXChooseFBConfig(dpy, DefaultScreen(dpy), attr, &nElements);
	vi = glXGetVisualFromFBConfig(dpy, fbc[0]);
	printf("VisualID: 0x%x\n", vi->visualid);

	cx = glXCreateNewContext(dpy, fbc[0], GLX_RGBA_TYPE, 0, GL_FALSE);
	cmap = XCreateColormap(dpy, RootWindow(dpy, vi->screen), vi->visual, AllocNone);

	swa.colormap = cmap;
	swa.border_pixel = 0;
	swa.event_mask = 0;
	win = XCreateWindow(dpy, RootWindow(dpy, vi->screen), 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual, CWBorderPixel | CWColormap | CWEventMask, &swa);
	XSelectInput(dpy, win, StructureNotifyMask | KeyPressMask | KeyReleaseMask);
	XMapWindow(dpy, win);
	XIfEvent(dpy, &event, WaitFor__MapNotify, (char *)win);

	XMoveWindow(dpy, win, 64, 64);
	XIfEvent(dpy, &event, WaitFor__ConfigureNotify, (char *)win);

	glXMakeCurrent(dpy, win, cx);
}

extern void colorspace_init(void);

int main(int argc, char *argv[]) {
	int inFile = open(argv[1], O_RDONLY);
	if (inFile == -1) {
		perror("can't open infile");
		exit(-1);
	}

	colorspace_init();

	yCompressorInit();

	struct stat statBuffer;
	fstat(inFile, &statBuffer);

	unsigned char *sourceData = (unsigned char *)mmap(0, statBuffer.st_size, PROT_READ, MAP_SHARED, inFile, 0);

	unsigned char *currentPosition = sourceData;

	uint64_t width = *(uint64_t *) currentPosition;
	currentPosition += sizeof(uint64_t);
	uint64_t height = *(uint64_t *) currentPosition;
	currentPosition += sizeof(uint64_t);

	uint64_t cFrameSize = 0;
	uint64_t cFrameCount = 0;
	uint64_t cFrameTotal = 0;

	unsigned char *mem = currentPosition;
	uint64_t time[2];
	time[0] = *(uint64_t *) mem;
	for (;;) {
		time[1] = *(uint64_t *) mem;
		mem += sizeof(uint64_t);
		for (int i = 0; i < 3; ++i) {
			uint64_t cSize = *(uint64_t *) mem;
			mem += sizeof(uint64_t) + cSize;
		}
		++cFrameTotal;

		uint64_t c = (mem - sourceData) * 1000.0 / statBuffer.st_size;
		fprintf(stderr, "analysing video... %04.1f%% \r", (float)c / 10.0);

		if (mem >= sourceData + statBuffer.st_size) {
			break;
		}
	}

	fprintf(stderr, "analysing video... done  \n");
	uint64_t tt = (time[1] - time[0]) / 1000000;
	float fps = (float)cFrameTotal / tt;
	float mbs = (float)statBuffer.st_size / 1024 / 1024 / tt;
	fprintf(stderr, "%llu frames, %llu seconds, %.1f fps, %.1f MiB/s\n", cFrameTotal, tt, fps, mbs);

	uint64_t cTimeTotal = *(uint64_t *) (sourceData + statBuffer.st_size - (width * height * 3 / 2 + sizeof(uint64_t))) - *(uint64_t *) currentPosition;
	printf("size: %llu:%llu, cFrameTotal: %llu, time: %llu\n", width, height, cFrameTotal, cTimeTotal / 1000000);

	uint64_t rawFrames = (statBuffer.st_size - 2 * sizeof(uint64_t)) / (width * height * 3 / 2 + sizeof(uint64_t));
	printf("ratio: %.3f\n", (float)rawFrames / cFrameTotal);
	
	yuvPlanesSizes[0] = width * height;
	yuvPlanesSizes[1] = width * height / 4;
	yuvPlanesSizes[2] = width * height / 4;
	
	rgbFrame = malloc(width * height * 4);
	yuvPlanes[0] = malloc(width * height * 3 / 2);
	yuvPlanes[1] = yuvPlanes[0] + yuvPlanesSizes[0];
	yuvPlanes[2] = yuvPlanes[1] + yuvPlanesSizes[1];

	struct timeval currentTime = { 0 };
	gettimeofday(&currentTime, 0);

	glCaptureCreateWindow(width, height);

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glEnable(GL_TEXTURE_2D);

	uint64_t firstFrame = *(uint64_t *) currentPosition;
	uint64_t pts = firstFrame;

	gettimeofday(&currentTime, 0);
	uint64_t tdiff = currentTime.tv_sec * 1000000 + currentTime.tv_usec - pts;

	uint64_t fIndex = 0;
	
	int pause = 0;
	
	for (;;) {
		//currentPosition = sourceData + 16;
		if (currentPosition >= sourceData + statBuffer.st_size) {
			printf("\nending\n");

			break;
		}

		unsigned char *lastFrame = currentPosition;
		pts = *(uint64_t *) currentPosition;
		currentPosition += sizeof(uint64_t);
		
		for (int i = 0; i < 3; ++i) {
			uint64_t cSize = *(uint64_t *) currentPosition;
			currentPosition += sizeof(uint64_t);
			huffDecompress(yuvPlanes[i], (uint32_t *)currentPosition, yuvPlanes[i] + yuvPlanesSizes[i], &huffDecodeTable);
			for (int j = 1; j < yuvPlanesSizes[i]; ++j) {
				yuvPlanes[i][j] = yuvPlanes[i][j] + yuvPlanes[i][j - 1];
			}
			currentPosition += cSize;
		}

		yv12_to_rgba_c(rgbFrame, width * 4, yuvPlanes[0], yuvPlanes[1], yuvPlanes[2], width, width / 2, width, height, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, rgbFrame);

		gettimeofday(&currentTime, 0);
		uint64_t now = currentTime.tv_sec * 1000000 + currentTime.tv_usec;
		if (now < pts + tdiff) {
			usleep((uint64_t) pts + tdiff - now);
		} else {
			tdiff = currentTime.tv_sec * 1000000 + currentTime.tv_usec - pts;
		}

		glBegin(GL_QUADS);
		glTexCoord2d(0.0, 0.0);
		glVertex2d(-1.0, -1.0);
		glTexCoord2d(1.0, 0.0);
		glVertex2d(1.0, -1.0);
		glTexCoord2d(1.0, 1.0);
		glVertex2d(1.0, 1.0);
		glTexCoord2d(0.0, 1.0);
		glVertex2d(-1.0, 1.0);
		glEnd();
		glFlush();

		if (pause) {
			currentPosition = lastFrame;
		} else {
			++fIndex;
		}

		fprintf(stderr, "encoded frames: %llu/%llu \r", fIndex, cFrameTotal);

		int skipFrames = 0;
		while (XPending(dpy)) {
			XEvent e;
			XClientMessageEvent event;
			XNextEvent(dpy, &e);
			long key;

			switch (e.type) {
			case KeyPress:
				key = XLookupKeysym((XKeyEvent *) & e, 0);
				switch (key) {
				case XK_Right:
					skipFrames += 1;
					break;
				case XK_Left:
					skipFrames += -1;
					break;
				case XK_Up:
					skipFrames += 10;
					break;
				case XK_Down:
					skipFrames += -10;
					break;
				case XK_o:
					XResizeWindow(dpy, win, width, height);
					//XIfEvent(dpy, &e, WaitFor__ConfigureNotify, (char *)win);
					glViewport(0, 0, width, height);
					break;
				case XK_f:
					memset(&event, 0, sizeof(XClientMessageEvent));
					event.type = ClientMessage;
					event.message_type = XInternAtom(dpy, "_NET_WM_STATE", False);
					event.display = dpy;
					event.window = win;
					event.format = 32;
					event.data.l[0] = 2;
					event.data.l[1] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);

					XSendEvent(dpy, DefaultRootWindow(dpy), False, SubstructureRedirectMask, (XEvent *) & event);
					break;
				case XK_Escape:
					currentPosition = sourceData + statBuffer.st_size;
					break;
				case XK_space:
					pause = !pause;
					break;
				}
				break;
			case ConfigureNotify:
				glViewport(0, 0, e.xconfigure.width, e.xconfigure.height);
				break;
			default:
				break;
			}
		}
		
		if (skipFrames < 0 && -skipFrames > fIndex) {
			fIndex = 0;
			skipFrames = 0;
			currentPosition = sourceData + 2 * sizeof(uint64_t);
		} else if (fIndex + skipFrames > cFrameTotal - 50) {
			skipFrames = 0;
		} 

		if (skipFrames) {
			fIndex += skipFrames;
			currentPosition = sourceData + 2 * sizeof(uint64_t);
			for (int i = 0; i < fIndex; ++i) {
				pts = *(uint64_t *) currentPosition;
				currentPosition += sizeof(uint64_t);
				for (int i = 0; i < 3; ++i) {
					uint64_t cSize = *(uint64_t *) currentPosition;
					currentPosition += sizeof(uint64_t) + cSize;
				}

				if (currentPosition >= sourceData + statBuffer.st_size) {
					break;
				}
			}
			pts = *(uint64_t *) currentPosition;
			gettimeofday(&currentTime, 0);
			tdiff = currentTime.tv_sec * 1000000 + currentTime.tv_usec - pts;
		}
	}
	
	munmap(sourceData, statBuffer.st_size);
	close(inFile);
}
