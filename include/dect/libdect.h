/*
 * libdect main header file
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_DECT_LIBDECT_H
#define _LIBDECT_DECT_LIBDECT_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#include <dect/identities.h>
#include <dect/auth.h>
#include <dect/lce.h>
#include <dect/cc.h>
#include <dect/mm.h>
#include <dect/ss.h>
#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

struct dect_handle;

/**
 * struct dect_msg_buf - DECT message buffer
 *
 * @list:	Data link TX queue node
 * @type:	Message type
 * @len:	Data length
 * @data:	Data pointer
 * @head:	Storage area for on-stack buffers
 */
struct dect_msg_buf {
	struct list_head	list;
	uint8_t			type;
	uint8_t			len;
	uint8_t			*data;
	uint8_t			head[128];
};

extern struct dect_msg_buf *dect_mbuf_alloc(const struct dect_handle *dh);

static inline void dect_mbuf_pull(struct dect_msg_buf *mb, unsigned int len)
{
	assert(len <= mb->len);
	mb->data += len;
	mb->len -= len;
}

static inline void dect_mbuf_push(struct dect_msg_buf *mb, unsigned int len)
{
	mb->data -= len;
	mb->len += len;
	assert(mb->data >= mb->head);
}

static inline void dect_mbuf_reserve(struct dect_msg_buf *mb, unsigned int len)
{
	mb->data += len;
	assert(mb->data < mb->head + sizeof(mb->head));
}

/**
 * @addtogroup io
 * @{
 */

/**
 * enum dect_fd_events - file descriptor events
 *
 * @arg DECT_FD_READ:	fd readable
 * @arg DECT_FD_WRITE:	fd writable
 */
enum dect_fd_events {
	DECT_FD_READ	= 0x1,
	DECT_FD_WRITE	= 0x2
};

struct dect_fd;
extern void *dect_fd_priv(struct dect_fd *dfd);
extern int dect_fd_num(const struct dect_fd *dfd);
extern void dect_handle_fd(struct dect_handle *dh, struct dect_fd *dfd,
			   uint32_t events);
/** @} */

/**
 * @addtogroup timer
 * @{
 */

struct dect_timer;
extern void *dect_timer_priv(struct dect_timer *timer);
extern void dect_run_timer(struct dect_handle *dh, struct dect_timer *timer);

/** @} */

/**
 * @addtogroup events
 * @{
 */

struct timeval;
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

struct dect_ops {
	void				*(*malloc)(size_t size);
	void				(*free)(void *ptr);

	const struct dect_event_ops	*event_ops;
	const struct dect_lce_ops	*lce_ops;
	const struct dect_cc_ops	*cc_ops;
	const struct dect_mm_ops	*mm_ops;
	const struct dect_ss_ops	*ss_ops;
};

extern struct dect_handle *dect_alloc_handle(struct dect_ops *ops);
extern void dect_close_handle(struct dect_handle *dh);

extern int dect_init(struct dect_handle *dh);

extern void dect_set_debug_hook(int (*fn)(const char *fmt, va_list ap));

#ifdef __cplusplus
extern "C" {
#endif
#endif /* _LIBDECT_DECT_LIBDECT_H */
