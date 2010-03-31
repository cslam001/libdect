#ifndef _LIBDECT_TIMER_H
#define _LIBDECT_TIMER_H

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
	uint8_t			priv[];
};

extern struct dect_timer *dect_alloc_timer(const struct dect_handle *dh);
extern void dect_setup_timer(struct dect_timer *timer,
			     void (*cb)(struct dect_handle *, struct dect_timer *),
			     void *data);
extern void dect_start_timer(const struct dect_handle *dh,
			     struct dect_timer *timer, unsigned int timeout);
extern void dect_stop_timer(const struct dect_handle *dh, struct dect_timer *timer);
extern bool dect_timer_running(const struct dect_timer *timer);

#endif /* _LIBDECT_TIMER_H */
