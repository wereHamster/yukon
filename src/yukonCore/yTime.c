
#include <yukonCore/yCore.h>

void yTimeGet(yTime *time)
{
	struct timeval tv;
	gettimeofday(&tv, 0);

	time->sec = tv.tv_sec;
	time->usec = tv.tv_usec;
}

void yTimeAdd(yTime *a, yTime *b, yTime *res)
{
	struct timeval tv_a;
	struct timeval tv_b;
	struct timeval tv_res;

	tv_a.tv_sec = a->sec;
	tv_a.tv_usec = a->usec;

	tv_b.tv_sec = b->sec;
	tv_b.tv_usec = b->usec;

	timeradd(&tv_a, &tv_b, &tv_res);

	res->sec = tv_res.tv_sec;
	res->usec = tv_res.tv_usec;
}

void yTimeSub(yTime *a, yTime *b, yTime *res)
{
	struct timeval tv_a;
	struct timeval tv_b;
	struct timeval tv_res;

	tv_a.tv_sec = a->sec;
	tv_a.tv_usec = a->usec;

	tv_b.tv_sec = b->sec;
	tv_b.tv_usec = b->usec;

	timersub(&tv_a, &tv_b, &tv_res);

	res->sec = tv_res.tv_sec;
	res->usec = tv_res.tv_usec;
}
