
#include <loader.h>
#include <GL/glx.h>

static void (*core)(Display *dpy, GLXDrawable drawable);

void glXSwapBuffers(Display *dpy, GLXDrawable drawable)
{
	GLint readBuffer;
	glGetIntegerv(GL_READ_BUFFER, &readBuffer);

	glReadBuffer(GL_BACK);
	glue("yukonCoreCapture", dpy, drawable);
	glReadBuffer(readBuffer);

	chain(glXSwapBuffers, dpy, drawable);
}

#include <string.h>
void (*glXGetProcAddressARB(const GLubyte *procName)) (void)
{
	if (strcmp((char *)procName, "glXGetProcAddressARB") == 0) {
		return (void *)glXGetProcAddressARB;
	} else if (strcmp((char *)procName, "glXSwapBuffers") == 0) {
		return (void *)glXSwapBuffers;
	}

	return chain(glXGetProcAddressARB, procName);
}
