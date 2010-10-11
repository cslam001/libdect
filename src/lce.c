/*
 * DECT Link Control Entity (LCE)
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/**
 * @defgroup lce Link Control Entity
 * @{
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
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
#include <timer.h>
#include <io.h>
#include <s_fmt.h>
#include <b_fmt.h>
#include <clms.h>
#include <lce.h>
#include <cc.h>
#include <mm.h>
#include <ss.h>
#include <dect/auth.h>

static DECT_SFMT_MSG_DESC(lce_page_response,
	DECT_SFMT_IE(DECT_IE_PORTABLE_IDENTITY,		IE_NONE,      IE_MANDATORY, 0),
	DECT_SFMT_IE(DECT_IE_FIXED_IDENTITY,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_NWK_ASSIGNED_IDENTITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CIPHER_INFO,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(lce_page_reject,
	DECT_SFMT_IE(DECT_IE_PORTABLE_IDENTITY,		IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_FIXED_IDENTITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_REJECT_REASON,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static const struct dect_nwk_protocol *protocols[DECT_PD_MAX + 1];

#define lce_debug(fmt, args...) \
	dect_debug(DECT_DEBUG_LCE, "LCE: " fmt, ## args)

void dect_lce_register_protocol(const struct dect_nwk_protocol *protocol)
{
	protocols[protocol->pd] = protocol;
	lce_debug("registered protocol %u (%s)\n",
		  protocol->pd, protocol->name);
}

/**
 * Allocate a libdect message buffer
 *
 * @param dh	libdect DECT handle
 *
 * Allocate a libdect message buffer. The buffer needs to be released again
 * using dect_mbuf_free().
 */
struct dect_msg_buf *dect_mbuf_alloc(const struct dect_handle *dh)
{
	struct dect_msg_buf *mb;

	mb = dect_malloc(dh, sizeof(*mb));
	if (mb == NULL)
		return NULL;
	memset(mb->head, 0, sizeof(mb->head));
	mb->data   = mb->head;
	mb->len    = 0;
	mb->type   = 0;
	mb->refcnt = 1;
	return mb;
}
EXPORT_SYMBOL(dect_mbuf_alloc);

/**
 * Release reference to a libdect message buffer
 *
 * @param dh	libdect DECT handle
 * @param mb	libdect message buffer
 *
 * Release reference to a libdect message buffer. When the reference count
 * drops to zero, the buffer will be freed.
 */
void dect_mbuf_free(const struct dect_handle *dh, struct dect_msg_buf *mb)
{
	if (--mb->refcnt > 0)
		return;
	dect_free(dh, mb);
}
EXPORT_SYMBOL(dect_mbuf_free);

/**
 * Pull data from the head of a libdect message buffer
 *
 * @param mb	libdect message buffer
 * @param len	amount of data to pull
 */
void *dect_mbuf_pull(struct dect_msg_buf *mb, unsigned int len)
{
	dect_assert(len <= mb->len);
	mb->data += len;
	mb->len  -= len;
	return mb->data;
}
EXPORT_SYMBOL(dect_mbuf_pull);

/**
 * Push data to the head of a libdect message buffer
 *
 * @param mb	libdect message buffer
 * @param len	amount of data to push
 */
void *dect_mbuf_push(struct dect_msg_buf *mb, unsigned int len)
{
	mb->data -= len;
	mb->len  += len;
	dect_assert(mb->data >= mb->head);
	return mb->data;
}
EXPORT_SYMBOL(dect_mbuf_push);

/**
 * Reserve space at the head of a libdect message buffer
 *
 * @param mb	libdect message buffer
 * @param len	amount of space to reserve
 */
void dect_mbuf_reserve(struct dect_msg_buf *mb, unsigned int len)
{
	mb->data += len;
	dect_assert(mb->data < mb->head + sizeof(mb->head));
}
EXPORT_SYMBOL(dect_mbuf_reserve);

/**
 * Put data at the tail of a libdect message buffer
 *
 * @param mb	libdect message buffer
 * @param len	amount of data to put
 */
void *dect_mbuf_put(struct dect_msg_buf *mb, unsigned int len)
{
	void *ptr = mb->data + mb->len;
	mb->len += len;
	return ptr;
}
EXPORT_SYMBOL(dect_mbuf_put);

static ssize_t dect_mbuf_rcv(const struct dect_fd *dfd, struct msghdr *msg,
			     struct dect_msg_buf *mb)
{
	struct iovec iov;
	ssize_t len;

	msg->msg_name		= NULL;
	msg->msg_namelen	= 0;
	msg->msg_iov		= &iov;
	msg->msg_iovlen		= 1;
	msg->msg_flags		= MSG_NOSIGNAL;

	iov.iov_base		= mb->data;
	iov.iov_len		= sizeof(mb->head);

	len = recvmsg(dfd->fd, msg, 0);
	if (len < 0) {
		lce_debug("recvmsg: %s\n", strerror(errno));
		return len;
	}

	mb->len = len;
	return len;
}

static ssize_t dect_mbuf_send(const struct dect_handle *dh,
			      const struct dect_fd *dfd,
			      struct msghdr *msg, const struct dect_msg_buf *mb)
{
	struct iovec iov;
	ssize_t len;

	msg->msg_name		= NULL;
	msg->msg_namelen	= 0;
	msg->msg_iov		= &iov;
	msg->msg_iovlen		= 1;
	msg->msg_flags		= MSG_NOSIGNAL;

	iov.iov_base		= mb->data;
	iov.iov_len		= mb->len;

