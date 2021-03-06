/*
 * DECT Supplementary Services (SS)
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/**
 * @defgroup ss Supplementary Services
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
#include <s_fmt.h>
#include <lce.h>
#include <ss.h>

static DECT_SFMT_MSG_DESC(ciss_register,
	DECT_SFMT_IE(DECT_IE_PORTABLE_IDENTITY,		IE_OPTIONAL,  IE_MANDATORY, 0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_FACILITY,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_SINGLE_KEYPAD,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_FEATURE_ACTIVATE,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_FEATURE_INDICATE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(ciss_release_com,
	DECT_SFMT_IE(DECT_IE_RELEASE_REASON,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_FACILITY,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_SINGLE_KEYPAD,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_FEATURE_ACTIVATE,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_FEATURE_INDICATE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(ciss_facility,
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_FACILITY,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_SINGLE_KEYPAD,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_FEATURE_ACTIVATE,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_FEATURE_INDICATE,      	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_TIME_DATE,			IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_EVENTS_NOTIFICATION,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CALL_INFORMATION,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

#define __ss_debug(pfx, fmt, args...) \
	dect_debug(DECT_DEBUG_SS, "%sSS (link %d): " fmt "\n", pfx, \
		   (sse)->transaction.link && (sse)->transaction.link->dfd ? \
			(sse)->transaction.link->dfd->fd : -1, \
		   ## args)

#define ss_debug(sse, fmt, args...) \
	__ss_debug("", fmt, ## args)
#define ss_debug_entry(sse, fmt, args...) \
	__ss_debug("\n", fmt, ## args)

void dect_clss_rcv(struct dect_handle *dh, struct dect_msg_buf *mb)
{
	struct dect_ciss_facility_msg msg;

	if (mb->type != DECT_CISS_FACILITY)
		return;

	if (dect_parse_sfmt_msg(dh, &ciss_facility_msg_desc, &msg.common, mb) < 0)
		return;

	dect_msg_free(dh, &ciss_facility_msg_desc, &msg.common);
}

/**
 * Get a pointer to the private data area from a Supplementary Services Endpoint
 *
 * @param sse		Supplementary Services Endpoint
 */
void *dect_ss_priv(struct dect_ss_endpoint *sse)
{
	return sse->priv;
}
EXPORT_SYMBOL(dect_ss_priv);

struct dect_ss_endpoint *dect_ss_endpoint_alloc(struct dect_handle *dh,
						const struct dect_ipui *ipui)
{
	struct dect_ss_endpoint *sse;

	sse = dect_zalloc(dh, sizeof(*sse) + dh->ops->ss_ops->priv_size);
	if (sse == NULL)
		goto err1;
	sse->ipui = *ipui;

	return sse;

err1:
	return NULL;
}
EXPORT_SYMBOL(dect_ss_endpoint_alloc);

void dect_ss_endpoint_destroy(struct dect_handle *dh, struct dect_ss_endpoint *sse)
{
	dect_free(dh, sse);
}
EXPORT_SYMBOL(dect_ss_endpoint_destroy);

static struct dect_ss_endpoint *dect_ss_endpoint(struct dect_transaction *ta)
{
	return container_of(ta, struct dect_ss_endpoint, transaction);
}

/**
 * MNSS_SETUP-req primitive
 *
 * @param dh		libdect DECT handle
 * @param sse		Supplementary Services Endpoint
 * @param ipui		PT IPUI
 * @param param		Supplementary Services parameters
 */
int dect_mnss_setup_req(struct dect_handle *dh, struct dect_ss_endpoint *sse,
			const struct dect_mnss_param *param)
{
	struct dect_ie_portable_identity portable_identity;
	struct dect_ciss_register_msg msg = {
		.facility		= param->facility,
		.display		= param->display,
		.keypad			= param->keypad,
		.feature_activate	= param->feature_activate,
		.feature_indicate	= param->feature_indicate,
	};

	ss_debug_entry(sse, "MNSS_SETUP-req");
	if (dect_transaction_open(dh, &sse->transaction, &sse->ipui,
				  DECT_PD_CISS) < 0)
		goto err1;

	if (dh->mode == DECT_MODE_PP) {
		portable_identity.type = DECT_PORTABLE_ID_TYPE_IPUI;
		portable_identity.ipui = sse->ipui;
		msg.portable_identity  = &portable_identity;
	}

	if (dect_lce_send(dh, &sse->transaction, &ciss_register_msg_desc,
			  &msg.common, DECT_CISS_REGISTER) < 0)
		goto err2;
	return 0;

err2:
	dect_transaction_close(dh, &sse->transaction, DECT_DDL_RELEASE_NORMAL);
err1:
	return -1;
}
EXPORT_SYMBOL(dect_mnss_setup_req);

