
#include <yukon.h>

void y4mWriteHeader(int fd, struct seomPacket *packet);
void y4mWriteData(int fd, struct seomPacket *packet);

void wavWriteHeader(int fd, struct seomPacket *packet);
void wavWriteData(int fd, struct seomPacket *packet);

struct filterStream {
	int fileDescriptor;
};

static unsigned long put(void *private, const struct iovec vec[], unsigned long num)
{
	struct filterStream *stream = private;
	return writev(stream->fileDescriptor, vec, num);
}

static unsigned long get(void *private, const struct iovec vec[], unsigned long num)
{
	struct filterStream *stream = private;
	return readv(stream->fileDescriptor, vec, num);
}

static struct seomStreamOps ops = { put, get };

struct seomPacket *getPacket(seomStream *stream, uint8_t type)
{
	for (;;) {
		struct seomPacket *packet = seomStreamGet(stream);
		if (packet == NULL)
			return NULL;

		if (packet->type == type || packet->type == type + 1)
			return packet;

		seomPacketDestroy(packet);
	}
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

	static struct filterStream private;
	private.fileDescriptor = fd;
	seomStream *stream = seomStreamCreate(&ops, &private);
	if (stream == NULL)
		return 0;

	for (;;) {
		struct seomPacket *packet = getPacket(stream, type);
		if (packet == NULL)
			break;

		if (type == 0x00) {
			if (packet->type == 0x00) {
				y4mWriteHeader(1, packet);
			} else {
				y4mWriteData(1, packet);
			}
		} else if (type == 0x02) {
			if (packet->type == 0x02) {
				wavWriteHeader(1, packet);
			} else {
				wavWriteData(1, packet);
			}
		}

		seomPacketDestroy(packet);
	}

	close(fd);
}