	len = sendmsg(dfd->fd, msg, 0);
	if (len < 0)
		lce_debug("sendmsg: %u bytes: %s\n", mb->len, strerror(errno));

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
 * Data links
 */

#define ddl_debug(ddl, fmt, args...) \
	lce_debug("link %d (%s): " fmt "\n", \
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

int dect_ddl_set_ipui(struct dect_handle *dh, struct dect_data_link *ddl,
		      const struct dect_ipui *ipui)
{
	if (ddl->flags & DECT_DATA_LINK_IPUI_VALID) {
		if (dect_ipui_cmp(&ddl->ipui, ipui))
			return -1;
	} else {
		ddl_debug(ddl, "set IPUI N EMC: %04x PSN: %05x",
			  ipui->pun.n.ipei.emc, ipui->pun.n.ipei.psn);

		ddl->ipui   = *ipui;
		ddl->flags |= DECT_DATA_LINK_IPUI_VALID;
	}
	return 0;
}

static struct dect_data_link *dect_ddl_get_by_ipui(const struct dect_handle *dh,
						   const struct dect_ipui *ipui)
{
	struct dect_data_link *ddl;

	list_for_each_entry(ddl, &dh->links, list) {
		if (ddl->flags & DECT_DATA_LINK_IPUI_VALID &&
		    !dect_ipui_cmp(&ddl->ipui, ipui))
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
	ddl->sdu_timer = dect_timer_alloc(dh);
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
	unsigned int i;

	ddl_debug(ddl, "destroy");
	dect_assert(list_empty(&ddl->transactions));

	for (i = 0; i < array_size(protocols); i++) {
		if (protocols[i] && protocols[i]->rebind != NULL)
			protocols[i]->rebind(dh, ddl, NULL);
	}

	list_del(&ddl->list);
	list_for_each_entry_safe(mb, next, &ddl->msg_queue, list)
		dect_mbuf_free(dh, mb);

	if (ddl->dfd != NULL) {
		dect_fd_unregister(dh, ddl->dfd);
		dect_close(dh, ddl->dfd);
	}

	if (dect_timer_running(ddl->sdu_timer))
		dect_timer_stop(dh, ddl->sdu_timer);
	dect_timer_free(dh, ddl->sdu_timer);

	if (ddl->release_timer != NULL && dect_timer_running(ddl->release_timer))
		dect_timer_stop(dh, ddl->release_timer);
	dect_timer_free(dh, ddl->release_timer);
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
	dect_timer_stop(dh, ddl->release_timer);
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

	ddl->release_timer = dect_timer_alloc(dh);
	if (ddl->release_timer == NULL)
		goto err1;
	dect_timer_setup(ddl->release_timer, dect_ddl_release_timer, ddl);
	dect_timer_start(dh, ddl->release_timer, DECT_DDL_RELEASE_TIMEOUT);
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
	/* Timer may already be running if a transaction is aborted before the
	 * first message has been sent.
	 */
	if (dect_timer_running(ddl->sdu_timer))
		dect_timer_stop(dh, ddl->sdu_timer);

	dect_timer_setup(ddl->sdu_timer, dect_ddl_partial_release_timer, ddl);
	dect_timer_start(dh, ddl->sdu_timer, DECT_DDL_ESTABLISH_SDU_TIMEOUT);
}

static void dect_ddl_shutdown(struct dect_handle *dh,
			      struct dect_data_link *ddl)
{
	struct dect_transaction *ta, *next;
	bool last = false;

	ddl_debug(ddl, "shutdown");
	ddl->state = DECT_DATA_LINK_RELEASED;

	/* If no transactions are present, the link is waiting for a partial
	 * release timeout. Destroy immediately since destruction won't be
	 * triggered by closing transactions.
	 */
	if (list_empty(&ddl->transactions))
		return dect_ddl_destroy(dh, ddl);

	list_for_each_entry_safe(ta, next, &ddl->transactions, list) {
		if (&next->list == &ddl->transactions)
			last = true;
		protocols[ta->pd]->shutdown(dh, ta);
		if (last)
			break;
	}
}

/**
 * dect_ddl_set_cipher_key - set cipher key for datalink
 *
 * @param ddl		Datalink
 * @param ck		Cipher key
 */
int dect_ddl_set_cipher_key(const struct dect_data_link *ddl,
			    const uint8_t ck[DECT_CIPHER_KEY_LEN])
{
	int err;

	ddl_debug(ddl, "DL_ENC_KEY-req: %.16" PRIx64, *(uint64_t *)ck);
	err = setsockopt(ddl->dfd->fd, SOL_DECT, DECT_DL_ENC_KEY,
			 ck, DECT_CIPHER_KEY_LEN);
	if (err != 0)
		ddl_debug(ddl, "setsockopt: %s", strerror(errno));
	return err;
}

/**
 * dect_ddl_encrypt_req - enable/disable encryption for a datalink
 *
 * @param ddl		Datalink
 * @param status	desired ciphering state (enabled/disabled)
 */
int dect_ddl_encrypt_req(const struct dect_data_link *ddl,
			 enum dect_cipher_states status)
{
	struct dect_dl_encrypt dle = { .status = status };
	int err;

	ddl_debug(ddl, "DL_ENCRYPT-req: status: %u\n", status);
	err = setsockopt(ddl->dfd->fd, SOL_DECT, DECT_DL_ENCRYPT,
			 &dle, sizeof(dle));
	if (err != 0)
		ddl_debug(ddl, "setsockopt: %s", strerror(errno));
	return err;
}

static void dect_ddl_encrypt_ind(struct dect_handle *dh,
				 struct dect_data_link *ddl,
				 enum dect_cipher_states state)
{
	struct dect_transaction *ta, *next;

	ddl_debug(ddl, "DL_ENCRYPT-ind");
	ddl->cipher = state;
	list_for_each_entry_safe(ta, next, &ddl->transactions, list) {
		if (protocols[ta->pd]->encrypt_ind)
			protocols[ta->pd]->encrypt_ind(dh, ta, state);
	}
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
	dect_timer_setup(ddl->sdu_timer, dect_ddl_sdu_timer, ddl);
	dect_timer_start(dh, ddl->sdu_timer, DECT_DDL_ESTABLISH_SDU_TIMEOUT);
	ddl_debug(ddl, "start SDU timer");
	return 0;
}

static void dect_ddl_stop_sdu_timer(const struct dect_handle *dh,
				    struct dect_data_link *ddl)
{
	ddl_debug(ddl, "stop SDU timer");
	dect_timer_stop(dh, ddl->sdu_timer);
}

static ssize_t dect_ddl_send(const struct dect_handle *dh,
			     const struct dect_data_link *ddl,
			     struct dect_msg_buf *mb)
{
	struct msghdr msg;
	ssize_t size;

	memset(&msg, 0, sizeof(msg));
	dect_mbuf_dump(DECT_DEBUG_LCE, mb, "LCE: TX");
	size = dect_mbuf_send(dh, ddl->dfd, &msg, mb);
	dect_mbuf_free(dh, mb);
	return size;
}

/**
 * dect_lce_send - Queue a S-Format message for transmission to the LCE
 */
int dect_lce_send(const struct dect_handle *dh,
		  struct dect_transaction *ta,
		  const struct dect_sfmt_msg_desc *desc,
		  const struct dect_msg_common *msg, uint8_t type)
{
	struct dect_data_link *ddl = ta->link;
	struct dect_msg_buf *mb;
	int err;

	mb = dect_mbuf_alloc(dh);
	if (mb == NULL)
		return -1;

	dect_mbuf_reserve(mb, DECT_S_HDR_SIZE);
	err = dect_build_sfmt_msg(dh, desc, msg, mb);
	if (err < 0)
		return err;

	if (ddl->sdu_timer && dect_timer_running(ddl->sdu_timer))
		dect_ddl_stop_sdu_timer(dh, ddl);

	dect_mbuf_push(mb, DECT_S_HDR_SIZE);
	mb->data[1]  = type;
	mb->data[0]  = ta->pd;
	mb->data[0] |= ta->tv << DECT_S_TI_TV_SHIFT;
	if (ta->role == DECT_TRANSACTION_RESPONDER)
		mb->data[0] |= DECT_S_TI_F_FLAG;

	if (ta->mb != NULL)
		dect_mbuf_free(dh, ta->mb);
	ta->mb = mb;
	mb->refcnt++;

	switch (ddl->state) {
	case DECT_DATA_LINK_ESTABLISHED:
		return dect_ddl_send(dh, ddl, mb);
	case DECT_DATA_LINK_ESTABLISH_PENDING:
		list_add_tail(&mb->list, &ddl->msg_queue);
		return 0;
	default:
		ddl_debug(ddl, "Invalid state: %u\n", ddl->state);
		BUG();
	}
}

int dect_lce_retransmit(const struct dect_handle *dh,
			struct dect_transaction *ta)
{
	struct dect_data_link *ddl = ta->link;

	if (ta->mb != NULL &&
	    ddl->state == DECT_DATA_LINK_ESTABLISHED) {
		ta->mb->refcnt++;
		return dect_ddl_send(dh, ddl, ta->mb);
	} else
		return 0;
}

static void dect_ddl_rcv_msg(struct dect_handle *dh, struct dect_data_link *ddl)
{
	DECT_DEFINE_MSG_BUF_ONSTACK(_mb), *mb = &_mb;
	struct dect_transaction *ta;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	char cmsg_buf[4 * CMSG_SPACE(16)];
	uint8_t pd, tv;
	bool f;

	msg.msg_control		= cmsg_buf;
	msg.msg_controllen	= sizeof(cmsg_buf);

	if (dect_mbuf_rcv(ddl->dfd, &msg, mb) < 0) {
		switch (errno) {
		case ENOTCONN:
			if (ddl->state == DECT_DATA_LINK_RELEASE_PENDING)
				return dect_ddl_release_complete(dh, ddl);
			else
				return dect_ddl_shutdown(dh, ddl);
		case ETIMEDOUT:
		case ECONNRESET:
		case EHOSTUNREACH:
			return dect_ddl_shutdown(dh, ddl);
		default:
			ddl_debug(ddl, "unhandled receive error: %s",
				  strerror(errno));
			BUG();
		}
	}

	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
		const struct dect_dl_encrypt *dle;

		if (cmsg->cmsg_level != SOL_DECT)
			continue;

		switch (cmsg->cmsg_type) {
		case DECT_DL_ENCRYPT:
			if (cmsg->cmsg_len < CMSG_LEN(sizeof(*dle)))
				continue;
			dle = (struct dect_dl_encrypt *)CMSG_DATA(cmsg);
			dect_ddl_encrypt_ind(dh, ddl, dle->status);
			continue;
		default:
			ddl_debug(ddl, "unhandled cmsg: %u\n", cmsg->cmsg_type);
			continue;
		}
	}

	dect_mbuf_dump(DECT_DEBUG_LCE, mb, "LCE: RX");

	if (mb->len < DECT_S_HDR_SIZE)
		return;
	f  = (mb->data[0] & DECT_S_TI_F_FLAG);
	tv = (mb->data[0] & DECT_S_TI_TV_MASK) >> DECT_S_TI_TV_SHIFT;
	pd = (mb->data[0] & DECT_S_PD_MASK);
	mb->type = (mb->data[1] & DECT_S_PD_MSG_TYPE_MASK);
	dect_mbuf_pull(mb, DECT_S_HDR_SIZE);

	if (pd >= array_size(protocols) || protocols[pd] == NULL) {
		ddl_debug(ddl, "unknown protocol %u", pd);
		return;
	}

	if (tv >= protocols[pd]->max_transactions) {
		ddl_debug(ddl, "invalid %s transaction value %u\n",
			  protocols[pd]->name, tv);
		return;
	}

	if (dect_timer_running(ddl->sdu_timer))
		dect_ddl_stop_sdu_timer(dh, ddl);

	if (pd == DECT_PD_CLMS && tv == DECT_TV_CONNECTIONLESS)
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

static void dect_ddl_complete_direct_establish(struct dect_handle *dh,
					       struct dect_data_link *ddl)
{
	struct dect_msg_buf *mb, *mb_next;

	ddl->state = DECT_DATA_LINK_ESTABLISHED;
	ddl_debug(ddl, "complete direct link establishment");

	dect_fd_unregister(dh, ddl->dfd);
	if (dect_fd_register(dh, ddl->dfd, DECT_FD_READ) < 0)
		return dect_ddl_shutdown(dh, ddl);

	/* Send queued messages */
	list_for_each_entry_safe(mb, mb_next, &ddl->msg_queue, list) {
		list_del(&mb->list);
		dect_ddl_send(dh, ddl, mb);
	}
}

static void dect_ddl_complete_indirect_establish(struct dect_handle *dh,
						 struct dect_data_link *ddl,
						 struct dect_data_link *req)
{
	struct dect_transaction *ta, *ta_next;
	struct dect_msg_buf *mb, *mb_next;
	unsigned int i;

	/* Stop page timer */
	dect_timer_stop(dh, req->page_timer);
	dect_timer_free(dh, req->page_timer);

	ddl_debug(ddl, "complete indirect link establishment req %p", req);
	dect_ddl_set_ipui(dh, ddl, &req->ipui);

	for (i = 0; i < array_size(protocols); i++) {
		if (protocols[i] && protocols[i]->rebind != NULL)
			protocols[i]->rebind(dh, req, ddl);
	}

	/* Transfer transactions to the new link */
	list_for_each_entry_safe(ta, ta_next, &req->transactions, list) {
		ddl_debug(ta->link, "transfer transaction to link %p", ddl);
		list_move_tail(&ta->list, &ddl->transactions);
		ta->link = ddl;
	}

	/* Send queued messages */
	list_for_each_entry_safe(mb, mb_next, &req->msg_queue, list) {
		list_del(&mb->list);
		dect_ddl_send(dh, ddl, mb);
	}

	/* Release pending link */
	dect_ddl_destroy(dh, req);
}

static void dect_ddl_page_timer(struct dect_handle *dh, struct dect_timer *timer);
static void dect_lce_data_link_event(struct dect_handle *dh,
				     struct dect_fd *dfd, uint32_t events);

/**
 * dect_ddl_establish - Establish an outgoing data link
 */
static struct dect_data_link *dect_ddl_establish(struct dect_handle *dh,
						 const struct dect_ipui *ipui)
{
	struct dect_data_link *ddl;

	//lte = dect_lte_get_by_ipui(dh, lte);
	ddl = dect_ddl_alloc(dh);
	if (ddl == NULL)
		goto err1;
	ddl->state = DECT_DATA_LINK_ESTABLISH_PENDING;
	dect_ddl_set_ipui(dh, ddl, ipui);

	if (dh->mode == DECT_MODE_FP) {
		ddl->page_timer = dect_timer_alloc(dh);
		if (ddl->page_timer == NULL)
			goto err2;
		dect_timer_setup(ddl->page_timer, dect_ddl_page_timer, ddl);
		dect_ddl_page_timer(dh, ddl->page_timer);
	} else {
		ddl->dfd = dect_socket(dh, SOCK_SEQPACKET, DECT_S_SAP);
		if (ddl->dfd == NULL)
			goto err2;

		ddl->dlei.dect_family = AF_DECT;
		ddl->dlei.dect_index  = dh->index;
		ddl->dlei.dect_ari = dect_build_ari(&dh->pari) >> 24;
		ddl->dlei.dect_pmid = dh->pmid;
		ddl->dlei.dect_lln = 1;
		ddl->dlei.dect_sapi = 0;

		dect_fd_setup(ddl->dfd, dect_lce_data_link_event, ddl);
		if (dect_fd_register(dh, ddl->dfd, DECT_FD_WRITE) < 0)
			goto err2;

		if (connect(ddl->dfd->fd, (struct sockaddr *)&ddl->dlei,
			    sizeof(ddl->dlei)) < 0 && errno != EAGAIN)
			goto err3;
	}

	list_add_tail(&ddl->list, &dh->links);
	return ddl;

err3:
	dect_fd_unregister(dh, ddl->dfd);
err2:
	dect_free(dh, ddl);
err1:
	lce_debug("dect_ddl_establish: %s\n", strerror(errno));
	return NULL;
}

struct dect_data_link *dect_ddl_connect(struct dect_handle *dh,
					const struct dect_ipui *ipui)
{
	struct dect_data_link *ddl;

	ddl = dect_ddl_get_by_ipui(dh, ipui);
	if (ddl == NULL)
		ddl = dect_ddl_establish(dh, ipui);
	return ddl;
}

static void dect_lce_data_link_event(struct dect_handle *dh,
				     struct dect_fd *dfd, uint32_t events)
{
	struct dect_data_link *ddl = dfd->data;

	dect_debug(DECT_DEBUG_LCE, "\n");
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

		/* Close the page transaction after receiving the first
		 * message, which is expected to initiate a higher layer
		 * protocol transaction or reject the page response.
		 */
		if (dh->page_transaction.state == DECT_TRANSACTION_OPEN) {
			dect_debug(DECT_DEBUG_LCE, "\n");
			dect_transaction_close(dh, &dh->page_transaction,
					       DECT_DDL_RELEASE_NORMAL);
		}
	}
}

static void dect_lce_ssap_listener_event(struct dect_handle *dh,
					 struct dect_fd *dfd, uint32_t events)
{
	struct dect_data_link *ddl;
	struct dect_fd *nfd;

	dect_debug(DECT_DEBUG_LCE, "\n");
	ddl = dect_ddl_alloc(dh);
	if (ddl == NULL)
		goto err1;

	nfd = dect_accept(dh, dfd, (struct sockaddr *)&ddl->dlei,
			  sizeof(ddl->dlei));
	if (nfd == NULL)
		goto err2;
	ddl->dfd = nfd;

	dect_fd_setup(nfd, dect_lce_data_link_event, ddl);
	if (dect_fd_register(dh, nfd, DECT_FD_READ) < 0)
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
	dect_fd_unregister(dh, nfd);
err3:
	dect_close(dh, nfd);
err2:
	dect_free(dh, ddl);
err1:
	lce_debug("dect_lce_ssap_listener_event: %s\n", strerror(errno));
	return;
}

/*
 * Paging
 */

ssize_t dect_lce_broadcast(const struct dect_handle *dh,
			   const struct dect_msg_buf *mb,
			   bool long_page)
{
	struct msghdr msg;
	struct dect_bsap_auxdata aux;
	struct cmsghdr *cmsg;
	union {
		struct cmsghdr		cmsg;
		char			buf[CMSG_SPACE(sizeof(aux))];
	} cmsg_buf;
	ssize_t size;

	if (long_page) {
		memset(cmsg_buf.buf, 0, sizeof(cmsg_buf.buf));
		msg.msg_control		= &cmsg_buf;
		msg.msg_controllen	= sizeof(cmsg_buf);

		cmsg			= CMSG_FIRSTHDR(&msg);
		cmsg->cmsg_len		= CMSG_LEN(sizeof(aux));
		cmsg->cmsg_level	= SOL_DECT;
		cmsg->cmsg_type		= DECT_BSAP_AUXDATA;

		aux.long_page		= true;
		memcpy(CMSG_DATA(cmsg), &aux, sizeof(aux));
	} else {
		msg.msg_control		= NULL;
		msg.msg_controllen	= 0;
	}

	dect_mbuf_dump(DECT_DEBUG_LCE, mb, "LCE: BCAST TX");
	size = dect_mbuf_send(dh, dh->b_sap, &msg, mb);
	dect_assert(size == (ssize_t)mb->len);
	return 0;
}

/**
 * Request collective or group ringing
 *
 * @param dh		libdect DECT handle
 * @param pattern	ring pattern
 */
int dect_lce_group_ring_req(struct dect_handle *dh,
			    enum dect_alerting_patterns pattern)
{
	DECT_DEFINE_MSG_BUF_ONSTACK(_mb), *mb = &_mb;
	struct dect_short_page_msg *msg;
	uint16_t page;

	dect_debug(DECT_DEBUG_LCE, "\nLCE: LCE_GROUP_RING-req\n");
	msg = dect_mbuf_put(mb, sizeof(*msg));

	msg->hdr  = DECT_LCE_PAGE_W_FLAG;
	msg->hdr |= DECT_LCE_PAGE_GENERAL_VOICE;

	page = pattern << DECT_LCE_SHORT_PAGE_RING_PATTERN_SHIFT;
	page = 0;
	page |= DECT_TPUI_CBI & DECT_LCE_SHORT_PAGE_TPUI_MASK;
	msg->information = __cpu_to_be16(page);

	return dect_lce_broadcast(dh, mb, false);
}
EXPORT_SYMBOL(dect_lce_group_ring_req);

static int dect_lce_send_short_page(const struct dect_handle *dh,
				    const struct dect_ipui *ipui)
{
	DECT_DEFINE_MSG_BUF_ONSTACK(_mb), *mb = &_mb;
	struct dect_short_page_msg *msg;
	struct dect_tpui tpui;
	uint16_t page;

	msg = dect_mbuf_put(mb, sizeof(*msg));
	msg->hdr = DECT_LCE_PAGE_GENERAL_VOICE;

	page = dect_build_tpui(dect_ipui_to_tpui(&tpui, ipui)) &
	       DECT_LCE_SHORT_PAGE_TPUI_MASK;
	msg->information = __cpu_to_be16(page);

	return dect_lce_broadcast(dh, mb, false);
}

static int dect_lce_send_full_page(const struct dect_handle *dh,
				   const struct dect_ipui *ipui)
{
	DECT_DEFINE_MSG_BUF_ONSTACK(_mb), *mb = &_mb;
	struct dect_full_page_msg *msg;
	struct dect_tpui tpui;
	uint8_t ipui_buf[8];
	uint32_t page;

	msg = dect_mbuf_put(mb, sizeof(*msg));
	msg->hdr = DECT_LCE_PAGE_GENERAL_VOICE;

	if (1) {
		msg->hdr |= DECT_LCE_PAGE_W_FLAG;

		page  = dect_build_tpui(dect_ipui_to_tpui(&tpui, ipui)) <<
			DECT_LCE_FULL_PAGE_TPUI_SHIFT;
		page |= DECT_LCE_PAGE_FULL_SLOT <<
			DECT_LCE_FULL_PAGE_SLOT_TYPE_SHIFT;
		page |= DECT_LCE_PAGE_BASIC_CONN_ATTR_OPTIONAL <<
			DECT_LCE_FULL_PAGE_SETUP_INFO_SHIFT;
	} else {
		dect_build_ipui(ipui_buf, ipui);
		page  = ipui->put << 24;
		page |= (ipui_buf[1] & 0x0f) << 24;
		page |= ipui_buf[2] << 16;
		page |= ipui_buf[3] << 8;
		page |= ipui_buf[4];
	}
	msg->information = __cpu_to_be32(page);

	return dect_lce_broadcast(dh, mb, false);
}

static int dect_lce_page(const struct dect_handle *dh,
			 const struct dect_ipui *ipui)
{
	if (1)
		return dect_lce_send_short_page(dh, ipui);
	else
		return dect_lce_send_full_page(dh, ipui);
}

static void dect_ddl_page_timer(struct dect_handle *dh, struct dect_timer *timer)
{
	struct dect_data_link *ddl = timer->data;

	if (ddl->page_count) {
		dect_debug(DECT_DEBUG_LCE, "\n");
		ddl_debug(ddl, "Page timer");
	}

	if (ddl->page_count++ == DECT_DDL_PAGE_RETRANS_MAX)
		dect_ddl_shutdown(dh, ddl);
	else {
		dect_lce_page(dh, &ddl->ipui);
		dect_timer_start(dh, ddl->page_timer, DECT_DDL_PAGE_TIMEOUT);
	}
}

static int dect_lce_send_page_reject(const struct dect_handle *dh,
				     struct dect_transaction *ta,
				     struct dect_ie_portable_identity *portable_identity,
				     enum dect_reject_reasons reason)
{
	struct dect_ie_reject_reason reject_reason;
	struct dect_lce_page_reject_msg msg = {
		.portable_identity	= portable_identity,
		.reject_reason		= &reject_reason,
	};

	reject_reason.reason = reason;

	return dect_lce_send(dh, ta, &lce_page_reject_msg_desc,
			     &msg.common, DECT_LCE_PAGE_REJECT);
}

static void dect_lce_rcv_page_response(struct dect_handle *dh,
				       struct dect_transaction *ta,
				       struct dect_msg_buf *mb)
{
	struct dect_lce_page_response_msg msg;
	struct dect_data_link *i, *req = NULL;
	enum dect_sfmt_error err;
	bool reject = true;

	ddl_debug(ta->link, "LCE-PAGE-RESPONSE");
	err = dect_parse_sfmt_msg(dh, &lce_page_response_msg_desc,
				  &msg.common, mb);
	if (err < 0) {
		dect_lce_send_page_reject(dh, ta, NULL, dect_sfmt_reject_reason(err));
		return dect_ddl_release(dh, ta->link);
	}

	list_for_each_entry(i, &dh->links, list) {
		if (dect_ipui_cmp(&i->ipui, &msg.portable_identity->ipui))
			continue;
		if (i->state != DECT_DATA_LINK_ESTABLISH_PENDING)
			continue;
		req = i;
		break;
	}

	dect_ddl_set_ipui(dh, ta->link, &msg.portable_identity->ipui);

	if (req == NULL && dh->ops->lce_ops &&
	    dh->ops->lce_ops->lce_page_response) {
		struct dect_lce_page_param *param;

		param = dect_ie_collection_alloc(dh, sizeof(*param));
		if (param == NULL)
			goto err;

		param->portable_identity	= dect_ie_hold(msg.portable_identity);
		param->fixed_identity		= dect_ie_hold(msg.fixed_identity);
		param->nwk_assigned_identity	= dect_ie_hold(msg.nwk_assigned_identity);
		param->cipher_info		= dect_ie_hold(msg.cipher_info);
		param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

		reject = !dh->ops->lce_ops->lce_page_response(dh, param);
		dect_ie_collection_put(dh, param);
	}
err:
	if (req != NULL)
		dect_ddl_complete_indirect_establish(dh, ta->link, req);
	else if (reject) {
		dect_lce_send_page_reject(dh, ta, msg.portable_identity,
					  DECT_REJECT_IPUI_UNKNOWN);
		dect_ddl_release(dh, ta->link);
	}

	dect_msg_free(dh, &lce_page_response_msg_desc, &msg.common);
}

static void dect_lce_rcv_page_reject(struct dect_handle *dh,
				     struct dect_transaction *ta,
				     struct dect_msg_buf *mb)
{
	struct dect_lce_page_reject_msg msg;

	ddl_debug(ta->link, "LCE-PAGE-REJECT");
	if (dect_parse_sfmt_msg(dh, &lce_page_reject_msg_desc,
				&msg.common, mb) < 0)
		return;
	dect_msg_free(dh, &lce_page_reject_msg_desc, &msg.common);
}

static void dect_lce_send_page_response(struct dect_handle *dh)
{
	struct dect_ie_portable_identity portable_identity;
	struct dect_ie_fixed_identity fixed_identity;
	struct dect_lce_page_response_msg msg = {
		.portable_identity	= &portable_identity,
		.fixed_identity		= &fixed_identity,
	};

	portable_identity.type = DECT_PORTABLE_ID_TYPE_IPUI;
	portable_identity.ipui = dh->ipui;

	fixed_identity.type    = DECT_FIXED_ID_TYPE_PARK;
	fixed_identity.ari     = dh->pari;

	if (dect_transaction_open(dh, &dh->page_transaction, &dh->ipui,
				  DECT_PD_LCE) < 0)
		return;

	dect_lce_send(dh, &dh->page_transaction, &lce_page_response_msg_desc,
		      &msg.common, DECT_LCE_PAGE_RESPONSE);
}

static void dect_lce_rcv_short_page(struct dect_handle *dh,
				    struct dect_msg_buf *mb)
{
	struct dect_short_page_msg *msg = (void *)mb->data;
	struct dect_tpui tpui;
	uint16_t info, t;
	uint8_t hdr, pattern;
	bool w;

	w    = msg->hdr & DECT_LCE_PAGE_W_FLAG;
	hdr  = msg->hdr & DECT_LCE_PAGE_HDR_MASK;
	info = __be16_to_cpu(msg->information);
	lce_debug("short page: w: %u hdr: %u information: %04x\n", w, hdr, info);

	if (hdr == DECT_LCE_PAGE_UNKNOWN_RINGING) {
		pattern = (info & DECT_LCE_SHORT_PAGE_RING_PATTERN_MASK) >>
			  DECT_LCE_SHORT_PAGE_RING_PATTERN_SHIFT;

		if (w == 0) {
			/* assigned connectionless group TPUI or CBI */
			if ((info ^ DECT_TPUI_CBI) &
			    DECT_LCE_SHORT_PAGE_GROUP_MASK)
				return;
		} else {
			/* group mask */
			return;
		}

		lce_debug("LCE_GROUP_RING-ind: pattern: %x\n", pattern);
		dh->ops->lce_ops->lce_group_ring_ind(dh, pattern);
	} else {
		if (w == 0) {
			/* default individual TPUI */
			t = dect_build_tpui(dect_ipui_to_tpui(&tpui, &dh->ipui));
			if (info != t)
				return;
		} else {
			/* assigned TPUI or CBI */
			t = dect_build_tpui(&dh->tpui);
			if (info != t && info != DECT_TPUI_CBI)
				return;
		}

		dect_lce_send_page_response(dh);
	}
}

static void dect_lce_rcv_full_page(struct dect_handle *dh,
				   struct dect_msg_buf *mb)
{
	struct dect_full_page_msg *msg = (void *)mb->data;
	uint32_t info, ipui, tpui, t;
	uint8_t ipui_buf[8];
	uint8_t hdr, pattern;
	bool w;

	w    = msg->hdr & DECT_LCE_PAGE_W_FLAG;
	hdr  = msg->hdr & DECT_LCE_PAGE_HDR_MASK;
	info = __be32_to_cpu(msg->information);
	lce_debug("full page: w: %u hdr: %u information: %08x\n", w, hdr, info);

	if (hdr == DECT_LCE_PAGE_UNKNOWN_RINGING) {
		pattern = (info & DECT_LCE_FULL_PAGE_RING_PATTERN_MASK) >>
			  DECT_LCE_FULL_PAGE_RING_PATTERN_SHIFT;
		tpui    = (info & DECT_LCE_FULL_PAGE_GROUP_MASK) >>
			  DECT_LCE_FULL_PAGE_GROUP_SHIFT;

		if (w == 0) {
			/* assigned connectionless group TPUI or CBI */
			if (tpui != DECT_TPUI_CBI)
				return;
		} else {
			/* group mask */
			return;
		}

		lce_debug("LCE_GROUP_RING-ind: pattern: %x\n", pattern);
		dh->ops->lce_ops->lce_group_ring_ind(dh, pattern);
	} else {
		if (w == 0) {
			/* IPUI */
			dect_build_ipui(ipui_buf, &dh->ipui);
			ipui  = dh->ipui.put << 24;
			ipui |= (ipui_buf[1] & 0x0f) << 24;
			ipui |= ipui_buf[2] << 16;
			ipui |= ipui_buf[3] << 8;
			ipui |= ipui_buf[4];

			if (info != ipui)
				return;
		} else {
			/* assigned TPUI or CBI */
			tpui = (info & DECT_LCE_FULL_PAGE_TPUI_MASK) >>
			       DECT_LCE_FULL_PAGE_TPUI_SHIFT;

			t = dect_build_tpui(&dh->tpui);
			if (tpui != t && tpui != DECT_TPUI_CBI)
				return;
		}

		dect_lce_send_page_response(dh);
	}
}

static void dect_lce_rcv_long_page(struct dect_handle *dh,
				   struct dect_msg_buf *mb)
{
	lce_debug("long page: length: %u\n", mb->len);
	dect_clms_rcv_fixed(dh, mb);
}

static void dect_lce_bsap_event(struct dect_handle *dh, struct dect_fd *dfd,
				uint32_t events)
{
	DECT_DEFINE_MSG_BUF_ONSTACK(_mb), *mb = &_mb;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	char cmsg_buf[4 * CMSG_SPACE(16)];
	bool long_page = false;

	dect_debug(DECT_DEBUG_LCE, "\n");

	msg.msg_control		= cmsg_buf;
	msg.msg_controllen	= sizeof(cmsg_buf);

	if (dect_mbuf_rcv(dfd, &msg, mb) < 0)
		return;

	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
		const struct dect_bsap_auxdata *aux;

		if (cmsg->cmsg_level != SOL_DECT)
			continue;

		switch (cmsg->cmsg_type) {
		case DECT_BSAP_AUXDATA:
			if (cmsg->cmsg_len < CMSG_LEN(sizeof(*aux)))
				continue;
			aux = (struct dect_bsap_auxdata *)CMSG_DATA(cmsg);
			long_page = aux->long_page;
			continue;
		default:
			lce_debug("LCE: unhandled cmsg: %u\n", cmsg->cmsg_type);
			continue;
		}
	}

	dect_mbuf_dump(DECT_DEBUG_LCE, mb, "LCE: BCAST RX");

	switch (mb->len) {
	case 3:
		return dect_lce_rcv_short_page(dh, mb);
	case 5:
		if (!long_page)
			return dect_lce_rcv_full_page(dh, mb);
	default:
		return dect_lce_rcv_long_page(dh, mb);
	}
}

static void dect_lce_rcv(struct dect_handle *dh, struct dect_transaction *ta,
			 struct dect_msg_buf *mb)
{
	switch (mb->type) {
	case DECT_LCE_PAGE_REJECT:
		return dect_lce_rcv_page_reject(dh, ta, mb);
	default:
		ddl_debug(ta->link, "unknown message type %x", mb->type);
		return;
	}
}

static void dect_lce_open(struct dect_handle *dh,
			  struct dect_transaction *ta,
			  struct dect_msg_buf *mb)
{
	switch (mb->type) {
	case DECT_LCE_PAGE_RESPONSE:
		return dect_lce_rcv_page_response(dh, ta, mb);
	default:
		ddl_debug(ta->link, "unknown message type %x", mb->type);
		return;
	}
}

static void dect_lce_shutdown(struct dect_handle *dh,
			      struct dect_transaction *ta)
{
	lce_debug("shutdown page transaction\n");
	dect_transaction_close(dh, ta, DECT_DDL_RELEASE_NORMAL);
}

static const struct dect_nwk_protocol lce_protocol = {
	.name			= "Link Control",
	.pd			= DECT_PD_LCE,
	.max_transactions	= 1,
	.open			= dect_lce_open,
	.rcv			= dect_lce_rcv,
	.shutdown		= dect_lce_shutdown,
};

/*
 * Transactions
 */

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

static void dect_transaction_link(struct dect_data_link *ddl,
				  struct dect_transaction *ta)
{
	struct dect_transaction *last;

