
#ifndef __YUKON_PRELOAD_HOOKS_H__
#define __YUKON_PRELOAD_HOOKS_H__

#include <yukonPreload/yEngine.h>


/***
 * X11
 */
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


/***
 * GLX
 */
typedef void *	(*type_glXGetProcAddressARB)	(const GLubyte *procName);
typedef void	(*type_glXSwapBuffers)			(Display *dpy, GLXDrawable drawable);


#endif /* __YUKON_PRELOAD_HOOKS_H__ */
