/*
 * DECT PP location update example
 *
 * Copyright (c) 2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <dect/libdect.h>
#include "common.h"
#include <lce.h>

static const struct dect_ipui ipui = {
	.put		= DECT_IPUI_N,
	.pun.n.ipei = {
		.emc	= 0x0ba8,
		.psn	= 0xa782a,
	}
};

static struct dect_ops ops;

int main(int argc, char **argv)
{
	dect_common_init(&ops, argv[1]);
	dect_pp_set_ipui(dh, &ipui);

	dect_event_loop();

	dect_common_cleanup(dh);
	return 0;
}
