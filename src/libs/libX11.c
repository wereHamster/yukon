
#include <loader.h>
#include <X11/Xlib.h>

static void (*core)(Display *dpy, XEvent *event);

int XNextEvent(Display *dpy, XEvent *event)
{
	int ret = chain(XNextEvent, dpy, event);
	glue("yukonCoreEvent", dpy, event);
	return ret;
}

int XPeekEvent(Display *dpy, XEvent *event)
{
	int ret = chain(XPeekEvent, dpy, event);
	glue("yukonCoreEvent", dpy, event);
	return ret;
}

int XWindowEvent(Display *dpy, Window w, long mask, XEvent *event)
{
	int ret = chain(XWindowEvent, dpy, w, mask, event);
	glue("yukonCoreEvent", dpy, event);
	return ret;
}

Bool XCheckWindowEvent(Display *dpy, Window w, long mask, XEvent *event)
{
	Bool ret = chain(XCheckWindowEvent, dpy, w, mask, event);
	if (ret)
		glue("yukonCoreEvent", dpy, event);
	return ret;
}

int XMaskEvent(Display *dpy, long mask, XEvent *event)
{
	int ret = chain(XMaskEvent, dpy, mask, event);
	glue("yukonCoreEvent", dpy, event);
	return ret;
}

Bool XCheckMaskEvent(Display *dpy, long mask, XEvent *event)
{
	Bool ret = chain(XCheckMaskEvent, dpy, mask, event);
	if (ret)
		glue("yukonCoreEvent", dpy, event);
	return ret;
}

Bool XCheckTypedEvent(Display *dpy, int type, XEvent *event)
{
	Bool ret = chain(XCheckTypedEvent, dpy, type, event);
	if (ret)
		glue("yukonCoreEvent", dpy, event);
	return ret;
}

Bool XCheckTypedWindowEvent(Display *dpy, Window w, int type, XEvent *event)
{
	Bool ret = chain(XCheckTypedWindowEvent, dpy, w, type, event);
	if (ret)
		glue("yukonCoreEvent", dpy, event);
	return ret;
}

int XIfEvent(Display *dpy, XEvent *event, Bool (*predicate)(), XPointer arg)
{
	int ret = chain(XIfEvent, dpy, event, predicate, arg);
	glue("yukonCoreEvent", dpy, event);
	return ret;
}

Bool XCheckIfEvent(Display *dpy, XEvent *event, Bool (*predicate)(), XPointer arg)
{
	Bool ret = chain(XCheckIfEvent, dpy, event, predicate, arg);
	if (ret)
		glue("yukonCoreEvent", dpy, event);
	return ret;
}

int XPeekIfEvent(Display * dpy, XEvent * event, Bool(*predicate) (), XPointer arg)
{
	int ret = chain(XPeekIfEvent, dpy, event, predicate, arg);
	glue("yukonCoreEvent", dpy, event);
	return ret;
}
