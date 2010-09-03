/*
 * DECT PP detach example
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

static int mm_detach_req(struct dect_handle *dh, struct dect_mm_endpoint *mme)
{
	struct dect_ie_portable_identity portable_identity;
	struct dect_mm_detach_param param = {
		.portable_identity	= &portable_identity,
	};

	portable_identity.type	= DECT_PORTABLE_ID_TYPE_IPUI;
	portable_identity.ipui	= ipui;

	return dect_mm_detach_req(dh, mme, &param);
}

static struct dect_ops ops;

int main(int argc, char **argv)
{
        struct dect_mm_endpoint *mme;

	dect_pp_common_options(argc, argv);
	dect_pp_common_init(&ops, cluster, &ipui);

	mme = dect_mm_endpoint_alloc(dh, &ipui);
	if (mme == NULL)
		pexit("dect_mm_endpoint_alloc");

	mm_detach_req(dh, mme);
	dect_event_loop();

	dect_common_cleanup(dh);
	return 0;
}
