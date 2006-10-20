#include <yukon.h>

static int doCapture;
static seomClient *client;
static KeySym hotkey;
static char hotkeybuf[32];
static char output[1024];
static char scale[5];
static int insets[4] = { 0, 0, 0, 0 };
static float fps = 30.0f;

static int argcheck(char *value)
{
	if (value && strlen(value) && value[0] != '\n')
		return 1;

	return 0;
}

// The reason we have separate functions here is so that we can add some
// advanced parsing and error handling per case.
static void yConfigOptionOutput(char *value)
{
	if (argcheck(value))
		sscanf(value, "%1024s", output);
}

static void yConfigOptionFPS(char *value)
{
	if (argcheck(value))
		sscanf(value, "%f", &fps);
}

static void yConfigOptionInsets(char *value)
{
	if (argcheck(value))
		sscanf(value, "%d %d %d %d", &insets[0], &insets[1], &insets[2], &insets[3]);
}

static void yConfigOptionScale(char *value)
{
	if (argcheck(value))
		sscanf(value, "%5s", scale);
}

static void yConfigOptionHotkey(char *value)
{
	if (argcheck(value))
		sscanf(value, "%32s", hotkeybuf);
}

// TODO: Please note! This parsing function isn't perfect! Parsing in C is
// about the stupidest, dumbest thing ever and I simply refuse to write ugly
// code just to conver "fringe" use cases that probably won't result in any
// problems anyway.
static void yConfigParse(char *configfile)
{
	char buffer[1024];

	FILE *config = fopen(configfile, "r");
	char *line = NULL;
	int linenum = 1;

	if (!config)
		return;

	do {
		char *pos = NULL;

		memset(buffer, 0, sizeof(buffer));

		line = fgets(buffer, sizeof(buffer), config);

		if (line)
			pos = strchr(line, '=');

		if (pos) {
			char option[10];

			memset(option, 0, sizeof(option));

			pos++;

			sscanf(line, "%9s=", option);

			if (!strncmp(option, "OUTPUT", 6))
				yConfigOptionOutput(pos);
			else if (!strncmp(option, "FPS", 3))
				yConfigOptionFPS(pos);
			else if (!strncmp(option, "INSETS", 6))
				yConfigOptionInsets(pos);
			else if (!strncmp(option, "SCALE", 5))
				yConfigOptionScale(pos);
			else if (!strncmp(option, "HOTKEY", 6))
				yConfigOptionHotkey(pos);

			linenum++;
		}
	} while (line);

	fclose(config);
}

static void yConfigLoad(void)
{
	char buffer[1024];

	strncpy(output, "file:///tmp/yukon.seom", sizeof(output));
	strncpy(scale, "full", sizeof(scale));
	strncpy(hotkeybuf, "F8", sizeof(hotkeybuf));

	sprintf(buffer, "%s/.yukon/conf", getenv("HOME"));

	yConfigParse("/etc/yukon.conf");
	yConfigParse(buffer);

	// Setup the ENV overrides.
	yConfigOptionOutput(getenv("YUKON_OUTPUT"));
	yConfigOptionFPS(getenv("YUKON_FPS"));
	yConfigOptionHotkey(getenv("YUKON_HOTKEY"));
	yConfigOptionScale(getenv("YUKON_SCALE"));
	yConfigOptionInsets(getenv("YUKON_INSETS"));

	hotkey = XStringToKeysym(hotkeybuf);

	printf(
		"Yukon setup information:\n"
		" - OUTPUT: %s\n"
		" - SCALE: %s\n"
		" - HOTKEY: %s\n"
		" - FPS: %.1f\n"
		" - INSETS: %d %d %d %d\n",
		output, scale, hotkeybuf, fps, insets[0], insets[1], insets[2], insets[3]);
}

static Time eventTime;
void yEngineEvent(Display * dpy, XEvent * event)
{
	switch (event->type) {
	case KeyPress:
		if (event->xkey.keycode == XKeysymToKeycode(dpy, hotkey)) {
			if (event->xkey.time == eventTime) {
				return;
			}

			eventTime = event->xkey.time;

			if (client) {
				printf("yEngineEvent(): stop\n");
				doCapture = 0;
				seomClientDestroy(client);
				client = NULL;
			} else {
				printf("yEngineEvent(): start\n");
				doCapture = 1;
			}
		}
		break;
	default:
		break;
	}
}

void yEngineCapture(Display * dpy, GLXDrawable drawable)
{
	if (client == NULL) {
		if (doCapture) {
			Window root;
			unsigned int width, height, uunused;
			int sunused;

			XGetGeometry(dpy, drawable, &root, &sunused, &sunused, &width, &height, &uunused, &uunused);

			yConfigLoad();
			client = seomClientCreate(output, width, height, fps);

			if (client == NULL) {
				doCapture = 0;
				printf("yEngineCapture(): couldn't create seom client\n");
				return;
			}
		} else {
			return;
		}
	}
	seomClientCapture(client, 0, 0);
}

static type_glXGetProcAddressARB orig_glXGetProcAddressARB;
static type_glXSwapBuffers orig_glXSwapBuffers;