	/* Insert MM transactions at the end of the list to make sure they get
	 * destroyed last on shutdown. This makes sure that other protocols
	 * which might invoke and wait for the completion of MM transactions
	 * have their transactions terminated first and don't mistake a link
	 * shutdown or a protocol specific error for a MM error.
	 *
	 * Ordering among MM transactions is such that the transaction opened
	 * last is shut down first.
	 */
	if (ta->pd == DECT_PD_MM) {
		last = list_last_entry(&ddl->transactions, struct dect_transaction, list);

		if (!list_empty(&ddl->transactions) && last->pd == DECT_PD_MM)
			list_add_tail(&ta->list, &last->list);
		else
			list_add_tail(&ta->list, &ddl->transactions);
	} else
		list_add(&ta->list, &ddl->transactions);
}

int dect_ddl_transaction_open(struct dect_handle *dh, struct dect_transaction *ta,
			      struct dect_data_link *ddl, enum dect_pds pd)
{
	const struct dect_nwk_protocol *protocol = protocols[pd];
	int tv;

	tv = dect_transaction_alloc_tv(ddl, protocol);
	if (tv < 0)
		return -1;

	ddl_debug(ddl, "open transaction: %s TV: %u", protocol->name, tv);
	ta->link  = ddl;
	ta->mb    = NULL;
	ta->pd	  = pd;
	ta->role  = DECT_TRANSACTION_INITIATOR;
	ta->state = DECT_TRANSACTION_OPEN;
	ta->tv    = tv;

