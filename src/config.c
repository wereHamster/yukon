
#include <yukon.h>

#define parse(str, key, val) \
do { \
	if (strncmp(str, #key, sizeof(#key)) == 0) \
		parse##key(val); \
} while (0) \

static int check(const char *str)
{
	return str && str[0] != '\n' && strlen(str);
}

static void parseOUTPUT(const char *str)
{
	if (check(str))
		sscanf(str, "%4096s", yukonGlobal.output);
}

static void parseFPS(const char *str)
{
	if (check(str))
		sscanf(str, "%lf", &yukonGlobal.fps);
}

static void parseINSETS(const char *str)
{
	if (check(str))
		sscanf(str, "%u %u %u %u", &yukonGlobal.insets[0], &yukonGlobal.insets[1], &yukonGlobal.insets[2], &yukonGlobal.insets[3]);
}

static void parseSCALE(const char *str)
{
	if (check(str))
		sscanf(str, "%u", &yukonGlobal.scale);
}

static void parseHOTKEY(const char *str)
{
	if (check(str)) {
		char buf[32];
		sscanf(str, "%32s", buf);
		yukonGlobal.hotkey = XStringToKeysym(buf);
	}	
}

static void parseVERBOSE(const char *str)
{
	if (check(str))
		sscanf(str, "%ul", &yukonGlobal.logLevel);
}

// TODO: Please note! This parsing function isn't perfect! Parsing in C is
// about the stupidest, dumbest thing ever and I simply refuse to write ugly
// code just to conver "fringe" use cases that probably won't result in any
// problems anyway.
static void yConfigParse(char *configfile)
{
	char buffer[1024];

	FILE *config = fopen(configfile, "r");
	char *line = NULL;
	int linenum = 1;

	if (!config)
		return;

	do {
		char *pos = NULL;

		memset(buffer, 0, sizeof(buffer));

		line = fgets(buffer, sizeof(buffer), config);

		if (line)
			pos = strchr(line, '=');

		if (pos) {
			char option[10];

			memset(option, 0, sizeof(option));

			pos++;

			sscanf(line, "%9s=", option);

			parse(option, OUTPUT, pos);
			parse(option, FPS, pos);
			parse(option, INSETS, pos);
			parse(option, SCALE, pos);
			parse(option, HOTKEY, pos);
			parse(option, VERBOSE, pos);

			linenum++;
		}
	} while (line);

	fclose(config);
}

void yukonConfig(void)
{
	/* let's set some reasonable defaults */
	parseOUTPUT("file:///tmp/yukon.seom");
	parseFPS("30.0");
	parseSCALE("0");
	parseHOTKEY("F8");

	char buffer[1024];
	sprintf(buffer, "%s/.yukon/conf", getenv("HOME"));

	yConfigParse("/etc/yukon.conf");
	yConfigParse(buffer);

	// Setup the ENV overrides.
	parseOUTPUT(getenv("YUKON_OUTPUT"));
	parseFPS(getenv("YUKON_FPS"));
	parseHOTKEY(getenv("YUKON_HOTKEY"));
	parseSCALE(getenv("YUKON_SCALE"));
	parseINSETS(getenv("YUKON_INSETS"));
	parseVERBOSE(getenv("YUKON_VERBOSE"));

	yukonLog(1,
		"Yukon setup information:\n"
		" - VERBOSE: %u\n"
		" - OUTPUT: %s\n"
		" - SCALE: %u\n"
		" - HOTKEY: %s\n"
		" - FPS: %.1f\n"
		" - INSETS: %u %u %u %u\n",
		yukonGlobal.logLevel, yukonGlobal.output, yukonGlobal.scale, XKeysymToString(yukonGlobal.hotkey), yukonGlobal.fps,
		yukonGlobal.insets[0], yukonGlobal.insets[1], yukonGlobal.insets[2], yukonGlobal.insets[3]);
}

__attribute__ ((constructor))
static void yukonConfigInit()
{
	memset(&yukonGlobal, 0, sizeof(yukonGlobal));
	yukonConfig();
}

__attribute__ ((destructor))
static void yukonConfigFini()
{
}
