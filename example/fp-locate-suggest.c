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
}

static void mm_info_req(struct dect_handle *dh,
			const struct dect_ipui *ipui)
{
	struct dect_ie_info_type info_type;
	struct dect_mm_info_param req = { .info_type = &info_type };
	struct dect_mm_endpoint *mme;

	mme = dect_mm_endpoint_alloc(dh, ipui);
	if (mme == NULL)
		return;

	info_type.num = 1;
	info_type.type[0] = DECT_INFO_LOCATE_SUGGEST;
	dect_mm_info_req(dh, mme, &req);
}

static struct dect_mm_ops mm_ops = {
	.mm_locate_ind		= mm_locate_ind,
};

static struct dect_ops ops = {
	.mm_ops			= &mm_ops,
};

int main(int argc, char **argv)
{
	dect_fp_common_options(argc, argv);
	dect_common_init(&ops, argv[1]);

	mm_info_req(dh, &ipui);
	dect_event_loop();

	dect_common_cleanup(dh);
	return 0;
}
