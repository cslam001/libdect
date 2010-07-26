/*
 * libdect IO functions
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/**
 * @addtogroup events
 * @{
 *
 * @defgroup io I/O
 *
 * libdect file and socket I/O.
 *
 * libdect uses various file descriptors for I/O internally. The application
 * using libdect must register the callback functions
 * dect_event_ops::register_fd() and dect_event_ops::unregister_fd() in
 * struct dect_event_ops to allow libdect to register it's file descriptors
 * with the application's event handler. The function dect_fd_num() can be used
 * to get the file decriptor number. When an event occurs, the function
 * dect_handle_fd() must be invoked with a bitmask of enum #dect_fd_events
 * specifying the events that occured. All events except the file descriptor
 * becoming writable map to #DECT_FD_READ.
 *
 * Each libdect file descriptor contains a storage area of the size specified
 * in dect_event_ops::fd_priv_size, which can be used by the application to
 * associate data with the file descriptor. The function dect_fd_priv() returns
 * a pointer to this data area.
 *
 * @{
 */

#include <stdint.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <libdect.h>
#include <utils.h>
#include <io.h>

#ifndef SOCK_NONBLOCK
#define SOCK_NONBLOCK O_NONBLOCK
#endif

struct dect_fd *dect_alloc_fd(const struct dect_handle *dh)
{
	struct dect_fd *dfd;

	dfd = dect_malloc(dh, sizeof(struct dect_fd) +
			  dh->ops->event_ops->fd_priv_size);
	if (dfd == NULL)
		return NULL;
	dfd->fd = -1;
	return dfd;
}
EXPORT_SYMBOL(dect_alloc_fd);

/**
 * Get a pointer to the private data from a DECT file descriptor
 *
 * @param dfd		DECT file descriptor
 */
void *dect_fd_priv(struct dect_fd *dfd)
{
	return dfd->priv;
}
EXPORT_SYMBOL(dect_fd_priv);

/**
 * Get the file descriptor number from a DECT file descriptor.
 *
 * @param dfd		DECT file descriptor
 */
int dect_fd_num(const struct dect_fd *dfd)
{
	return dfd->fd;
}
EXPORT_SYMBOL(dect_fd_num);

void dect_setup_fd(struct dect_fd *dfd,
		   void (*cb)(struct dect_handle *, struct dect_fd *, uint32_t),
		   void *data)
{
	dfd->callback = cb;
	dfd->data = data;
}
EXPORT_SYMBOL(dect_setup_fd);

int dect_register_fd(const struct dect_handle *dh, struct dect_fd *dfd,
		     uint32_t events)
{
	return dh->ops->event_ops->register_fd(dh, dfd, events);
}
EXPORT_SYMBOL(dect_register_fd);

void dect_unregister_fd(const struct dect_handle *dh, struct dect_fd *dfd)
{
	dh->ops->event_ops->unregister_fd(dh, dfd);
}
EXPORT_SYMBOL(dect_unregister_fd);

/**
 * Process DECT file descriptor events
 *
 * @param dh		libdect DECT handle
 * @param dfd		DECT file descriptor
 * @param events	Bitmask of file descriptor events (#dect_fd_events)
 *
 * Process the events specified by the events bitmask for the given file
 * descriptor.
 */
void dect_handle_fd(struct dect_handle *dh, struct dect_fd *dfd, uint32_t events)
{
	dfd->callback(dh, dfd, events);
}
EXPORT_SYMBOL(dect_handle_fd);

void dect_close(const struct dect_handle *dh, struct dect_fd *dfd)
{
	if (dfd->fd >= 0)
		close(dfd->fd);
	dect_free(dh, dfd);
}
EXPORT_SYMBOL(dect_close);

struct dect_fd *dect_socket(const struct dect_handle *dh, int type, int protocol)
{
	struct dect_fd *dfd;

	dfd = dect_alloc_fd(dh);
	if (dfd == NULL)
		goto err1;

	dfd->fd = socket(AF_DECT, type | SOCK_NONBLOCK, protocol);
	if (dfd->fd < 0)
		goto err2;

	return dfd;

err2:
	dect_close(dh, dfd);
err1:
	return NULL;
}

struct dect_fd *dect_accept(const struct dect_handle *dh,
			    const struct dect_fd *dfd,
			    struct sockaddr *addr, socklen_t len)
{
	struct dect_fd *nfd;

	nfd = dect_alloc_fd(dh);
	if (nfd == NULL)
		goto err1;

	nfd->fd = accept(dfd->fd, addr, &len);
	if (nfd->fd < 0)
		goto err2;
	if (fcntl(nfd->fd, F_SETFL, O_NONBLOCK) < 0)
		goto err2;

	return nfd;

err2:
	dect_close(dh, nfd);
err1:
	return NULL;
}

/** @} */
/** @} */
