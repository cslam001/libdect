/*
 * libdect IO functions
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdint.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <libdect.h>
#include <utils.h>

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

void *dect_fd_priv(struct dect_fd *fd)
{
	return fd->priv;
}
EXPORT_SYMBOL(dect_fd_priv);

void dect_setup_fd(struct dect_fd *fd,
		   void (*cb)(struct dect_handle *, struct dect_fd *, uint32_t),
		   void *data)
{
	fd->callback = cb;
	fd->data = data;
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
