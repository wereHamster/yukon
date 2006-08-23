
#include <yukonPreload/yHooks.h>

static void *glHandle;

static type_dlopen orig_dlopen;
static type_dlsym orig_dlsym;

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

extern char *captureOverride;

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
	glHandle = dlopen("libGL.so.1", RTLD_LAZY | RTLD_GLOBAL);
	
#define loadHook(hookName)                                            \
	orig_##hookName = (type_##hookName) dlsym(RTLD_NEXT, #hookName);  \
	if (orig_##hookName == 0) {                                       \
		yHooksError(#hookName);                                       \
	}                                                                 \

	loadHook(dlopen);
    loadHook(dlsym);
    

	loadHook(glXGetProcAddressARB);
    loadHook(glXSwapBuffers);

    loadHook(XNextEvent);
    loadHook(XPeekEvent);
    loadHook(XWindowEvent);
    loadHook(XCheckWindowEvent);
    loadHook(XMaskEvent);
    loadHook(XCheckMaskEvent);
    loadHook(XCheckTypedEvent);
    loadHook(XCheckTypedWindowEvent);

    loadHook(XIfEvent);
    loadHook(XCheckIfEvent);
    loadHook(XPeekIfEvent);
    loadHook(XNextEvent);
    loadHook(XNextEvent);
    loadHook(XNextEvent);

#undef loadHook

    printf("yukon hooks established !\n");

    captureOverride = getenv("yukonOverride");
    if (captureOverride) {
        printf("yukonOverride enabled, capturing all windows !\n");
    }
    
    yCompressorInit();
}

__attribute__ ((destructor))
static void yHooksDestructor()
{
	dlclose(glHandle);
	printf("yukon hooks cleaned up !\n");
}

/*
void *dlopen(const char *filename, int flag)
{
	printf("opening library: %p\n", orig_dlopen);
	if (filename) {
	if (strcmp(filename, "libGL.so") == 0) {
		filename = "libFG.so";
		printf("opening fake OpenGL library\n");
	} else if (strcmp(filename, "libGL.so.2") == 0) {
		filename = "libFG.so.1";
		printf("opening fake OpenGL library\n");
	}
	}
	
	return orig_dlopen(filename, flag);
}

void *dlsym(void *handle, const char *symbol)
{
	return orig_dlsym(handle, symbol);
}
*/


void glXSwapBuffers(Display * dpy, GLXDrawable drawable)
{
    yEngineCapture(dpy, drawable);

    orig_glXSwapBuffers(dpy, drawable);
}

int XNextEvent(Display * dpy, XEvent * event)
{
    int retVal = orig_XNextEvent(dpy, event);
    while (yEngineEvent(dpy, event)) {
        retVal = orig_XNextEvent(dpy, event);
    }
    return retVal;
}

int XPeekEvent(Display * dpy, XEvent * event)
{
    int retVal = orig_XPeekEvent(dpy, event);
    while (yEngineEvent(dpy, event)) {
        retVal = orig_XPeekEvent(dpy, event);
    }
    return retVal;
}

int XWindowEvent(Display * dpy, Window w, long mask, XEvent * event)
{
    int retVal = orig_XWindowEvent(dpy, w, mask, event);
    while (yEngineEvent(dpy, event)) {
        retVal = orig_XWindowEvent(dpy, w, mask, event);
    }
    return retVal;
}

Bool XCheckWindowEvent(Display * dpy, Window w, long mask, XEvent * event)
{
    Bool retVal = orig_XCheckWindowEvent(dpy, w, mask, event);
    while (retVal && yEngineEvent(dpy, event)) {
        retVal = orig_XCheckWindowEvent(dpy, w, mask, event);
    }
    return retVal;
}

int XMaskEvent(Display * dpy, long mask, XEvent * event)
{
    int retVal = orig_XMaskEvent(dpy, mask, event);
    while (yEngineEvent(dpy, event)) {
        retVal = orig_XMaskEvent(dpy, mask, event);
    }
    return retVal;
}

Bool XCheckMaskEvent(Display * dpy, long mask, XEvent * event)
{
    Bool retVal = orig_XCheckMaskEvent(dpy, mask, event);
    while (retVal && yEngineEvent(dpy, event)) {
        retVal = orig_XCheckMaskEvent(dpy, mask, event);
    }
    return retVal;
}

Bool XCheckTypedEvent(Display * dpy, int type, XEvent * event)
{
    Bool retVal = orig_XCheckTypedEvent(dpy, type, event);
    while (retVal && yEngineEvent(dpy, event)) {
        retVal = orig_XCheckTypedEvent(dpy, type, event);
    }
    return retVal;
}

Bool XCheckTypedWindowEvent(Display * dpy, Window w, int type, XEvent * event)
{
    Bool retVal = orig_XCheckTypedWindowEvent(dpy, w, type, event);
    while (retVal && yEngineEvent(dpy, event)) {
        retVal = orig_XCheckTypedWindowEvent(dpy, w, type, event);
    }
    return retVal;
}


int XIfEvent(Display * dpy, XEvent * event, Bool(*predicate) (), XPointer arg)
{
    int retVal = orig_XIfEvent(dpy, event, predicate, arg);
    while (yEngineEvent(dpy, event)) {
        retVal = orig_XIfEvent(dpy, event, predicate, arg);
    }
    return retVal;
}

Bool XCheckIfEvent(Display * dpy, XEvent * event, Bool(*predicate) (), XPointer arg)
{
    Bool retVal = orig_XCheckIfEvent(dpy, event, predicate, arg);
    while (retVal && yEngineEvent(dpy, event)) {
        retVal = orig_XCheckIfEvent(dpy, event, predicate, arg);
    }
    return retVal;
}

int XPeekIfEvent(Display * dpy, XEvent * event, Bool(*predicate) (), XPointer arg)
{
    int retVal = orig_XPeekIfEvent(dpy, event, predicate, arg);
    while (yEngineEvent(dpy, event)) {
        retVal = orig_XPeekIfEvent(dpy, event, predicate, arg);
    }
    return retVal;
}

void (*glXGetProcAddressARB(const GLubyte *procName))(void)
{
	if (strcmp(procName, "glXSwapBuffers") == 0) {
		return (void *) glXSwapBuffers;
	}
	
	return orig_glXGetProcAddressARB(procName);
}
