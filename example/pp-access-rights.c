/*
 * DECT PP access rights request example
 *
 * Copyright (c) 2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <string.h>
#include <inttypes.h>

#include <dect/libdect.h>
#include <dect/auth.h>
#include "common.h"

#define debug(fmt, args...)	printf("P-IWU: " fmt, ## args)

static const char *pin = "0000";
static int rand_fd;

struct mm_priv {
	uint64_t	rand;
	uint64_t	rs;
	uint8_t		uak[DECT_AUTH_KEY_LEN];
};

static void mm_authenticate_ind(struct dect_handle *dh,
				struct dect_mm_endpoint *mme,
				struct dect_mm_authenticate_param *param)
{
	struct mm_priv *priv = dect_mm_priv(mme);
	struct dect_ie_auth_res res1;
	struct dect_mm_authenticate_param reply = {
		.res	= &res1,
	};
	uint8_t k[DECT_AUTH_KEY_LEN], ks[DECT_AUTH_KEY_LEN];
	uint8_t dck[DECT_CIPHER_KEY_LEN];

	dect_auth_b1(priv->uak, sizeof(priv->uak), k);

	dect_auth_a11(k, param->rs->value, ks);
	dect_auth_a12(ks, param->rand->value, dck, &res1.value);

	dect_mm_authenticate_res(dh, mme, true, &reply);
}

static void mm_authenticate_cfm(struct dect_handle *dh,
				struct dect_mm_endpoint *mme, bool accept,
				struct dect_mm_authenticate_param *param)
{
	struct mm_priv *priv = dect_mm_priv(mme);
	uint8_t k[DECT_AUTH_KEY_LEN], ks[DECT_AUTH_KEY_LEN];
	uint8_t ac[DECT_AUTH_CODE_LEN];
	uint32_t res2;

	if (!accept)
		goto out;

	dect_pin_to_ac(pin, ac, sizeof(ac));
	dect_auth_b1(ac, sizeof(ac), k);

	dect_auth_a21(k, priv->rs, ks);
	dect_auth_a22(ks, priv->rand, &res2);

	if (res2 == param->res->value) {
		debug("authentication success\n");
		memcpy(priv->uak, ks, sizeof(priv->uak));
		dect_write_uak(&ipui, priv->uak);
		return;
	} else
		debug("authentication failure: rand: %" PRIx64 " %.8x | %.8x\n",
		      priv->rand, res2, param->res->value);
out:
	dect_event_loop_stop();
}

static void mm_key_allocate_ind(struct dect_handle *dh,
				struct dect_mm_endpoint *mme,
				struct dect_mm_key_allocate_param *param)
{
	struct mm_priv *priv = dect_mm_priv(mme);
	struct dect_ie_auth_type auth_type;
	struct dect_ie_auth_value rand;
	struct dect_ie_auth_res res1;
	struct dect_mm_authenticate_param reply = {
		.auth_type	= &auth_type,
		.rand		= &rand,
		.res		= &res1,
	};
	uint8_t k[DECT_AUTH_KEY_LEN], ks[DECT_AUTH_KEY_LEN];
	uint8_t dck[DECT_CIPHER_KEY_LEN];
	uint8_t ac[DECT_AUTH_CODE_LEN];

	auth_type.auth_id		= DECT_AUTH_DSAA;
	auth_type.auth_key_type		= DECT_KEY_AUTHENTICATION_CODE;
	auth_type.auth_key_num		= 0 | DECT_AUTH_KEY_IPUI_PARK;
	auth_type.cipher_key_num	= 0;
	auth_type.flags			= DECT_AUTH_FLAG_UPC;

	read(rand_fd, &rand.value, sizeof(rand.value));
	priv->rand = rand.value;

	dect_pin_to_ac(pin, ac, sizeof(ac));
	dect_auth_b1(ac, sizeof(ac), k);

	dect_auth_a11(k, param->rs->value, ks);
	dect_auth_a12(ks, param->rand->value, dck, &res1.value);

	priv->rs = param->rs->value;

	dect_mm_authenticate_req(dh, mme, &reply);
}

static void mm_access_rights_cfm(struct dect_handle *dh,
				 struct dect_mm_endpoint *mme, bool accept,
				 struct dect_mm_access_rights_param *param)
{
	dect_event_loop_stop();
}

static int mm_access_rights_req(struct dect_handle *dh, struct dect_mm_endpoint *mme)
{
	struct dect_ie_portable_identity portable_identity;
	struct dect_ie_auth_type auth_type;
	struct dect_ie_terminal_capability terminal_capability;
	struct dect_mm_access_rights_param param = {
		.portable_identity	= &portable_identity,
		.auth_type		= &auth_type,
		.terminal_capability	= &terminal_capability,
	};

	portable_identity.type		= DECT_PORTABLE_ID_TYPE_IPUI;
	portable_identity.ipui		= ipui;

	auth_type.auth_id		= DECT_AUTH_DSAA;
	auth_type.auth_key_type		= DECT_KEY_AUTHENTICATION_CODE;
	auth_type.auth_key_num		= 0 | DECT_AUTH_KEY_IPUI_PARK;
	auth_type.cipher_key_num	= 0;
	auth_type.flags			= 0;

	dect_pp_init_terminal_capability(&terminal_capability);

	return dect_mm_access_rights_req(dh, mme, &param);
}

static void dl_establish_cfm(struct dect_handle *dh, bool success,
			     struct dect_data_link *ddl,
			     const struct dect_mac_conn_params *mcp)
{
	struct dect_mm_endpoint *mme;

	mme = dect_mm_endpoint_alloc(dh, ddl);
	if (mme == NULL)
		pexit("dect_mm_endpoint_alloc");

	mm_access_rights_req(dh, mme);
}

static void llme_mac_me_info_ind(struct dect_handle *dh,
				 const struct dect_ari *pari,
				 const struct dect_fp_capabilities *fpc)
{
	if (fpc->hlc & DECT_HLC_ACCESS_RIGHTS_REQUESTS)
		dect_event_loop_stop();
}

static struct dect_lce_ops lce_ops = {
	.dl_establish_cfm	= dl_establish_cfm,
};

static struct dect_llme_ops_ llme_ops = {
	.mac_me_info_ind	= llme_mac_me_info_ind,
};

static struct dect_mm_ops mm_ops = {
	.priv_size		= sizeof(struct mm_priv),
	.mm_key_allocate_ind	= mm_key_allocate_ind,
	.mm_authenticate_cfm	= mm_authenticate_cfm,
	.mm_authenticate_ind	= mm_authenticate_ind,
	.mm_access_rights_cfm	= mm_access_rights_cfm,
};

static struct dect_ops ops = {
	.llme_ops		= &llme_ops,
	.lce_ops		= &lce_ops,
	.mm_ops			= &mm_ops,
};

enum {
	OPT_CLUSTER	= 'c',
	OPT_IPUI	= 'i',
	OPT_PIN		= 'p',
	OPT_HELP	= 'h',
};

static const struct option options[] = {
	{ .name = "cluster",	.has_arg = true,  .flag = NULL, .val = OPT_CLUSTER },
	{ .name = "ipui",	.has_arg = true,  .flag = NULL, .val = OPT_IPUI },
	{ .name = "pin",	.has_arg = true,  .flag = NULL, .val = OPT_PIN },
	{ .name = "help",	.has_arg = false, .flag = NULL, .val = OPT_HELP },
	{ },
};

int main(int argc, char **argv)
{
	const char *cluster = NULL;
	int optidx = 0, c;

	for (;;) {
		c = getopt_long(argc, argv, "c:p:h", options, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case OPT_CLUSTER:
			cluster = optarg;
			break;
		case OPT_IPUI:
			if (dect_parse_ipui(&ipui, optarg) < 0)
				exit(1);
			break;
		case OPT_PIN:
			pin = optarg;
			break;
		case OPT_HELP:
			printf("%s [ -c/--cluster NAME ] [ -p/--pin PIN ] [ -h/--help ]\n",
			      argv[0]);
			exit(0);
		case '?':
			exit(1);
		}
	}

	rand_fd = open("/dev/urandom", O_RDONLY);
	if (rand_fd < 0)
		pexit("open /dev/urandom");

	dect_common_init(&ops, cluster);

	if (!(dect_llme_fp_capabilities(dh)->hlc &
	      DECT_HLC_ACCESS_RIGHTS_REQUESTS)) {
		debug("waiting for ACCESS_RIGHTS_REQUESTS capability ...\n");
		dect_event_loop();
	}

	dect_dl_establish_req(dh, &ipui, NULL);
	dect_event_loop();

	dect_common_cleanup(dh);
	close(rand_fd);
	return 0;
}
