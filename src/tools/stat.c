
#include <yukon.h>
#include <sys/time.h>

static uint64_t type[4];
static uint64_t size[4];
static uint64_t tstamp[4][2];

static int usage(char *self)
{
	return printf("Usage: %s yukon-stream.seom\n", self);
}

static void dateFormat(char *buf, uint64_t time)
{
	time_t tim = (time_t) time / 1000000;
	struct tm *tm = localtime(&tim);
	snprintf(buf, 64, "%04d/%02d/%02d %02d:%02d:%02d.%03d", 1900 + tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, (int) (time / 1000) % 1000);
}

static void timeFormat(char *buf, uint64_t time)
{
	uint64_t sec = time / 1000000;
	snprintf(buf, 64, "%02d:%02d:%02d.%03d", sec / 60 / 60, sec / 60 % 60, sec % 60, time / 1000 % 1000);
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

		static struct yukonPacket start;
		static struct yukonPacket old;
		if (packet.type == 0x02) {
			start = packet;
		} else if (packet.type == 0x03) {
			if (packet.time - old.time > 180000) {
				char buf[64];
				timeFormat(buf, packet.time - start.time);
				fprintf(stderr, "packet diff %llu at %llu (%s)\n", packet.time - old.time, type[packet.type], buf);
			}
			old = packet;
		}
			
	}

	printf(" == Statistics:\n");

	for (unsigned char t = 0x00; t < 0x04; ++t)
		printStat(t);

	close(fd);
}
