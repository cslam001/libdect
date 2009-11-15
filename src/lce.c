/*
 * DECT Link Control Entity (LCE)
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
#include <linux/byteorder/little_endian.h>
#include <linux/dect.h>
#include <asm/byteorder.h>

#include <libdect.h>
#include <identities.h>
#include <utils.h>
#include <s_fmt.h>
#include <b_fmt.h>
#include <lce.h>
#include <ss.h>

static DECT_SFMT_MSG_DESC(lce_page_response,
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_NONE,      IE_MANDATORY, 0),
	DECT_SFMT_IE(S_VL_IE_FIXED_IDENTITY,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_NWK_ASSIGNED_IDENTITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CIPHER_INFO,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(lce_page_reject,
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_FIXED_IDENTITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_REJECT_REASON,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static const struct dect_nwk_protocol *protocols[DECT_PD_MAX + 1];

void dect_lce_register_protocol(const struct dect_nwk_protocol *protocol)
{
	protocols[protocol->pd] = protocol;
	dect_debug("LCE: registered protocol %u (%s)\n",
		   protocol->pd, protocol->name);
}

struct dect_msg_buf *dect_mbuf_alloc(const struct dect_handle *dh)
{
	struct dect_msg_buf *mb;

	mb = dect_malloc(dh, sizeof(*mb));
	if (mb == NULL)
		return NULL;
	memset(mb->head, 0, sizeof(mb->head));
	mb->data = mb->head;
	mb->len  = 0;
	mb->type = 0;
	return mb;
}

static ssize_t dect_mbuf_rcv(const struct dect_fd *dfd, struct dect_msg_buf *mb)
{
	ssize_t len;

	memset(mb, 0, sizeof(*mb));
	mb->data = mb->head;
	len = recv(dfd->fd, mb->data, sizeof(mb->head), 0);
	if (len < 0) {
		dect_debug("recv: %s\n", strerror(errno));
		return len;
	}
	mb->len = len;
	return len;
}

#if 0
/*
 * Location Table
 */

static struct dect_lte *dect_lte_get_by_ipui(const struct dect_handle *dh,
					     const struct dect_ipui *ipui)
{
	struct dect_lte *lte;

	list_for_each_entry(lte, &dh->ldb.entries, list) {
		if (!dect_ipui_cmp(&lte->ipui, ipui))
			return lte;
	}
	return NULL;
}

static struct dect_lte *dect_lte_alloc(const struct dect_handle *dh,
				       const struct dect_ipui *ipui)
{
	struct dect_lte *lte;

	lte = dect_malloc(dh, sizeof(*lte));
	if (lte == NULL)
		return NULL;
	memcpy(&lte->ipui, ipui, sizeof(lte->ipui));
	return lte;
}
#endif

/*
 * Paging
 */

static int dect_lce_broadcast(const struct dect_handle *dh,
			      const uint8_t *msg, size_t len)
{
	ssize_t size;

	dect_hexdump("BROADCAST", msg, len);
	size = send(dh->b_sap->fd, msg, len, 0);
	assert(size == (ssize_t)len);
	return 0;
}

int dect_lce_group_ring(struct dect_handle *dh, enum dect_ring_patterns pattern)
{
	struct dect_short_page_msg msg;
	uint16_t page;

	msg.hdr  = DECT_LCE_PAGE_W_FLAG;
	msg.hdr |= DECT_LCE_PAGE_GENERAL_VOICE;

	page = pattern << DECT_LCE_SHORT_PAGE_RING_PATTERN_SHIFT;
	page = 0;
	page |= DECT_TPUI_CBI & DECT_LCE_SHORT_PAGE_TPUI_MASK;
	msg.information = __cpu_to_be16(page);

	return dect_lce_broadcast(dh, &msg.hdr, sizeof(msg));
}

