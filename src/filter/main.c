#include <yukon.h>

static uint32_t streamHeader[3];
static struct yukonPacket packet;
static char buffer[2500 * 2500* 4];

uint64_t getPacket(int fd, uint8_t type)
{
	read(fd, &packet, sizeof(struct yukonPacket));

	if (packet.type == type)
		return read(fd, buffer, packet.size);

	lseek(fd, packet.size, SEEK_CUR);
	return getPacket(fd, type);
}

static void writeFrame(int fd, uint32_t width, uint32_t height)
{
	static const char header[] = "FRAME\n";
	write(fd, header, sizeof(header) - 1);

	uint8_t *data = (uint8_t *) buffer;

	for (uint32_t y = height - 1; y < height; --y) {
		write(fd, data + y * width, width);
	}

	data += width * height;

	for (int i = 0; i < 2; ++i) {
		for (uint32_t y = (height / 2) - 1; y < (height / 2); --y) {
			write(fd, data + y * (width / 2), (width / 2));
		}
		data += width * height / 4;
	}
}

static int usage(char *self)
{
	return printf("Usage: %s --video|--audio yukon-stream.seom\n", self);
}

int main(int argc, char *argv[])
{
	int fps = 25;

	if (argc < 3)
		return usage(argv[0]);

	uint8_t type = 0x00;
	if (strcmp(argv[1], "--video") == 0) {
		type = 0x01;
	} else if (strcmp(argv[1], "--audio") == 0) {
		type = 0x02;
	} else {
		return usage(argv[0]);
	}

	int fd = open(argv[2], O_RDONLY);
	if (fd < 0)
		return 0;

	uint64_t len = getPacket(fd, 0x00);
	memcpy(streamHeader, buffer, len);

	if (type == 0x01) {
		char header[4096];
		int n = snprintf(header, 4096, "YUV4MPEG2 W%d H%d F%d:1 Ip\n", streamHeader[1], streamHeader[2], fps);
		write(1, header, n);
	}

	uint64_t inc = 1000000 / fps;
	uint64_t cur = packet.time;

	for (;;) {
		uint64_t size = getPacket(fd, type);
		if (size == 0)
			break;

		if (type == 0x01) {
			while (cur < packet.time) {
				writeFrame(1, streamHeader[1], streamHeader[2]);
				cur += inc;
			}
		} else if (type == 0x02) {
			write(1, buffer, size);
		}
	}

	close(fd);
}
