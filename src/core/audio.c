
#include <alsa/asoundlib.h>
#include <yukon.h>

static snd_pcm_t *pcm;
static snd_pcm_uframes_t periodSize = 4096;
static snd_output_t *output= NULL;

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

void setupAudio(void)
{
	snd_pcm_open(&pcm, "default", SND_PCM_STREAM_CAPTURE, 0);

	snd_output_stdio_attach(&output, stdout, 0);

	if (snd_pcm_set_params(pcm, SND_PCM_FORMAT_S32_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 2, 48000, 0, 500000) < 0) {
		fprintf (stderr, "snd_pcm_set_params() failed.\n");
		exit (-1);
	}

	snd_pcm_dump(pcm, output);
	xrun_recovery(pcm, snd_pcm_start(pcm));
}

void stopAudio(void)
{
	snd_pcm_close(pcm);
}

void *audioThreadCallback(void *data)
{
	struct buffer *buffer = data;

	for (;;) {
		if (bufferCount(buffer) > 0)
			break;

		struct packet *packet = packetCreate(0x02, periodSize * 8);
		if (packet == NULL)
			continue;
		
		snd_pcm_sframes_t ret = snd_pcm_readi(pcm, packetPayload(packet), periodSize);
		if (ret < 0) {
			logMessage(4, "xrun!!!\n");
			ret = xrun_recovery(pcm, ret);
		}
		
		multiplexerPut(packet);
	}

	return NULL;
}
