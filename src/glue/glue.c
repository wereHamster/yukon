
#include <yukon.h>

static int hotkeyPressed;

void glueEvent(Display *dpy, XEvent *event)
{
	static Time lastEvent;

	switch (event->type) {
	case KeyPress:
		if (event->xkey.keycode == XKeysymToKeycode(dpy, yukonGlobal.hotkey)) {
			if (event->xkey.time == lastEvent)
				return;

			lastEvent = event->xkey.time;
			hotkeyPressed = 1;
			if (coreRunning())
				coreStop();
		}
		break;
	default:
		break;
	}
}

void glueDrawable(Display *dpy, GLXDrawable drawable)
{
	if (hotkeyPressed && !coreRunning()) {
		hotkeyPressed = 0;

		/* reload configuration in case it changed */
		updateConfiguration();

		/* set up threads etc */
		setupEngine(dpy, drawable);
	}

	if (coreRunning())
		captureVideo();
}