	dect_transaction_link(ddl, ta);
	return 0;
}

int dect_transaction_open(struct dect_handle *dh, struct dect_transaction *ta,
			  const struct dect_ipui *ipui, enum dect_pds pd)
{
	struct dect_data_link *ddl;

	ddl = dect_ddl_connect(dh, ipui);
	if (ddl == NULL)
		return -1;

	return dect_ddl_transaction_open(dh, ta, ddl, pd);
}

void dect_transaction_confirm(struct dect_handle *dh, struct dect_transaction *ta,
			      const struct dect_transaction *req)
{
	ta->link  = req->link;
	ta->mb    = NULL;
	ta->tv    = req->tv;
	ta->role  = req->role;
	ta->pd    = req->pd;
	ta->state = DECT_TRANSACTION_OPEN;

	ddl_debug(req->link, "confirm transaction: %s TV: %u Role: %u",
		  protocols[ta->pd]->name, ta->tv, ta->role);
	dect_transaction_link(req->link, ta);
}

void dect_transaction_close(struct dect_handle *dh, struct dect_transaction *ta,
			    enum dect_release_modes mode)
{
	struct dect_data_link *ddl = ta->link;

	ddl_debug(ddl, "close transaction: %s TV: %u Role: %u",
		  protocols[ta->pd]->name, ta->tv, ta->role);

