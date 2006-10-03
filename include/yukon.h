
#ifndef __YUKON_H__
#define __YUKON_H__

#include <seom/seom.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glu.h>


typedef int 	(*type_XNextEvent)				(Display *dpy, XEvent *event);
typedef int 	(*type_XPeekEvent)				(Display *dpy, XEvent *event);
typedef int 	(*type_XWindowEvent)			(Display *dpy, Window w, long mask, XEvent *event);
typedef Bool 	(*type_XCheckWindowEvent)		(Display *dpy, Window w, long mask, XEvent *event);
typedef int 	(*type_XMaskEvent)				(Display *dpy, long mask, XEvent *event);
typedef Bool 	(*type_XCheckMaskEvent)			(Display *dpy, long mask, XEvent *event);
typedef Bool 	(*type_XCheckTypedEvent)		(Display *dpy, int type, XEvent *event);
typedef Bool 	(*type_XCheckTypedWindowEvent)	(Display *dpy, Window w, int type, XEvent *event);

typedef int 	(*type_XIfEvent)				(Display *dpy, XEvent *event, Bool (*predicate)(), XPointer arg);
typedef Bool 	(*type_XCheckIfEvent) 			(Display *dpy, XEvent *event, Bool (*predicate)(), XPointer arg);
typedef int 	(*type_XPeekIfEvent)			(Display *dpy, XEvent *event, Bool (*predicate)(), XPointer arg);


typedef void *	(*type_glXGetProcAddressARB)	(const GLubyte *procName);
typedef void	(*type_glXSwapBuffers)			(Display *dpy, GLXDrawable drawable);


#endif /* __YUKON_H__ */
