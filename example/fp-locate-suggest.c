/*
 * DECT Mobility Management FP LOCATE-SUGGEST example
 *
 * Copyright (c) 2011 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

#include <dect/libdect.h>
#include "common.h"

static void mm_locate_ind(struct dect_handle *dh,
                          struct dect_mm_endpoint *mme,
                          struct dect_mm_locate_param *param)
{
	struct dect_mm_locate_param res = {
		.portable_identity	= param->portable_identity,
		.location_area		= param->location_area,
	};

	dect_mm_locate_res(dh, mme, true, &res);
}

static void mm_info_req(struct dect_handle *dh, struct dect_mm_endpoint *mme)
{
	struct dect_ie_info_type info_type;
	struct dect_mm_info_param req = { .info_type = &info_type };

	info_type.num = 1;
	info_type.type[0] = DECT_INFO_LOCATE_SUGGEST;
	dect_mm_info_req(dh, mme, &req);
}

static void dl_establish_cfm(struct dect_handle *dh, bool success,
			     struct dect_data_link *ddl,
			     const struct dect_mac_conn_params *mcp)
{
	struct dect_mm_endpoint *mme;

	if (!success)
		return dect_event_loop_stop();

	mme = dect_mm_endpoint_alloc(dh, ddl);
	if (mme == NULL)
		pexit("dect_mm_endpoint_alloc");

	mm_info_req(dh, mme);
}

static struct dect_mm_ops mm_ops = {
	.mm_locate_ind		= mm_locate_ind,
};

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

	dect_fp_common_options(argc, argv);
	dect_common_init(&ops, argv[1]);

	dect_dl_establish_req(dh, &ipui, &mcp);
	dect_event_loop();

	dect_common_cleanup(dh);
	return 0;
}
