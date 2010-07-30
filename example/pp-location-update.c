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
#include <dect/auth.h>
#include "common.h"

static const struct dect_ipui ipui = {
	.put		= DECT_IPUI_N,
	.pun.n.ipei = {
		.emc	= 0x0ba8,
		.psn	= 0xa782a,
	}
};

static void init_terminal_capability(struct dect_ie_terminal_capability *tcap)
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
	tcap->profile_indicator	= DECT_PROFILE_GAP_SUPPORTED;
}

static void mm_locate_cfm(struct dect_handle *dh, struct dect_mm_endpoint *mme,
			  bool accept, struct dect_mm_locate_param *param)
{
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

	init_terminal_capability(&terminal_capability);

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
        struct dect_mm_endpoint *mme;

	dect_pp_auth_init(&ops, &ipui);
	dect_common_init(&ops, argv[1]);

	mme = dect_mm_endpoint_alloc(dh, &ipui);
	if (mme == NULL)
		pexit("dect_mm_endpoint_alloc");

	mm_locate_req(dh, mme);
	dect_event_loop();

	dect_common_cleanup(dh);
	dect_mm_endpoint_destroy(dh, mme);
	return 0;
}
