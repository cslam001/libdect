#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <libdect.h>
#include <utils.h>

#ifndef SOCK_NONBLOCK
#define SOCK_NONBLOCK O_NONBLOCK
#endif

void dect_hexdump(const char *prefix, const uint8_t *buf, size_t size)
{
	unsigned int i;

	for (i = 0; i < size; i++) {
		if (i % 16 == 0)
			dect_debug("%s%s: ", i ? "\n" : "", prefix);
		dect_debug("%.2x ", buf[i]);
	}
	dect_debug("\n\n");
}

void *dect_malloc(const struct dect_handle *dh, size_t size)
{
	return dh->ops->malloc(size);
}

void *dect_zalloc(const struct dect_handle *dh, size_t size)
{
	void *ptr;

	ptr = dect_malloc(dh, size);
	if (ptr != NULL)
		memset(ptr, 0, size);
	return ptr;
}

void dect_free(const struct dect_handle *dh, void *ptr)
{
	dh->ops->free(ptr);
}

struct dect_timer *dect_alloc_timer(const struct dect_handle *dh)
{
	return dect_malloc(dh, sizeof(struct dect_timer) +
			   dh->ops->event_ops->timer_priv_size);
}

void dect_setup_timer(struct dect_timer *timer,
		      void (*cb)(struct dect_handle *, struct dect_timer *),
		      void *data)
{
	timer->callback = cb;
	timer->data = data;
}

void dect_start_timer(const struct dect_handle *dh,
		      struct dect_timer *timer, unsigned int timeout)
{
	struct timeval tv = {
		.tv_sec = timeout,
	};

	timer->state = DECT_TIMER_RUNNING;
	dh->ops->event_ops->start_timer(dh, timer, &tv);
}

void dect_stop_timer(const struct dect_handle *dh, struct dect_timer *timer)
{
	dh->ops->event_ops->stop_timer(dh, timer);
	timer->state = DECT_TIMER_STOPPED;
}

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

void dect_setup_fd(struct dect_fd *fd,
		   void (*cb)(struct dect_handle *, struct dect_fd *, uint32_t),
		   void *data)
{
	fd->callback = cb;
	fd->data = data;
}

int dect_register_fd(const struct dect_handle *dh, struct dect_fd *dfd,
		     uint32_t events)
{
	return dh->ops->event_ops->register_fd(dh, dfd, events);
}

void dect_unregister_fd(const struct dect_handle *dh, struct dect_fd *dfd)
{
	dh->ops->event_ops->unregister_fd(dh, dfd);
}

void dect_close(const struct dect_handle *dh, struct dect_fd *dfd)
{
	if (dfd->fd >= 0)
		close(dfd->fd);
	dect_free(dh, dfd);
}

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
