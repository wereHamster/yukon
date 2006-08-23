
#ifndef __YUKON_CORE_H__
#define __YUKON_CORE_H__

#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE

#define _GNU_SOURCE
#define _BSD_SOURCE

#include <errno.h>
#include <assert.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <avcodec.h>
#include <avformat.h>

#include <yukonCore/yTime.h>
#include <yukonCore/yBuffer.h>
#include <yukonCore/yConfig.h>
#include <yukonCore/yCompressor.h>

#endif /* __YUKON_CORE_H__ */
