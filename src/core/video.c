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

void readFramebuffer(unsigned int width, unsigned int height, void *data)
{
	glReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, data);
}


#define ri 2
#define gi 1
#define bi 0

#define scale 8
#define f(x) ((unsigned short) ((x) * (1L<<scale) + 0.5))

#define byte(ptr) ( *(unsigned char *) (ptr) )

static const unsigned short m[3][3] = {
	{ f(0.06200706), f(0.61423059), f(0.18258588) },
	{ f(0.43921569), f(0.33857195), f(0.10064373) },
	{ f(0.04027352), f(0.39894216), f(0.43921569) }
};

void frameResample(void *buf, unsigned long w, unsigned long h)
{
	for (unsigned long y = 0; y < h; y += 2) {
		for (unsigned long x = 0; x < w; x += 2) {
			#define c(xo,yo,s) ( byte(buf + (y+yo) * (w*4) + (x+xo) * 4 + s) )
			unsigned char p[2][2][3] = {
				{ { c(0,0,0), c(0,0,1), c(0,0,2) }, { c(1,0,0), c(1,0,1), c(1,0,2) } },
				{ { c(0,1,0), c(0,1,1), c(0,1,2) }, { c(1,1,0), c(1,1,1), c(1,1,2) } },
			};
			#undef c
			
			unsigned char r[3] = {
				( p[0][0][0] + p[1][0][0] + p[0][1][0] + p[1][1][0] ) / 4,
				( p[0][0][1] + p[1][0][1] + p[0][1][1] + p[1][1][1] ) / 4,
				( p[0][0][2] + p[1][0][2] + p[0][1][2] + p[1][1][2] ) / 4,
			};
			
			void *tmp = buf + ( y / 2 ) * ( w / 2) * 4 + ( x / 2 ) * 4;
			byte(tmp + 0) = r[0];
			byte(tmp + 1) = r[1];
			byte(tmp + 2) = r[2];
		}
	}
}

void frameConvert(void *dst[3], void *src, unsigned long w, unsigned long h)
{
	for (unsigned long y = 0; y < h; y += 2) {
		for (unsigned long x = 0; x < w; x += 2) {
			#define c(xo,yo,s) ( byte(src + (y+yo) * (w*4) + (x+xo) * 4 + s) )
			unsigned char p[2][2][3] = {
				{ { c(0,0,0), c(0,0,1), c(0,0,2) }, { c(0,1,0), c(0,1,1), c(0,1,2) } },
				{ { c(1,0,0), c(1,0,1), c(1,0,2) }, { c(1,1,0), c(1,1,1), c(1,1,2) } },
			};
			#undef c
			
			unsigned short r[3] = {
				p[0][0][0] + p[1][0][0] + p[0][1][0] + p[1][1][0],
				p[0][0][1] + p[1][0][1] + p[0][1][1] + p[1][1][1],
				p[0][0][2] + p[1][0][2] + p[0][1][2] + p[1][1][2],
			};
			
			#define sy(xo,yo) byte(dst[0]+(y+yo)*w+(x+xo)) = (uint8_t) ((m[0][ri] * p[xo][yo][ri] + m[0][gi] * p[xo][yo][gi] + m[0][bi] * p[xo][yo][bi]) >> scale) + 16
			sy(0,0);
			sy(1,0);
			sy(0,1);
			sy(1,1);
			#undef sy
			
			byte(dst[1]+y/2*w/2+x/2) = (uint8_t) ((-m[1][ri] * r[ri] - m[1][gi] * r[gi] + m[1][bi] * r[bi]) >> (scale + 2)) + 128;
			byte(dst[2]+y/2*w/2+x/2) = (uint8_t) ((m[2][ri] * r[ri] - m[2][gi] * r[gi] - m[2][bi] * r[bi]) >> (scale + 2)) + 128;
		}
	}
}
