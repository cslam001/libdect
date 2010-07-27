/*
 * libdect IO handling
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_IO_H
#define _LIBDECT_IO_H

#include <sys/socket.h>
#include <utils.h>

enum dect_fd_state {
	DECT_FD_UNREGISTERED,
	DECT_FD_REGISTERED,
};

/**
 * struct dect_fd - libdect file descriptor
 *
 * @callback:		callback to invoke for events
 * @fd:			file descriptor numer
 * @state:		file descriptor registration state (debugging)
 * @data:		libdect internal data
 * @priv:		libdect user private file-descriptor storage
 */
struct dect_fd {
	void			(*callback)(struct dect_handle *,
					    struct dect_fd *, uint32_t);
	int			fd;
	enum dect_fd_state	state;
	void			*data;
	uint8_t			priv[] __aligned(__alignof__(uint64_t));
};

extern struct dect_fd *dect_alloc_fd(const struct dect_handle *dh);
extern void dect_setup_fd(struct dect_fd *fd,
			  void (*cb)(struct dect_handle *, struct dect_fd *,
				     uint32_t),
			  void *data);
extern void dect_close(const struct dect_handle *dh, struct dect_fd *dfd);

extern struct dect_fd *dect_socket(const struct dect_handle *dh,
				   int type, int protocol);
extern struct dect_fd *dect_accept(const struct dect_handle *dh,
				   const struct dect_fd *dfd,
				   struct sockaddr *addr, socklen_t len);

extern int dect_register_fd(const struct dect_handle *dh, struct dect_fd *dfd,
			    uint32_t events);
extern void dect_unregister_fd(const struct dect_handle *dh, struct dect_fd *dfd);

#endif /* _LIBDECT_IO_H */
