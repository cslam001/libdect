/*
 * DECT PP authentication helpers
 *
 * Copyright (c) 2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>

#include <dect/libdect.h>
#include <dect/auth.h>
#include "common.h"

static const struct dect_ipui *auth_ipui;

static void dect_mm_cipher_ind(struct dect_handle *dh,
			       struct dect_mm_endpoint *mme,
			       struct dect_mm_cipher_param *param)
{
	struct mm_auth_priv *priv = dect_mm_priv(mme);
	struct dect_mm_cipher_param reply = {
		.cipher_info	= param->cipher_info,
	};

	dect_mm_cipher_res(dh, mme, true, &reply, priv->dck);
}

static void dect_mm_authenticate_ind(struct dect_handle *dh,
				     struct dect_mm_endpoint *mme,
				     struct dect_mm_authenticate_param *param)
{
	struct mm_auth_priv *priv = dect_mm_priv(mme);
	struct dect_ie_auth_res res1;
	struct dect_mm_authenticate_param reply = {
		.res	= &res1,
	};
	uint8_t uak[DECT_AUTH_KEY_LEN];
	uint8_t k[DECT_AUTH_KEY_LEN], ks[DECT_AUTH_KEY_LEN];
	bool accept = false;

	if (dect_read_uak(auth_ipui, uak) < 0)
		goto out;

	dect_auth_b1(uak, sizeof(uak), k);

	dect_auth_a11(k, param->rs->value, ks);
	dect_auth_a12(ks, param->rand->value, priv->dck, &res1.value);
	accept = true;
out:
	dect_mm_authenticate_res(dh, mme, accept, &reply);
}

static struct dect_mm_ops dect_mm_ops;

void dect_pp_auth_init(struct dect_ops *ops, const struct dect_ipui *ipui)
{
	struct dect_mm_ops *mm_ops;

	if (!ops->mm_ops)
		ops->mm_ops = &dect_mm_ops;
	mm_ops = (struct dect_mm_ops *)ops->mm_ops;

	if (!mm_ops->priv_size)
		mm_ops->priv_size = sizeof(struct mm_auth_priv);
	mm_ops->mm_authenticate_ind = dect_mm_authenticate_ind;
	mm_ops->mm_cipher_ind       = dect_mm_cipher_ind;

	auth_ipui = ipui;
}
