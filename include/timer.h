/*
 * libdect timer handling
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_TIMER_H
#define _LIBDECT_TIMER_H

#include <utils.h>
#include <dect/timer.h>

struct dect_handle;

enum dect_timer_state {
	DECT_TIMER_STOPPED,
	DECT_TIMER_RUNNING,
};

/**
 * struct dect_timer - libdect timer
 *
 * @callback:		callback to invoke on timer expiry
 * @data:		libdect internal data
 * @state:		libdect internal state
 * @priv:		libdect user private timer storage
 */
struct dect_timer {
	void			(*callback)(struct dect_handle *,
					    struct dect_timer *);
	void			*data;
	enum dect_timer_state	state;
	uint8_t			priv[] __aligned(__alignof__(uint64_t));
};

#endif /* _LIBDECT_TIMER_H */
