
#include <stream.h>

void streamFrameResample(void *buf, unsigned long w, unsigned long h);
void streamFrameConvert(void *dst[3], void *src, unsigned long w, unsigned long h);

static char buffer[2500 * 2500 * 4];

static uint64_t copyFrame(struct yukonPacket *packet, uint32_t scale, uint32_t size[2])
{
	void *dst[3] = { buffer, buffer + size[0] * size[1], buffer + size[0] * size[1] * 5 / 4 };
	uint32_t tmp[2] = { size[0] << scale, size[1] << scale };

	while (scale--) {
		streamFrameResample(yukonPacketPayload(packet), tmp[0], tmp[1]);
		tmp[0] >>= 1; tmp[1] >>= 1;
	};

	streamFrameConvert(dst, yukonPacketPayload(packet), tmp[0], tmp[1]);

	return size[0] * size[1] * 3 / 2;
}

static void *streamMultiplexerThread(void *data)
{
	struct yukonStream *stream = data;

	uint32_t header[3];

	for (;;) {
		struct yukonPacket *packet = yukonBufferGet(stream->buffer);
		if (packet == NULL)
			break;
		
		switch (packet->type) {
		case 0x00:
			memcpy(&header, yukonPacketPayload(packet), sizeof(header));
			write(stream->fileDescriptor, packet, sizeof(struct yukonPacket));
			write(stream->fileDescriptor, yukonPacketPayload(packet), packet->size);
			break;
		case 0x01:
			packet->size = copyFrame(packet, header[0], header + 1);
			write(stream->fileDescriptor, packet, sizeof(struct yukonPacket));
			write(stream->fileDescriptor, buffer, packet->size);
			break;
		case 0x02:
			write(stream->fileDescriptor, packet, sizeof(struct yukonPacket));
			write(stream->fileDescriptor, yukonPacketPayload(packet), packet->size);
			break;
		default:
			break;
		}

		yukonPacketDestroy(packet);
	}

	return NULL;
}

struct yukonStream *yukonStreamCreate(const char *spec, unsigned long size)
{
	struct yukonStream *stream = malloc(sizeof(struct yukonStream));
	if (stream == NULL)
		return NULL;

	stream->buffer = yukonBufferCreate(size);
	if (stream->buffer == NULL) {
		free(stream);
		return NULL;
	}

	if (strncmp(spec, "file://", 7) == 0) {
		stream->fileDescriptor = open(&spec[7], O_WRONLY | O_CREAT | O_TRUNC, 0664);
	} else {
		stream->fileDescriptor = -1;
	}
	
	if (stream->fileDescriptor < 0) {
		free(stream);
		return NULL;
	}

	pthread_create(&stream->multiplexerThread, NULL, streamMultiplexerThread, stream);

	return stream;
}

unsigned long yukonStreamStatus(struct yukonStream *stream)
{
	return yukonBufferCount(stream->buffer);
}

void yukonStreamPut(struct yukonStream *stream, struct yukonPacket *packet)
{
	yukonBufferPut(stream->buffer, packet);
}

void yukonStreamDestroy(struct yukonStream *stream)
{
	yukonBufferPut(stream->buffer, NULL);
	pthread_join(stream->multiplexerThread, NULL);
	yukonBufferDestroy(stream->buffer);
	close(stream->fileDescriptor);
	free(stream);
}
