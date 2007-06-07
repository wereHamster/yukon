
#include <yukon.h>

static char executableName[4096];

#define parse(str, key, val) \
do { \
	if (strncmp(str, #key, sizeof(#key)) == 0 && check(val)) \
		parse##key(val); \
} while (0) \

static int check(const char *str)
{
	return str && str[0] != '\n' && strlen(str);
}

static void parseOUTPUT(const char *str)
{
	sscanf(str, "%4096s", yukonGlobal.output);
}

static void parseFPS(const char *str)
{
	sscanf(str, "%lf", &yukonGlobal.fps);
}

static void parseINSETS(const char *str)
{
	sscanf(str, "%u %u %u %u", &yukonGlobal.insets[0], &yukonGlobal.insets[1], &yukonGlobal.insets[2], &yukonGlobal.insets[3]);
}

static void parseSCALE(const char *str)
{
	sscanf(str, "%u", &yukonGlobal.scale);
}

static void parseHOTKEY(const char *str)
{
	char buf[32];
	sscanf(str, "%32s", buf);
	yukonGlobal.hotkey = XStringToKeysym(buf);
}

static void parseVERBOSE(const char *str)
{
	sscanf(str, "%lu", &yukonGlobal.logLevel);
}

// TODO: Please note! This parsing function isn't perfect! Parsing in C is
// about the stupidest, dumbest thing ever and I simply refuse to write ugly
// code just to conver "fringe" use cases that probably won't result in any
// problems anyway.
static void merge(const char *fmt, ...)
{
	va_list args;
	char buffer[4096];

	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	logMessage(4, "Merging configuration from '%s'\n", buffer);

	FILE *config = fopen(buffer, "r");
	if (!config)
		return;

	char *line = NULL;
	do {
		line = fgets(buffer, sizeof(buffer), config);
		if (line == NULL)
			break;

		char *pos = strchr(line, '=');
		if (pos) {
			++pos;

			char option[10];
			sscanf(line, "%9s=", option);

			parse(option, OUTPUT, pos);
			parse(option, FPS, pos);
			parse(option, INSETS, pos);
			parse(option, SCALE, pos);
			parse(option, HOTKEY, pos);
			parse(option, VERBOSE, pos);
		}
	} while (line);

	fclose(config);
}

void updateConfiguration(void)
{
	memset(&yukonGlobal, 0, sizeof(yukonGlobal));

	/* let's set some reasonable defaults */
	parseOUTPUT("file:///tmp/yukon.seom");
	parseHOTKEY("F8");
	parseFPS("30.0");
	parseVERBOSE("1");

	merge("/etc/yukon.conf");
	merge("%s/.yukon/conf", getenv("HOME"));
	merge("%s/.yukon/profiles/%s/conf", getenv("HOME"), executableName);

	logMessage(3, "Active configuration (log-level: %u):\n", yukonGlobal.logLevel);
	logMessage(3, "   HOTKEY: %s, FPS: %.1f, INSETS: %u/%u/%u/%u\n", XKeysymToString(yukonGlobal.hotkey), yukonGlobal.fps, yukonGlobal.insets[0], yukonGlobal.insets[1], yukonGlobal.insets[2], yukonGlobal.insets[3]);
	logMessage(3, "   OUTPUT: %s, SCALE: %u\n", yukonGlobal.output, yukonGlobal.scale);
}

static void __attribute__((constructor)) setup(void)
{
	/* extract the executable name from /proc/self/cmdline */
	int fd = open("/proc/self/cmdline", O_RDONLY);
	read(fd, executableName, sizeof(executableName));
	close(fd);

	char *slash = strrchr(executableName, '/');
	if (slash)
		memcpy(executableName, slash + 1, sizeof(executableName) - (slash - executableName));

	char *dot = strrchr(executableName, '.');
	if (dot)
		*dot = '\0';

	updateConfiguration();
}
