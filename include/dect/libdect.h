/*
 * libdect main header file
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_DECT_LIBDECT_H
#define _LIBDECT_DECT_LIBDECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#include <dect/identities.h>
#include <dect/auth.h>
#include <dect/llme.h>
#include <dect/lce.h>
#include <dect/cc.h>
#include <dect/mm.h>
#include <dect/ss.h>
#include <dect/clms.h>
#include <dect/debug.h>
#include <list.h>

struct dect_handle;

/**
 * DECT message buffer
 *
 * @arg list	Data link TX queue node
 * @arg refcnt	Reference count
 * @arg type	Message type
 * @arg len	Data length
 * @arg data	Data pointer
 * @arg head	Storage area for on-stack buffers
 */
struct dect_msg_buf {
	struct list_head	list;
	uint8_t			refcnt;
	uint8_t			type;
	uint8_t			len;
	uint8_t			*data;
	uint8_t			head[128];
};

/** Define a dect_msg_buf on the stack and initialize it approriately. */
#define DECT_DEFINE_MSG_BUF_ONSTACK(name)	\
	struct dect_msg_buf name = {		\
		.data = name.head,		\
	}

extern struct dect_msg_buf *dect_mbuf_alloc(const struct dect_handle *dh);
extern void dect_mbuf_free(const struct dect_handle *dh, struct dect_msg_buf *mb);
extern void *dect_mbuf_pull(struct dect_msg_buf *mb, unsigned int len);
extern void *dect_mbuf_push(struct dect_msg_buf *mb, unsigned int len);
extern void dect_mbuf_reserve(struct dect_msg_buf *mb, unsigned int len);
extern void *dect_mbuf_put(struct dect_msg_buf *mb, unsigned int len);

/**
 * @addtogroup io
 * @{
 */

/** libdect file descriptor events */
enum dect_fd_events {
	DECT_FD_READ	= 0x1,	/**< file descriptor is readable */
	DECT_FD_WRITE	= 0x2	/**< file descriptor is writable */
};

struct dect_fd;
extern void *dect_fd_priv(struct dect_fd *dfd);
extern int dect_fd_num(const struct dect_fd *dfd);
extern void dect_fd_process(struct dect_handle *dh, struct dect_fd *dfd,
			    uint32_t events);
/** @} */

/**
 * @addtogroup timer
 * @{
 */

struct dect_timer;
extern void *dect_timer_priv(struct dect_timer *timer);
extern void dect_timer_run(struct dect_handle *dh, struct dect_timer *timer);

/** @} */

/**
 * @addtogroup events
 * @{
 */

struct timeval;
/**
 * Event Ops.
 *
 * The event ops are used to register callback functions for libdect event handling.
 */
struct dect_event_ops {
	size_t		fd_priv_size;
	size_t		timer_priv_size;

	int		(*register_fd)(const struct dect_handle *dh,
				       struct dect_fd *dfd,
				       uint32_t events);
	void		(*unregister_fd)(const struct dect_handle *dh,
					 struct dect_fd *dfd);

	void		(*start_timer)(const struct dect_handle *dh,
				       struct dect_timer *timer,
				       const struct timeval *tv);
	void		(*stop_timer)(const struct dect_handle *dh,
				      struct dect_timer *timer);
};

/** @} */

/**
 * DECT Ops.
 *
 * The DECT ops contain references to the individual ops of the libdect subsystems.
 */
struct dect_ops {
	void				*(*malloc)(size_t size);
	void				(*free)(void *ptr);

	const struct dect_event_ops	*event_ops;
	const struct dect_llme_ops_	*llme_ops;
	const struct dect_lce_ops	*lce_ops;
	const struct dect_cc_ops	*cc_ops;
	const struct dect_mm_ops	*mm_ops;
	const struct dect_ss_ops	*ss_ops;
	const struct dect_clms_ops	*clms_ops;
	const struct dect_raw_ops	*raw_ops;
};

extern struct dect_handle *dect_open_handle(struct dect_ops *ops,
					    const char *cluster);
extern void dect_close_handle(struct dect_handle *dh);

extern void dect_pp_set_ipui(struct dect_handle *dh,
			     const struct dect_ipui *ipui);
extern void dect_pp_set_tpui(struct dect_handle *dh,
			     const struct dect_tpui *tpui);

#ifdef __cplusplus
}
#endif
#endif /* _LIBDECT_DECT_LIBDECT_H */
