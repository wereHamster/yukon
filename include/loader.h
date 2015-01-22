
#ifndef __LOADER_H__
#define __LOADER_H__

#define _GNU_SOURCE
#include <dlfcn.h>

#define chain(name, ...) ({ \
	static __typeof__(&name) next; \
	if (__builtin_expect(next == NULL, 0)) \
		next = (__typeof__(next)) dlsym(RTLD_NEXT, #name); \
	(*next)(__VA_ARGS__); }) \

#define glue(name, ...) ({ \
	if (__builtin_expect(core == NULL, 0)) { \
		void *handle = dlopen("yukon-core-lib", RTLD_LAZY); \
		dlerror(); \
		if (handle) \
			core = (__typeof__(core)) dlsym(handle, name); \
	} (*core)(__VA_ARGS__); }) \

#endif /* __LOADER_H__ */
