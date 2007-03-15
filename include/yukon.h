
#ifndef __YUKON_H__
#define __YUKON_H__

#include <stdarg.h>

#include <seom/seom.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>

void yukonCapture(Display *dpy, GLXDrawable drawable);
void yukonConfig(void);
void yukonEvent(Display *dpy, XEvent *event);
void yukonLog(unsigned long logLevel, const char *fmt, ...);

typedef struct {
	unsigned long logLevel;
	KeySym hotkey;

	unsigned int insets[4];
	unsigned int scale;
	double fps;
	char output[4096];	
} yukonGlobalData;

extern yukonGlobalData yukonGlobal;

#endif /* __YUKON_H__ */
