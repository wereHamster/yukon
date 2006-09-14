
#include <yukonCore/yCore.h>

int yConfigOption(const char *name, char *buffer, int length)
{
	char path[4096];

	snprintf(path, 4096, "%s/.yukon/%s", getenv("HOME"), name);

	int fd = open(path, O_RDONLY);
	if (fd >= 0) {
		struct stat statBuffer;
		fstat(fd, &statBuffer);
		int size = statBuffer.st_size > length ? length : statBuffer.st_size;
		read(fd, buffer, size);
		buffer[size - 1] = 0;

		close(fd);
		
		return 0;
	} else {
		return 1;
	}
}

void yConfigServer(char server[256])
{
	if (yConfigOption("server", server, 256)) {
		strncpy(server, "127.0.0.1 9000", 256);
	}
}

void yConfigInterval(double *v)
{
	char interval[64];
	
	*v = 36000.0;
	
	if (yConfigOption("interval", interval, 64)) {
		return;
	}
	
	unsigned int ret = 0;
	int success = sscanf(interval, "%u", &ret);
	if (success) {
		*v = (double) ret;
	}
}

void yConfigScale(char scale[64])
{
	if (yConfigOption("scale", scale, 64)) {
		strncpy(scale, "half", 64);
	}
}

void yConfigInsets(uint64_t v[4])
{
	char insets[64];
	
	v[0] = v[1] = v[2] = v[3] = 0;
	
	if (yConfigOption("insets", insets, 64)) {
		return;
	}
	
	unsigned int ins[4];
	int success = sscanf(insets, "%u %u %u %u", &ins[0], &ins[1], &ins[2], &ins[3]);
	if (success) {
		v[0] = ins[0];
		v[1] = ins[1];
		v[2] = ins[2];
		v[3] = ins[3];
	}
}

void yConfigHotkey(char hotkey[64])
{
	if (yConfigOption("hotkey", hotkey, 64)) {
		strncpy(hotkey, "F8", 64);
	}
}
