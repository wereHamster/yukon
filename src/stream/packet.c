
#include <stream.h>
#include <sys/time.h>

static uint64_t getTime()
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

struct yukonPacket *yukonPacketCreate(unsigned char type, unsigned long size)
{
	struct yukonPacket *packet = malloc(sizeof(struct yukonPacket) + size);
	if (packet == NULL)
		return NULL;

	packet->type = type;
	packet->time = getTime();
	packet->size = size;

	return packet;
}

void *yukonPacketPayload(struct yukonPacket *packet)
{
	return (void *) (packet + 1);
}

void yukonPacketDestroy(struct yukonPacket *packet)
{
	free(packet);
}
