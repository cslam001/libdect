/*
 * DECT Mobility Management FP example
 *
 * Copyright (c) 2009 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <dect/libdect.h>
#include <dect/auth.h>
#include "common.h"

static uint8_t prefix[3] = { 1, 0, 0};
static uint16_t num;
static int rand_fd;

#define debug(fmt, args...)	printf("IWU: FP-MM: " fmt, ## args)

struct mm_priv {
	struct dect_mm_locate_param	*locate;
	uint64_t			rand;
};

static void mm_authenticate_ind(struct dect_handle *dh,
				struct dect_mm_endpoint *mme,
				struct dect_mm_authenticate_param *param)
{
}

static void mm_identity_assign_cfm(struct dect_handle *dh,
				   struct dect_mm_endpoint *mme, bool accept,
				   struct dect_mm_identity_assign_param *param)
{
}

static void mm_locate_res(struct dect_handle *dh,
			  struct dect_mm_endpoint *mme)
{
	struct mm_priv *priv = dect_mm_priv(mme);
	struct dect_mm_locate_param *param = priv->locate;
	struct dect_ie_portable_identity portable_identity;
	struct dect_ie_duration duration;
	struct dect_mm_locate_param reply = {
		.location_area		= param->location_area,
		.portable_identity	= &portable_identity,
		.duration		= &duration,
	};

	dect_ie_init(&portable_identity);
	portable_identity.type = DECT_PORTABLE_ID_TYPE_TPUI;
	portable_identity.tpui.type = DECT_TPUI_INDIVIDUAL_ASSIGNED;
	memcpy(&portable_identity.tpui.ia.digits, prefix, sizeof(prefix));
	portable_identity.tpui.ia.digits[3] = num / 10;
	portable_identity.tpui.ia.digits[4] = num % 10;
	num++;

	dect_ie_init(&duration);
	duration.lock = DECT_LOCK_TEMPORARY_USER_LIMIT_1;
	duration.time = DECT_TIME_LIMIT_DEFINED_TIME_LIMIT_1;
	duration.duration = 1;

	dect_mm_locate_res(dh, mme, true, &reply);
}

static void mm_cipher_cfm(struct dect_handle *dh,
			  struct dect_mm_endpoint *mme, bool accept,
			  struct dect_mm_cipher_param *param)
{
	struct mm_priv *priv = dect_mm_priv(mme);

	if (accept)
		mm_locate_res(dh, mme);

	dect_ie_collection_put(dh, priv->locate);
}

static void mm_cipher_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
			  uint8_t ck[DECT_CIPHER_KEY_LEN])
{
	struct dect_ie_cipher_info cipher_info;
	struct dect_mm_cipher_param param = {
		.cipher_info	= &cipher_info,
	};

	cipher_info.enable		= true;
	cipher_info.cipher_alg_id	= DECT_CIPHER_STANDARD_1;
	cipher_info.cipher_key_type	= DECT_CIPHER_DERIVED_KEY;
	cipher_info.cipher_key_num	= 0;

	dect_mm_cipher_req(dh, mme, &param, ck);
}

static void mm_authenticate_cfm(struct dect_handle *dh,
				struct dect_mm_endpoint *mme, bool accept,
				struct dect_mm_authenticate_param *param)
{
	struct mm_priv *priv = dect_mm_priv(mme);
	uint8_t k[DECT_AUTH_KEY_LEN], ks[DECT_AUTH_KEY_LEN];
	uint8_t dck[DECT_CIPHER_KEY_LEN];
	uint8_t ac[4];
	uint32_t res1;

	if (!accept)
		goto err;

	dect_pin_to_ac("1234", ac, sizeof(ac));
	dect_auth_b1(ac, sizeof(ac), k);

	dect_auth_a11(k, 0, ks);
	dect_auth_a12(ks, priv->rand, dck, &res1);

	if (res1 == param->res->value) {
		debug("authentication success\n");
		mm_cipher_req(dh, mme, dck);
	} else {
		debug("authentication failure\n");
err:
		dect_ie_collection_put(dh, priv->locate);
	}
}

static void mm_authenticate_req(struct dect_handle *dh,
				struct dect_mm_endpoint *mme)
{
	struct mm_priv *priv = dect_mm_priv(mme);
	struct dect_ie_auth_type auth_type;
	struct dect_ie_auth_value rand, rs;
	struct dect_mm_authenticate_param param = {
		.auth_type	= &auth_type,
		.rand		= &rand,
		.rs		= &rs,
	};

	read(rand_fd, &priv->rand, sizeof(priv->rand));

	auth_type.auth_id	 = DECT_AUTH_DSAA;
	auth_type.auth_key_type	 = DECT_KEY_AUTHENTICATION_CODE;
	auth_type.auth_key_num   = 0 | DECT_AUTH_KEY_IPUI_PARK;
	auth_type.cipher_key_num = 0;
	auth_type.flags		 = DECT_AUTH_FLAG_UPC;
	rand.value		 = priv->rand;
	rs.value		 = 0;

	dect_mm_authenticate_req(dh, mme, &param);
}

static void mm_identity_cfm(struct dect_handle *dh, struct dect_mm_endpoint *mme,
			    struct dect_mm_identity_param *param)
{
	mm_authenticate_req(dh, mme);
}

static int mm_identity_req(struct dect_handle *dh, struct dect_mm_endpoint *mme)
{
	struct dect_ie_identity_type identity_type[3] = {};
	struct dect_mm_identity_param param = {	};

	identity_type[0].group	= DECT_IDENTITY_PORTABLE_IDENTITY;
	identity_type[0].type	= DECT_PORTABLE_ID_TYPE_IPEI;
	dect_ie_list_add(&identity_type[0], &param.identity_type);

	identity_type[1].group	= DECT_IDENTITY_PORTABLE_IDENTITY;
	identity_type[1].type	= DECT_PORTABLE_ID_TYPE_IPUI;
	dect_ie_list_add(&identity_type[1], &param.identity_type);

	identity_type[2].group	= DECT_IDENTITY_PORTABLE_IDENTITY;
	identity_type[2].type	= DECT_PORTABLE_ID_TYPE_TPUI;
	dect_ie_list_add(&identity_type[2], &param.identity_type);

	param.identity_type.type = DECT_IE_LIST_NORMAL;

	return dect_mm_identity_req(dh, mme, &param);
}

static void mm_locate_ind(struct dect_handle *dh,
			  struct dect_mm_endpoint *mme,
			  struct dect_mm_locate_param *param)
{
	struct mm_priv *priv = dect_mm_priv(mme);

	priv->locate = dect_ie_collection_hold(param);
	mm_identity_req(dh, mme);
}

static void mm_access_rights_ind(struct dect_handle *dh,
				 struct dect_mm_endpoint *mme,
				 struct dect_mm_access_rights_param *param)
{
	dect_mm_access_rights_res(dh, mme, true, param);
}

static struct dect_mm_ops mm_ops = {
	.priv_size		= sizeof(struct mm_priv),
	.mm_authenticate_ind	= mm_authenticate_ind,
	.mm_authenticate_cfm	= mm_authenticate_cfm,
	.mm_cipher_cfm		= mm_cipher_cfm,
	.mm_access_rights_ind	= mm_access_rights_ind,
	.mm_locate_ind		= mm_locate_ind,
	.mm_identity_cfm	= mm_identity_cfm,
	.mm_identity_assign_cfm	= mm_identity_assign_cfm,
};

static struct dect_ops ops = {
	.mm_ops			= &mm_ops,
};

int main(int argc, char **argv)
{
	rand_fd = open("/dev/urandom", O_RDONLY);
	if (rand_fd < 0)
		exit(1);

	dect_common_init(&ops);

	dect_event_loop();
	dect_close_handle(dh);

	dect_common_cleanup(dh);
	close(rand_fd);
	return 0;
}
