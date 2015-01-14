
#include <stream.h>
#include <stdio.h>

void streamFrameResample(void *buf, unsigned long w, unsigned long h);
void streamFrameConvert(void *dst[3], void *src, unsigned long w, unsigned long h);

static struct seomPacket * copyFrame(struct seomPacket *packet, uint32_t scale, uint32_t size[2])
{
	struct seomPacket *ret = seomPacketCreate(packet->type, size[0] * size[1] * 3 / 2);
	if (ret == NULL)
		return packet;

	void *dst[3] = { seomPacketPayload(ret), seomPacketPayload(ret) + size[0] * size[1], seomPacketPayload(ret) + size[0] * size[1] * 5 / 4 };
	uint32_t tmp[2] = { size[0] << scale, size[1] << scale };

	while (scale--) {
		streamFrameResample(seomPacketPayload(packet), tmp[0], tmp[1]);
		tmp[0] >>= 1; tmp[1] >>= 1;
	};

	streamFrameConvert(dst, seomPacketPayload(packet), tmp[0], tmp[1]);
	seomPacketDestroy(packet);

	return ret;
}

static void *streamMultiplexerThread(void *data)
{
	struct yukonStream *stream = data;

	uint32_t header[4];

	for (;;) {
		struct seomPacket *packet = yukonBufferGet(stream->buffer);
		if (packet == NULL)
			break;
		
		switch (packet->type) {
		case 0x00:
			memcpy(&header, seomPacketPayload(packet), sizeof(header));
			seomStreamPut(stream->stream, packet);
			break;
		case 0x01:
			packet = copyFrame(packet, header[0], header + 1);
			seomStreamPut(stream->stream, packet);
			break;
		case 0x02:
			seomStreamPut(stream->stream, packet);
			break;
		case 0x03:
			seomStreamPut(stream->stream, packet);
			break;
		default:
			break;
		}
	}

	return NULL;
}

static unsigned long put(void *private, const struct iovec vec[], unsigned long num)
{
	struct yukonStream *stream = private;
	return writev(stream->fileDescriptor, vec, num);
}

static unsigned long get(void *private, const struct iovec vec[], unsigned long num)
{
	struct yukonStream *stream = private;
	return readv(stream->fileDescriptor, vec, num);
}

static struct seomStreamOps ops = { put, get };

struct yukonStream *yukonStreamCreate(const char *spec, unsigned long size)
{
	struct yukonStream *stream = malloc(sizeof(struct yukonStream));
	if (stream == NULL){
	  printf("[yukonStreamCreate] couldn't create stream\n");
	  return NULL;
	}

	stream->fileDescriptor = -1;
	if (strncmp(spec, "file://", 7) == 0) {
	        printf("[yukonStreamCreate] is a file://!, %s", &spec[7]); 
		stream->fileDescriptor = open(&spec[7], O_WRONLY | O_CREAT | O_TRUNC, 0664);
	} else if (strncmp(spec, "ipv4://", 7) == 0) {
		struct sockaddr_in addr = {
			.sin_family = AF_INET,
			.sin_port = htons(42803),
			.sin_addr.s_addr = inet_addr(&spec[7])
		};

		stream->fileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
		if (connect(stream->fileDescriptor, (const struct sockaddr *) &addr, sizeof(addr)) < 0) {
			close(stream->fileDescriptor);
			perror("connect");
			stream->fileDescriptor = -1;
		}
	}
	
	if (stream->fileDescriptor < 0) {
	        printf("[yukonStreamCreate] fileDescriptor is less than zero, couldn't create file!\n");
		free(stream);
		return NULL;
	}

	stream->stream = seomStreamCreate(&ops, stream);
	if (stream->stream == NULL) {
	        printf("[yukonStreamCreate] couldn't create stream\n");
		free(stream);
		return NULL;
	}

	stream->buffer = yukonBufferCreate(size);
	if (stream->buffer == NULL) {
	        printf("[yukonStreamCreate] couldn't create buffer\n");
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

void yukonStreamPut(struct yukonStream *stream, struct seomPacket *packet)
{
	yukonBufferPut(stream->buffer, packet);
}

void yukonStreamDestroy(struct yukonStream *stream)
{
	yukonBufferPut(stream->buffer, NULL);
	pthread_join(stream->multiplexerThread, NULL);
	yukonBufferDestroy(stream->buffer);
	seomStreamDestroy(stream->stream);
	close(stream->fileDescriptor);
	free(stream);
}
