
#include <yukonPreload/yHooks.h>

static void *x11Handle;
static void *glHandle;

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

extern char *yukonOverride;

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
#define loadHook(hookName, src)                                       \
	orig_##hookName = (type_##hookName) dlsym(RTLD_NEXT, #hookName);  \
	if (orig_##hookName == 0) {                                       \
		yHooksError(#hookName);                                       \
	}                                                                 \

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

    printf("yukon hooks established !\n");

    yukonOverride = getenv("yukonOverride");
    if (yukonOverride) {
        printf("yukonOverride enabled, magic key combination will start capturing on all windows !\n");
    }
    
    yCompressorInit();
}

__attribute__ ((destructor))
static void yHooksDestructor()
{
	printf("yukon hooks cleaned up !\n");
}

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
