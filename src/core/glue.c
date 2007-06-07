
#include <yukon.h>

yukonGlobalData yukonGlobal;

static int doCapture;
static seomClient *client;

void yukonCoreEvent(Display *dpy, XEvent *event)
{
	static Time lastEvent;

	switch (event->type) {
	case KeyPress:
		if (event->xkey.keycode == XKeysymToKeycode(dpy, yukonGlobal.hotkey)) {
			if (event->xkey.time == lastEvent)
				return;

			lastEvent = event->xkey.time;
			doCapture = !client;
			if (client) {
				seomClientDestroy(client);
				logMessage(3, "Yukon stopped capturing\n");
				client = NULL;
			} else {
				logMessage(3, "Yukon is starting to capture\n");
			}
		}
		break;
	default:
		break;
	}
}

void yukonCoreCapture(Display *dpy, GLXDrawable drawable)
{
	if (client == NULL && doCapture) {
		doCapture = 0;

		/* reload configuration in case it changed */
		updateConfiguration();

		Window root;
		unsigned int width, height, unused;
		XGetGeometry(dpy, drawable, &root, (int *)&unused, (int *)&unused, &width, &height, &unused, &unused);

		if (yukonGlobal.insets[0] + yukonGlobal.insets[2] > height || yukonGlobal.insets[1] + yukonGlobal.insets[3] > width) {
			logMessage(2, "%s(): insets too big, fix it!\n", __func__);
			return;
		}

		seomClientConfig config = {
			.size = { width - (yukonGlobal.insets[1] + yukonGlobal.insets[3]), height - (yukonGlobal.insets[0] + yukonGlobal.insets[2]) },
			.scale = yukonGlobal.scale,
			.fps = yukonGlobal.fps,
			.output = yukonGlobal.output,
		};

		client = seomClientCreate(&config);
		if (client == NULL)
			logMessage(1, "%s(): couldn't create seom client\n", __func__);
	}

	if (client)
		seomClientCapture(client, yukonGlobal.insets[3], yukonGlobal.insets[2]);
}
