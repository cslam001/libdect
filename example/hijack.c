#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/dect.h>
#include <event.h>

#include <dect/libdect.h>
#include "common.h"
#include "mac.h"
#include <lce.h>

#define BITS_PER_BYTE	8

static unsigned char buf[DECT_A_FIELD_SIZE];

static void pexit(const char *str)
{
	perror(str);
	exit(1);
}

static ssize_t dect_raw_tx(int fd, uint8_t slot, unsigned char *buf, size_t len)
{
	struct iovec iov;
	struct msghdr msg;
	struct dect_raw_auxdata *aux;
	struct cmsghdr *cmsg;
	union {
		struct cmsghdr		cmsg;
		char			buf[CMSG_SPACE(sizeof(*aux))];
	} cmsg_buf;

	msg.msg_name		= NULL;
	msg.msg_namelen		= 0;
	msg.msg_iov		= &iov;
	msg.msg_iovlen		= 1;
	msg.msg_control		= &cmsg_buf;
	msg.msg_controllen	= sizeof(cmsg_buf);
	msg.msg_flags		= 0;

	iov.iov_len		= len;
	iov.iov_base		= buf;

	cmsg			= CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_len		= CMSG_LEN(sizeof(*aux));
	cmsg->cmsg_level	= SOL_DECT;
	cmsg->cmsg_type		= DECT_RAW_AUXDATA;

	aux			= (void *)CMSG_DATA(cmsg);
	aux->mfn		= 0;
	aux->frame		= 0;
	aux->slot		= slot;

	return sendmsg(fd, &msg, 0);
}

static void raw_sock_event(struct dect_handle *dh, struct dect_fd *dfd,
			   uint32_t events)
{
	uint8_t slot = (unsigned long)dfd->data;
	unsigned int i;

	for (i = 0; ; i++) {
		printf("\rqueueing bearer information messages: %u", i);

		if (dect_raw_tx(dfd->fd, slot, buf, sizeof(buf)) < 0) {
			if (errno == EAGAIN)
				break;
			pexit("raw_tx");
		}
	}
	printf("\n");
}

static void dect_build_msg(unsigned char *buf, uint8_t sn, uint8_t cn)
{
	unsigned int i;
	uint64_t t;

	t  = DECT_PT_SHORT_PAGE;
	t |= DECT_PT_IT_RECOMMENDED_OTHER_BEARER;
	t |= (uint64_t)sn << DECT_PT_BEARER_SN_SHIFT;
	t |= (uint64_t)cn << DECT_PT_BEARER_CN_SHIFT;

	memset(buf, 0, sizeof(buf));
	for (i = 0; i < DECT_T_FIELD_SIZE; i++)
		buf[i + 1] = t >> ((sizeof(t) - i - 1) * BITS_PER_BYTE);
	buf[DECT_HDR_TA_OFF] |= DECT_TI_PT;
	buf[DECT_HDR_BA_OFF] |= DECT_BI_NONE;
}

static void page_timer(struct dect_handle *dh, struct dect_timer *timer)
{
	dect_lce_group_ring(dh, 0);
	dect_start_timer(dh, timer, 1);
}

static struct dect_ops ops;

int main(int argc, char **argv)
{
	struct sockaddr_dect da;
	struct dect_fd *dfd;
	struct dect_timer *timer;
	uint8_t slot;

	if (argc < 3) {
		printf("Usage: %s rx-slot tx-slot tx-carrier\n", argv[0]);
		exit(1);
	}
	slot = atoi(argv[1]);
	dect_build_msg(buf, atoi(argv[2]), atoi(argv[3]));

	if (dect_event_ops_init(&ops) < 0)
		pexit("dect_event_ops_init");

	dh = dect_alloc_handle(&ops);
	if (dh == NULL)
		pexit("dect_alloc_handle");

	if (dect_init(dh) < 0)
		pexit("dect_init");

	dfd = dect_socket(dh, SOCK_RAW, 0);
	if (dfd == NULL)
		pexit("dect_socket");

	memset(&da, 0, sizeof(da));
	da.dect_family = AF_DECT;
	da.dect_index  = 1;

	if (bind(dfd->fd, (struct sockaddr *)&da, sizeof(da)) < 0)
		pexit("bind");

	dect_setup_fd(dfd, raw_sock_event, (void *)(unsigned long)slot);
	if (dect_register_fd(dh, dfd, DECT_FD_WRITE) < 0)
		pexit("dect_register_fd");

	timer = dect_alloc_timer(dh);
	if (timer == NULL)
		pexit("dect_alloc_timer");
	dect_setup_timer(timer, page_timer, NULL);
	dect_start_timer(dh, timer, 1);

	dect_event_loop();

	dect_stop_timer(dh, timer);
	dect_unregister_fd(dh, dfd);
	dect_close_handle(dh);
	dect_event_ops_cleanup();
	return 0;
}