static int dect_lce_page(const struct dect_handle *dh,
			 const struct dect_ipui *ipui)
{
	struct dect_short_page_msg msg;
	struct dect_tpui tpui;
	uint16_t page;

	tpui.type = DECT_TPUI_INDIVIDUAL_DEFAULT;
	tpui.id.ipui = ipui;

	msg.hdr = DECT_LCE_PAGE_GENERAL_VOICE;
	page = dect_build_tpui(&tpui) & DECT_LCE_SHORT_PAGE_TPUI_MASK;
	msg.information = __cpu_to_be16(page);

	return dect_lce_broadcast(dh, &msg.hdr, sizeof(msg));
}

static void dect_lce_rcv_short_page(struct dect_handle *dh,
				    struct dect_msg_buf *mb)
{
	struct dect_short_page_msg *msg = (void *)mb->data;
	uint8_t hdr;
	bool w;

	w   = msg->hdr & DECT_LCE_PAGE_W_FLAG;
	hdr = msg->hdr & DECT_LCE_PAGE_HDR_MASK;
	dect_debug("short page: w=%u hdr=%u information=%x\n",
		   w, hdr, __be16_to_cpu(msg->information));
}

static void dect_lce_bsap_event(struct dect_handle *dh, struct dect_fd *dfd,
				uint32_t events)
{
	struct dect_msg_buf _mb, *mb = &_mb;

	if (dect_mbuf_rcv(dfd, mb) < 0)
		return;
	dect_mbuf_dump(mb, "BCAST RX");

	switch (mb->len) {
	case 3:
		return dect_lce_rcv_short_page(dh, mb);
	default:
		break;
	}
}

/*
 * Data links
 */

