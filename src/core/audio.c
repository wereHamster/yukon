
#include <alsa/asoundlib.h>
#include <yukon.h>

#define PERIOD 4096

static int xrun_recovery(snd_pcm_t *handle, int err)
{
	if (err == -EPIPE) { /* under-run */
		err = snd_pcm_prepare(handle);
		if (err < 0)
			printf("Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
		return 0;
	} else if (err == -ESTRPIPE) {
		while ((err = snd_pcm_resume(handle)) == -EAGAIN)
			sleep(1); /* wait until the suspend flag is released */
		if (err < 0) {
			err = snd_pcm_prepare(handle);
			if (err < 0)
				printf("Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
		}
		return 0;
	}
	return err;
}

void *audioThreadCallback(void *data)
{
	struct yukonEngine *engine = data;

	snd_pcm_t *pcm = NULL;
	snd_pcm_open(&pcm, "default", SND_PCM_STREAM_CAPTURE, 0);
	if (snd_pcm_set_params(pcm, SND_PCM_FORMAT_S32_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 2, 48000, 0, 500000) < 0) {
		logMessage(3, "Failed to open or configure alsa device\n");
		return NULL;
	}

	for (;;) {
		struct yukonPacket *packet = yukonPacketCreate(0x02, PERIOD * 8);
		if (packet == NULL)
			continue;

		snd_pcm_sframes_t delay;
		snd_pcm_delay(pcm, &delay);
		packet->time -= 1000000 / 48000 * delay;

		snd_pcm_sframes_t ret = snd_pcm_readi(pcm, yukonPacketPayload(packet), PERIOD);
		if (ret < 0) {
			logMessage(3, "overrun!\n");
			ret = xrun_recovery(pcm, ret);
		}
		
		yukonStreamPut(engine->stream, packet);
	}

	snd_pcm_close(pcm);

	return NULL;
}
