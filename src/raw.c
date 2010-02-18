/*
 * DECT RAW socket support
 *
 * Copyright (c) 2009 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
#include <dect/raw.h>

ssize_t dect_raw_transmit(const struct dect_fd *dfd, uint8_t slot,
			  const struct dect_msg_buf *mb)
{
	struct iovec iov;
	struct msghdr msg;
	struct dect_raw_auxdata aux;
	struct cmsghdr *cmsg;
	union {
		struct cmsghdr		cmsg;
		char			buf[CMSG_SPACE(sizeof(aux))];
	} cmsg_buf;

	msg.msg_name		= NULL;
	msg.msg_namelen		= 0;
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

struct dect_fd *dect_raw_socket(struct dect_handle *dh)
{
	struct sockaddr_dect da;
	struct dect_fd *dfd;

	dfd = dect_socket(dh, SOCK_RAW, 0);
	if (dfd == NULL)
		goto err1;

	memset(&da, 0, sizeof(da));
	da.dect_family = AF_DECT;
	da.dect_index  = dh->index;

	if (bind(dfd->fd, (struct sockaddr *)&da, sizeof(da)) < 0)
		goto err2;

	return dfd;

err2:
	dect_close(dh, dfd);
err1:
	return NULL;
}
