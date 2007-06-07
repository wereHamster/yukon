
#ifndef __YUKON_H__
#define __YUKON_H__

#include <stdarg.h>
#include <linux/limits.h>

#include <seom/seom.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>

void logMessage(unsigned long level, const char *fmt, ...);
void updateConfiguration(void);

void yukonCoreCapture(Display *dpy, GLXDrawable drawable);
void yukonCoreEvent(Display *dpy, XEvent *event);

typedef struct {
	unsigned long logLevel;
	KeySym hotkey;

	unsigned int insets[4];
	unsigned int scale;
	double fps;
	char output[PATH_MAX];	
} yukonGlobalData;

extern yukonGlobalData yukonGlobal;

#endif /* __YUKON_H__ */
