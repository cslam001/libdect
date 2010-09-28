/*
 * DECT PP common functions
 *
 * Copyright (c) 2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include "common.h"

enum {
	OPT_CLUSTER,
	OPT_HELP,
};

static const struct option options[] = {
	{ .name = "cluster",	.has_arg = true,  .flag = NULL, .val = OPT_CLUSTER },
	{ .name = "help",	.has_arg = false, .flag = NULL, .val = OPT_HELP },

	{ },
};

void dect_fp_common_options(int argc, char **argv)
{
	int optidx = 0, c;

	for (;;) {
		c = getopt_long(argc, argv, "c:h", options, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case OPT_CLUSTER:
			cluster = optarg;
			break;
		case OPT_HELP:
			printf("%s: [ -c/--cluster NAME ] [ -h/--help ]\n",
			       argv[0]);
			exit(0);
		case '?':
			exit(1);
		}
	}
}
