/*
 * DECT Keypad Protocol helpers
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdint.h>
#include <libdect.h>
#include <dect/keypad.h>
#include <utils.h>

struct dect_keypad_buffer {
	struct dect_timer	*timer;
	struct dect_ie_keypad	keypad;
	uint8_t			timeout;
	void			*priv;
	void			(*complete)(struct dect_handle *, void *,
					    struct dect_ie_keypad *);
};

static void dect_keypad_timer(struct dect_handle *dh, struct dect_timer *timer)
{
	struct dect_keypad_buffer *kb = timer->data;

	kb->complete(dh, kb->priv, &kb->keypad);
}

void dect_keypad_append(struct dect_handle *dh, struct dect_keypad_buffer *kb,
			const struct dect_ie_keypad *keypad,
			bool sending_complete)
{
	unsigned int len;

	if (keypad->len > 0)
		dect_stop_timer(dh, kb->timer);

	len = sizeof(kb->keypad.info) - kb->keypad.len;
	len = min((unsigned int)keypad->len, len);
	memcpy(kb->keypad.info + kb->keypad.len, keypad->info, len);
	kb->keypad.len += len;

	if (sending_complete || kb->keypad.len == sizeof(kb->keypad.info))
		kb->complete(dh, kb->priv, &kb->keypad);
	else if (keypad->len > 0)
		dect_start_timer(dh, kb->timer, kb->timeout);
}
EXPORT_SYMBOL(dect_keypad_append);

struct dect_keypad_buffer *
dect_keypad_buffer_init(const struct dect_handle *dh, uint8_t timeout,
			void (*complete)(struct dect_handle *, void *priv,
					 struct dect_ie_keypad *keypad),
			void *priv)
{
	struct dect_keypad_buffer *kb;

	kb = dect_zalloc(dh, sizeof(*kb));
	if (kb == NULL)
		goto err1;

	kb->timer = dect_alloc_timer(dh);
	if (kb->timer == NULL)
		goto err2;
	dect_setup_timer(kb->timer, dect_keypad_timer, kb);

	kb->complete = complete;
	kb->priv     = priv;
	kb->timeout  = timeout;
	return kb;

err1:
	dect_free(dh, kb);
err2:
	return NULL;
}
EXPORT_SYMBOL(dect_keypad_buffer_init);
