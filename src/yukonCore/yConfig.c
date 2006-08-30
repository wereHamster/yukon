
#include <yukonCore/yCore.h>

static char *yConfigOption(const char *optionName)
{
	char *optionValue = 0;
	char configName[4096];

	char *homeDirectory = getenv("HOME");

	snprintf(configName, 4096, "%s/.yukon/%s", homeDirectory, optionName);

	int configFile = open(configName, O_RDONLY);
	if (configFile >= 0) {
		struct stat statBuffer;
		fstat(configFile, &statBuffer);

		if (statBuffer.st_size > 0) {
			optionValue = malloc(statBuffer.st_size + 1);
			if (optionValue) {
				read(configFile, optionValue, statBuffer.st_size);
				optionValue[statBuffer.st_size] = 0;
			}
		}

		close(configFile);
	}

	return optionValue;
}


char *yConfigServer(void)
{
	char *optionValue = 0;
	char *ret = 0;
	
	optionValue = yConfigOption("server");
	if (optionValue) {
		optionValue[strlen(optionValue) - 1] = 0;
		ret = optionValue;
	} else {
		ret = strdup("127.0.0.1 63729");
	}
	
	return ret;
}

double yConfigInterval(void)
{
	char *optionValue = 0;
	double ret = 36000.0;
	
	optionValue = yConfigOption("interval");
	if (optionValue) {
		unsigned int interval = 0;
		int success = sscanf(optionValue, "%u", &interval);
		if (success) {
			ret = (double) interval;
		}
		free(optionValue);
	}
	
	return ret;
}

char *yConfigScale(void)
{
	char *optionValue = 0;
	char *ret = 0;
	
	optionValue = yConfigOption("scale");
	if (optionValue) {
		optionValue[strlen(optionValue) - 1] = 0;
		ret = optionValue;
	} else {
		ret = strdup("half");
	}
	
	return ret;
}

void yConfigInsets(uint64_t v[4])
{
	char *optionValue = 0;
	
	v[0] = v[1] = v[2] = v[3] = 0;
	
	optionValue = yConfigOption("insets");
	if (optionValue) {
		unsigned int ins[4];
		int success = sscanf(optionValue, "%u %u %u %u", &ins[0], &ins[1], &ins[2], &ins[3]);
		if (success) {
			v[0] = ins[0];
			v[1] = ins[1];
			v[2] = ins[2];
			v[3] = ins[3];
		}
		free(optionValue);
	}
}
