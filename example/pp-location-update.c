/*
 * DECT PP location update example
 *
 * Copyright (c) 2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>

#include <dect/libdect.h>
#include <dect/auth.h>
#include "common.h"

static void mm_locate_cfm(struct dect_handle *dh, struct dect_mm_endpoint *mme,
			  bool accept, struct dect_mm_locate_param *param)
{
	struct dect_mm_identity_assign_param reply = {};

	if (param->portable_identity)
		dect_mm_identity_assign_res(dh, mme, true, &reply);

	dect_event_loop_stop();
}

static int mm_locate_req(struct dect_handle *dh, struct dect_mm_endpoint *mme)
{
	struct dect_ie_portable_identity portable_identity;
	struct dect_ie_location_area location_area;
	struct dect_ie_terminal_capability terminal_capability;
	struct dect_mm_locate_param param = {
		.portable_identity	= &portable_identity,
		.location_area		= &location_area,
		.terminal_capability	= &terminal_capability,
	};

	portable_identity.type	= DECT_PORTABLE_ID_TYPE_IPUI;
	portable_identity.ipui	= ipui;

	location_area.type	= DECT_LOCATION_AREA_LEVEL;
	location_area.level	= 36;

	dect_pp_init_terminal_capability(&terminal_capability);

	return dect_mm_locate_req(dh, mme, &param);
}

static struct dect_mm_ops mm_ops = {
	.mm_locate_cfm		= mm_locate_cfm,
};

static struct dect_ops ops = {
	.mm_ops			= &mm_ops,
};

int main(int argc, char **argv)
{
	const struct dect_fp_capabilities *fpc;
	struct dect_mm_endpoint *mme;

	dect_pp_common_init(&ops, argv[1], &ipui);

	fpc = dect_llme_fp_capabilities(dh);
	if (!(fpc->hlc & DECT_HLC_LOCATION_REGISTRATION)) {
		fprintf(stderr, "FP does not support location registration\n");
		goto out;
	}

	mme = dect_mm_endpoint_alloc(dh, &ipui);
	if (mme == NULL)
		pexit("dect_mm_endpoint_alloc");

	mm_locate_req(dh, mme);
	dect_event_loop();
out:
	dect_common_cleanup(dh);
	return 0;
}
