/*
 * libdect timer functions
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/**
 * @defgroup events Event handling
 * @{
 *
 * @defgroup timer Timers
 *
 * libdect timers.
 *
 * libdect uses various timers internally. The application using libdect must
 * register the callback functions dect_event_ops::start_timer() and
 * dect_event_ops::stop_timer() to allow libdect to register it's timers with
 * the application's event handler. When a timeout occurs, the function
 * dect_timer_run() must be invoked.
 *
 * Each libdect timer contains a storage area of the size specified in
 * dect_event_ops::timer_priv_size, which can be used by the application to
 * associate data with the timer. The function dect_timer_priv() returns
 * a pointer to this data area.
 *
 * @{
 */

#include <sys/time.h>

#include <libdect.h>
#include <utils.h>
#include <timer.h>

struct dect_timer *dect_timer_alloc(const struct dect_handle *dh)
{
	struct dect_timer *timer;

	timer = dect_zalloc(dh, sizeof(struct dect_timer) +
			    dh->ops->event_ops->timer_priv_size);
	if (timer != NULL)
		timer->state = DECT_TIMER_STOPPED;

	return timer;
}
EXPORT_SYMBOL(dect_timer_alloc);

void dect_timer_free(const struct dect_handle *dh, struct dect_timer *timer)
{
	if (timer != NULL)
		dect_assert(timer->state == DECT_TIMER_STOPPED);
	dect_free(dh, timer);
}
EXPORT_SYMBOL(dect_timer_free);

/**
 * Get a pointer to the private data area from a DECT timer
 *
 * @param timer		DECT timer
 */
void *dect_timer_priv(struct dect_timer *timer)
{
	return timer->priv;
}
EXPORT_SYMBOL(dect_timer_priv);

void dect_timer_setup(struct dect_timer *timer,
		      void (*cb)(struct dect_handle *, struct dect_timer *),
		      void *data)
{
	dect_assert(timer->state == DECT_TIMER_STOPPED);
	timer->callback	= cb;
	timer->data	= data;
}
EXPORT_SYMBOL(dect_timer_setup);

void dect_timer_start(const struct dect_handle *dh,
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
EXPORT_SYMBOL(dect_timer_start);

void dect_timer_stop(const struct dect_handle *dh, struct dect_timer *timer)
{
	dect_assert(timer->state == DECT_TIMER_RUNNING);
	dh->ops->event_ops->stop_timer(dh, timer);
	timer->state = DECT_TIMER_STOPPED;
}
EXPORT_SYMBOL(dect_timer_stop);

bool dect_timer_running(const struct dect_timer *timer)
{
	return timer->state == DECT_TIMER_RUNNING;
}
EXPORT_SYMBOL(dect_timer_running);

/**
 * Run the timer on timeout expiration
 *
 * @param dh		libdect DECT handle
 * @param timer		DECT timer
 */
void dect_timer_run(struct dect_handle *dh, struct dect_timer *timer)
{
	dect_assert(timer->state == DECT_TIMER_RUNNING);
	timer->state = DECT_TIMER_STOPPED;
	timer->callback(dh, timer);
}
EXPORT_SYMBOL(dect_timer_run);

/** @} */
/** @} */
