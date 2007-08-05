
#include <yukon.h>

static struct yukonPacket packet;
static char buffer[2500 * 2500* 4];

void y4mWriteHeader(int fd, struct yukonPacket *packet, void *data, unsigned int size);
void y4mWriteData(int fd, struct yukonPacket *packet, void *data, unsigned int size);

void wavWriteHeader(int fd, struct yukonPacket *packet, void *data, unsigned int size);
void wavWriteData(int fd, struct yukonPacket *packet, void *data, unsigned int size);

uint64_t getPacket(int fd, uint8_t type)
{
	if (read(fd, &packet, sizeof(struct yukonPacket)) == 0)
		return 0;

	if (packet.type == type || packet.type == type + 1)
		return read(fd, buffer, packet.size);

	lseek(fd, packet.size, SEEK_CUR);
	return getPacket(fd, type);
}

static int usage(char *self)
{
	return printf("Usage: %s --video|--audio yukon-stream.seom\n", self);
}

int main(int argc, char *argv[])
{
	if (argc < 3)
		return usage(argv[0]);

	uint8_t type = 0x00;
	if (strcmp(argv[1], "--video") == 0) {
		type = 0x00;
	} else if (strcmp(argv[1], "--audio") == 0) {
		type = 0x02;
	} else {
		return usage(argv[0]);
	}

	int fd = open(argv[2], O_RDONLY);
	if (fd < 0)
		return 0;

	for (;;) {
		uint64_t size = getPacket(fd, type);
		if (size == 0)
			break;

		if (type == 0x00) {
			if (packet.type == 0x00) {
				y4mWriteHeader(1, &packet, buffer, size);
			} else {
				y4mWriteData(1, &packet, buffer, size);
			}
		} else if (type == 0x02) {
			if (packet.type == 0x02) {
				wavWriteHeader(1, &packet, buffer, size);
			} else {
				wavWriteData(1, &packet, buffer, size);
			}
		}
	}

	close(fd);
}
