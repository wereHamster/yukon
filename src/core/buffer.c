
#include <yukon.h>

static unsigned long num(struct buffer *buffer)
{
	return ((buffer->head - buffer->tail) + buffer->size) % buffer->size;
}

struct buffer *bufferCreate(unsigned long size)
{
	struct buffer *buffer = malloc(sizeof(struct buffer) + size * sizeof(struct packet *));
	if (buffer == NULL)
		return NULL;

	pthread_mutex_init(&buffer->mutex, NULL);
	pthread_cond_init(&buffer->cond, NULL);

	buffer->size = size;
	buffer->head = buffer->tail = 0;

	return buffer;
}

void bufferPut(struct buffer *buffer, struct packet *packet)
{

	pthread_mutex_lock(&buffer->mutex);
	while (num(buffer) == buffer->size)
		pthread_cond_wait(&buffer->cond, &buffer->mutex);

	buffer->array[buffer->head] = packet;
	buffer->head = (buffer->head + 1) % buffer->size;

	pthread_mutex_unlock(&buffer->mutex);
	pthread_cond_broadcast(&buffer->cond);
}

struct packet *bufferGet(struct buffer *buffer)
{
	struct packet *packet = NULL;

	pthread_mutex_lock(&buffer->mutex);
	while (num(buffer) == 0)
		pthread_cond_wait(&buffer->cond, &buffer->mutex);

	packet = buffer->array[buffer->tail];
	buffer->tail = (buffer->tail + 1) % buffer->size;

	pthread_mutex_unlock(&buffer->mutex);
	pthread_cond_broadcast(&buffer->cond);

	return packet;
}

unsigned long bufferCount(struct buffer *buffer)
{
	unsigned long ret;

	pthread_mutex_lock(&buffer->mutex);
	ret = num(buffer);
	pthread_mutex_unlock(&buffer->mutex);

	return ret;
}

void bufferDestroy(struct buffer *buffer)
{
	pthread_mutex_destroy(&buffer->mutex);
	pthread_cond_destroy(&buffer->cond);

	free(buffer);
}
