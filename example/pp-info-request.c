/*
 * DECT PP information request example
 *
 * Copyright (c) 2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <dect/libdect.h>
#include <dect/auth.h>
#include "common.h"

static void mm_info_cfm(struct dect_handle *dh,
			struct dect_mm_endpoint *mme, bool accept,
			struct dect_mm_info_param *param)
{
	dect_event_loop_stop();
}

static int mm_info_req(struct dect_handle *dh, struct dect_mm_endpoint *mme)
{
	struct dect_ie_portable_identity portable_identity;
	struct dect_ie_info_type info_type;
	struct dect_mm_info_param param = {
		.portable_identity	= &portable_identity,
		.info_type		= &info_type,
	};

	portable_identity.type	= DECT_PORTABLE_ID_TYPE_IPUI;
	portable_identity.ipui	= ipui;

	info_type.num		= 1;
	info_type.type[0]	= DECT_INFO_LOCATION_AREA;

	return dect_mm_info_req(dh, mme, &param);
}

static struct dect_mm_ops mm_ops = {
	.mm_info_cfm		= mm_info_cfm,
};

static void dl_establish_cfm(struct dect_handle *dh, bool success,
			     struct dect_data_link *ddl,
			     const struct dect_mac_conn_params *mcp)
{
	struct dect_mm_endpoint *mme;

	mme = dect_mm_endpoint_alloc(dh, ddl);
	if (mme == NULL)
		pexit("dect_mm_endpoint_alloc");

	mm_info_req(dh, mme);
}

static struct dect_lce_ops lce_ops = {
	.dl_establish_cfm	= dl_establish_cfm,
};

static struct dect_ops ops = {
	.lce_ops		= &lce_ops,
	.mm_ops			= &mm_ops,
};

int main(int argc, char **argv)
{
	struct dect_mac_conn_params mcp = {};

	dect_pp_common_options(argc, argv);
	dect_pp_common_init(&ops, cluster, &ipui);

	dect_dl_establish_req(dh, &ipui, &mcp);
	dect_event_loop();

	dect_common_cleanup(dh);
	return 0;
}