#define ddl_debug(ddl, fmt, args...) \
	dect_debug("link %d (%s): " fmt "\n", \
		   (ddl)->dfd ? (ddl)->dfd->fd : -1, \
		   ddl_states[(ddl)->state], ## args)

static const char * const ddl_states[DECT_DATA_LINK_STATE_MAX + 1] = {
	[DECT_DATA_LINK_RELEASED]		= "RELEASED",
	[DECT_DATA_LINK_ESTABLISHED]		= "ESTABLISHED",
	[DECT_DATA_LINK_ESTABLISH_PENDING]	= "ESTABLISH_PENDING",
	[DECT_DATA_LINK_RELEASE_PENDING]	= "RELEASE_PENDING",
	[DECT_DATA_LINK_SUSPENDED]		= "SUSPENDED",
	[DECT_DATA_LINK_SUSPEND_PENDING]	= "SUSPEND_PENDING",
	[DECT_DATA_LINK_RESUME_PENDING]		= "RESUME_PENDING",
};

static struct dect_data_link *dect_ddl_get_by_ipui(const struct dect_handle *dh,
						   const struct dect_ipui *ipui)
{
	struct dect_data_link *ddl;

	list_for_each_entry(ddl, &dh->links, list) {
		if (!dect_ipui_cmp(&ddl->ipui, ipui))
			return ddl;
	}
	return NULL;
}

static struct dect_transaction *
dect_ddl_transaction_lookup(const struct dect_data_link *ddl, uint8_t pd,
			    uint8_t tv, enum dect_transaction_role role)
{
	struct dect_transaction *ta;

	list_for_each_entry(ta, &ddl->transactions, list) {
		if (ta->pd == pd && ta->tv == tv && ta->role == role)
			return ta;
	}
	return NULL;
}

static struct dect_data_link *dect_ddl_alloc(const struct dect_handle *dh)
{
	struct dect_data_link *ddl;

	ddl = dect_zalloc(dh, sizeof(*ddl));
	if (ddl == NULL)
		goto err1;
	ddl->sdu_timer = dect_alloc_timer(dh);
	if (ddl->sdu_timer == NULL)
		goto err2;
	ddl->state = DECT_DATA_LINK_RELEASED;
	init_list_head(&ddl->list);
	init_list_head(&ddl->transactions);
	init_list_head(&ddl->msg_queue);
	ddl_debug(ddl, "alloc");
	return ddl;

err2:
	dect_free(dh, ddl);
err1:
	return NULL;
}

static void dect_ddl_destroy(struct dect_handle *dh, struct dect_data_link *ddl)
{
	struct dect_msg_buf *mb, *next;

	ddl_debug(ddl, "destroy");
	assert(list_empty(&ddl->transactions));

	list_del(&ddl->list);
	list_for_each_entry_safe(mb, next, &ddl->msg_queue, list)
		dect_free(dh, mb);

	if (ddl->dfd != NULL) {
		dect_unregister_fd(dh, ddl->dfd);
		dect_close(dh, ddl->dfd);
	}
	dect_free(dh, ddl->sdu_timer);
	dect_free(dh, ddl->release_timer);
	dect_free(dh, ddl);
}

static void dect_ddl_release_timer(struct dect_handle *dh, struct dect_timer *timer)
{
	struct dect_data_link *ddl = timer->data;

	ddl_debug(ddl, "normal release timeout");
	dect_ddl_destroy(dh, ddl);
}

static void dect_ddl_release_complete(struct dect_handle *dh,
				      struct dect_data_link *ddl)
{
	ddl_debug(ddl, "normal release complete");
	ddl->state = DECT_DATA_LINK_RELEASED;
	dect_stop_timer(dh, ddl->release_timer);
	dect_ddl_destroy(dh, ddl);
}

static void dect_ddl_release(struct dect_handle *dh,
			     struct dect_data_link *ddl)
{
	ddl_debug(ddl, "normal release");

	/* Shut down transmission and wait until all outstanding frames
	 * are successfully transmitted or the release timeout occurs.
	 */
	if (shutdown(ddl->dfd->fd, SHUT_WR) < 0)
		goto err1;
	ddl->state = DECT_DATA_LINK_RELEASE_PENDING;

	ddl->release_timer = dect_alloc_timer(dh);
	if (ddl->release_timer == NULL)
		goto err1;
	dect_setup_timer(ddl->release_timer, dect_ddl_release_timer, ddl);
	dect_start_timer(dh, ddl->release_timer, DECT_DDL_RELEASE_TIMEOUT);
	return;

err1:
	dect_ddl_destroy(dh, ddl);
}

static void dect_ddl_partial_release_timer(struct dect_handle *dh, struct dect_timer *timer)
{
	struct dect_data_link *ddl = timer->data;

	ddl_debug(ddl, "partial release timeout");
	if (list_empty(&ddl->transactions))
		dect_ddl_destroy(dh, ddl);
}

static void dect_ddl_partial_release(struct dect_handle *dh,
				     struct dect_data_link *ddl)
{
	ddl_debug(ddl, "partial release");
	dect_setup_timer(ddl->sdu_timer, dect_ddl_partial_release_timer, ddl);
	dect_start_timer(dh, ddl->sdu_timer, DECT_DDL_ESTABLISH_SDU_TIMEOUT);
}

static void dect_ddl_shutdown(struct dect_handle *dh,
			      struct dect_data_link *ddl)
{
	struct dect_transaction *ta, *next;
	LIST_HEAD(transactions);

	ddl_debug(ddl, "shutdown");
	list_splice_init(&ddl->transactions, &transactions);
	list_for_each_entry_safe(ta, next, &transactions, list)
		protocols[ta->pd]->shutdown(dh, ta);
}

static void dect_ddl_sdu_timer(struct dect_handle *dh, struct dect_timer *timer)
{
	struct dect_data_link *ddl = timer->data;

	ddl_debug(ddl, "SDU timer");
	dect_ddl_destroy(dh, ddl);
}

static int dect_ddl_schedule_sdu_timer(const struct dect_handle *dh,
				       struct dect_data_link *ddl)
{
	dect_setup_timer(ddl->sdu_timer, dect_ddl_sdu_timer, ddl);
	dect_start_timer(dh, ddl->sdu_timer, DECT_DDL_ESTABLISH_SDU_TIMEOUT);
	ddl_debug(ddl, "start SDU timer");
	return 0;
}

static void dect_ddl_stop_sdu_timer(const struct dect_handle *dh,
				    struct dect_data_link *ddl)
{
	ddl_debug(ddl, "stop SDU timer");
	dect_stop_timer(dh, ddl->sdu_timer);
}

static int dect_send(const struct dect_handle *dh,
		       const struct dect_data_link *ddl,
		       struct dect_msg_buf *mb)
{
	ssize_t len;

	dect_mbuf_dump(mb, "TX");
	len = send(ddl->dfd->fd, mb->data, mb->len, 0);
	if (len < 0)
		ddl_debug(ddl, "send %Zd: %s\n", len, strerror(errno));
	dect_free(dh, mb);
	return len;
}

/**
 * dect_send - Queue a S-Format message for transmission to the LCE
 *
 */
int dect_lce_send(const struct dect_handle *dh,
		  const struct dect_transaction *ta,
		  const struct dect_sfmt_msg_desc *desc,
		  const struct dect_msg_common *msg, uint8_t type)
{
	struct dect_data_link *ddl = ta->link;
	struct dect_msg_buf *mb;

	mb = dect_mbuf_alloc(dh);
	if (mb == NULL)
		return -1;

	dect_mbuf_reserve(mb, DECT_S_HDR_SIZE);
	dect_build_sfmt_msg(dh, desc, msg, mb);

	if (ddl->sdu_timer && dect_timer_running(ddl->sdu_timer))
		dect_ddl_stop_sdu_timer(dh, ddl);

	dect_mbuf_push(mb, DECT_S_HDR_SIZE);
	mb->data[1]  = type;
	mb->data[0]  = ta->pd;
	mb->data[0] |= ta->tv << DECT_S_TI_TV_SHIFT;
	if (ta->role == DECT_TRANSACTION_RESPONDER)
		mb->data[0] |= DECT_S_TI_F_FLAG;

	switch (ddl->state) {
	case DECT_DATA_LINK_ESTABLISHED:
		return dect_send(dh, ddl, mb);
	case DECT_DATA_LINK_ESTABLISH_PENDING:
		list_add_tail(&mb->list, &ddl->msg_queue);
		return 0;
	default:
		BUG();
	}
}

static void dect_ddl_page_timer(struct dect_handle *dh, struct dect_timer *timer)
{
	struct dect_data_link *ddl = timer->data;

	ddl_debug(ddl, "Page timer");
	if (ddl->page_count++ == DECT_DDL_PAGE_RETRANS_MAX)
		dect_ddl_shutdown(dh, ddl);
	else {
		dect_lce_page(dh, &ddl->ipui);
		dect_start_timer(dh, ddl->page_timer, DECT_DDL_PAGE_TIMEOUT);
	}
}

/**
 * dect_ddl_establish - Establish an outgoing data link
 *
 */
static void dect_lce_data_link_event(struct dect_handle *dh,
				     struct dect_fd *dfd, uint32_t events);

static struct dect_data_link *dect_ddl_establish(struct dect_handle *dh,
						 const struct dect_ipui *ipui)
{
	struct dect_data_link *ddl;

	//lte = dect_lte_get_by_ipui(dh, lte);
	ddl = dect_ddl_alloc(dh);
	if (ddl == NULL)
		goto err1;
	ddl->state = DECT_DATA_LINK_ESTABLISH_PENDING;

	if (dh->mode == DECT_MODE_FP) {
		memcpy(&ddl->ipui, ipui, sizeof(ddl->ipui));

		ddl->page_timer = dect_alloc_timer(dh);
		if (ddl->page_timer == NULL)
			goto err2;
		dect_setup_timer(ddl->page_timer, dect_ddl_page_timer, ddl);
		dect_ddl_page_timer(dh, ddl->page_timer);
	} else {
		ddl->dfd = dect_socket(dh, SOCK_SEQPACKET, DECT_S_SAP);
		if (ddl->dfd == NULL)
			goto err2;

		ddl->dlei.dect_family = AF_DECT;
		ddl->dlei.dect_ari = dect_build_ari(&dh->pari) >> 24;
		ddl->dlei.dect_pmid = 0xe98a1;
		ddl->dlei.dect_lln = 1;
		ddl->dlei.dect_sapi = 0;

		dect_setup_fd(ddl->dfd, dect_lce_data_link_event, ddl);
		if (dect_register_fd(dh, ddl->dfd, DECT_FD_WRITE) < 0)
			goto err2;

		if (connect(ddl->dfd->fd, (struct sockaddr *)&ddl->dlei,
			    sizeof(ddl->dlei)) < 0 && errno != EAGAIN)
			goto err3;
	}

	list_add_tail(&ddl->list, &dh->links);
	return ddl;

err3:
	dect_unregister_fd(dh, ddl->dfd);
err2:
	dect_free(dh, ddl);
err1:
	dect_debug("LCE: dect_ddl_establish: %s\n", strerror(errno));
	return NULL;
}

static void dect_ddl_complete_direct_establish(struct dect_handle *dh,
					       struct dect_data_link *ddl)
{
	struct dect_msg_buf *mb, *mb_next;

	ddl->state = DECT_DATA_LINK_ESTABLISHED;
	ddl_debug(ddl, "complete direct link establishment");

	/* Send queued messages */
	list_for_each_entry_safe(mb, mb_next, &ddl->msg_queue, list) {
		list_del(&mb->list);
		dect_send(dh, ddl, mb);
	}

	dect_unregister_fd(dh, ddl->dfd);
	dect_register_fd(dh, ddl->dfd, DECT_FD_READ);
}

static void dect_ddl_complete_indirect_establish(struct dect_handle *dh,
						 struct dect_data_link *ddl,
						 struct dect_data_link *req)
{
	struct dect_transaction *ta, *ta_next;
	struct dect_msg_buf *mb, *mb_next;

	/* Stop page timer */
	dect_stop_timer(dh, req->page_timer);
	dect_free(dh, req->page_timer);

	ddl_debug(ddl, "complete indirect link establishment req %p", req);
	/* Transfer transactions to the new link */
	list_for_each_entry_safe(ta, ta_next, &req->transactions, list) {
		ddl_debug(ta->link, "transfer transaction to link %p\n", ddl);
		list_move_tail(&ta->list, &ddl->transactions);
		ta->link = ddl;
	}

	/* Send queued messages */
	list_for_each_entry_safe(mb, mb_next, &req->msg_queue, list) {
		list_del(&mb->list);
		dect_send(dh, ddl, mb);
	}

	/* Release pending link */
	dect_ddl_destroy(dh, req);
}

static int dect_send_reject(const struct dect_handle *dh,
			    const struct dect_transaction *ta,
			    enum dect_reject_reasons reason)
{
	struct dect_ie_reject_reason reject_reason;
	struct dect_lce_page_reject msg = {
		.portable_identity	= NULL,
		.reject_reason		= &reject_reason,
	};

	dect_ie_init(&reject_reason);
	reject_reason.reason = reason;

	return dect_lce_send(dh, ta, &lce_page_reject_msg_desc,
			     &msg.common, DECT_LCE_PAGE_REJECT);
}

static void dect_lce_rcv_page_response(struct dect_handle *dh,
				       const struct dect_transaction *ta,
				       struct dect_msg_buf *mb)
{
	struct dect_lce_page_response msg;
	struct dect_data_link *i, *req = NULL;

	ddl_debug(ta->link, "LCE-PAGE-RESPONSE");
	if (dect_parse_sfmt_msg(dh, &lce_page_response_msg_desc,
				&msg.common, mb) < 0)
		return;

	list_for_each_entry(i, &dh->links, list) {
		if (dect_ipui_cmp(&i->ipui, &msg.portable_identity->ipui))
			continue;
		if (i->state != DECT_DATA_LINK_ESTABLISH_PENDING)
			continue;
		req = i;
		break;
	}

	if (req != NULL)
		dect_ddl_complete_indirect_establish(dh, ta->link, req);
	else {
		dect_send_reject(dh, ta, DECT_REJECT_IPUI_UNKNOWN);
		dect_ddl_release(dh, ta->link);
	}

	dect_msg_free(dh, &lce_page_response_msg_desc, &msg.common);
}

static void dect_lce_rcv_page_reject(struct dect_handle *dh,
				     struct dect_transaction *ta,
				     struct dect_msg_buf *mb)
{
	struct dect_lce_page_reject msg;

	ddl_debug(ta->link, "LCE-PAGE-REJECT");
	if (dect_parse_sfmt_msg(dh, &lce_page_reject_msg_desc,
				&msg.common, mb) < 0)
		return;
	dect_msg_free(dh, &lce_page_reject_msg_desc, &msg.common);
}

static void dect_lce_rcv(struct dect_handle *dh, struct dect_transaction *ta,
			 struct dect_msg_buf *mb)
{
	switch (mb->type) {
	case DECT_LCE_PAGE_REJECT:
		return dect_lce_rcv_page_reject(dh, ta, mb);
	default:
		ddl_debug(ta->link, "LCE: unknown message type %x", mb->type);
		return;
	}
}

static void dect_lce_open(struct dect_handle *dh,
			  const struct dect_transaction *ta,
			  struct dect_msg_buf *mb)
{
	switch (mb->type) {
	case DECT_LCE_PAGE_RESPONSE:
		return dect_lce_rcv_page_response(dh, ta, mb);
	default:
		ddl_debug(ta->link, "LCE: unknown message type %x", mb->type);
		return;
	}
}

static const struct dect_nwk_protocol lce_protocol = {
	.name			= "Link Control",
	.pd			= DECT_PD_LCE,
	.max_transactions	= 1,
	.open			= dect_lce_open,
	.rcv			= dect_lce_rcv,
};

static void dect_ddl_rcv_msg(struct dect_handle *dh, struct dect_data_link *ddl)
{
	struct dect_msg_buf _mb, *mb = &_mb;
	struct dect_transaction *ta;
	uint8_t pd, tv;
	bool f;

	if (dect_mbuf_rcv(ddl->dfd, mb) < 0) {
		switch (errno) {
		case ENOTCONN:
			if (ddl->state == DECT_DATA_LINK_RELEASE_PENDING)
				return dect_ddl_release_complete(dh, ddl);
			else {
				ddl->state = DECT_DATA_LINK_RELEASED;
				if (list_empty(&ddl->transactions))
					return dect_ddl_destroy(dh, ddl);
				else
					return dect_ddl_shutdown(dh, ddl);
			}
		case ETIMEDOUT:
		case ECONNRESET:
			ddl->state = DECT_DATA_LINK_RELEASED;
			return dect_ddl_shutdown(dh, ddl);
		default:
			ddl_debug(ddl, "unhandled receive error: %s",
				  strerror(errno));
			BUG();
		}
	}

	dect_debug("\n");
	dect_mbuf_dump(mb, "RX");

	if (mb->len < DECT_S_HDR_SIZE)
		return;
	f  = (mb->data[0] & DECT_S_TI_F_FLAG);
	tv = (mb->data[0] & DECT_S_TI_TV_MASK) >> DECT_S_TI_TV_SHIFT;
	pd = (mb->data[0] & DECT_S_PD_MASK);
	mb->type = (mb->data[1] & DECT_S_PD_MSG_TYPE_MASK);
	dect_mbuf_pull(mb, DECT_S_HDR_SIZE);

	if (pd >= array_size(protocols) || protocols[pd] == NULL) {
		ddl_debug(ddl, "unknown protocol %u\n", pd);
		return;
	}

	if (dect_timer_running(ddl->sdu_timer))
		dect_ddl_stop_sdu_timer(dh, ddl);

	if (tv == DECT_TV_CONNECTIONLESS)
		return dect_clss_rcv(dh, mb);

	ta = dect_ddl_transaction_lookup(ddl, pd, tv, !f);
	if (ta == NULL) {
		struct dect_transaction req = {
			.link	= ddl,
			.pd	= pd,
			.role	= DECT_TRANSACTION_RESPONDER,
			.tv	= tv,
		};
		ddl_debug(ddl, "new transaction: protocol: %s F: %u TV: %u",
			  protocols[pd]->name, f, tv);
		protocols[pd]->open(dh, &req, mb);
	} else
		protocols[pd]->rcv(dh, ta, mb);
}

static void dect_lce_data_link_event(struct dect_handle *dh,
				     struct dect_fd *dfd, uint32_t events)
{
	struct dect_data_link *ddl = dfd->data;

	if (events & DECT_FD_WRITE) {
		switch (ddl->state) {
		case DECT_DATA_LINK_ESTABLISH_PENDING:
			dect_ddl_complete_direct_establish(dh, ddl);
			break;
		default:
			break;
		}
	}

	if (events & DECT_FD_READ) {
		dect_ddl_rcv_msg(dh, ddl);
	}
}

static int dect_transaction_alloc_tv(const struct dect_data_link *ddl,
				     const struct dect_nwk_protocol *protocol)
{
	uint16_t tv;

	for (tv = 0; tv < protocol->max_transactions; tv++) {
		if (dect_ddl_transaction_lookup(ddl, protocol->pd, tv,
						DECT_TRANSACTION_INITIATOR))
			continue;
		return tv;
	}
	return -1;
}

int dect_ddl_open_transaction(struct dect_handle *dh, struct dect_transaction *ta,
			      struct dect_data_link *ddl, enum dect_pds pd)
{
	int tv;

	ddl_debug(ddl, "open transaction");
	tv = dect_transaction_alloc_tv(ddl, protocols[pd]);
	if (tv < 0)
		return -1;

	ta->link = ddl;
	ta->pd	 = pd;
	ta->role = DECT_TRANSACTION_INITIATOR;
	ta->tv	 = tv;

	list_add_tail(&ta->list, &ddl->transactions);
	return 0;
}

int dect_open_transaction(struct dect_handle *dh, struct dect_transaction *ta,
			  const struct dect_ipui *ipui, enum dect_pds pd)
{
	struct dect_data_link *ddl;

	ddl = dect_ddl_get_by_ipui(dh, ipui);
	if (ddl == NULL) {
		ddl = dect_ddl_establish(dh, ipui);
		if (ddl == NULL)
			return -1;
	}

	return dect_ddl_open_transaction(dh, ta, ddl, pd);
}

void dect_confirm_transaction(struct dect_handle *dh, struct dect_transaction *ta,
			      const struct dect_transaction *req)
{
	ta->link = req->link;
	ta->tv   = req->tv;
	ta->role = req->role;
	ta->pd   = req->pd;

	ddl_debug(req->link, "confirm transaction");
	list_add_tail(&ta->list, &req->link->transactions);
}

void dect_close_transaction(struct dect_handle *dh, struct dect_transaction *ta,
			    enum dect_release_modes mode)
{
	struct dect_data_link *ddl = ta->link;

	ddl_debug(ddl, "close transaction");
	list_del(&ta->list);

	switch (ddl->state) {
	case DECT_DATA_LINK_RELEASED:
		/* If link is already down, destroy immediately */
		if (!list_empty(&ddl->transactions))
			break;
	case DECT_DATA_LINK_ESTABLISH_PENDING:
		/* link establishment was unsucessful */
		return dect_ddl_destroy(dh, ddl);
	default:
		break;
	}

	switch (mode) {
	case DECT_DDL_RELEASE_NORMAL:
		if (!list_empty(&ddl->transactions))
			return;
		return dect_ddl_release(dh, ddl);
	case DECT_DDL_RELEASE_PARTIAL:
		return dect_ddl_partial_release(dh, ddl);
	}
}

void dect_transaction_get_ulei(struct sockaddr_dect_lu *addr,
			       const struct dect_transaction *ta)
{
	struct dect_data_link *ddl = ta->link;

	memset(addr, 0, sizeof(*addr));
	addr->dect_family = AF_DECT;
	addr->dect_ari    = ddl->dlei.dect_ari;
	addr->dect_pmid   = ddl->dlei.dect_pmid;
	addr->dect_lcn    = ddl->dlei.dect_lcn;
}

static void dect_lce_ssap_listener_event(struct dect_handle *dh,
					 struct dect_fd *dfd, uint32_t events)
{
	struct dect_data_link *ddl;
	struct dect_fd *nfd;

	ddl = dect_ddl_alloc(dh);
	if (ddl == NULL)
		goto err1;

	nfd = dect_accept(dh, dfd, (struct sockaddr *)&ddl->dlei,
			  sizeof(ddl->dlei));
	if (nfd == NULL)
		goto err2;
	ddl->dfd = nfd;

	dect_setup_fd(nfd, dect_lce_data_link_event, ddl);
	if (dect_register_fd(dh, nfd, DECT_FD_READ) < 0)
		goto err3;

	ddl->state = DECT_DATA_LINK_ESTABLISHED;
	if (dect_ddl_schedule_sdu_timer(dh, ddl) < 0)
		goto err4;

	list_add_tail(&ddl->list, &dh->links);
	ddl_debug(ddl, "new link: PMID: %x LCN: %u LLN: %u SAPI: %u",
		  ddl->dlei.dect_pmid, ddl->dlei.dect_lcn,
		  ddl->dlei.dect_lln, ddl->dlei.dect_sapi);
	return;

err4:
	dect_unregister_fd(dh, nfd);
err3:
	dect_close(dh, nfd);
err2:
	dect_free(dh, ddl);
err1:
	dect_debug("LCE: dect_lce_ssap_listener_event: %s\n", strerror(errno));
	return;
}

int dect_lce_init(struct dect_handle *dh)
{
	struct sockaddr_dect_ssap s_addr;
	struct sockaddr_dect b_addr;

	/* Open B-SAP socket */
	dh->b_sap = dect_socket(dh, SOCK_DGRAM, DECT_B_SAP);
	if (dh->b_sap == NULL)
		goto err1;

	memset(&b_addr, 0, sizeof(b_addr));
	b_addr.dect_family = AF_DECT;
	b_addr.dect_index = dh->index;
	if (bind(dh->b_sap->fd, (struct sockaddr *)&b_addr, sizeof(b_addr)) < 0)
		goto err2;

	dect_setup_fd(dh->b_sap, dect_lce_bsap_event, NULL);
	if (dect_register_fd(dh, dh->b_sap, DECT_FD_READ) < 0)
		goto err2;

	/* Open S-SAP listener socket */
	dh->s_sap = dect_socket(dh, SOCK_SEQPACKET, DECT_S_SAP);
	if (dh->s_sap == NULL)
		goto err3;

	memset(&s_addr, 0, sizeof(s_addr));
	s_addr.dect_family = AF_DECT;
	s_addr.dect_lln    = 1;
	s_addr.dect_sapi   = 0;

	if (bind(dh->s_sap->fd, (struct sockaddr *)&s_addr, sizeof(s_addr)) < 0)
		goto err4;
	if (listen(dh->s_sap->fd, 10) < 0)
		goto err4;

	dect_setup_fd(dh->s_sap, dect_lce_ssap_listener_event, NULL);
	if (dect_register_fd(dh, dh->s_sap, DECT_FD_READ) < 0)
		goto err4;

	dect_lce_register_protocol(&lce_protocol);
	return 0;

err4:
	dect_close(dh, dh->s_sap);
err3:
	dect_unregister_fd(dh, dh->b_sap);
err2:
	dect_close(dh, dh->b_sap);
err1:
	dect_debug("LCE: dect_lce_init: %s\n", strerror(errno));
	return -1;
}

void dect_lce_exit(struct dect_handle *dh)
{
	struct dect_data_link *ddl, *next;

	list_for_each_entry_safe(ddl, next, &dh->links, list)
		dect_ddl_shutdown(dh, ddl);

	dect_unregister_fd(dh, dh->s_sap);
	dect_close(dh, dh->s_sap);

	dect_unregister_fd(dh, dh->b_sap);
	dect_close(dh, dh->b_sap);
}
