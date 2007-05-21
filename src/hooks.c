
#include <yukon.h>

static int (*pXNextEvent)(Display *dpy, XEvent *event);
int XNextEvent(Display * dpy, XEvent * event)
{
	int ret = pXNextEvent(dpy, event);
	yukonEvent(dpy, event);
	return ret;
}

static int (*pXPeekEvent)(Display *dpy, XEvent *event);
int XPeekEvent(Display *dpy, XEvent *event)
{
	int ret = pXPeekEvent(dpy, event);
	yukonEvent(dpy, event);
	return ret;
}

static int (*pXWindowEvent)(Display *dpy, Window w, long mask, XEvent *event);
int XWindowEvent(Display *dpy, Window w, long mask, XEvent *event)
{
	int ret = pXWindowEvent(dpy, w, mask, event);
	yukonEvent(dpy, event);
	return ret;
}

static Bool (*pXCheckWindowEvent)(Display *dpy, Window w, long mask, XEvent *event);
Bool XCheckWindowEvent(Display *dpy, Window w, long mask, XEvent *event)
{
	Bool ret = pXCheckWindowEvent(dpy, w, mask, event);
	if (ret)
		yukonEvent(dpy, event);
	return ret;
}

static int (*pXMaskEvent)(Display *dpy, long mask, XEvent *event);
int XMaskEvent(Display *dpy, long mask, XEvent *event)
{
	int ret = pXMaskEvent(dpy, mask, event);
	yukonEvent(dpy, event);
	return ret;
}

static Bool (*pXCheckMaskEvent)(Display *dpy, long mask, XEvent *event);
Bool XCheckMaskEvent(Display *dpy, long mask, XEvent *event)
{
	Bool ret = pXCheckMaskEvent(dpy, mask, event);
	if (ret)
		yukonEvent(dpy, event);
	return ret;
}

static Bool (*pXCheckTypedEvent)(Display *dpy, int type, XEvent *event);
Bool XCheckTypedEvent(Display *dpy, int type, XEvent *event)
{
	Bool ret = pXCheckTypedEvent(dpy, type, event);
	if (ret)
		yukonEvent(dpy, event);
	return ret;
}

static Bool (*pXCheckTypedWindowEvent)(Display *dpy, Window w, int type, XEvent *event);
Bool XCheckTypedWindowEvent(Display *dpy, Window w, int type, XEvent *event)
{
	Bool ret = pXCheckTypedWindowEvent(dpy, w, type, event);
	if (ret)
		yukonEvent(dpy, event);
	return ret;
}

static int (*pXIfEvent)(Display *dpy, XEvent *event, Bool (*predicate)(), XPointer arg);
int XIfEvent(Display *dpy, XEvent *event, Bool (*predicate)(), XPointer arg)
{
	int ret = pXIfEvent(dpy, event, predicate, arg);
	yukonEvent(dpy, event);
	return ret;
}

static Bool (*pXCheckIfEvent)(Display *dpy, XEvent *event, Bool (*predicate)(), XPointer arg);
Bool XCheckIfEvent(Display *dpy, XEvent *event, Bool (*predicate)(), XPointer arg)
{
	Bool ret = pXCheckIfEvent(dpy, event, predicate, arg);
	if (ret)
		yukonEvent(dpy, event);
	return ret;
}

static int (*pXPeekIfEvent)(Display *dpy, XEvent *event, Bool (*predicate)(), XPointer arg);
int XPeekIfEvent(Display * dpy, XEvent * event, Bool(*predicate) (), XPointer arg)
{
	int ret = pXPeekIfEvent(dpy, event, predicate, arg);
	yukonEvent(dpy, event);
	return ret;
}

static void (*pglXSwapBuffers)(Display *dpy, GLXDrawable drawable);
void glXSwapBuffers(Display *dpy, GLXDrawable drawable)
{
	GLint buf;
	glGetIntegerv(GL_READ_BUFFER, &buf);

	glReadBuffer(GL_BACK);
	yukonCapture(dpy, drawable);
	glReadBuffer(buf);

	pglXSwapBuffers(dpy, drawable);
}

static void *(*pglXGetProcAddressARB)(const GLubyte *procName);
void (*glXGetProcAddressARB(const GLubyte * procName)) (void) {
	if (strcmp((char *)procName, "glXGetProcAddressARB") == 0) {
		return (void *)glXGetProcAddressARB;
	} else if (strcmp((char *)procName, "glXSwapBuffers") == 0) {
		return (void *)glXSwapBuffers;
	}

	return pglXGetProcAddressARB(procName);
}


__attribute__ ((constructor))
static void yukonHooksInit()
{
#define loadHook(hookName) \
	p##hookName = dlsym(RTLD_NEXT, #hookName); \
	if (p##hookName == NULL) { \
		yukonLog(1, "dlsym(\"%s\") failed: %s\n", #hookName, dlerror()); \
		exit(0); \
	}

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

	loadHook(glXSwapBuffers);
	loadHook(glXGetProcAddressARB);

#undef loadHook
}

__attribute__ ((destructor))
static void yukonHooksFini()
{
}
