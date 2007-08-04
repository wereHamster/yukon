
#include <yukon.h>
#include <sys/time.h>

static uint64_t getTime()
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

struct packet *packetCreate(unsigned char type, unsigned long size)
{
	struct packet *packet = malloc(sizeof(struct packet) + size);
	if (packet == NULL)
		return NULL;

	packet->type = type;
	packet->time = getTime();
	packet->size = size;

	return packet;
}

void *packetPayload(struct packet *packet)
{
	return (void *) (packet + 1);
}

void packetDestroy(struct packet *packet)
{
	free(packet);
}
