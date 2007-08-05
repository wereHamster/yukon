
#include <yukon.h>
#include <arpa/inet.h>

struct wavHeader {
	uint32_t chunkID;
	uint32_t chunkSize;
	uint32_t riffType;
} __attribute__((packed));

struct wavFormat {
	uint32_t 	chunkID;
	uint32_t 	chunkSize;
	uint16_t 	compressionCode;
	uint16_t 	numChannels;
	uint32_t 	sampleRate;
	uint32_t 	avgBytesPerSecond;
	uint16_t 	blockAlign;
	uint16_t 	significantBitsPerSample;
} __attribute__((packed));

struct wavData {
	uint32_t 	chunkID;
	uint32_t 	chunkSize;
} __attribute__((packed));

void wavWriteHeader(int fd, struct yukonPacket *packet, void *data, unsigned int size)
{
	uint32_t *sh = data;
	unsigned int format = sh[0];

	struct wavHeader header = { htonl(0x52494646), 0xffffffff, htonl(0x57415645) };
	write(fd, &header, sizeof(struct wavHeader));

	struct wavFormat fmt = { htonl(0x666D7420), 16, 1, 2, 48000, 48000 * format * 2, format * 2, format * 8 };
	write(fd, &fmt, sizeof(struct wavFormat));

	struct wavData chunk = { htonl(0x64617461), 0xffffffff };
	write(fd, &chunk, sizeof(struct wavData));
}

void wavWriteData(int fd, struct yukonPacket *packet, void *data, unsigned int size)
{
	write(fd, data, size);
}
