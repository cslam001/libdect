/*
 * DECT RAW socket support
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/**
 * @defgroup raw Raw sockets
 *
 * libdect RAW sockets can be used to transmit or receive raw DECT
 * MAC frames. For raw frame reception, a callback function for
 * #dect_raw_ops::raw_rcv() must be provided by the user.
 *
 * @{
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/dect.h>

#include <libdect.h>
#include <utils.h>
#include <io.h>
#include <dect/raw.h>

static void dect_raw_fill_sockaddr(struct dect_handle *dh,
				   struct sockaddr_dect *da)
{
	memset(da, 0, sizeof(*da));
	da->dect_family = AF_DECT;
	da->dect_index  = dh->index;
}

/**
 * Transmit a DECT frame on the specified slot
 *
 * @param dh	libdect handle
 * @param dfd	libdect raw socket file descriptor
 * @param slot	slot number to transmit on
 * @param mb	libdect message buffer
 */
ssize_t dect_raw_transmit(struct dect_handle *dh, struct dect_fd *dfd,
			  uint8_t slot, struct dect_msg_buf *mb)
{
	struct sockaddr_dect da;
	struct iovec iov;
	struct msghdr msg;
	struct dect_raw_auxdata aux;
	struct cmsghdr *cmsg;
	union {
		struct cmsghdr		cmsg;
		char			buf[CMSG_SPACE(sizeof(aux))];
	} cmsg_buf;

	dect_raw_fill_sockaddr(dh, &da);

	msg.msg_name		= &da;
	msg.msg_namelen		= sizeof(da);
	msg.msg_iov		= &iov;
	msg.msg_iovlen		= 1;
	msg.msg_control		= &cmsg_buf;
	msg.msg_controllen	= sizeof(cmsg_buf);
	msg.msg_flags		= 0;

	iov.iov_len		= mb->len;
	iov.iov_base		= mb->data;

	cmsg			= CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_len		= CMSG_LEN(sizeof(aux));
	cmsg->cmsg_level	= SOL_DECT;
	cmsg->cmsg_type		= DECT_RAW_AUXDATA;

	aux.mfn			= 0;
	aux.frame		= 0;
	aux.slot		= slot;
	memcpy(CMSG_DATA(cmsg), &aux, sizeof(aux));

	return sendmsg(dfd->fd, &msg, 0);
}
EXPORT_SYMBOL(dect_raw_transmit);

static void dect_raw_event(struct dect_handle *dh, struct dect_fd *dfd,
			   uint32_t events)
{
	DECT_DEFINE_MSG_BUF_ONSTACK(_mb), *mb = &_mb;
	struct msghdr msg;
	struct dect_raw_auxdata *aux;
	struct cmsghdr *cmsg;
	char cmsg_buf[4 * CMSG_SPACE(16)];
	struct iovec iov;
	ssize_t len;

	dect_assert(!(events & ~DECT_FD_READ));

	msg.msg_name		= NULL;
	msg.msg_namelen		= 0;
	msg.msg_control		= cmsg_buf;
	msg.msg_controllen	= sizeof(cmsg_buf);
	msg.msg_iov		= &iov;
	msg.msg_iovlen		= 1;
	msg.msg_flags		= 0;

	iov.iov_base		= mb->data;
	iov.iov_len		= sizeof(mb->head);

	len = recvmsg(dfd->fd, &msg, 0);
	if (len < 0)
		return;
	mb->len = len;

	aux = NULL;
	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
		if (cmsg->cmsg_level != SOL_DECT)
			continue;

		switch (cmsg->cmsg_type) {
		case DECT_RAW_AUXDATA:
			if (cmsg->cmsg_len < CMSG_LEN(sizeof(*aux)))
				continue;
			aux = (struct dect_raw_auxdata *)CMSG_DATA(cmsg);
			continue;
		default:
			continue;
		}
	}

	if (aux == NULL)
		return;

	dh->ops->raw_ops->raw_rcv(dh, dfd, aux->slot, mb);
}

/**
 * Open a new DECT raw socket
 *
 * @param dh	libdect handle
 */
struct dect_fd *dect_raw_socket(struct dect_handle *dh)
{
	struct sockaddr_dect da;
	struct dect_fd *dfd;

	dfd = dect_socket(dh, SOCK_RAW, 0);
	if (dfd == NULL)
		goto err1;

	/* Only bind socket if user wants to receive packets */
	if (dh->ops->raw_ops == NULL ||
	    dh->ops->raw_ops->raw_rcv == NULL)
		goto out;

	dect_raw_fill_sockaddr(dh, &da);

	if (bind(dfd->fd, (struct sockaddr *)&da, sizeof(da)) < 0)
		goto err2;

	dect_fd_setup(dfd, dect_raw_event, dfd);
	if (dect_fd_register(dh, dfd, DECT_FD_READ) < 0)
		goto err2;
out:
	return dfd;

err2:
	dect_close(dh, dfd);
err1:
	return NULL;
}
EXPORT_SYMBOL(dect_raw_socket);

/** @} */
