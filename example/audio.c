#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>
#include "common.h"

void dect_audio_queue(struct dect_audio_handle *ah, struct dect_msg_buf *mb)
{
	SDL_LockAudio();
	list_add_tail(&mb->list, &ah->queue);
	SDL_UnlockAudio();
}

static void dect_decode_g721(struct g72x_state *codec,
			     int16_t *dst, const uint8_t *src,
			     unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len * 2; i += 2) {
		dst[i + 0] = g721_decoder(src[i / 2] >> 4,
					  AUDIO_ENCODING_LINEAR, codec);
		dst[i + 1] = g721_decoder(src[i / 2] & 0x0f,
					  AUDIO_ENCODING_LINEAR, codec);
	}
}

static void dect_audio_dequeue(void *data, uint8_t *stream, int len)
{
	struct dect_audio_handle *ah = data;
	struct dect_msg_buf *mb;
	int copy;

	len /= 4;
	while (1) {
		if (list_empty(&ah->queue))
			goto underrun;
		mb = list_first_entry(&ah->queue, struct dect_msg_buf, list);
		copy = mb->len;
		if (copy > len)
			copy = len;

		dect_decode_g721(&ah->codec, (int16_t *)stream, mb->data, copy);
		dect_mbuf_pull(mb, copy);
		if (mb->len == 0) {
			list_del(&mb->list);
			free(mb);
		}

		len -= copy;
		if (len == 0)
			return;
		stream += 4 * copy;
	}

underrun:
	printf("audio underrun, missing %u bytes\n", len * 4);
	memset(stream, 0, len * 4);
}

struct dect_audio_handle *dect_audio_open(void)
{
	struct dect_audio_handle *ah;
	SDL_AudioSpec spec = {
		.freq		= 8000,
		.format		= AUDIO_S16SYS,
		.channels	= 1,
		.samples	= 512,
		.callback	= dect_audio_dequeue,
	};

	ah = malloc(sizeof(*ah));
	if (ah == NULL)
		goto err1;
	init_list_head(&ah->queue);
	g72x_init_state(&ah->codec);

	spec.userdata = ah;
	if (SDL_OpenAudio(&spec, NULL) < 0)
		goto err2;
	SDL_PauseAudio(0);

	return ah;

err2:
	free(ah);
err1:
	return NULL;
}

static void __init dect_audio_init(void)
{
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
		pexit("SDL_Init");
}