/**
 * MNSS_FACILITY-req primitive
 *
 * @param dh		libdect DECT handle
 * @param sse		Supplementary Services Endpoint
 * @param param		Supplementary Services parameters
 */
int dect_mnss_facility_req(struct dect_handle *dh, struct dect_ss_endpoint *sse,
			   const struct dect_mnss_param *param)
{
	struct dect_ciss_facility_msg msg = {
		.facility		= param->facility,
		.display		= param->display,
		.keypad			= param->keypad,
		.feature_activate	= param->feature_activate,
		.feature_indicate	= param->feature_indicate,
		.iwu_to_iwu		= param->iwu_to_iwu,
		.escape_to_proprietary	= param->escape_to_proprietary,
		.time_date		= param->time_date,
		.events_notification	= param->events_notification,
		.call_information	= param->call_information,
	};

	ss_debug_entry(sse, "MNSS_FACILITY-req");

	if (sse->transaction.link != NULL)
		return dect_lce_send(dh, &sse->transaction, &ciss_facility_msg_desc,
				     &msg.common, DECT_CISS_FACILITY);
	else
		return dect_lce_send_cl(dh, &sse->ipui, &ciss_facility_msg_desc,
					&msg.common, DECT_PD_CISS,
					DECT_CISS_FACILITY);
}
EXPORT_SYMBOL(dect_mnss_facility_req);

static void dect_ciss_rcv_facility(struct dect_handle *dh,
				   struct dect_ss_endpoint *sse,
				   struct dect_msg_buf *mb)
{
	struct dect_ciss_facility_msg msg;
	struct dect_mnss_param *param;

