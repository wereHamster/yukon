
#include <alsa/asoundlib.h>
#include <yukon.h>
#include <alloca.h>

static int xrun(snd_pcm_t *handle, int err)
{
	if (err == -EPIPE) { /* xrun */
		return snd_pcm_prepare(handle);
	} else if (err == -ESTRPIPE) {
		while ((err = snd_pcm_resume(handle)) == -EAGAIN)
			sleep(1); /* wait until the suspend flag is released */
		if (err < 0)
			return snd_pcm_prepare(handle);
		return 0;
	} else {
		return err;
	}
}

snd_pcm_hw_params_t *getParams(snd_pcm_t *pcm)
{
	snd_pcm_hw_params_t *params;
	snd_pcm_hw_params_malloc(&params);
	snd_pcm_hw_params_any(pcm, params);

	logMessage(3, "params: access\n");
	if (snd_pcm_hw_params_set_access(pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
		goto failed;

	snd_pcm_format_mask_t *formats = alloca(snd_pcm_format_mask_sizeof());
	snd_pcm_format_mask_none(formats);
	snd_pcm_format_mask_set(formats, SND_PCM_FORMAT_S16_LE);
	snd_pcm_format_mask_set(formats, SND_PCM_FORMAT_S24_LE);
	snd_pcm_format_mask_set(formats, SND_PCM_FORMAT_S32_LE);

	logMessage(3, "params: format\n");
	if (snd_pcm_hw_params_set_format_mask(pcm, params, formats) < 0)
		goto failed;
	logMessage(3, "params: channels\n");
	if (snd_pcm_hw_params_set_channels(pcm, params, 2) < 0)
		goto failed;
	logMessage(3, "params: rate\n");
	if (snd_pcm_hw_params_set_rate(pcm, params, 48000, 0) < 0)
		goto failed;

	snd_pcm_uframes_t size;
	if (snd_pcm_hw_params_get_buffer_size_max(params, &size) < 0)
		goto failed;
	logMessage(3, "params: buffer size\n");
	if (snd_pcm_hw_params_set_buffer_size(pcm, params, size) < 0)
		goto failed;

	int dir;
	unsigned int periods;
	if (snd_pcm_hw_params_get_periods_min(params, &periods, &dir) < 0)
		goto failed;
	logMessage(3, "params: periods\n");
	if (snd_pcm_hw_params_set_periods(pcm, params, periods < 2 ? 2 : periods, dir) < 0)
		goto failed;

	return params;

failed:
	snd_pcm_hw_params_free(params);
	return NULL;
}

snd_pcm_t *openAudioDevice(const char *device, snd_pcm_uframes_t *period)
{
	snd_pcm_t *pcm= NULL;
	if (snd_pcm_open(&pcm, device, SND_PCM_STREAM_CAPTURE, 0) < 0)
		return NULL;

	snd_pcm_hw_params_t *params = getParams(pcm);
	if (params == NULL)
		goto failed;

	if (snd_pcm_hw_params(pcm, params) < 0)
		goto failed;

	snd_pcm_hw_params_get_period_size(params, period, NULL);
	snd_pcm_hw_params_free(params);

	snd_output_t *output= NULL;
	snd_output_stdio_attach(&output, stdout, 0);
	snd_pcm_dump(pcm, output);

	return pcm;

failed:
	logMessage(3, "failed to open audio device\n");

	snd_pcm_hw_params_free(params);
	snd_pcm_close(pcm);

	return NULL;
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

	uint32_t header[1] = { snd_pcm_frames_to_bytes(pcm, 1) / 2 };
	struct yukonPacket *pkt = yukonPacketCreate(0x02, sizeof(header));
	memcpy(yukonPacketPayload(pkt), &header, sizeof(header));
	yukonStreamPut(engine->stream, pkt);

	for (;;) {
		if (snd_pcm_state(pcm) == SND_PCM_STATE_PREPARED)
			snd_pcm_start(pcm);

		pthread_mutex_lock(&engine->audioMutex);
		if (engine->audioRunning == 0)
			break;
		pthread_mutex_unlock(&engine->audioMutex);

		int err = xrun(pcm, wait(pcm, ufds, count)); 
		if (err < 0)
			continue;

		struct yukonPacket *packet = yukonPacketCreate(0x03, snd_pcm_frames_to_bytes(pcm, period));
		if (packet == NULL)
			continue;

		snd_pcm_sframes_t delay;
		snd_pcm_delay(pcm, &delay);
		packet->time -= 1000000 / 48000 * delay;

		snd_pcm_sframes_t ret = snd_pcm_readi(pcm, yukonPacketPayload(packet), period);
		if (ret < 0) {
			logMessage(2, "overrun!\n");
			ret = xrun(pcm, ret);
		}

		yukonStreamPut(engine->stream, packet);
	}

	pthread_mutex_unlock(&engine->audioMutex);
	
	snd_pcm_drain(pcm);
	snd_pcm_uframes_t left = snd_pcm_avail_update(pcm);
	if (left > 0) {
		struct yukonPacket *packet = yukonPacketCreate(0x03, snd_pcm_frames_to_bytes(pcm, left));
		if (packet) {
			snd_pcm_sframes_t delay;
			snd_pcm_delay(pcm, &delay);
			packet->time -= 1000000 / 48000 * delay;

			snd_pcm_sframes_t ret = snd_pcm_readi(pcm, yukonPacketPayload(packet), left);
			if (ret < 0) {
				logMessage(2, "overrun (%d)!\n", ret);
				ret = xrun(pcm, ret);
			}

			yukonStreamPut(engine->stream, packet);
		}
	}

	snd_pcm_close(pcm);

	return NULL;
}
