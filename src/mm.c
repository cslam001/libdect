/*
 * DECT Mobility Management (MM)
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
#include <mm.h>

static const struct dect_sfmt_ie_desc mm_access_rights_request_msg_desc[] = {
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_NONE,      IE_MANDATORY, 0),
	DECT_SFMT_IE(S_VL_IE_AUTH_TYPE,			IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CIPHER_INFO,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SETUP_CAPABILITY,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_TERMINAL_CAPABILITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_MODEL_IDENTIFIER,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CODEC_LIST,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
};

static const struct dect_sfmt_ie_desc mm_access_rights_reject_msg_desc[] = {
	DECT_SFMT_IE(S_VL_IE_REJECT_REASON,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
};

static const struct dect_sfmt_ie_desc mm_locate_accept_msg_desc[] = {
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_LOCATION_AREA,		IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_SE_IE_USE_TPUI,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_NWK_ASSIGNED_IDENTITY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_EXT_HO_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_SETUP_CAPABILITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_MODEL_IDENTIFIER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CODEC_LIST,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
};

static const struct dect_sfmt_ie_desc mm_locate_reject_msg_desc[] = {
	DECT_SFMT_IE(S_VL_IE_REJECT_REASON,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
};

static const struct dect_sfmt_ie_desc mm_locate_request_msg_desc[] = {
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_NONE,      IE_MANDATORY, 0),
	DECT_SFMT_IE(S_VL_IE_FIXED_IDENTITY,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_LOCATION_AREA,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_NWK_ASSIGNED_IDENTITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CIPHER_INFO,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SETUP_CAPABILITY,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_TERMINAL_CAPABILITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_NETWORK_PARAMETER,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_MODEL_IDENTIFIER,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CODEC_LIST,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
};

static const struct dect_sfmt_ie_desc mm_temporary_identity_assign_msg_desc[] = {
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_LOCATION_AREA,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_NWK_ASSIGNED_IDENTITY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_NETWORK_PARAMETER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
};

static const struct dect_sfmt_ie_desc mm_temporary_identity_assign_ack_msg_desc[] = {
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
};

static const struct dect_sfmt_ie_desc mm_temporary_identity_assign_rej_msg_desc[] = {
	DECT_SFMT_IE(S_VL_IE_REJECT_REASON,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
};


#define mm_debug(fmt, args...) \
	dect_debug("MM: " fmt "\n", ## args)

static struct dect_mm_transaction *dect_mm_transaction_alloc(const struct dect_handle *dh)
{
	struct dect_mm_transaction *mmta;

	mmta = dect_zalloc(dh, sizeof(*dh));
	if (mmta == NULL)
		goto err1;
	return mmta;

err1:
	return NULL;
}

static int dect_mm_send_msg(struct dect_handle *dh,
			    const struct dect_mm_transaction *mmta,
			    const struct dect_sfmt_ie_desc *desc,
			    const struct dect_msg_common *msg,
			    enum dect_mm_msg_types type, const char *prefix)
{
	return dect_lce_send(dh, &mmta->transaction, desc, msg, type, prefix);
}

int dect_mm_access_rights_req(struct dect_handle *dh,
			      const struct dect_mm_access_rights_param *param)
{
	static struct dect_transaction transaction;
	struct dect_ipui ipui;
	struct dect_mm_access_rights_request_msg msg = {
		.portable_identity	= param->portable_identity,
		.auth_type		= param->auth_type,
		.cipher_info		= param->cipher_info,
		.setup_capability	= NULL,
		//.terminal_capability	= param->terminal_capability,
		.model_identifier	= param->model_identifier,
		.codec_list		= NULL,
		.escape_to_proprietary	= NULL,
	};

	mm_debug("access rights request");
	transaction.pd = DECT_S_PD_MM;

	if (dect_open_transaction(dh, &transaction, &ipui) < 0)
		goto err1;

	if (dect_lce_send(dh, &transaction, mm_access_rights_request_msg_desc,
			  &msg.common, DECT_MM_ACCESS_RIGHTS_REQUEST,
			  "MM-ACCESS_RIGHTS_REQUEST") < 0)
		goto err2;
	return 0;

err2:
	dect_close_transaction(dh, &transaction, DECT_DDL_RELEASE_PARTIAL);
err1:
	return -1;
}

static void dect_mm_rcv_access_rights_reject(struct dect_handle *dh,
					     struct dect_msg_buf *mb)
{
	struct dect_mm_access_rights_reject_msg msg;

	if (dect_parse_sfmt_msg(dh, mm_access_rights_reject_msg_desc, &msg.common, mb) < 0)
		return;
}

static int dect_mm_send_locate_accept(struct dect_handle *dh,
				      struct dect_mm_transaction *mmta,
				      const struct dect_mm_locate_param *param)
{
	struct dect_mm_locate_accept_msg msg = {
		.portable_identity	= param->portable_identity,
		.location_area		= param->location_area,
		.nwk_assigned_identity	= param->nwk_assigned_identity,
		.duration		= param->duration,
		.iwu_to_iwu		= param->iwu_to_iwu,
		.model_identifier	= param->model_identifier,
	};

	return dect_mm_send_msg(dh, mmta, mm_locate_accept_msg_desc, &msg.common,
				DECT_MM_LOCATE_ACCEPT, "MM-LOCATE-ACCEPT");
}

static int dect_mm_send_locate_reject(struct dect_handle *dh,
				      struct dect_mm_transaction *mmta,
				      const struct dect_mm_locate_param *param)
{
	struct dect_mm_locate_reject_msg msg = {
		.reject_reason		= param->reject_reason,
		.duration		= param->duration,
		.segmented_info		= {},
		.iwu_to_iwu		= param->iwu_to_iwu,
		.escape_to_proprietary	= NULL,
	};

	return dect_mm_send_msg(dh, mmta, mm_locate_reject_msg_desc, &msg.common,
				DECT_MM_LOCATE_REJECT, "MM-LOCATE-REJECT");
}

int dect_mm_locate_res(struct dect_handle *dh, struct dect_mm_transaction *mmta,
		       const struct dect_mm_locate_param *param)
{
	if (param->reject_reason == NULL)
		return dect_mm_send_locate_accept(dh, mmta, param);
	else
		return dect_mm_send_locate_reject(dh, mmta, param);
}

static void dect_mm_locate_ind(struct dect_handle *dh,
			       struct dect_mm_transaction *mmta,
			       const struct dect_mm_locate_request_msg *msg)
{
	struct dect_mm_locate_param param = {
		.portable_identity	= msg->portable_identity,
		.fixed_identity		= msg->fixed_identity,
		.location_area		= msg->location_area,
		.nwk_assigned_identity	= msg->nwk_assigned_identity,
		.cipher_info		= msg->cipher_info,
		.setup_capability	= msg->setup_capability,
		.terminal_capability	= msg->terminal_capability,
		.iwu_to_iwu		= msg->iwu_to_iwu,
		.model_identifier	= msg->model_identifier,
	};

	dh->ops->mm_ops->mm_locate_ind(dh, mmta, &param);
}

static void dect_mm_rcv_locate_request(struct dect_handle *dh,
				       const struct dect_transaction *req,
				       struct dect_msg_buf *mb)
{
	struct dect_mm_locate_request_msg msg;
	struct dect_mm_transaction *mmta;

	mm_debug("LOCATE-REQUEST");
	if (dect_parse_sfmt_msg(dh, mm_locate_request_msg_desc, &msg.common, mb) < 0)
		goto err1;

	mmta = dect_mm_transaction_alloc(dh);
	if (mmta == NULL)
		goto err2;
	dect_confirm_transaction(dh, &mmta->transaction, req);

	dect_mm_locate_ind(dh, mmta, &msg);
err2:
	dect_msg_free(dh, mm_locate_request_msg_desc, &msg.common);
err1:
	return;
}

static void dect_mm_rcv_locate_accept(struct dect_handle *dh,
				      struct dect_msg_buf *mb)
{
	struct dect_mm_locate_accept_msg msg;

	mm_debug("LOCATE-ACCEPT");
	if (dect_parse_sfmt_msg(dh, mm_locate_accept_msg_desc, &msg.common, mb) < 0)
		return;
}

static void dect_mm_rcv_locate_reject(struct dect_handle *dh,
				      struct dect_msg_buf *mb)
{
	struct dect_mm_locate_reject_msg msg;

	mm_debug("LOCATE-REJECT");
	if (dect_parse_sfmt_msg(dh, mm_locate_reject_msg_desc, &msg.common, mb) < 0)
		return;
}

static void dect_mm_rcv_temporary_identity_assign_ack(struct dect_handle *dh,
						      struct dect_mm_transaction *mmta,
						      struct dect_msg_buf *mb)
{
	struct dect_mm_temporary_identity_assign_ack_msg msg;
	struct dect_mm_identity_assign_param param;

	mm_debug("TEMPORARY-IDENTITY-ASSIGN-ACK");
	if (dect_parse_sfmt_msg(dh, mm_temporary_identity_assign_ack_msg_desc,
				&msg.common, mb) < 0)
		return;

	memset(&param, 0, sizeof(param));
	dh->ops->mm_ops->mm_identity_assign_cfm(dh, mmta, &param);
}

static void dect_mm_rcv_temporary_identity_assign_rej(struct dect_handle *dh,
						      struct dect_mm_transaction *mmta,
						      struct dect_msg_buf *mb)
{
	struct dect_mm_temporary_identity_assign_rej_msg msg;
	struct dect_mm_identity_assign_param param;

	mm_debug("TEMPORARY-IDENTITY-ASSIGN-REJ");
	if (dect_parse_sfmt_msg(dh, mm_temporary_identity_assign_rej_msg_desc,
				&msg.common, mb) < 0)
		return;

	memset(&param, 0, sizeof(param));
	param.reject_reason = msg.reject_reason;
	dh->ops->mm_ops->mm_identity_assign_cfm(dh, mmta, &param);
}

static void dect_mm_rcv(struct dect_handle *dh, struct dect_transaction *ta,
			struct dect_msg_buf *mb)
{
	struct dect_mm_transaction *mmta;

	mmta = container_of(ta, struct dect_mm_transaction, transaction);
	switch (mb->type) {
	case DECT_MM_AUTHENTICATION_REQUEST:
	case DECT_MM_AUTHENTICATION_REPLY:
	case DECT_MM_KEY_ALLOCATE:
	case DECT_MM_AUTHENTICATION_REJECT:
	case DECT_MM_ACCESS_RIGHTS_REQUEST:
		break;
	case DECT_MM_ACCESS_RIGHTS_ACCEPT:
		break;
	case DECT_MM_ACCESS_RIGHTS_REJECT:
		return dect_mm_rcv_access_rights_reject(dh, mb);
	case DECT_MM_ACCESS_RIGHTS_TERMINATE_REQUEST:
	case DECT_MM_ACCESS_RIGHTS_TERMINATE_ACCEPT:
	case DECT_MM_ACCESS_RIGHTS_TERMINATE_REJECT:
	case DECT_MM_CIPHER_REQUEST:
	case DECT_MM_CIPHER_SUGGEST:
	case DECT_MM_CIPHER_REJECT:
	case DECT_MM_INFO_REQUEST:
	case DECT_MM_INFO_ACCEPT:
	case DECT_MM_INFO_SUGGEST:
	case DECT_MM_INFO_REJECT:
		break;
	case DECT_MM_LOCATE_ACCEPT:
		return dect_mm_rcv_locate_accept(dh, mb);
	case DECT_MM_LOCATE_REJECT:
		return dect_mm_rcv_locate_reject(dh, mb);
	case DECT_MM_DETACH:
	case DECT_MM_IDENTITY_REQUEST:
	case DECT_MM_IDENTITY_REPLY:
	case DECT_MM_TEMPORARY_IDENTITY_ASSIGN:
		break;
	case DECT_MM_TEMPORARY_IDENTITY_ASSIGN_ACK:
		return dect_mm_rcv_temporary_identity_assign_ack(dh, mmta, mb);
	case DECT_MM_TEMPORARY_IDENTITY_ASSIGN_REJ:
		return dect_mm_rcv_temporary_identity_assign_rej(dh, mmta, mb);
	}

	mm_debug("receive unknown msg type %x", mb->type);
}

static void dect_mm_open(struct dect_handle *dh,
			 const struct dect_transaction *req,
			 struct dect_msg_buf *mb)
{
	dect_debug("MM: unknown transaction msg type: %x\n", mb->type);

	switch (mb->type) {
	case DECT_MM_LOCATE_REQUEST:
		return dect_mm_rcv_locate_request(dh, req, mb);
	default:
		break;
	}
}

static void dect_mm_shutdown(struct dect_handle *dh,
			     struct dect_transaction *ta)
{
	struct dect_mm_transaction *mmta;

	mmta = container_of(ta, struct dect_mm_transaction, transaction);
	mm_debug("shutdown");
	dect_close_transaction(dh, &mmta->transaction, DECT_DDL_RELEASE_NORMAL);
}

static const struct dect_nwk_protocol mm_protocol = {
	.name			= "Mobility Management",
	.pd			= DECT_S_PD_MM,
	.max_transactions	= 1,
	.open			= dect_mm_open,
	.shutdown		= dect_mm_shutdown,
	.rcv			= dect_mm_rcv,
};

static void __init dect_mm_init(void)
{
	dect_lce_register_protocol(&mm_protocol);
}
