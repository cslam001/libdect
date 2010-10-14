#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

#include <dect/libdect.h>
#include <dect/raw.h>
#include <timer.h>
#include <io.h>
#include "common.h"
#include "mac.h"
#include <lce.h>

#define BITS_PER_BYTE	8

static struct dect_msg_buf msg = { .data = msg.head };

static void mm_locate_ind(struct dect_handle *dh,
                          struct dect_mm_endpoint *mme,
                          struct dect_mm_locate_param *param)
{
	struct dect_mm_locate_param reply = {};

	dect_mm_locate_res(dh, mme, false, &reply);
}

static struct dect_mm_ops mm_ops = {
	.mm_locate_ind		= mm_locate_ind,
};

static bool lce_page_response(struct dect_handle *dh, struct dect_lce_page_param *param)
{
	struct dect_ie_info_type info_type;
	struct dect_mm_info_param req = { .info_type = &info_type };
	struct dect_mm_endpoint *mme;

	mme = dect_mm_endpoint_alloc(dh, &param->portable_identity->ipui);
	if (mme == NULL)
		return false;

	info_type.num = 1;
	info_type.type[0] = DECT_INFO_LOCATE_SUGGEST;
	dect_mm_info_req(dh, mme, &req);
	return true;
}

static struct dect_lce_ops lce_ops = {
	.lce_page_response	= lce_page_response,
};

static void raw_sock_event(struct dect_handle *dh, struct dect_fd *dfd,
			   uint32_t events)
{
	void *priv = dfd->data;
	uint8_t slot = (unsigned long)priv;
	unsigned int i;

	for (i = 0; ; i++) {
		printf("\rqueueing bearer information messages: %u", i);

		if (dect_raw_transmit(dh, dfd, slot, &msg) < 0) {
			if (errno == EAGAIN)
				break;
			pexit("raw_tx");
		}
	}
	printf("\n");
}

static void dect_build_msg(struct dect_msg_buf *mb, uint8_t sn, uint8_t cn)
{
	unsigned int i;
	uint64_t t;

	t  = DECT_PT_SHORT_PAGE;
	t |= DECT_PT_IT_RECOMMENDED_OTHER_BEARER;
	t |= (uint64_t)sn << DECT_PT_BEARER_SN_SHIFT;
	t |= (uint64_t)cn << DECT_PT_BEARER_CN_SHIFT;

	for (i = 0; i < DECT_T_FIELD_SIZE; i++)
		mb->data[DECT_T_FIELD_OFF + i] = t >> ((sizeof(t) - i - 1) * BITS_PER_BYTE);
	mb->data[DECT_HDR_TA_OFF] |= DECT_TI_PT;
	mb->data[DECT_HDR_BA_OFF] |= DECT_BI_NONE;
	mb->len = DECT_A_FIELD_SIZE;
}

static void page_timer(struct dect_handle *dh, struct dect_timer *timer)
{
	dect_lce_group_ring_req(dh, 0);
	dect_timer_start(dh, timer, 1);
}

static struct dect_ops ops = {
	.lce_ops	= &lce_ops,
	.mm_ops		= &mm_ops,
};

int main(int argc, char **argv)
{
	struct dect_fd *dfd;
	struct dect_timer *timer;
	uint8_t slot;

	if (argc < 3) {
		printf("Usage: %s cluster rx-slot tx-slot tx-carrier\n",
			argv[0]);
		exit(1);
	}
	slot = atoi(argv[2]);
	dect_build_msg(&msg, atoi(argv[3]), atoi(argv[4]));

	dect_common_init(&ops, argv[1]);

	dfd = dect_raw_socket(dh);
	if (dfd == NULL)
		pexit("dect_raw_socket");

	dect_fd_setup(dfd, raw_sock_event, (void *)(unsigned long)slot);
	if (dect_fd_register(dh, dfd, DECT_FD_WRITE) < 0)
		pexit("dect_register_fd");

	timer = dect_timer_alloc(dh);
	if (timer == NULL)
		pexit("dect_alloc_timer");
	dect_timer_setup(timer, page_timer, NULL);
	dect_timer_start(dh, timer, 1);

	dect_event_loop();

	dect_timer_stop(dh, timer);
	dect_fd_unregister(dh, dfd);
	dect_close(dh, dfd);

	dect_common_cleanup(dh);
	return 0;
}
