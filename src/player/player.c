
#include <yukon.h>

static char buffer[1920 * 1200 * 4];

uint64_t readPacket(int fd, void *buffer)
{
	struct packet packet;
	read(fd, &packet, sizeof(struct packet));
	if (packet.type == 0x02)
		return read(fd, buffer, packet.size);
	return readPacket(fd, buffer);
}

int main(int argc, char *argv[])
{
	if (argc != 3)
		return 0;

	int fd = open(argv[2], O_RDONLY);
	if (fd < 0)
		return 0;

	for (;;) {
		uint64_t ps = readPacket(fd, buffer);
		if (ps == 0)
			break;

		write(1, buffer, ps);
	}
	
	close(fd);
}
