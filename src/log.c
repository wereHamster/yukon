
#include <yukon.h>

static const char *prefix[] = {
	"DEBUG", "INFO ", "WARN ", "ERROR", "PANIC", "?????"
};

static const char *timestamp(void)
{
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);

	static char buffer[64];
	strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", tm);

	return buffer;
}
 
void yukonLog(unsigned long level, const char *fmt, ...)
{
	if (level > yukonGlobal.logLevel)
		return;

	va_list args;
	static char buffer[4096];
	int ret = snprintf(buffer, sizeof(buffer), "[ %s | %s ]: ", timestamp(), prefix[level]);

	va_start(args, fmt);
	vsnprintf(buffer + ret, sizeof(buffer) - ret, fmt, args);
	va_end(args);
	
	fprintf(stderr, buffer);
}
