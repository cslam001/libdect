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

static DECT_SFMT_MSG_DESC(mm_access_rights_accept,
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_FIXED_IDENTITY,		IE_MANDATORY, IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_LOCATION_AREA,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_AUTH_TYPE,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CIPHER_INFO,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ZAP_FIELD,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_SERVICE_CLASS,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_SETUP_CAPABILITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_MODEL_IDENTIFIER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CODEC_LIST,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_access_rights_request,
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
);

static DECT_SFMT_MSG_DESC(mm_access_rights_reject,
	DECT_SFMT_IE(S_VL_IE_REJECT_REASON,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_authentication_reject,
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_AUTH_TYPE,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_REJECT_REASON,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	//DECT_SFMT_IE(S_VL_IE_AUTH_REJECT_PARAMETER,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_authentication_reply,
	DECT_SFMT_IE(S_VL_IE_RES,			IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(S_VL_IE_RS,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ZAP_FIELD,			IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SERVICE_CLASS,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_KEY,			IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_authentication_request,
	DECT_SFMT_IE(S_VL_IE_AUTH_TYPE,			IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(S_VL_IE_RAND,			IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(S_VL_IE_RES,			IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_RS,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CIPHER_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_cipher_suggest,
	DECT_SFMT_IE(S_VL_IE_CIPHER_INFO,		IE_NONE,      IE_MANDATORY, 0),
	DECT_SFMT_IE(S_VL_IE_CALL_IDENTITY,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CONNECTION_IDENTITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_cipher_request,
	DECT_SFMT_IE(S_VL_IE_CIPHER_INFO,		IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CALL_IDENTITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CONNECTION_IDENTITY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_cipher_reject,
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CIPHER_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_REJECT_REASON,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_key_allocate,
	DECT_SFMT_IE(S_VL_IE_ALLOCATION_TYPE,		IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_RAND,			IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_RS,			IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_locate_accept,
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
);

static DECT_SFMT_MSG_DESC(mm_locate_reject,
	DECT_SFMT_IE(S_VL_IE_REJECT_REASON,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_locate_request,
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
);

static DECT_SFMT_MSG_DESC(mm_temporary_identity_assign,
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_LOCATION_AREA,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_NWK_ASSIGNED_IDENTITY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_NETWORK_PARAMETER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_temporary_identity_assign_ack,
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_temporary_identity_assign_rej,
	DECT_SFMT_IE(S_VL_IE_REJECT_REASON,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

#define mm_debug(mme, fmt, args...) \
	dect_debug("MM (link %d): " fmt "\n", (mme)->link->dfd->fd, ## args)

void *dect_mm_priv(struct dect_mm_endpoint *mme)
{
	return mme->priv;
}

static struct dect_mm_endpoint *
dect_mm_endpoint_get_by_link(const struct dect_handle *dh,
			     const struct dect_data_link *link)
{
	struct dect_mm_endpoint *mme;

	list_for_each_entry(mme, &dh->mme_list, list) {
		if (mme->link == link)
			return mme;
	}
	return NULL;
}

struct dect_mm_endpoint *dect_mm_endpoint_alloc(struct dect_handle *dh)
{
	struct dect_mm_endpoint *mme;

	mme = dect_zalloc(dh, sizeof(*mme) + dh->ops->mm_ops->priv_size);
	if (mme == NULL)
		goto err1;

	mme->procedure[DECT_TRANSACTION_INITIATOR].timer = dect_alloc_timer(dh);
	if (mme->procedure[DECT_TRANSACTION_INITIATOR].timer == NULL)
		goto err2;

	mme->procedure[DECT_TRANSACTION_RESPONDER].timer = dect_alloc_timer(dh);
	if (mme->procedure[DECT_TRANSACTION_RESPONDER].timer == NULL)
		goto err3;

	list_add_tail(&mme->list, &dh->mme_list);
	return mme;

err3:
	dect_free(dh, mme->procedure[DECT_TRANSACTION_INITIATOR].timer);
err2:
	dect_free(dh, mme);
err1:
	return NULL;
}

static void dect_mm_endpoint_destroy(struct dect_handle *dh,
				     struct dect_mm_endpoint *mme)
{
	dect_free(dh, mme->procedure[DECT_TRANSACTION_RESPONDER].timer);
	dect_free(dh, mme->procedure[DECT_TRANSACTION_INITIATOR].timer);
	dect_free(dh, mme);
}

static struct dect_mm_endpoint *dect_mm_endpoint(struct dect_transaction *ta)
{
	return container_of(ta, struct dect_mm_endpoint, procedure[ta->role].transaction);
}

static int dect_mm_send_msg(struct dect_handle *dh,
			    const struct dect_mm_endpoint *mme,
			    enum dect_transaction_role role,
			    const struct dect_sfmt_msg_desc *desc,
			    const struct dect_msg_common *msg,
			    enum dect_mm_msg_types type)
{
	return dect_lce_send(dh, &mme->procedure[role].transaction, desc, msg, type);
}

/**
 * dect_mm_key_allocate_req - MM_KEY_ALLOCATE-req primitive
 *
 * @dh:		libdect DECT handle
 * @mme:	Mobility Management Endpoint
 * @param:	key allocate request parameters
 */
int dect_mm_key_allocate_req(struct dect_handle *dh,
			     struct dect_mm_endpoint *mme,
			     const struct dect_mm_key_allocate_param *param)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_key_allocate_msg msg;
	int err;

	mm_debug(mme, "KEY_ALLOCATE-req");
	if (mp->type != DECT_MMP_NONE)
		return -1;

	err = dect_ddl_open_transaction(dh, &mp->transaction, mme->link,
					DECT_PD_MM);
	if (err < 0)
		goto err1;

	memset(&msg, 0, sizeof(msg));
	msg.allocation_type	= param->allocation_type;
	msg.rand		= param->rand;
	msg.rs			= param->rs;

	err = dect_mm_send_msg(dh, mme, DECT_TRANSACTION_INITIATOR,
			       &mm_key_allocate_msg_desc,
			       &msg.common, DECT_MM_KEY_ALLOCATE);
	if (err < 0)
		goto err2;

	mp->type = DECT_MMP_KEY_ALLOCATION;
	return 0;

err2:
	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
err1:
	return err;
}

static void dect_mm_rcv_key_allocate(struct dect_handle *dh,
				     struct dect_mm_endpoint *mme,
				     struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_key_allocate_msg msg;
	struct dect_mm_key_allocate_param *param;

	mm_debug(mme, "KEY-ALLOCATE");
	if (mp->type != 0)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_key_allocate_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->allocation_type	= dect_ie_hold(msg.allocation_type);
	param->rand		= dect_ie_hold(msg.rand);
	param->rs		= dect_ie_hold(msg.rs);

	dh->ops->mm_ops->mm_key_allocate_ind(dh, mme, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_key_allocate_msg_desc, &msg.common);
}

/**
 * dect_mm_authenticate_req - MM_AUTHENTICATE-req primitive
 *
 * @dh:		libdect DECT handle
 * @mme:	Mobility Management Endpoint
 * @param:	authenticate request parameters
 */
int dect_mm_authenticate_req(struct dect_handle *dh,
			     struct dect_mm_endpoint *mme,
			     const struct dect_mm_authenticate_param *param)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_authentication_request_msg msg;
	int err;

	mm_debug(mme, "AUTHENTICATE-req");
	if (mp->type != DECT_MMP_NONE)
		return -1;

	err = dect_ddl_open_transaction(dh, &mp->transaction, mme->link,
					DECT_PD_MM);
	if (err < 0)
		goto err1;

	memset(&msg, 0, sizeof(msg));
	msg.auth_type			= param->auth_type;
	msg.rand			= param->rand;
	msg.res				= param->res;
	msg.rs				= param->rs;
	msg.cipher_info			= param->cipher_info;
	msg.iwu_to_iwu			= param->iwu_to_iwu;
	msg.escape_to_proprietary	= param->escape_to_proprietary;

	err = dect_mm_send_msg(dh, mme, DECT_TRANSACTION_INITIATOR,
			       &mm_authentication_request_msg_desc,
			       &msg.common, DECT_MM_AUTHENTICATION_REQUEST);
	if (err < 0)
		goto err2;

	mp->type = DECT_MMP_AUTHENTICATE;
	return 0;

err2:
	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
err1:
	return err;
}

/**
 * dect_mm_authenticate_req - MM_AUTHENTICATE-res primitive
 *
 * @dh:		libdect DECT handle
 * @mme:	Mobility Management Endpoint
 * @param:	authenticate response parameters
 */
int dect_mm_authenticate_res(struct dect_handle *dh,
			     struct dect_mm_endpoint *mme,
			     const struct dect_mm_authenticate_param *param)
{
	return 0;
}

static void dect_mm_rcv_authentication_request(struct dect_handle *dh,
					       struct dect_mm_endpoint *mme,
					       struct dect_msg_buf *mb)
{
	struct dect_mm_authentication_request_msg msg;

	mm_debug(mme, "AUTHENTICATION-REQUEST");
	if (dect_parse_sfmt_msg(dh, &mm_authentication_request_msg_desc,
				&msg.common, mb) < 0)
		return;
}

static void dect_mm_rcv_authentication_reply(struct dect_handle *dh,
					     struct dect_mm_endpoint *mme,
					     struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_authentication_reply_msg msg;
	struct dect_mm_authenticate_param *param;

	mm_debug(mme, "AUTHENTICATION-REPLY");
	if (mp->type != DECT_MMP_AUTHENTICATE)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_authentication_reply_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->res			= dect_ie_hold(msg.res);
	param->rs			= dect_ie_hold(msg.rs);
	param->zap_field		= dect_ie_hold(msg.zap_field);
	param->service_class		= dect_ie_hold(msg.service_class);
	param->key			= dect_ie_hold(msg.key);
	param->iwu_to_iwu		= *dect_ie_list_hold(&msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
	mp->type = DECT_MMP_NONE;

	dh->ops->mm_ops->mm_authenticate_cfm(dh, mme, true, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_authentication_reply_msg_desc, &msg.common);
}

static void dect_mm_rcv_authentication_reject(struct dect_handle *dh,
					      struct dect_mm_endpoint *mme,
					      struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_authentication_reject_msg msg;
	struct dect_mm_authenticate_param *param;

	mm_debug(mme, "AUTHENTICATION-REJECT");
	if (dect_parse_sfmt_msg(dh, &mm_authentication_reject_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	//param->auth_type	= *dect_ie_list_hold(&msg.auth_type);
	param->reject_reason		= dect_ie_hold(msg.reject_reason);
	param->iwu_to_iwu		= *dect_ie_list_hold(&msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
	mp->type = DECT_MMP_NONE;

	dh->ops->mm_ops->mm_authenticate_cfm(dh, mme, false, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_authentication_reject_msg_desc, &msg.common);
}

/**
 * dect_mm_access_rights_req - MM_ACCESS_RIGHTS-req primitive
 *
 * @dh:		libdect DECT handle
 * @mme:	Mobility Management Endpoint
 * @param:	access rights request parameters
 */
int dect_mm_access_rights_req(struct dect_handle *dh,
			      struct dect_mm_endpoint *mme,
			      const struct dect_mm_access_rights_param *param)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_access_rights_request_msg msg;
	int err;

	mm_debug(mme, "ACCESS_RIGHTS-req");
	err = dect_ddl_open_transaction(dh, &mp->transaction, mme->link,
				        DECT_PD_MM);
	if (err < 0)
		goto err1;

	memset(&msg, 0, sizeof(msg));
	msg.portable_identity		= param->portable_identity;
	msg.auth_type			= param->auth_type;
	msg.cipher_info			= param->cipher_info;
	msg.setup_capability		= NULL;
	msg.terminal_capability		= param->terminal_capability;
	msg.model_identifier		= param->model_identifier;
	msg.codec_list			= NULL;
	msg.escape_to_proprietary	= param->escape_to_proprietary;

	err = dect_mm_send_msg(dh, mme, DECT_TRANSACTION_INITIATOR,
			       &mm_access_rights_request_msg_desc,
			       &msg.common, DECT_MM_ACCESS_RIGHTS_REQUEST);
	if (err < 0)
		goto err2;

	mp->type = DECT_MMP_ACCESS_RIGHTS;
	return 0;

err2:
	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
err1:
	return -1;
}

/**
 * dect_mm_access_rights_res - MM_ACCESS_RIGHTS-res primitive
 *
 * @dh:		libdect DECT handle
 * @mme:	Mobility Management Endpoint
 * @param:	access rights response parameters
 */
int dect_mm_access_rights_res(struct dect_handle *dh,
			      struct dect_mm_endpoint *mme, bool accept,
			      const struct dect_mm_access_rights_param *param)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_access_rights_accept_msg msg;
	struct dect_ie_fixed_identity fixed_identity;
	int err;

	mm_debug(mme, "ACCESS_RIGHTS-res");
	if (mp->type != DECT_MMP_ACCESS_RIGHTS)
		return -1;

	memset(&msg, 0, sizeof(msg));
	msg.portable_identity		= param->portable_identity;
	msg.fixed_identity		= param->fixed_identity;
	msg.auth_type			= param->auth_type;
	msg.location_area		= param->location_area;
	msg.cipher_info			= param->cipher_info;
	msg.setup_capability		= NULL;
	msg.model_identifier		= param->model_identifier;
	//msg.iwu_to_iwu		= param->iwu_to_iwu;
	//msg.codec_list		= param->codec_list;
	msg.escape_to_proprietary	= param->escape_to_proprietary;

	if (param->fixed_identity.list == NULL) {
		fixed_identity.type = DECT_FIXED_ID_TYPE_PARK;
		fixed_identity.ari = dh->pari;
		fixed_identity.rpn = 0;
		dect_ie_list_add(&fixed_identity, &msg.fixed_identity);
	}

	err = dect_mm_send_msg(dh, mme, DECT_TRANSACTION_RESPONDER,
			       &mm_access_rights_accept_msg_desc,
			       &msg.common, DECT_MM_ACCESS_RIGHTS_ACCEPT);

	dect_close_transaction(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
	dect_mm_endpoint_destroy(dh, mme);
	return err;
}

static void dect_mm_rcv_access_rights_request(struct dect_handle *dh,
					      struct dect_mm_endpoint *mme,
					      struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_access_rights_request_msg msg;
	struct dect_mm_access_rights_param *param;

	mm_debug(mme, "ACCESS-RIGHTS-REQUEST");
	if (mp->type != DECT_MMP_NONE)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_access_rights_request_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->portable_identity	= dect_ie_hold(msg.portable_identity);
	param->auth_type		= dect_ie_hold(msg.auth_type);
	param->cipher_info		= dect_ie_hold(msg.cipher_info);
	param->terminal_capability	= dect_ie_hold(msg.terminal_capability);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	mp->type = DECT_MMP_ACCESS_RIGHTS;

	dh->ops->mm_ops->mm_access_rights_ind(dh, mme, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_access_rights_request_msg_desc, &msg.common);
}

static void dect_mm_rcv_access_rights_reject(struct dect_handle *dh,
					     struct dect_msg_buf *mb)
{
	struct dect_mm_access_rights_reject_msg msg;

	if (dect_parse_sfmt_msg(dh, &mm_access_rights_reject_msg_desc, &msg.common, mb) < 0)
		return;
}

static int dect_mm_send_locate_accept(struct dect_handle *dh,
				      struct dect_mm_endpoint *mme,
				      const struct dect_mm_locate_param *param)
{
	struct dect_mm_locate_accept_msg msg = {
		.portable_identity	= param->portable_identity,
		.location_area		= param->location_area,
		.nwk_assigned_identity	= param->nwk_assigned_identity,
		.duration		= param->duration,
		.iwu_to_iwu		= param->iwu_to_iwu,
		.escape_to_proprietary	= param->escape_to_proprietary,
		.model_identifier	= param->model_identifier,
	};

	return dect_mm_send_msg(dh, mme, DECT_TRANSACTION_RESPONDER,
				&mm_locate_accept_msg_desc,
				&msg.common, DECT_MM_LOCATE_ACCEPT);
}

static int dect_mm_send_locate_reject(struct dect_handle *dh,
				      struct dect_mm_endpoint *mme,
				      const struct dect_mm_locate_param *param)
{
	struct dect_mm_locate_reject_msg msg = {
		.reject_reason		= param->reject_reason,
		.duration		= param->duration,
		.segmented_info		= {},
		.iwu_to_iwu		= param->iwu_to_iwu,
		.escape_to_proprietary	= param->escape_to_proprietary,
	};

	return dect_mm_send_msg(dh, mme, DECT_TRANSACTION_RESPONDER,
				&mm_locate_reject_msg_desc,
				&msg.common, DECT_MM_LOCATE_REJECT);
}

int dect_mm_locate_res(struct dect_handle *dh, struct dect_mm_endpoint *mme,
		       const struct dect_mm_locate_param *param)
{
	if (param->reject_reason == NULL)
		return dect_mm_send_locate_accept(dh, mme, param);
	else
		return dect_mm_send_locate_reject(dh, mme, param);
}

static void dect_mm_locate_ind(struct dect_handle *dh,
			       struct dect_mm_endpoint *mme,
			       const struct dect_mm_locate_request_msg *msg)
{
	struct dect_mm_locate_param *param;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		return;

	param->portable_identity	= dect_ie_hold(msg->portable_identity),
	param->fixed_identity		= dect_ie_hold(msg->fixed_identity),
	param->location_area		= dect_ie_hold(msg->location_area),
	param->nwk_assigned_identity	= dect_ie_hold(msg->nwk_assigned_identity),
	param->cipher_info		= dect_ie_hold(msg->cipher_info),
	param->setup_capability		= dect_ie_hold(msg->setup_capability),
	param->terminal_capability	= dect_ie_hold(msg->terminal_capability),
	param->iwu_to_iwu		= dect_ie_hold(msg->iwu_to_iwu),
	param->model_identifier		= dect_ie_hold(msg->model_identifier),

	dh->ops->mm_ops->mm_locate_ind(dh, mme, param);
	dect_ie_collection_put(dh, param);
}

static void dect_mm_rcv_locate_request(struct dect_handle *dh,
				       struct dect_mm_endpoint *mme,
				       struct dect_msg_buf *mb)
{
	struct dect_mm_locate_request_msg msg;

	mm_debug(mme, "LOCATE-REQUEST");
	if (dect_parse_sfmt_msg(dh, &mm_locate_request_msg_desc, &msg.common, mb) < 0)
		goto err1;

	dect_mm_locate_ind(dh, mme, &msg);
	dect_msg_free(dh, &mm_locate_request_msg_desc, &msg.common);
err1:
	return;
}

static void dect_mm_rcv_locate_accept(struct dect_handle *dh,
				      struct dect_mm_endpoint *mme,
				      struct dect_msg_buf *mb)
{
	struct dect_mm_locate_accept_msg msg;

	mm_debug(mme, "LOCATE-ACCEPT");
	if (dect_parse_sfmt_msg(dh, &mm_locate_accept_msg_desc, &msg.common, mb) < 0)
		return;

	dect_msg_free(dh, &mm_locate_accept_msg_desc, &msg.common);
}

static void dect_mm_rcv_locate_reject(struct dect_handle *dh,
				      struct dect_mm_endpoint *mme,
				      struct dect_msg_buf *mb)
{
	struct dect_mm_locate_reject_msg msg;

	mm_debug(mme, "LOCATE-REJECT");
	if (dect_parse_sfmt_msg(dh, &mm_locate_reject_msg_desc, &msg.common, mb) < 0)
		return;

	dect_msg_free(dh, &mm_locate_reject_msg_desc, &msg.common);
}

static void dect_mm_rcv_temporary_identity_assign_ack(struct dect_handle *dh,
						      struct dect_mm_endpoint *mme,
						      struct dect_msg_buf *mb)
{
	struct dect_mm_temporary_identity_assign_ack_msg msg;
	struct dect_mm_identity_assign_param param;

	mm_debug(mme, "TEMPORARY-IDENTITY-ASSIGN-ACK");
	if (dect_parse_sfmt_msg(dh, &mm_temporary_identity_assign_ack_msg_desc,
				&msg.common, mb) < 0)
		return;

	memset(&param, 0, sizeof(param));
	dh->ops->mm_ops->mm_identity_assign_cfm(dh, mme, true, &param);
}

static void dect_mm_rcv_temporary_identity_assign_rej(struct dect_handle *dh,
						      struct dect_mm_endpoint *mme,
						      struct dect_msg_buf *mb)
{
	struct dect_mm_temporary_identity_assign_rej_msg msg;
	struct dect_mm_identity_assign_param *param;

	mm_debug(mme, "TEMPORARY-IDENTITY-ASSIGN-REJ");
	if (dect_parse_sfmt_msg(dh, &mm_temporary_identity_assign_rej_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->reject_reason = dect_ie_hold(msg.reject_reason);

	dh->ops->mm_ops->mm_identity_assign_cfm(dh, mme, false, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_temporary_identity_assign_rej_msg_desc, &msg.common);
}

static void dect_mm_rcv(struct dect_handle *dh, struct dect_transaction *ta,
			struct dect_msg_buf *mb)
{
	struct dect_mm_endpoint *mme = dect_mm_endpoint(ta);

	switch (mb->type) {
	case DECT_MM_AUTHENTICATION_REQUEST:
		return dect_mm_rcv_authentication_request(dh, mme, mb);
	case DECT_MM_AUTHENTICATION_REPLY:
		return dect_mm_rcv_authentication_reply(dh, mme, mb);
	case DECT_MM_KEY_ALLOCATE:
		return dect_mm_rcv_key_allocate(dh, mme, mb);
	case DECT_MM_AUTHENTICATION_REJECT:
		return dect_mm_rcv_authentication_reject(dh, mme, mb);
	case DECT_MM_ACCESS_RIGHTS_REQUEST:
		return dect_mm_rcv_access_rights_request(dh, mme, mb);
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
	case DECT_MM_LOCATE_REQUEST:
		return dect_mm_rcv_locate_request(dh, mme, mb);
	case DECT_MM_LOCATE_ACCEPT:
		return dect_mm_rcv_locate_accept(dh, mme, mb);
	case DECT_MM_LOCATE_REJECT:
		return dect_mm_rcv_locate_reject(dh, mme, mb);
	case DECT_MM_DETACH:
	case DECT_MM_IDENTITY_REQUEST:
	case DECT_MM_IDENTITY_REPLY:
	case DECT_MM_TEMPORARY_IDENTITY_ASSIGN:
		break;
	case DECT_MM_TEMPORARY_IDENTITY_ASSIGN_ACK:
		return dect_mm_rcv_temporary_identity_assign_ack(dh, mme, mb);
	case DECT_MM_TEMPORARY_IDENTITY_ASSIGN_REJ:
		return dect_mm_rcv_temporary_identity_assign_rej(dh, mme, mb);
	}

	mm_debug(mme, "receive unknown msg type %x", mb->type);
}

static void dect_mm_open(struct dect_handle *dh,
			 const struct dect_transaction *req,
			 struct dect_msg_buf *mb)
{
	struct dect_mm_endpoint *mme;
	struct dect_transaction *ta;

	dect_debug("MM: unknown transaction msg type: %x\n", mb->type);

	switch (mb->type) {
	case DECT_MM_AUTHENTICATION_REQUEST:
	case DECT_MM_ACCESS_RIGHTS_REQUEST:
	case DECT_MM_LOCATE_REQUEST:
	case DECT_MM_KEY_ALLOCATE:
		break;
	default:
		return;
	}

	mme = dect_mm_endpoint_get_by_link(dh, req->link);
	if (mme == NULL) {
		mme = dect_mm_endpoint_alloc(dh);
		if (mme == NULL)
			return;
		mme->link = req->link;
	}

	ta = &mme->procedure[DECT_TRANSACTION_RESPONDER].transaction;
	dect_confirm_transaction(dh, ta, req);

	dect_mm_rcv(dh, ta, mb);
}

static void dect_mm_shutdown(struct dect_handle *dh,
			     struct dect_transaction *ta)
{
	struct dect_mm_endpoint *mme = dect_mm_endpoint(ta);

	mm_debug(mme, "shutdown");
	dect_close_transaction(dh, ta, DECT_DDL_RELEASE_NORMAL);
}

static const struct dect_nwk_protocol mm_protocol = {
	.name			= "Mobility Management",
	.pd			= DECT_PD_MM,
	.max_transactions	= 1,
	.open			= dect_mm_open,
	.shutdown		= dect_mm_shutdown,
	.rcv			= dect_mm_rcv,
};

static void __init dect_mm_init(void)
{
	dect_lce_register_protocol(&mm_protocol);
}