	ss_debug(sse, "CISS-FACILITY");
	if (dect_parse_sfmt_msg(dh, &ciss_facility_msg_desc, &msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto out;

	param->facility			= *dect_ie_list_hold(&msg.facility);
	param->display			= dect_ie_hold(msg.display);
	param->keypad			= dect_ie_hold(msg.keypad);
	param->feature_activate		= dect_ie_hold(msg.feature_activate);
	param->feature_indicate		= dect_ie_hold(msg.feature_indicate);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	ss_debug(sse, "MNSS_FACILITY-ind");
	dh->ops->ss_ops->mnss_facility_ind(dh, sse, param);
	dect_ie_collection_put(dh, param);
out:
	dect_msg_free(dh, &ciss_facility_msg_desc, &msg.common);
}

/**
 * MNSS_RELEASE-req primitive
 *
 * @param dh		libdect DECT handle
 * @param sse		Supplementary Services Endpoint
 * @param param		Supplementary Services parameters
 */
int dect_mnss_release_req(struct dect_handle *dh, struct dect_ss_endpoint *sse,
			  const struct dect_mnss_param *param)
{
	struct dect_ciss_release_com_msg msg = {
		.facility		= param->facility,
		.display		= param->display,
		.keypad			= param->keypad,
		.feature_activate	= param->feature_activate,
		.feature_indicate	= param->feature_indicate,
	};

	ss_debug_entry(sse, "MNSS_RELEASE-req");
	return dect_lce_send(dh, &sse->transaction, &ciss_release_com_msg_desc,
			     &msg.common, DECT_CISS_RELEASE_COM);
}
EXPORT_SYMBOL(dect_mnss_release_req);

static void dect_ciss_rcv_release_com(struct dect_handle *dh,
				      struct dect_ss_endpoint *sse,
				      struct dect_msg_buf *mb)
{
	struct dect_ciss_release_com_msg msg;
	struct dect_mnss_param *param;

	ss_debug(sse, "CISS-RELEASE-COM");
	if (dect_parse_sfmt_msg(dh, &ciss_release_com_msg_desc, &msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto out;

	param->release_reason		= dect_ie_hold(msg.release_reason);
	param->facility			= *dect_ie_list_hold(&msg.facility);
	param->display			= dect_ie_hold(msg.display);
	param->keypad			= dect_ie_hold(msg.keypad);
	param->feature_activate		= dect_ie_hold(msg.feature_activate);
	param->feature_indicate		= dect_ie_hold(msg.feature_indicate);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	ss_debug(sse, "MNSS_RELEASE-ind");
	dh->ops->ss_ops->mnss_release_ind(dh, sse, param);
	dect_ie_collection_put(dh, param);

	dect_transaction_close(dh, &sse->transaction, DECT_DDL_RELEASE_PARTIAL);
	dect_ss_endpoint_destroy(dh, sse);
out:
	dect_msg_free(dh, &ciss_release_com_msg_desc, &msg.common);
}

static void dect_ciss_rcv(struct dect_handle *dh, struct dect_transaction *ta,
			  struct dect_msg_buf *mb)
{
	struct dect_ss_endpoint *sse = dect_ss_endpoint(ta);

	switch (mb->type) {
	case DECT_CISS_FACILITY:
		return dect_ciss_rcv_facility(dh, sse, mb);
	case DECT_CISS_RELEASE_COM:
		return dect_ciss_rcv_release_com(dh, sse, mb);
	}

	ss_debug(sse, "receive unknown msg type %x", mb->type);
}

static void dect_ciss_rcv_register(struct dect_handle *dh,
				   const struct dect_transaction *req,
				   struct dect_msg_buf *mb)
{
	struct dect_ciss_register_msg msg;
	struct dect_mnss_param *param;
	struct dect_ss_endpoint *sse;

	dect_debug(DECT_DEBUG_SS, "CISS-REGISTER\n");
	if (dect_parse_sfmt_msg(dh, &ciss_register_msg_desc, &msg.common, mb) < 0)
		return;

	if (msg.portable_identity->type != DECT_PORTABLE_ID_TYPE_IPUI &&
	    msg.portable_identity->type != DECT_PORTABLE_ID_TYPE_IPEI)
		goto out;
	if (dect_ddl_set_ipui(dh, req->link, &msg.portable_identity->ipui) < 0)
		goto out;

	sse = dect_ss_endpoint_alloc(dh, &msg.portable_identity->ipui);
	if (sse == NULL)
		goto out;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto out;

	param->facility			= *dect_ie_list_hold(&msg.facility);
	param->display			= dect_ie_hold(msg.display);
	param->keypad			= dect_ie_hold(msg.keypad);
	param->feature_activate		= dect_ie_hold(msg.feature_activate);
	param->feature_indicate		= dect_ie_hold(msg.feature_indicate);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_transaction_confirm(dh, &sse->transaction, req);

	ss_debug(sse, "MNSS_SETUP-ind");
	dh->ops->ss_ops->mnss_setup_ind(dh, sse, param);
	dect_ie_collection_put(dh, param);
out:
	dect_msg_free(dh, &ciss_register_msg_desc, &msg.common);
}

static void dect_ciss_open(struct dect_handle *dh,
			   struct dect_transaction *req,
			   struct dect_msg_buf *mb)
{
	dect_debug(DECT_DEBUG_SS, "SS: unknown transaction: msg type: %x\n", mb->type);
	switch (mb->type) {
	case DECT_CISS_REGISTER:
		return dect_ciss_rcv_register(dh, req, mb);
	default:
		return;
	}
}

static void dect_ciss_shutdown(struct dect_handle *dh,
			       struct dect_transaction *ta)
{
	struct dect_ss_endpoint *sse = dect_ss_endpoint(ta);

	ss_debug(sse, "shutdown");
	dect_transaction_close(dh, &sse->transaction, DECT_DDL_RELEASE_NORMAL);
	dect_ss_endpoint_destroy(dh, sse);
}

const struct dect_nwk_protocol dect_ciss_protocol = {
	.name			= "Call Independant Supplementary Services",
	.pd			= DECT_PD_CISS,
	.max_transactions	= 7,
	.open			= dect_ciss_open,
	.shutdown		= dect_ciss_shutdown,
	.rcv			= dect_ciss_rcv,
};

/** @} */
