
#include <yukon.h>

static struct yukonEngine *engine;
static int hotkeyPressed;

void glueEvent(Display *dpy, XEvent *event)
{
	static Time lastEvent;

	switch (event->type) {
	case KeyPress:
		if (event->xkey.keycode == XKeysymToKeycode(dpy, yukonGlobal.hotkey)) {
			if (event->xkey.time == lastEvent)
				return;

			logMessage(4, "Hotkey pressed\n");

			lastEvent = event->xkey.time;
			hotkeyPressed = !engine;
			if (engine)
				engine = yukonEngineDestroy(engine);
		}
		break;
	default:
		break;
	}
}

void glueDrawable(Display *dpy, GLXDrawable drawable)
{
	if (hotkeyPressed && engine == NULL) {
		hotkeyPressed = 0;

		/* reload configuration in case it changed */
		updateConfiguration();

		Window root;
		unsigned int width, height, unused;
		XGetGeometry(dpy, drawable, &root, (int *)&unused, (int *)&unused, &width, &height, &unused, &unused);

		unsigned long size[2] = { width, height };

		/* create the main engine */
		engine = yukonEngineCreate(yukonGlobal.output, yukonGlobal.scale, size);

		if (engine) {
			logMessage(4, "Yukon engine is active now\n");
		} else {
			logMessage(4, "Failed to create the yukon engine\n");
		}
	}

	if (engine)
		yukonEngineCapture(engine);
}
