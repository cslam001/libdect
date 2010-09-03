/*
 * DECT PP paging example
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

static struct dect_ops ops;

int main(int argc, char **argv)
{
	dect_pp_common_options(argc, argv);
	dect_pp_common_init(&ops, cluster, &ipui);

	dect_event_loop();

	dect_common_cleanup(dh);
	return 0;
}
