
#include <alsa/asoundlib.h>
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

static unsigned int bps;
static char silence[48000 * 4 * 2 * 8];

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

	bps = format;
}

static uint64_t streamStart;
static uint64_t dataSize;

void wavWriteData(int fd, struct yukonPacket *packet, void *data, unsigned int size)
{
	if (streamStart == 0)
		streamStart = packet->time;

	int64_t delay = packet->time - streamStart - 1000000 * dataSize / (2 * bps) / 48000;
	if (delay > 1000) {
		uint64_t insert = delay * 48000 * 2 * bps / 1000000;
		insert -= insert % (2 * bps);
		write(fd, silence, insert);
		dataSize += insert;
	}
		
	write(fd, data, size);
	dataSize += size;
}
