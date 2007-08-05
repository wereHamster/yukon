
#include <yukon.h>

static uint64_t type[4];
static uint64_t size[4];
static uint64_t tstamp[4][2];

static int usage(char *self)
{
	return printf("Usage: %s yukon-stream.seom\n", self);
}

static const char *timeFormat(uint64_t time)
{
	static char buf[64];
	time_t tim = (time_t) time / 1000000;
	struct tm *tm = localtime(&tim);
	snprintf(buf, 64, "%02d:%02d.%03d", tm->tm_min, tm->tm_sec, (int) (time / 1000) % 1000);
	return buf;
}

static void printStat(unsigned char t)
{
	printf("  type 0x%02x: (%llu)\n", t, type[t]);
	printf("    size: %llu\n", size[t]);
	printf("    time: %s\n", timeFormat(tstamp[t][1] - tstamp[t][0]));

	if (tstamp[t][1] == tstamp[t][0])
		return;

	double tt = (double) (tstamp[t][1] - tstamp[t][0]) / 1000000.0;
	double sps = (double) type[t] / tt;
	double mbs = (double) size[t] / 1024 / 1024 / tt;

	printf("    sp/s: %.1f\n", sps);
	printf("    mb/s: %.1f\n", mbs);
}

int main(int argc, char *argv[])
{
	if (argc < 2)
		return usage(argv[0]);

	int fd = open(argv[1], O_RDONLY);
	if (fd < 0)
		return usage(argv[0]);

	for (;;) {
		static struct yukonPacket packet;
		if (read(fd, &packet, sizeof(struct yukonPacket)) == 0)
			break;

		lseek(fd, packet.size, SEEK_CUR);

		type[packet.type] += 1;
		size[packet.type] += packet.size;
		if (tstamp[packet.type][0] == 0)
			tstamp[packet.type][0] = packet.time;
		tstamp[packet.type][1] = packet.time;
	}

	printf(" == Statistics:\n");

	for (unsigned char t = 0x00; t < 0x04; ++t)
		printStat(t);

	close(fd);
}
