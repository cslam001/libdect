/*
 * DECT Supplementary Services (SS)
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
#include <s_fmt.h>
#include <lce.h>
#include <ss.h>

static DECT_SFMT_MSG_DESC(ciss_register,
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_OPTIONAL,  IE_MANDATORY, 0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_FACILITY,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_DO_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_DO_IE_SINGLE_KEYPAD,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_FEATURE_ACTIVATE,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_FEATURE_INDICATE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(ciss_release_com,
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_OPTIONAL,  IE_MANDATORY, 0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_FACILITY,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_DO_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_DO_IE_SINGLE_KEYPAD,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_FEATURE_ACTIVATE,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_FEATURE_INDICATE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(ciss_facility,
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_FACILITY,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_DO_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_DO_IE_SINGLE_KEYPAD,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_FEATURE_ACTIVATE,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_FEATURE_INDICATE,      	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_TIME_DATE,			IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_EVENTS_NOTIFICATION,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CALL_INFORMATION,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

#define __ss_debug(pfx, fmt, args...) \
	dect_debug("%sSS (link %d): " fmt "\n", pfx, \
		   (sse)->transaction.link ? (sse)->transaction.link->dfd->fd : -1, \
		   ## args)

#define ss_debug(sse, fmt, args...) \
	__ss_debug("", fmt, ## args)
#define ss_debug_entry(sse, fmt, args...) \
	__ss_debug("\n", fmt, ## args)

void dect_clss_rcv(struct dect_handle *dh, struct dect_msg_buf *mb)
{
	struct dect_ciss_facility_msg msg;

	if (mb->type != CISS_FACILITY)
		return;

	if (dect_parse_sfmt_msg(dh, &ciss_facility_msg_desc, &msg.common, mb) < 0)
		return;

	dect_msg_free(dh, &ciss_facility_msg_desc, &msg.common);
}

void *dect_ss_priv(struct dect_ss_endpoint *sse)
{
	return sse->priv;
}
EXPORT_SYMBOL(dect_ss_priv);

struct dect_ss_endpoint *dect_ss_endpoint_alloc(struct dect_handle *dh)
{
	struct dect_ss_endpoint *sse;

	sse = dect_zalloc(dh, sizeof(*sse) + dh->ops->ss_ops->priv_size);
	if (sse == NULL)
		goto err1;

	return sse;

err1:
	return NULL;
}
EXPORT_SYMBOL(dect_ss_endpoint_alloc);

static struct dect_ss_endpoint *dect_ss_endpoint(struct dect_transaction *ta)
{
	return container_of(ta, struct dect_ss_endpoint, transaction);
}

/**
 * dect_mnss_setup_req - MNSS_SETUP-req primitive
 *
 * @dh:		libdect DECT handle
 * @sse:	Supplementary Services Endpoint
 * @ipui:	PT IPUI
 * @param:	Supplementary Services parameters
 */
int dect_mnss_setup_req(struct dect_handle *dh, struct dect_ss_endpoint *sse,
			const struct dect_ipui *ipui,
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
	if (dect_open_transaction(dh, &sse->transaction, ipui, DECT_PD_CISS) < 0)
		goto err1;

	if (dh->mode == DECT_MODE_PP) {
		portable_identity.type = DECT_PORTABLE_ID_TYPE_IPUI;
		portable_identity.ipui = *ipui;
		msg.portable_identity  = &portable_identity;
	}

	if (dect_lce_send(dh, &sse->transaction, &ciss_register_msg_desc,
			  &msg.common, CISS_REGISTER) < 0)
		goto err2;
	return 0;

err2:
	dect_close_transaction(dh, &sse->transaction, DECT_DDL_RELEASE_NORMAL);
err1:
	return -1;
}
EXPORT_SYMBOL(dect_mnss_setup_req);

/**
 * dect_mnss_facility_req - MNSS_FACILITY-req primitive
 *
 * @dh:		libdect DECT handle
 * @sse:	Supplementary Services Endpoint
 * @param:	Supplementary Services parameters
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
	};

	ss_debug_entry(sse, "MNSS_FACILITY-req");
	return dect_lce_send(dh, &sse->transaction, &ciss_facility_msg_desc,
			     &msg.common, CISS_FACILITY);
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
 * dect_mnss_release_req - MNSS_RELEASE-req primitive
 *
 * @dh:		libdect DECT handle
 * @sse:	Supplementary Services Endpoint
 * @param:	Supplementary Services parameters
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
			     &msg.common, CISS_RELEASE_COM);
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
	dect_msg_free(dh, &ciss_release_com_msg_desc, &msg.common);
}

static void dect_ciss_rcv(struct dect_handle *dh, struct dect_transaction *ta,
			  struct dect_msg_buf *mb)
{
	struct dect_ss_endpoint *sse = dect_ss_endpoint(ta);

	switch (mb->type) {
	case CISS_FACILITY:
		return dect_ciss_rcv_facility(dh, sse, mb);
	case CISS_RELEASE_COM:
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

	dect_debug("CISS-REGISTER");
	if (dect_parse_sfmt_msg(dh, &ciss_register_msg_desc, &msg.common, mb) < 0)
		return;

	sse = dect_ss_endpoint_alloc(dh);
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

	dect_confirm_transaction(dh, &sse->transaction, req);

	ss_debug(sse, "MNSS_SETUP-ind");
	dh->ops->ss_ops->mnss_setup_ind(dh, sse, param);
	dect_ie_collection_put(dh, param);
out:
	dect_msg_free(dh, &ciss_register_msg_desc, &msg.common);
}

static void dect_ciss_open(struct dect_handle *dh,
			   const struct dect_transaction *req,
			   struct dect_msg_buf *mb)
{
	dect_debug("SS: unknown transaction: msg type: %x\n", mb->type);
	switch (mb->type) {
	case CISS_REGISTER:
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
	dect_close_transaction(dh, &sse->transaction, DECT_DDL_RELEASE_NORMAL);
}

static const struct dect_nwk_protocol ciss_protocol = {
	.name			= "Call Independant Supplementary Services",
	.pd			= DECT_PD_CISS,
	.max_transactions	= 7,
	.open			= dect_ciss_open,
	.shutdown		= dect_ciss_shutdown,
	.rcv			= dect_ciss_rcv,
};

static void __init dect_ciss_init(void)
{
	dect_lce_register_protocol(&ciss_protocol);
}
