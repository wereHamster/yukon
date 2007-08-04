
#include <yukon.h>

void *streamThreadCallback(void *data)
{
	struct buffer *buffer = data;

	logMessage(4, "streamThreadCallback()\n");
	
	for (;;) {
		struct packet *packet = bufferGet(buffer);
		if (packet == NULL)
			break;

		if (packet->type == 0x01) {
			streamPutVideo(packet);
		} else if (packet->type == 0x02) {
			streamPutAudio(packet);
		}

		packetDestroy(packet);
	}
	
	logMessage(4, "end\n");

	return NULL;
}