	list_del(&ta->list);
	ta->state = DECT_TRANSACTION_CLOSED;
	if (ta->mb != NULL)
		dect_mbuf_free(dh, ta->mb);

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
	addr->dect_index  = ddl->dlei.dect_index;
	addr->dect_ari    = ddl->dlei.dect_ari;
	addr->dect_pmid   = ddl->dlei.dect_pmid;
	addr->dect_lcn    = ddl->dlei.dect_lcn;
}

/*
 * Identities
 */

static void dect_pp_set_default_pmid(struct dect_handle *dh)
{
	dect_assert(!(dh->flags & DECT_PP_TPUI));
	dh->pmid = DECT_PMID_DEFAULT_ID +
		   (rand() & DECT_PMID_DEFAULT_NUM_MASK);
	lce_debug("set default pmid %05x\n", dh->pmid);
}

void dect_pp_change_pmid(struct dect_handle *dh)
{
	dh->pmid = DECT_PMID_DEFAULT_ID +
		   ((dh->pmid + 1) & DECT_PMID_DEFAULT_NUM_MASK);
	lce_debug("change pmid %05x\n", dh->pmid);
}

static void dect_pp_set_assigned_pmid(struct dect_handle *dh)
{
	struct dect_pmid pmid;

	dect_assert(dh->flags & DECT_PP_TPUI &&
	       dh->tpui.type == DECT_TPUI_INDIVIDUAL_ASSIGNED);
	dh->pmid = dect_build_pmid(dect_tpui_to_pmid(&pmid, &dh->tpui));
	lce_debug("set assigned pmid %05x\n", dh->pmid);
}

/**
 * Set the PP's IPUI
 *
 * @param dh		libdect DECT handle
 * @param ipui		IPUI
 */
void dect_pp_set_ipui(struct dect_handle *dh, const struct dect_ipui *ipui)
{
	dh->ipui = *ipui;
	dh->flags |= DECT_PP_IPUI;
}
EXPORT_SYMBOL(dect_pp_set_ipui);

/**
 * Set the PP's TPUI
 *
 * @param dh		libdect DECT handle
 * @param tpui		TPUI
 */
void dect_pp_set_tpui(struct dect_handle *dh, const struct dect_tpui *tpui)
{
	dh->tpui = *tpui;
	dh->flags |= DECT_PP_TPUI;
	dect_pp_set_assigned_pmid(dh);
}
EXPORT_SYMBOL(dect_pp_set_tpui);

int dect_lce_init(struct dect_handle *dh)
{
	struct sockaddr_dect_ssap s_addr;
	struct sockaddr_dect b_addr;

	if (dh->mode == DECT_MODE_PP)
		dect_pp_set_default_pmid(dh);

	/* Open B-SAP socket */
	dh->b_sap = dect_socket(dh, SOCK_DGRAM, DECT_B_SAP);
	if (dh->b_sap == NULL)
		goto err1;

	memset(&b_addr, 0, sizeof(b_addr));
	b_addr.dect_family = AF_DECT;
	b_addr.dect_index = dh->index;
	if (bind(dh->b_sap->fd, (struct sockaddr *)&b_addr, sizeof(b_addr)) < 0)
		goto err2;

	dect_fd_setup(dh->b_sap, dect_lce_bsap_event, NULL);
	if (dect_fd_register(dh, dh->b_sap, DECT_FD_READ) < 0)
		goto err2;

	dh->page_transaction.state = DECT_TRANSACTION_CLOSED;

	/* Open S-SAP listener socket */
	if (dh->mode == DECT_MODE_FP) {
		dh->s_sap = dect_socket(dh, SOCK_SEQPACKET, DECT_S_SAP);
		if (dh->s_sap == NULL)
			goto err3;

		memset(&s_addr, 0, sizeof(s_addr));
		s_addr.dect_family = AF_DECT;
		s_addr.dect_index  = dh->index;
		s_addr.dect_lln    = 1;
		s_addr.dect_sapi   = 0;

		if (bind(dh->s_sap->fd, (struct sockaddr *)&s_addr,
			 sizeof(s_addr)) < 0)
			goto err4;
		if (listen(dh->s_sap->fd, 10) < 0)
			goto err4;

		dect_fd_setup(dh->s_sap, dect_lce_ssap_listener_event, NULL);
		if (dect_fd_register(dh, dh->s_sap, DECT_FD_READ) < 0)
			goto err4;
	}

	dect_lce_register_protocol(&lce_protocol);
	dect_lce_register_protocol(&dect_cc_protocol);
	dect_lce_register_protocol(&dect_ciss_protocol);
	dect_lce_register_protocol(&dect_clms_protocol);
	dect_lce_register_protocol(&dect_mm_protocol);
	return 0;

err4:
	dect_close(dh, dh->s_sap);
err3:
	dect_fd_unregister(dh, dh->b_sap);
err2:
	dect_close(dh, dh->b_sap);
err1:
	lce_debug("dect_lce_init: %s\n", strerror(errno));
	return -1;
}

void dect_lce_exit(struct dect_handle *dh)
{
	struct dect_data_link *ddl, *next;

	list_for_each_entry_safe(ddl, next, &dh->links, list)
		dect_ddl_shutdown(dh, ddl);

	if (dh->mode == DECT_MODE_FP) {
		dect_fd_unregister(dh, dh->s_sap);
		dect_close(dh, dh->s_sap);
	}

	dect_fd_unregister(dh, dh->b_sap);
	dect_close(dh, dh->b_sap);
}

/** @} */
/** @} */
