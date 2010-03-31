/*
 * libdect timer functions
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <libdect.h>
#include <utils.h>

struct dect_timer *dect_alloc_timer(const struct dect_handle *dh)
{
	return dect_zalloc(dh, sizeof(struct dect_timer) +
			   dh->ops->event_ops->timer_priv_size);
}
EXPORT_SYMBOL(dect_alloc_timer);

void dect_setup_timer(struct dect_timer *timer,
		      void (*cb)(struct dect_handle *, struct dect_timer *),
		      void *data)
{
	timer->callback = cb;
	timer->data = data;
	timer->state = DECT_TIMER_STOPPED;
}
EXPORT_SYMBOL(dect_setup_timer);

void dect_start_timer(const struct dect_handle *dh,
		      struct dect_timer *timer, unsigned int timeout)
{
	struct timeval tv = {
		.tv_sec = timeout,
	};

	/* Cancel timer if it is already running */
	if (timer->state == DECT_TIMER_RUNNING)
		dh->ops->event_ops->stop_timer(dh, timer);

	timer->state = DECT_TIMER_RUNNING;
	dh->ops->event_ops->start_timer(dh, timer, &tv);
}
EXPORT_SYMBOL(dect_start_timer);

void dect_stop_timer(const struct dect_handle *dh, struct dect_timer *timer)
{
	dh->ops->event_ops->stop_timer(dh, timer);
	timer->state = DECT_TIMER_STOPPED;
}
EXPORT_SYMBOL(dect_stop_timer);

bool dect_timer_running(const struct dect_timer *timer)
{
	return timer->state == DECT_TIMER_RUNNING;
}
EXPORT_SYMBOL(dect_timer_running);
