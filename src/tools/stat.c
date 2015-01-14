
#include <yukon.h>
#include <seom/stream.h>

struct statStream {
	int fileDescriptor;
};

static uint64_t type[4];
static uint64_t size[4];
static uint64_t tstamp[4][2];

static int usage(char *self)
{
	return printf("Usage: %s yukon-stream.seom\n", self);
}

static void dateFormat(char *buf, uint64_t time)
{
	time_t tim = time / 1000000;
	struct tm *tm = localtime(&tim);
	snprintf(buf, 64, "%04d/%02d/%02d %02d:%02d:%02d.%03d", 1900 + tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, (int) (time / 1000 % 1000));
}

static void timeFormat(char *buf, uint64_t time)
{
	uint64_t sec = time / 1000000;
	snprintf(buf, 64, "%02d:%02d:%02d.%03d", (int) (sec / 60 / 60), (int) (sec / 60 % 60), (int) (sec % 60), (int) (time / 1000 % 1000));
}

static void printStat(unsigned char t)
{
	printf("  type 0x%02x: (%llu)\n", t, type[t]);
	printf("    size: %llu\n", size[t]);

	char buf[3][64];
	dateFormat(buf[0], tstamp[t][0]);
	dateFormat(buf[1], tstamp[t][1]);
	timeFormat(buf[2], tstamp[t][1] - tstamp[t][0]);
	printf("    time: %s - %s (%s)\n", buf[0], buf[1], buf[2]);

	if (tstamp[t][1] == tstamp[t][0])
		return;

	double tt = (double) (tstamp[t][1] - tstamp[t][0]) / 1000000.0;
	double sps = (double) type[t] / tt;
	double mbs = (double) size[t] / 1024 / 1024 / tt;

	printf("    sp/s: %.1f\n", sps);
	printf("    mb/s: %.1f\n", mbs);
}

static unsigned long put(void *private, const struct iovec vec[], unsigned long num)
{
	struct statStream *stream = private;
	return writev(stream->fileDescriptor, vec, num);
}

static unsigned long get(void *private, const struct iovec vec[], unsigned long num)
{
	struct statStream *stream = private;
	return readv(stream->fileDescriptor, vec, num);
}

static struct seomStreamOps ops = { put, get };


int main(int argc, char *argv[])
{
	if (argc < 2)
		return usage(argv[0]);

	int fd = open(argv[1], O_RDONLY);
	if (fd < 0)
		return usage(argv[0]);

	static struct statStream private;
	private.fileDescriptor = fd;
	seomStream *stream = seomStreamCreate(&ops, &private);
	if (stream == NULL)
		return usage(argv[0]);

	for (;;) {
		struct seomPacket *packet = seomStreamGet(stream);
		if (packet == NULL)
			break;

		type[packet->type] += 1;
		size[packet->type] += packet->size;
		if (tstamp[packet->type][0] == 0)
			tstamp[packet->type][0] = packet->time;
		tstamp[packet->type][1] = packet->time;

		seomPacketDestroy(packet);
	}

	printf(" == Statistics:\n");

	for (unsigned char t = 0x00; t < 0x04; ++t)
		printStat(t);

	close(fd);
}
