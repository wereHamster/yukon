
#include <yukon.h>

static uint64_t targetInterval;

struct yukonEngine *yukonEngineCreate(const char *spec, unsigned long scale, unsigned long size[2])
{
	struct yukonEngine *engine = malloc(sizeof(struct yukonEngine));
	if (engine == NULL)
		return NULL;

	engine->stream = yukonStreamCreate(spec, 16);
	if (engine->stream == NULL) {
		free(engine);
		return NULL;
	}

	engine->size[0] = size[0] & ~((1 << (scale + 1)) - 1);
	engine->size[1] = size[1] & ~((1 << (scale + 1)) - 1);
	
	logMessage(4, "Capturing %u:%u\n", engine->size[0], engine->size[1]);

	pthread_create(&engine->audioThread, NULL, audioThreadCallback, engine);

	uint32_t header[4] = { scale, engine->size[0] >> scale, engine->size[1] >> scale, yukonGlobal.fps };
	struct yukonPacket *packet = yukonPacketCreate(0x00, sizeof(header));
	memcpy(yukonPacketPayload(packet), &header, sizeof(header));
	yukonStreamPut(engine->stream, packet);

	logMessage(4, "Header %u:%u\n", header[1], header[2]);

	targetInterval = 1000000 / yukonGlobal.fps;

	return engine;
}

#include <sys/time.h>

static uint64_t getTime()
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

void yukonEngineCapture(struct yukonEngine *engine)
{
	uint64_t now = getTime();
	static uint64_t lastCapture;

	unsigned long queueCount = yukonStreamStatus(engine->stream);
	targetInterval = targetInterval * 0.9 + (targetInterval + (-2.0 + queueCount) * 100.0) * 0.1;
	if (targetInterval < 1000000 / yukonGlobal.fps)
		targetInterval = 1000000 / yukonGlobal.fps;

	if (now - lastCapture < targetInterval * 0.9 || yukonStreamStatus(engine->stream) > 10)
		return;

	lastCapture = now;

	struct yukonPacket *packet = yukonPacketCreate(0x01, engine->size[0] * engine->size[1] * 4);
	if (packet == NULL)
		return;

	glReadPixels(0, 0, engine->size[0], engine->size[1], GL_BGRA, GL_UNSIGNED_BYTE, yukonPacketPayload(packet));
	yukonStreamPut(engine->stream, packet);
}

struct yukonEngine *yukonEngineDestroy(struct yukonEngine *engine)
{
	yukonStreamDestroy(engine->stream);
	free(engine);

	return NULL;
}
