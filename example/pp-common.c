/*
 * DECT PP common functions
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
