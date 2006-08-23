
#ifndef __YUKON_CORE_TIME_H__
#define __YUKON_CORE_TIME_H__

typedef struct _yTime yTime;

struct _yTime {
	uint64_t sec;
	uint64_t usec;
};

void yTimeGet(yTime *time);
void yTimeAdd(yTime *a, yTime *b, yTime *res);
void yTimeSub(yTime *a, yTime *b, yTime *res);

#endif /* __YUKON_CORE_TIME_H__ */
