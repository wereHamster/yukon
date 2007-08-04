
#include <yukon.h>

#define call(funcName, ...) \
	static __typeof__(&funcName) func; \
	if (__builtin_expect(func == NULL, 0)) \
		func = (__typeof__(func)) getProcAddress((const GLubyte *) #funcName); \
	if (__builtin_expect(func != NULL, 1)) \
		(*func)(__VA_ARGS__); \
 	
static void (*getProcAddress(const GLubyte *procName))(void)
{
	static __typeof__(&glXGetProcAddressARB) func;
	if (__builtin_expect(func == NULL, 0)) {
		void *handle = dlopen("libGL.so.1", RTLD_LAZY);
		func = dlsym(handle, "glXGetProcAddressARB");
	} if (__builtin_expect(func != NULL, 1))
		return (*func)(procName);
	return NULL;
}

void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{
	call(glReadPixels, x, y, width, height, format, type, pixels);
}