static type_XNextEvent orig_XNextEvent;
static type_XPeekEvent orig_XPeekEvent;
static type_XWindowEvent orig_XWindowEvent;
static type_XCheckWindowEvent orig_XCheckWindowEvent;
static type_XMaskEvent orig_XMaskEvent;
static type_XCheckMaskEvent orig_XCheckMaskEvent;
static type_XCheckTypedEvent orig_XCheckTypedEvent;
static type_XCheckTypedWindowEvent orig_XCheckTypedWindowEvent;

static type_XIfEvent orig_XIfEvent;
static type_XCheckIfEvent orig_XCheckIfEvent;
static type_XPeekIfEvent orig_XPeekIfEvent;

static void yHooksError(const char *hookName)
{
	printf("====================================================\n");
	printf("======= ERROR - ERROR -ERROR - ERROR - ERROR =======\n");
	printf("====================================================\n");
	printf("\n");
	printf("couldn't locate the real %s(...) function !\n", hookName);
	printf("dl reports: %s\n", dlerror());
	printf("exiting...\n");
	printf("\n");

	exit(0);
}

__attribute__ ((constructor))
static void yHooksConstructor()
{
#define loadHook(hookName, src)                                      \
	orig_##hookName = (type_##hookName) dlsym(RTLD_NEXT, #hookName); \
	if (orig_##hookName == 0) {                                      \
		yHooksError(#hookName);                                      \
	}                                                                \

	loadHook(glXGetProcAddressARB, gl);
	loadHook(glXSwapBuffers, gl);

	loadHook(XNextEvent, x11);
	loadHook(XPeekEvent, x11);
	loadHook(XWindowEvent, x11);
	loadHook(XCheckWindowEvent, x11);
	loadHook(XMaskEvent, x11);
	loadHook(XCheckMaskEvent, x11);
	loadHook(XCheckTypedEvent, x11);
	loadHook(XCheckTypedWindowEvent, x11);

	loadHook(XIfEvent, x11);
	loadHook(XCheckIfEvent, x11);
	loadHook(XPeekIfEvent, x11);

#undef loadHook

	//printf("yukon hooks established !\n");

	yConfigLoad();
}

__attribute__ ((destructor))
static void yHooksDestructor()
{
	//printf("yukon hooks cleaned up !\n");
}

void glXSwapBuffers(Display * dpy, GLXDrawable drawable)
{
	yEngineCapture(dpy, drawable);

	orig_glXSwapBuffers(dpy, drawable);
}

int XNextEvent(Display * dpy, XEvent * event)
{
	int retVal = orig_XNextEvent(dpy, event);
	yEngineEvent(dpy, event);
	return retVal;
}

int XPeekEvent(Display * dpy, XEvent * event)
{
	int retVal = orig_XPeekEvent(dpy, event);
	yEngineEvent(dpy, event);
	return retVal;
}

int XWindowEvent(Display * dpy, Window w, long mask, XEvent * event)
{
	int retVal = orig_XWindowEvent(dpy, w, mask, event);
	yEngineEvent(dpy, event);
	return retVal;
}

Bool XCheckWindowEvent(Display * dpy, Window w, long mask, XEvent * event)
{
	Bool retVal = orig_XCheckWindowEvent(dpy, w, mask, event);
	if (retVal) {
		yEngineEvent(dpy, event);
	}
	return retVal;
}

int XMaskEvent(Display * dpy, long mask, XEvent * event)
{
	int retVal = orig_XMaskEvent(dpy, mask, event);
	yEngineEvent(dpy, event);
	return retVal;
}

Bool XCheckMaskEvent(Display * dpy, long mask, XEvent * event)
{
	Bool retVal = orig_XCheckMaskEvent(dpy, mask, event);
	if (retVal) {
		yEngineEvent(dpy, event);
	}
	return retVal;
}

Bool XCheckTypedEvent(Display * dpy, int type, XEvent * event)
{
	Bool retVal = orig_XCheckTypedEvent(dpy, type, event);
	if (retVal) {
		yEngineEvent(dpy, event);
	}
	return retVal;
}

Bool XCheckTypedWindowEvent(Display * dpy, Window w, int type, XEvent * event)
{
	Bool retVal = orig_XCheckTypedWindowEvent(dpy, w, type, event);
	if (retVal) {
		yEngineEvent(dpy, event);
	}
	return retVal;
}

int XIfEvent(Display * dpy, XEvent * event, Bool(*predicate) (), XPointer arg)
{
	int retVal = orig_XIfEvent(dpy, event, predicate, arg);
	yEngineEvent(dpy, event);
	return retVal;
}

Bool XCheckIfEvent(Display * dpy, XEvent * event, Bool(*predicate) (), XPointer arg)
{
	Bool retVal = orig_XCheckIfEvent(dpy, event, predicate, arg);
	if (retVal) {
		yEngineEvent(dpy, event);
	}
	return retVal;
}

int XPeekIfEvent(Display * dpy, XEvent * event, Bool(*predicate) (), XPointer arg)
{
	int retVal = orig_XPeekIfEvent(dpy, event, predicate, arg);
	yEngineEvent(dpy, event);
	return retVal;
}

void (*glXGetProcAddressARB(const GLubyte * procName)) (void) {
	if (strcmp((char *)procName, "glXGetProcAddressARB") == 0) {
		return (void *)glXGetProcAddressARB;
	} else if (strcmp((char *)procName, "glXSwapBuffers") == 0) {
		return (void *)glXSwapBuffers;
	}

	return orig_glXGetProcAddressARB(procName);
}
