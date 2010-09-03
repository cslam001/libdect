/*
 * DECT PP common functions
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

#include <dect/libdect.h>
#include <dect/auth.h>
#include "common.h"

#define debug(fmt, args...)	printf("P-IWU: " fmt, ## args)

void dect_pp_init_terminal_capability(struct dect_ie_terminal_capability *tcap)
{
	tcap->tone		= DECT_TONE_CAPABILITY_DIAL_TONE_ONLY;
	tcap->echo		= DECT_ECHO_PARAMETER_FULL_TCLW;
	tcap->noise_rejection	= DECT_NOISE_REJECTION_NONE;
	tcap->volume_ctrl	= DECT_ADAPTIVE_VOLUME_PP_CONTROL_NONE;
	tcap->slot		= DECT_SLOT_CAPABILITY_FULL_SLOT;

	tcap->display		= DECT_DISPLAY_CAPABILITY_FULL_DISPLAY;
	tcap->display_memory	= 48;
	tcap->display_lines	= 3;
	tcap->display_columns	= 16;
	tcap->display_control	= DECT_DISPLAY_CONTROL_CODE_CODING_1;
	tcap->display_charsets	= 0;
	tcap->scrolling		= DECT_SCROLLING_NOT_SPECIFIED;
	tcap->profile_indicator	= DECT_PROFILE_GAP_SUPPORTED |
				  DECT_PROFILE_REKEYING_EARLY_ENCRYPTION_SUPPORTED |
				  DECT_PROFILE_NG_DECT_PART_1 |
				  DECT_PROFILE_NG_DECT_PART_3;
}

void dect_pp_common_init(struct dect_ops *ops, const char *cluster,
			 const struct dect_ipui *ipui)
{
	dect_pp_auth_init(ops, ipui);
	dect_common_init(ops, cluster);
	dect_pp_set_ipui(dh, ipui);
}

enum {
	OPT_CLUSTER	= 'c',
	OPT_IPUI	= 'i',
	OPT_HELP	= 'h',
};

static const struct option options[] = {
	{ .name = "cluster",	.has_arg = true,  .flag = NULL, .val = OPT_CLUSTER },
	{ .name = "ipui",	.has_arg = true,  .flag = NULL, .val = OPT_IPUI },
	{ .name = "help",	.has_arg = false, .flag = NULL, .val = OPT_HELP },
	{ }
};

void dect_pp_common_options(int argc, char **argv)
{
	int optidx = 0, c;

	for (;;) {
		c = getopt_long(argc, argv, "c:h", options, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case OPT_CLUSTER:
			break;
		case OPT_IPUI:
			if (dect_parse_ipui(&ipui, optarg))
				exit(1);
			break;
		case OPT_HELP:
			printf("%s [ -c/--cluster NAME ] [ -h/--help ]\n",
			       argv[0]);
			exit(0);
		case '?':
			exit(1);
		}
	}
}
