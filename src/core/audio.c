
#include <alsa/asoundlib.h>
#include <yukon.h>

static int xrun(snd_pcm_t *handle, int err)
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

snd_pcm_t *openAudioDevice(const char *device, snd_pcm_uframes_t *period)
{
	snd_pcm_t *pcm = NULL;
	if (snd_pcm_open(&pcm, device, SND_PCM_STREAM_CAPTURE, 0) < 0)
		return NULL;

	snd_pcm_hw_params_t *params;
	snd_pcm_hw_params_malloc(&params);
	snd_pcm_hw_params_any(pcm, params);
	
	snd_pcm_hw_params_set_access(pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(pcm, params, SND_PCM_FORMAT_S32_LE);
	snd_pcm_hw_params_set_channels(pcm, params, 2);
	snd_pcm_hw_params_set_rate(pcm, params, 48000, 0);

	snd_pcm_uframes_t size;
	snd_pcm_hw_params_get_buffer_size_max(params, &size);
	snd_pcm_hw_params_set_buffer_size(pcm, params, size);
	snd_pcm_hw_params_set_periods(pcm, params, 2, 1);

	snd_pcm_hw_params(pcm, params);
	snd_pcm_hw_params_get_period_size(params, period, NULL);

	free(params);
	
	snd_output_t *output = NULL;
	snd_output_stdio_attach(&output, stdout, 0);
	snd_pcm_dump(pcm, output);

	return pcm;
}

static int wait(snd_pcm_t *pcm, struct pollfd *ufds, unsigned int count)
{
	for (;;) {
		poll(ufds, count, -1);

		unsigned short revents;
		snd_pcm_poll_descriptors_revents(pcm, ufds, count, &revents);

		if (revents & POLLERR)
			return -EIO;
		else if (revents & POLLIN)
			return 0;
	}
}

void *audioThreadCallback(void *data)
{
	struct yukonEngine *engine = data;

	snd_pcm_uframes_t period;
	snd_pcm_t *pcm = openAudioDevice("hw:0", &period);
	if (pcm == NULL)
		return NULL;

	logMessage(4, "period: %u\n", period);

	int count = snd_pcm_poll_descriptors_count(pcm);
	struct pollfd ufds[count];

	snd_pcm_poll_descriptors(pcm, ufds, count);
	snd_pcm_start(pcm);

	for (;;) {
		int err = xrun(pcm, wait(pcm, ufds, count));
		if (err < 0)
			continue;

		struct yukonPacket *packet = yukonPacketCreate(0x02, period * 8);
		if (packet == NULL)
			continue;

		snd_pcm_sframes_t delay;
		snd_pcm_delay(pcm, &delay);
		packet->time -= 1000000 / 48000 * delay;

		snd_pcm_sframes_t ret = snd_pcm_readi(pcm, yukonPacketPayload(packet), period);
		if (ret < 0)
			ret = xrun(pcm, ret);
		
		yukonStreamPut(engine->stream, packet);
	}

	snd_pcm_close(pcm);

	return NULL;
}
