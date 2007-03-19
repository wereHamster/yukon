
#include <yukon.h>
 
void yukonLog(unsigned long logLevel, const char *fmt, ...)
{
	if (logLevel > yukonGlobal.logLevel)
		return;

	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}
