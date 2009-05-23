/*
 * DECT S-Format messages
 *
 * Copyright (c) 2009 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <asm/byteorder.h>

#include <libdect.h>
#include <identities.h>
#include <utils.h>
#include <s_fmt.h>
#include <lce.h>

static struct dect_ie_common *dect_ie_alloc(const struct dect_handle *dh,
					    unsigned int size)
{
	struct dect_ie_common *ie;

	ie = dect_zalloc(dh, size);
	if (ie == NULL)
		return NULL;
	__dect_ie_init(ie);
	return ie;
}

static int dect_sfmt_parse_repeat_indicator(const struct dect_handle *dh,
					    struct dect_ie_common **ie,
					    const struct dect_sfmt_ie *src)
{
	struct dect_ie_repeat_indicator *dst = dect_ie_container(dst, *ie);

	dst->type = src->data[0] & DECT_SFMT_IE_FIXED_VAL_MASK;
	switch (dst->type) {
	case DECT_SFMT_IE_LIST_NORMAL:
	case DECT_SFMT_IE_LIST_PRIORITIZED:
		return 0;
	default:
		dect_debug("invalid list type\n");
		return -1;
	}
}

static int dect_sfmt_build_repeat_indicator(struct dect_sfmt_ie *dst,
					    const struct dect_ie_common *ie)
{
	struct dect_ie_repeat_indicator *src = dect_ie_container(src, ie);

	dect_debug("build repeat indicator list %p\n", src->list);
	dst->data[0] = src->type;
	return 0;
}

static int dect_sfmt_parse_empty_single_octet(const struct dect_handle *dh,
					      struct dect_ie_common **ie,
					      const struct dect_sfmt_ie *src)
{
	return 0;
}

static int dect_sfmt_build_empty_single_octet(struct dect_sfmt_ie *dst,
					      const struct dect_ie_common *ie)
{
	dst->data[0] = 0;
	return 0;
}

static const char *call_classes[DECT_CALL_CLASS_MAX + 1] = {
	[DECT_CALL_CLASS_MESSAGE]		= "message call",
	[DECT_CALL_CLASS_DECT_ISDN]		= "DECT/ISDN IIP",
	[DECT_CALL_CLASS_NORMAL]		= "normal call",
	[DECT_CALL_CLASS_INTERNAL]		= "internal call",
	[DECT_CALL_CLASS_EMERGENCY]		= "emergency call",
	[DECT_CALL_CLASS_SERVICE]		= "service call",
	[DECT_CALL_CLASS_EXTERNAL_HO]		= "external handover call",
	[DECT_CALL_CLASS_SUPPLEMENTARY_SERVICE]	= "supplementary service call",
	[DECT_CALL_CLASS_QA_M]			= "QA&M call",
};

static const char *basic_services[DECT_SERVICE_MAX + 1] = {
	[DECT_SERVICE_BASIC_SPEECH_DEFAULT]	= "basic speech default attributes",
	[DECT_SERVICE_DECT_GSM_IWP]		= "DECT GSM IWP profile",
	[DECT_SERVICE_UMTS_IWP]			= "DECT UMTS IWP",
	[DECT_SERVICE_LRMS]			= "LRMS (E-profile) service",
	[DECT_SERVICE_GSM_IWP_SMS]		= "GSM IWP SMS",
	[DECT_SERVICE_WIDEBAND_SPEECH]		= "Wideband speech",
	[DECT_SERVICE_OTHER]			= "Other",
};

static void dect_sfmt_dump_basic_service(const struct dect_ie_common *_ie)
{
	const struct dect_ie_basic_service *ie = dect_ie_container(ie, _ie);

	dect_debug("basic service:\n\tcall class: %s\n\tservice: %s\n",
		   call_classes[ie->class], basic_services[ie->service]);
}

static int dect_sfmt_parse_basic_service(const struct dect_handle *dh,
					 struct dect_ie_common **ie,
					 const struct dect_sfmt_ie *src)
{
	struct dect_ie_basic_service *dst = dect_ie_container(dst, *ie);

	dst->class   = src->data[1] >> DECT_BASIC_SERVICE_CALL_CLASS_SHIFT;
	dst->service = src->data[1] & DECT_BASIC_SERVICE_SERVICE_MASK;
	dect_sfmt_dump_basic_service(*ie);
	return 0;
}

static int dect_sfmt_build_basic_service(struct dect_sfmt_ie *dst,
					 const struct dect_ie_common *ie)
{
	struct dect_ie_basic_service *src = dect_ie_container(src, ie);

	dst->data[1]  = src->class << DECT_BASIC_SERVICE_CALL_CLASS_SHIFT;
	dst->data[1] |= src->service;
	return 0;
}

static int dect_sfmt_parse_single_display(const struct dect_handle *dh,
					  struct dect_ie_common **ie,
					  const struct dect_sfmt_ie *src)
{
	struct dect_ie_display *dst = dect_ie_container(dst, *ie);

	dst->info[0] = src->data[1];
	dst->len     = 1;
	dect_debug("single display: '%c'\n", dst->info[0]);
	return 0;
}

static int dect_sfmt_build_single_display(struct dect_sfmt_ie *dst,
					  const struct dect_ie_common *src)
{
	struct dect_ie_display *ie = dect_ie_container(ie, src);

	dst->data[1] = ie->info[0];
	return 0;
}

static int dect_sfmt_parse_single_keypad(const struct dect_handle *dh,
					 struct dect_ie_common **ie,
					 const struct dect_sfmt_ie *src)
{
	struct dect_ie_keypad *dst = dect_ie_container(dst, *ie);

	dst->info[0] = src->data[1];
	dst->len = 1;
	dect_debug("single keypad: '%c'\n", dst->info[0]);
	return 0;
}

static int dect_sfmt_parse_release_reason(const struct dect_handle *dh,
					  struct dect_ie_common **ie,
					  const struct dect_sfmt_ie *src)
{
	struct dect_ie_release_reason *dst = dect_ie_container(dst, *ie);

	dst->reason = src->data[1];
	dect_debug("release reason: %x\n", dst->reason);
	return 0;
}

static int dect_sfmt_build_release_reason(struct dect_sfmt_ie *dst,
					  const struct dect_ie_common *ie)
{
	struct dect_ie_release_reason *src = dect_ie_container(src, ie);

	dst->data[1] = src->reason;
	return 0;
}

static int dect_sfmt_parse_signal(const struct dect_handle *dh,
				  struct dect_ie_common **ie,
				  const struct dect_sfmt_ie *src)
{
	struct dect_ie_signal *dst = dect_ie_container(dst, *ie);

	dst->code = src->data[1];
	return 0;
}

static int dect_sfmt_build_signal(struct dect_sfmt_ie *dst,
				  const struct dect_ie_common *src)
{
	struct dect_ie_signal *ie = dect_ie_container(ie, src);

	dst->data[1] = ie->code;
	return 0;
}

static int dect_sfmt_parse_timer_restart(const struct dect_handle *dh,
					 struct dect_ie_common **ie,
					 const struct dect_sfmt_ie *src)
{
	struct dect_ie_timer_restart *dst = dect_ie_container(dst, *ie);

	dst->code = src->data[1];
	switch (dst->code) {
	case DECT_TIMER_RESTART:
	case DECT_TIMER_STOP:
		return 0;
	default:
		return -1;
	}
}

static int dect_sfmt_parse_portable_identity(const struct dect_handle *dh,
					     struct dect_ie_common **ie,
					     const struct dect_sfmt_ie *src)
{
	struct dect_ie_portable_identity *dst = dect_ie_container(dst, *ie);
	uint8_t len;

	if (src->len < S_VL_IE_PORTABLE_IDENTITY_MIN_SIZE)
		return -1;

	if (!(src->data[2] & DECT_OCTET_GROUP_END))
		return -1;
	dst->type = src->data[2] & ~DECT_OCTET_GROUP_END;

	if (!(src->data[3] & DECT_OCTET_GROUP_END))
		return -1;
	len = src->data[3] & ~DECT_OCTET_GROUP_END;

	switch (dst->type) {
	case DECT_PORTABLE_ID_TYPE_IPUI:
		if (!dect_parse_ipui(&dst->ipui, src->data + 4, len))
			dect_debug("parsing failed\n");
		return 0;
	case DECT_PORTABLE_ID_TYPE_IPEI:
		return 0;
	case DECT_PORTABLE_ID_TYPE_TPUI:
		return 0;
	default:
		dect_debug("invalid type %u\n", dst->type);
		return -1;
	}
}

static int dect_sfmt_build_portable_identity(struct dect_sfmt_ie *dst,
					     const struct dect_ie_common *src)
{
	const struct dect_ie_portable_identity *ie = dect_ie_container(ie, src);
	uint32_t tpui;
	uint8_t len;

	switch (ie->type) {
	case DECT_PORTABLE_ID_TYPE_IPUI:
		len = dect_build_ipui(&dst->data[4], &ie->ipui);
		if (len == 0)
			return -1;
		break;
	case DECT_PORTABLE_ID_TYPE_IPEI:
		return -1;
	case DECT_PORTABLE_ID_TYPE_TPUI:
		tpui = dect_build_tpui(&ie->tpui);
		dst->data[6] = tpui;
		dst->data[5] = tpui >> 8;
		dst->data[4] = tpui >> 16;
		len = 20;
		break;
	default:
		return -1;
	}

	dst->data[3] = DECT_OCTET_GROUP_END | len;
	dst->data[2] = DECT_OCTET_GROUP_END | ie->type;
	dst->len = 4 + div_round_up(len, 8);
	return 0;
}

static int dect_sfmt_parse_fixed_identity(const struct dect_handle *dh,
					  struct dect_ie_common **ie,
					  const struct dect_sfmt_ie *src)
{
	struct dect_ie_fixed_identity *dst = dect_ie_container(dst, *ie);
	uint8_t len, ari_len;
	uint64_t ari;

	if (src->len < S_VL_IE_FIXED_IDENTITY_MIN_SIZE)
		return -1;

	if (!(src->data[2] & DECT_OCTET_GROUP_END))
		return -1;
	dst->type = src->data[2] & ~DECT_OCTET_GROUP_END;

	if (!(src->data[3] & DECT_OCTET_GROUP_END))
		return -1;
	len = src->data[3] & ~DECT_OCTET_GROUP_END;

	ari  = __be64_to_cpu(*(__be64 *)&src->data[4]);
	ari_len = dect_parse_ari(&dst->ari, ari << 1);
	if (ari_len == 0)
		return -1;

	switch (dst->type) {
	case DECT_FIXED_ID_TYPE_ARI:
	case DECT_FIXED_ID_TYPE_PARK:
		return ari_len + 1 == len;
	case DECT_FIXED_ID_TYPE_ARI_RPN:
	case DECT_FIXED_ID_TYPE_ARI_WRS:
		return 0;
	default:
		dect_debug("invalid type %u\n", dst->type);
		return -1;
	}
}

static int dect_sfmt_build_fixed_identity(struct dect_sfmt_ie *dst,
					  const struct dect_ie_common *ie)
{
	struct dect_ie_fixed_identity *src = dect_ie_container(src, ie);
	uint64_t ari;

	ari = dect_build_ari(&src->ari) >> 1;
	dst->data[8] = ari >> 24;
	dst->data[7] = ari >> 32;
	dst->data[6] = ari >> 40;
	dst->data[5] = ari >> 48;
	dst->data[4] = ari >> 56;
	dst->data[3] = DECT_OCTET_GROUP_END | (DECT_ARC_A_LEN + 1);
	dst->data[2] = DECT_OCTET_GROUP_END | src->type;
	dst->len = 9;
	return 0;
}

static int dect_sfmt_parse_location_area(const struct dect_handle *dh,
					 struct dect_ie_common **ie,
					 const struct dect_sfmt_ie *src)
{
	struct dect_ie_location_area *dst = dect_ie_container(dst, *ie);

	dst->type  = (src->data[2] & DECT_LOCATION_AREA_TYPE_MASK) >>
		     DECT_LOCATION_AREA_TYPE_SHIFT;
	dst->level = (src->data[2] & DECT_LOCATION_LEVEL_MASK);
	dect_debug("\ttype: %x level: %x\n", dst->type, dst->level);
	return 0;
}

static int dect_sfmt_build_location_area(struct dect_sfmt_ie *dst,
					 const struct dect_ie_common *ie)
{
	struct dect_ie_location_area *src = dect_ie_container(src, ie);

	dst->data[2]  = src->type << DECT_LOCATION_AREA_TYPE_SHIFT;
	dst->data[2] |= src->level;
	dst->len = 3;
	return 0;
}

static const char *dect_auth_algs[] = {
	[DECT_AUTH_DSAA]	= "DSAA",
	[DECT_AUTH_GSM]		= "GSM",
	[DECT_AUTH_UMTS]	= "UMTS",
	[DECT_AUTH_PROPRIETARY]	= "proprietary",
};

static const char *dect_auth_key_types[] = {
	[DECT_KEY_USER_AUTHENTICATION_KEY]	= "User authentication key",
	[DECT_KEY_USER_PERSONAL_IDENTITY]	= "User personal identity",
	[DECT_KEY_AUTHENTICATION_CODE]		= "Authentication code",
};

static void dect_sfmt_dump_auth_type(const struct dect_ie_common *_ie)
{
	const struct dect_ie_auth_type *ie = dect_ie_container(ie, _ie);

	dect_debug("\tauthentication algorithm: %s\n", dect_auth_algs[ie->auth_id]);
	dect_debug("\tauthentication key type: %s\n", dect_auth_key_types[ie->auth_key_type]);
	dect_debug("\tINC: %u TXC: %u UPC: %u\n",
		   ie->flags & DECT_AUTH_FLAG_INC ? 1 : 0,
		   ie->flags & DECT_AUTH_FLAG_TXC ? 1 : 0,
		   ie->flags & DECT_AUTH_FLAG_UPC ? 1 : 0);
}

static int dect_sfmt_parse_auth_type(const struct dect_handle *dh,
				     struct dect_ie_common **ie,
				     const struct dect_sfmt_ie *src)
{
	struct dect_ie_auth_type *dst = dect_ie_container(dst, *ie);
	uint8_t n = 2;

	dst->auth_id = src->data[n++];
	if (dst->auth_id == DECT_AUTH_PROPRIETARY)
		dst->proprietary_auth_id = src->data[n]++;

	dst->auth_key_type  = (src->data[n] & 0xf0) >> 4;
	dst->auth_key_num   = (src->data[n] & 0x0f);
	n++;

	dst->flags	    = src->data[n] & 0xf0;
	dst->cipher_key_num = src->data[n] & 0x0f;

	dect_sfmt_dump_auth_type(*ie);
	return 0;
}

static int dect_sfmt_parse_progress_indicator(const struct dect_handle *dh,
					      struct dect_ie_common **ie,
					      const struct dect_sfmt_ie *src)
{
	struct dect_ie_progress_indicator *dst = dect_ie_container(dst, *ie);

	dst->location = src->data[2] & DECT_SFMT_IE_PROGRESS_INDICATOR_LOCATION_MASK;
	dst->progress = src->data[3];
	return 0;
}

static int dect_sfmt_build_progress_indicator(struct dect_sfmt_ie *dst,
					      const struct dect_ie_common *ie)
{
	struct dect_ie_progress_indicator *src = dect_ie_container(src, ie);

	dst->data[3] = DECT_OCTET_GROUP_END | src->progress;
	dst->data[2] = DECT_OCTET_GROUP_END | src->location;
	dst->len = 4;
	return 0;
}

static int dect_sfmt_build_multi_display(struct dect_sfmt_ie *dst,
					 const struct dect_ie_common *ie)
{
	struct dect_ie_display *src = dect_ie_container(src, ie);

	memcpy(dst->data + 2, src->info, src->len);
	dst->len = src->len + 2;
	return 0;
}

static int dect_sfmt_parse_multi_keypad(const struct dect_handle *dh,
					struct dect_ie_common **ie,
					const struct dect_sfmt_ie *src)
{
	struct dect_ie_keypad *dst = dect_ie_container(dst, *ie);

	dst->len = src->len - 2;
	memcpy(dst->info, src->data + 2, src->len - 2);
	dect_debug("multi-keypad: '%.*s'\n", dst->len, dst->info);
	return 0;
}

static int dect_sfmt_parse_reject_reason(const struct dect_handle *dh,
					 struct dect_ie_common **ie,
					 const struct dect_sfmt_ie *src)
{
	struct dect_ie_reject_reason *dst = dect_ie_container(dst, *ie);

	dst->reason = src->data[2];
	dect_debug("reject reason: %x\n", dst->reason);
	return 0;
}

static int dect_sfmt_build_reject_reason(struct dect_sfmt_ie *dst,
					 const struct dect_ie_common *ie)
{
	struct dect_ie_reject_reason *src = dect_ie_container(src, ie);

	dst->data[2] = src->reason;
	dst->len = 3;
	return 0;
}

static const char *display_capabilities[] = {
	[DECT_DISPLAY_CAPABILITY_NOT_APPLICABLE]	= "not applicable",
	[DECT_DISPLAY_CAPABILITY_NO_DISPLAY]		= "no display",
	[DECT_DISPLAY_CAPABILITY_NUMERIC]		= "numeric",
	[DECT_DISPLAY_CAPABILITY_NUMERIC_PLUS]		= "numeric-plus",
	[DECT_DISPLAY_CAPABILITY_ALPHANUMERIC]		= "alphanumeric",
	[DECT_DISPLAY_CAPABILITY_FULL_DISPLAY]		= "full display",
};

static const char *tone_capabilities[] = {
	[DECT_TONE_CAPABILITY_NOT_APPLICABLE]		= "not applicable",
	[DECT_TONE_CAPABILITY_NO_TONE]			= "no tone",
	[DECT_TONE_CAPABILITY_DIAL_TONE_ONLY]		= "dial tone only",
	[DECT_TONE_CAPABILITY_ITU_T_E182_TONES]		= "ITU-T E.182 tones",
	[DECT_TONE_CAPABILITY_COMPLETE_DECT_TONES]	= "complete DECT tones",
};

static const char *echo_parameters[] = {
	[DECT_ECHO_PARAMETER_NOT_APPLICABLE]		= "not applicable",
	[DECT_ECHO_PARAMETER_MINIMUM_TCLW]		= "TCL > 34 dB",
	[DECT_ECHO_PARAMETER_FULL_TCLW]			= "TCL > 46 dB",
	[DECT_ECHO_PARAMETER_VOIP_COMPATIBLE_TLCW]	= "TCL > 55 dB",
};

static const char *noise_rejection_capabilities[] = {
	[DECT_NOISE_REJECTION_NOT_APPLICABLE]		= "not applicable",
	[DECT_NOISE_REJECTION_NONE]			= "none",
	[DECT_NOISE_REJECTION_PROVIDED]			= "provided",
};

static const char *volume_ctrl_provisions[] = {
	[DECT_ADAPTIVE_VOLUME_NOT_APPLICABLE]		= "not applicable",
	[DECT_ADAPTIVE_VOLUME_PP_CONTROL_NONE]		= "no PP adaptive volume control",
	[DECT_ADAPTIVE_VOLUME_PP_CONTROL_USED]		= "PP adaptive volume control",
	[DECT_ADAPTIVE_VOLUME_FP_CONTROL_DISABLE]	= "disable FP adaptive volume control",
};

static const char *scrolling_behaviour[] = {
	[DECT_SCROLLING_NOT_SPECIFIED]			= "not specified",
	[DECT_SCROLLING_TYPE_1]				= "type 1",
	[DECT_SCROLLING_TYPE_2]				= "type 2",
};

static void dect_sfmt_dump_terminal_capability(const struct dect_ie_common *_ie)
{
	const struct dect_ie_terminal_capability *ie = dect_ie_container(ie, _ie);

	dect_debug("\tdisplay capability: %s\n", display_capabilities[ie->display]);
	dect_debug("\ttone capability: %s\n", tone_capabilities[ie->tone]);
	dect_debug("\techo parameters: %s\n", echo_parameters[ie->echo]);
	dect_debug("\tnoise rejection capability: %s\n", noise_rejection_capabilities[ie->noise_rejection]);
	dect_debug("\tadaptive volume control provision: %s\n", volume_ctrl_provisions[ie->volume_ctrl]);
	dect_debug("\tslot capabilities: %x\n", ie->slot);
	dect_debug("\tdisplay memory: %u\n", ie->display_memory);
	dect_debug("\tdisplay lines: %u\n", ie->display_lines);
	dect_debug("\tdisplay columns: %u\n", ie->display_columns);
	dect_debug("\tscrolling behaviour: %s\n", scrolling_behaviour[ie->scrolling]);
	dect_debug("\tprofile indicator: %" PRIx64 "\n", ie->profile_indicator);
	dect_debug("\tdisplay control: %x\n", ie->display_control);
	dect_debug("\tdisplay charsets: %x\n", ie->display_charsets);
}

static int dect_sfmt_parse_terminal_capability(const struct dect_handle *dh,
					       struct dect_ie_common **ie,
					       const struct dect_sfmt_ie *src)
{
	struct dect_ie_terminal_capability *dst = dect_ie_container(dst, *ie);
	uint8_t i, n = 2;

	/* Octet group 3 */
	dst->display = (src->data[n] & DECT_TERMINAL_CAPABILITY_DISPLAY_MASK);
	dst->tone    = (src->data[n] & DECT_TERMINAL_CAPABILITY_TONE_MASK) >>
		       DECT_TERMINAL_CAPABILITY_TONE_SHIFT;
	if (src->data[n++] & DECT_OCTET_GROUP_END)
		goto group4;

	dst->echo	     = (src->data[n] & DECT_TERMINAL_CAPABILITY_ECHO_MASK) >>
			       DECT_TERMINAL_CAPABILITY_ECHO_SHIFT;
	dst->noise_rejection = (src->data[n] & DECT_TERMINAL_CAPABILITY_NOISE_MASK) >>
			       DECT_TERMINAL_CAPABILITY_NOISE_SHIFT;
	dst->volume_ctrl     = (src->data[n] & DECT_TERMINAL_CAPABILITY_VOLUME_MASK);
	if (src->data[n++] & DECT_OCTET_GROUP_END)
		goto group4;

	dst->slot = src->data[n] & ~DECT_OCTET_GROUP_END;
	if (src->data[n++] & DECT_OCTET_GROUP_END)
		goto group4;

	dst->display_memory = src->data[n] & ~DECT_OCTET_GROUP_END;
	if (src->data[n++] & DECT_OCTET_GROUP_END)
		goto group4;
	dst->display_memory <<= 7;

	dst->display_memory += src->data[n] & ~DECT_OCTET_GROUP_END;
	if (src->data[n++] & DECT_OCTET_GROUP_END)
		goto group4;

	dst->display_lines   = src->data[n] & ~DECT_OCTET_GROUP_END;
	if (src->data[n++] & DECT_OCTET_GROUP_END)
		goto group4;

	dst->display_columns = src->data[n] & ~DECT_OCTET_GROUP_END;
	if (src->data[n++] & DECT_OCTET_GROUP_END)
		goto group4;

	dst->scrolling	     = src->data[n] & ~DECT_OCTET_GROUP_END;
	if (src->data[n++] & DECT_OCTET_GROUP_END)
		goto group4;

group4:
	for (i = 0; i < 8; i++) {
		dst->profile_indicator |=
			(uint64_t)(src->data[n] & ~DECT_OCTET_GROUP_END) <<
			(64 - 8 * (i + 1));
		if (src->data[n++] & DECT_OCTET_GROUP_END)
			goto group5;
	}

group5:
	dst->display_control = src->data[n] & 0x7;
	if (src->data[n++] & DECT_OCTET_GROUP_END)
		goto group6;
	dst->display_charsets = src->data[n] & ~DECT_OCTET_GROUP_END;
	if (src->data[n++] & DECT_OCTET_GROUP_END)
		goto group6;

group6:
	/* Older equipment may not include octet group 6 */
	if (n == src->len)
		goto group7;
	if (src->data[n++] & DECT_OCTET_GROUP_END)
		goto group7;
	if (!(src->data[n++] & DECT_OCTET_GROUP_END))
		return -1;

group7:
	dect_sfmt_dump_terminal_capability(*ie);
	return 0;
}

static int dect_sfmt_parse_duration(const struct dect_handle *dh,
				    struct dect_ie_common **ie,
				    const struct dect_sfmt_ie *src)
{
	struct dect_ie_duration *dst = dect_ie_container(dst, *ie);

	dst->lock = src->data[2] & 0x70;
	dst->time = src->data[2] & 0x0f;
	if (!(src->data[2] & DECT_OCTET_GROUP_END))
		dst->duration = src->data[3];
	return 0;
}

static int dect_sfmt_build_duration(struct dect_sfmt_ie *dst,
				    const struct dect_ie_common *ie)
{
	struct dect_ie_duration *src = dect_ie_container(src, ie);

	dst->len = 3;
	dst->data[2] = (src->lock << 4) | src->time;
	if (src->time != DECT_TIME_LIMIT_DEFINED_TIME_LIMIT_1 &&
	    src->time != DECT_TIME_LIMIT_DEFINED_TIME_LIMIT_2)
		dst->data[2] |= DECT_OCTET_GROUP_END;
	else {
		dst->data[3] = src->duration;
		dst->len++;
	}
	return 0;
}

static int dect_sfmt_parse_escape_to_proprietary(const struct dect_handle *dh,
						 struct dect_ie_common **ie,
						 const struct dect_sfmt_ie *src)
{
	struct dect_ie_escape_to_proprietary *dst = dect_ie_container(dst, *ie);
	uint8_t dtype;

	dtype = (src->data[2] & DECT_ESC_TO_PROPRIETARY_IE_DESC_TYPE_MASK);
	if (dtype != DECT_ESC_TO_PROPRIETARY_IE_DESC_EMC)
		return -1;
	dst->emc = __be16_to_cpu(*(__be16 *)&src->data[3]);
	dect_debug("EMC %x\n", dst->emc);
	return 0;
}

static const struct dect_ie_handler {
	const char	*name;
	size_t		size;
	int		(*parse)(const struct dect_handle *dh,
				 struct dect_ie_common **dst,
				 const struct dect_sfmt_ie *ie);
	int		(*build)(struct dect_sfmt_ie *dst,
				 const struct dect_ie_common *ie);
} dect_ie_handlers[256] = {
	[S_SO_IE_REPEAT_INDICATOR]		= {
		.name	= "repeat indicator",
		.parse	= dect_sfmt_parse_repeat_indicator,
		.build	= dect_sfmt_build_repeat_indicator,
	},
	[S_SE_IE_SENDING_COMPLETE]		= {
		.name	= "sending complete",
		.size	= sizeof(struct dect_ie_sending_complete),
		.parse	= dect_sfmt_parse_empty_single_octet,
		.build	= dect_sfmt_build_empty_single_octet,
	},
	[S_SE_IE_DELIMITER_REQUEST]		= {
		.name	= "delimiter request",
		.size	= sizeof(struct dect_ie_delimiter_request),
		.parse	= dect_sfmt_parse_empty_single_octet,
		.build	= dect_sfmt_build_empty_single_octet,
	},
	[S_SE_IE_USE_TPUI]			= {
		.name	= "use TPUI",
		.size	= sizeof(struct dect_ie_use_tpui),
		.parse	= dect_sfmt_parse_empty_single_octet,
		.build	= dect_sfmt_build_empty_single_octet,
	},
	[S_DO_IE_BASIC_SERVICE]			= {
		.name	= "basic service",
		.size	= sizeof(struct dect_ie_basic_service),
		.parse	= dect_sfmt_parse_basic_service,
		.build	= dect_sfmt_build_basic_service,
	},
	[S_DO_IE_RELEASE_REASON]		= {
		.name	= "release reason",
		.size	= sizeof(struct dect_ie_release_reason),
		.parse	= dect_sfmt_parse_release_reason,
		.build	= dect_sfmt_build_release_reason,
	},
	[S_DO_IE_SIGNAL]			= {
		.name	= "signal",
		.size	= sizeof(struct dect_ie_signal),
		.parse	= dect_sfmt_parse_signal,
		.build	= dect_sfmt_build_signal,
	},
	[S_DO_IE_TIMER_RESTART]			= {
		.name	= "timer restart",
		.size	= sizeof(struct dect_ie_timer_restart),
		.parse	= dect_sfmt_parse_timer_restart,
	},
	[S_DO_IE_TEST_HOOK_CONTROL]		= {
		.name	= "test hook control",
	},
	[S_DO_IE_SINGLE_DISPLAY]		= {
		.name	= "single display",
		.size	= sizeof(struct dect_ie_display),
		.parse	= dect_sfmt_parse_single_display,
		.build	= dect_sfmt_build_single_display,
	},
	[S_DO_IE_SINGLE_KEYPAD]			= {
		.name	= "single keypad",
		.size	= sizeof(struct dect_ie_keypad),
		.parse	= dect_sfmt_parse_single_keypad,
	},
	[S_VL_IE_INFO_TYPE]			= {
		.name	= "info type",
		.size	= sizeof(struct dect_ie_info_type),
	},
	[S_VL_IE_IDENTITY_TYPE]			= {
		.name	= "identity type",
		.size	= sizeof(struct dect_ie_identity_type)
	},
	[S_VL_IE_PORTABLE_IDENTITY]		= {
		.name	= "portable identity",
		.size	= sizeof(struct dect_ie_portable_identity),
		.parse	= dect_sfmt_parse_portable_identity,
		.build	= dect_sfmt_build_portable_identity,
	},
	[S_VL_IE_FIXED_IDENTITY]		= {
		.name	= "fixed identity",
		.size	= sizeof(struct dect_ie_fixed_identity),
		.parse	= dect_sfmt_parse_fixed_identity,
		.build	= dect_sfmt_build_fixed_identity,
	},
	[S_VL_IE_LOCATION_AREA]			= {
		.name	= "location area",
		.size	= sizeof(struct dect_ie_location_area),
		.parse	= dect_sfmt_parse_location_area,
		.build	= dect_sfmt_build_location_area,
	},
	[S_VL_IE_NWK_ASSIGNED_IDENTITY]		= {
		.name	= "NWK assigned identity",
		.size	= sizeof(struct dect_ie_nwk_assigned_identity),
	},
	[S_VL_IE_AUTH_TYPE]			= {
		.name	= "auth type",
		.size	= sizeof(struct dect_ie_auth_type),
		.parse	= dect_sfmt_parse_auth_type,
	},
	[S_VL_IE_ALLOCATION_TYPE]		= {
		.name	= "allocation type",
		.size	= sizeof(struct dect_ie_allocation_type),
	},
	[S_VL_IE_RAND]				= {
		.name	= "RAND",
		.size	= sizeof(struct dect_ie_rand),
	},
	[S_VL_IE_RES]				= {
		.name	= "RES",
		.size	= sizeof(struct dect_ie_res),
	},
	[S_VL_IE_RS]				= {
		.name	= "RS",
		.size	= sizeof(struct dect_ie_rs),
	},
	[S_VL_IE_IWU_ATTRIBUTES]		= {
		.name	= "IWU attributes",
		.size	= sizeof(struct dect_ie_iwu_attributes),
	},
	[S_VL_IE_CALL_ATTRIBUTES]		= {
		.name	= "call attributes",
		.size	= sizeof(struct dect_ie_call_attributes),
	},
	[S_VL_IE_SERVICE_CHANGE_INFO]		= {
		.name	= "service change info",
		.size	= sizeof(struct dect_ie_service_change_info),
	},
	[S_VL_IE_CONNECTION_ATTRIBUTES]		= {
		.name	= "connection attributes",
		.size	= sizeof(struct dect_ie_connection_attributes),
	},
	[S_VL_IE_CIPHER_INFO]			= {
		.name	= "cipher info",
		.size	= sizeof(struct dect_ie_cipher_info),
	},
	[S_VL_IE_CALL_IDENTITY]			= {
		.name	= "call identity",
		.size	= sizeof(struct dect_ie_call_identity),
	},
	[S_VL_IE_CONNECTION_IDENTITY]		= {
		.name	= "connection identity",
		.size	= sizeof(struct dect_ie_connection_identity),
	},
	[S_VL_IE_FACILITY]			= {
		.name	= "facility",
		.size	= sizeof(struct dect_ie_facility),
	},
	[S_VL_IE_PROGRESS_INDICATOR]		= {
		.name	= "progress indicator",
		.size	= sizeof(struct dect_ie_progress_indicator),
		.parse	= dect_sfmt_parse_progress_indicator,
		.build	= dect_sfmt_build_progress_indicator,
	},
	[S_VL_IE_MMS_GENERIC_HEADER]		= {
		.name	= "MMS generic header",
		.size	= sizeof(struct dect_ie_mms_generic_header),
	},
	[S_VL_IE_MMS_OBJECT_HEADER]		= {
		.name	= "MMS object header",
		.size	= sizeof(struct dect_ie_mms_object_header),
	},
	[S_VL_IE_MMS_EXTENDED_HEADER]		= {
		.name	= "MMS extended header",
		.size	= sizeof(struct dect_ie_mms_extended_header),
	},
	[S_VL_IE_TIME_DATE]			= {
		.name	= "time-date",
		.size	= sizeof(struct dect_ie_time_date),
	},
	[S_VL_IE_MULTI_DISPLAY]			= {
		.name	= "multi display",
		.size	= sizeof(struct dect_ie_display),
		.build	= dect_sfmt_build_multi_display,
	},
	[S_VL_IE_MULTI_KEYPAD]			= {
		.name	= "multi keypad",
		.size	= sizeof(struct dect_ie_keypad),
		.parse	= dect_sfmt_parse_multi_keypad,
	},
	[S_VL_IE_FEATURE_ACTIVATE]		= {
		.name	= "feature activate",
		.size	= sizeof(struct dect_ie_feature_activate),
	},
	[S_VL_IE_FEATURE_INDICATE]		= {
		.name	= "feature indicate",
		.size	= sizeof(struct dect_ie_feature_indicate),
	},
	[S_VL_IE_NETWORK_PARAMETER]		= {
		.name	= "network parameter",
		.size	= sizeof(struct dect_ie_network_parameter),
	},
	[S_VL_IE_EXT_HO_INDICATOR]		= {
		.name	= "ext H/O indicator",
		.size	= sizeof(struct dect_ie_ext_ho_indicator),
	},
	[S_VL_IE_ZAP_FIELD]			= {
		.name	= "ZAP field",
		.size	= sizeof(struct dect_ie_zap_field),
	},
	[S_VL_IE_SERVICE_CLASS]			= {
		.name	= "service class",
		.size	= sizeof(struct dect_ie_service_class),
	},
	[S_VL_IE_KEY]				= {
		.name	= "key",
		.size	= sizeof(struct dect_ie_key),
	},
	[S_VL_IE_REJECT_REASON]			= {
		.name	= "reject reason",
		.size	= sizeof(struct dect_ie_reject_reason),
		.parse	= dect_sfmt_parse_reject_reason,
		.build	= dect_sfmt_build_reject_reason,
	},
	[S_VL_IE_SETUP_CAPABILITY]		= {
		.name	= "setup capability",
		.size	= sizeof(struct dect_ie_setup_capability),
	},
	[S_VL_IE_TERMINAL_CAPABILITY]		= {
		.name	= "terminal capability",
		.size	= sizeof(struct dect_ie_terminal_capability),
		.parse	= dect_sfmt_parse_terminal_capability,
	},
	[S_VL_IE_END_TO_END_COMPATIBILITY]	= {
		.name	= "end-to-end compatibility",
		.size	= sizeof(struct dect_ie_end_to_end_compatibility),
	},
	[S_VL_IE_RATE_PARAMETERS]		= {
		.name	= "rate parameters",
		.size	= sizeof(struct dect_ie_rate_parameters),
	},
	[S_VL_IE_TRANSIT_DELAY]			= {
		.name	= "transit delay",
		.size	= sizeof(struct dect_ie_transit_delay),
	},
	[S_VL_IE_WINDOW_SIZE]			= {
		.name	= "window size",
		.size	= sizeof(struct dect_ie_window_size),
	},
	[S_VL_IE_CALLING_PARTY_NUMBER]		= {
		.name	= "calling party number",
		.size	= sizeof(struct dect_ie_calling_party_number),
	},
	[S_VL_IE_CALLING_PARTY_NAME]		= {
		.name	= "calling party name",
		.size	= sizeof(struct dect_ie_calling_party_name),
	},
	[S_VL_IE_CALLED_PARTY_NUMBER]		= {
		.name	= "called party number",
		.size	= sizeof(struct dect_ie_called_party_number),
	},
	[S_VL_IE_CALLED_PARTY_SUBADDR]		= {
		.name	= "called party subaddress",
		.size	= sizeof(struct dect_ie_called_party_subaddress),
	},
	[S_VL_IE_DURATION]			= {
		.name	= "duration",
		.size	= sizeof(struct dect_ie_duration),
		.parse	= dect_sfmt_parse_duration,
		.build	= dect_sfmt_build_duration,
	},
	[S_VL_IE_SEGMENTED_INFO]		= {
		.name	= "segmented info",
		.size	= sizeof(struct dect_ie_segmented_info),
	},
	[S_VL_IE_ALPHANUMERIC]			= {
		.name	= "alphanumeric",
		.size	= sizeof(struct dect_ie_alphanumeric),
	},
	[S_VL_IE_IWU_TO_IWU]			= {
		.name	= "IWU-to-IWU",
		.size	= sizeof(struct dect_ie_iwu_to_iwu),
	},
	[S_VL_IE_MODEL_IDENTIFIER]		= {
		.name	= "model identifier",
		.size	= sizeof(struct dect_ie_model_identifier),
	},
	[S_VL_IE_IWU_PACKET]			= {
		.name	= "IWU-packet",
		.size	= sizeof(struct dect_ie_iwu_packet),
	},
	[S_VL_IE_ESCAPE_TO_PROPRIETARY]		= {
		.name	= "escape to proprietary",
		.size	= sizeof(struct dect_ie_escape_to_proprietary),
		.parse	= dect_sfmt_parse_escape_to_proprietary,
	},
	[S_VL_IE_CODEC_LIST]			= {
		.name	= "codec list",
		.size	= sizeof(struct dect_ie_codec_list),
	},
	[S_VL_IE_EVENTS_NOTIFICATION]		= {
		.name	= "events notification",
		.size	= sizeof(struct dect_ie_events_notification),
	},
	[S_VL_IE_CALL_INFORMATION]		= {
		.name	= "call information",
		.size	= sizeof(struct dect_ie_call_information),
	},
	[S_VL_IE_ESCAPE_FOR_EXTENSION]		= {
		.name	= "escape for extension",
	},
};

static enum dect_sfmt_ie_status dect_rx_status(const struct dect_handle *dh,
					       const struct dect_sfmt_ie_desc *desc)
{
	if (dh->mode == DECT_MODE_FP)
		return desc->pp_fp;
	else
		return desc->fp_pp;
}

static enum dect_sfmt_ie_status dect_tx_status(const struct dect_handle *dh,
					       const struct dect_sfmt_ie_desc *desc)
{
	if (dh->mode == DECT_MODE_FP)
		return desc->fp_pp;
	else
		return desc->pp_fp;
}

static struct dect_ie_common **
dect_next_ie(const struct dect_sfmt_ie_desc *desc, struct dect_ie_common **ie)
{
	if (desc->type == S_SO_IE_REPEAT_INDICATOR)
		return ((void *)ie) + sizeof(struct dect_ie_repeat_indicator);
	else if (!(desc->flags & DECT_SFMT_IE_REPEAT))
		return ie + 1;
	else
		return ie;
}

static void dect_msg_ie_init(const struct dect_sfmt_ie_desc *desc,
			     struct dect_ie_common **ie)
{
	struct dect_ie_repeat_indicator *repeat_indicator;

	if (desc->flags & DECT_SFMT_IE_END)
		return;

	//dect_debug("init message IE %p: <%s>\n",
	//	 ie, dect_ie_handlers[desc->type].name);

	if (desc->type == S_SO_IE_REPEAT_INDICATOR) {
		repeat_indicator = dect_ie_container(repeat_indicator, (struct dect_ie_common *)ie);
		repeat_indicator->list = NULL;
	} else if (!(desc->flags & DECT_SFMT_IE_REPEAT))
		*ie = NULL;
}

static int dect_parse_sfmt_ie_header(struct dect_sfmt_ie *ie,
				     const struct dect_msg_buf *mb)
{
	uint8_t val;

	if (mb->len < 1)
		return -1;

	ie->id = mb->data[0] & DECT_SFMT_IE_FIXED_LEN;
	if (ie->id & DECT_SFMT_IE_FIXED_LEN) {
		ie->id |= (mb->data[0] & DECT_SFMT_IE_FIXED_ID_MASK);
		val     = (mb->data[0] & DECT_SFMT_IE_FIXED_VAL_MASK);
		if (ie->id != S_SO_IE_DOUBLE_OCTET_ELEMENT) {
			ie->len = 1;
			if (ie->id == S_SO_IE_EXT_PREFIX)
				ie->id |= val;
		} else {
			if (mb->len < 2)
				return -1;
			ie->id |= val;
			ie->len = 2;
		}
	} else {
		if (mb->len < 2U || mb->len < 2U + mb->data[1])
			return -1;
		ie->id  = mb->data[0];
		ie->len = mb->data[1] + 2;
	}
	ie->data = mb->data;

	dect_debug("found IE: <%s> (%x) len: %u\n", dect_ie_handlers[ie->id].name,
		   ie->id, ie->len);
	return 0;
}

static int dect_build_sfmt_ie_header(struct dect_sfmt_ie *dst, uint8_t id)
{
	if (id & DECT_SFMT_IE_FIXED_LEN) {
		dst->data[0] |= id;
		if ((id & DECT_SFMT_IE_FIXED_ID_MASK) !=
		    (S_SO_IE_DOUBLE_OCTET_ELEMENT & DECT_SFMT_IE_FIXED_ID_MASK))
			dst->len = 1;
		else
			dst->len = 2;
	} else {
		if (dst->len == 2)
			dst->len = 0;
		else {
			assert(dst->len > 2);
			dst->data[1] = dst->len - 2;
			dst->data[0] = id;
		}
	}
	return 0;
}

static int dect_parse_sfmt_ie(const struct dect_handle *dh,
			      const struct dect_sfmt_ie_desc *desc,
			      struct dect_ie_common **dst,
			      struct dect_sfmt_ie *ie)
{
	const struct dect_ie_handler *ieh;
	int err = -1;

	ieh = &dect_ie_handlers[ie->id];
	if (ieh->parse == NULL)
		goto err1;

	if (ieh->size > 0) {
		*dst = dect_ie_alloc(dh, ieh->size);
		if (*dst == NULL)
			goto err1;
	}

	dect_debug("parse IE: <%s> dst %p len %u\n", ieh->name, *dst, ie->len);
	err = ieh->parse(dh, dst, ie);
	if (err < 0)
		goto err2;
	return 0;

err2:
	dect_free(dh, *dst);
	*dst = NULL;
err1:
	dect_debug("smsg: IE parsing error\n");
	return err;
}

enum dect_sfmt_error dect_parse_sfmt_msg(const struct dect_handle *dh,
					 const struct dect_sfmt_msg_desc *mdesc,
					 struct dect_msg_common *_dst,
					 struct dect_msg_buf *mb)
{
	const struct dect_sfmt_ie_desc *desc = mdesc->ie;
	struct dect_ie_common **dst = &_dst->ie[0];
	struct dect_sfmt_ie _ie[2], *ie;
	uint8_t idx = 0;

	dect_msg_ie_init(desc, dst);
	while (mb->len > 0) {
		/* Parse the next information element header */
		ie = &_ie[idx++ % array_size(_ie)];;
		if (dect_parse_sfmt_ie_header(ie, mb) < 0)
			return -1;

		/* Treat empty variable length IEs as absent */
		if (!(ie->id & DECT_SFMT_IE_FIXED_LEN) && ie->len == 2)
			goto next;

		/* Locate a matching member in the description and apply
		 * policy checks. */
		while (1) {
			if (desc->flags & DECT_SFMT_IE_END)
				goto out;

			switch (dect_rx_status(dh, desc)) {
			case DECT_SFMT_IE_MANDATORY:
				if (desc->type == ie->id)
					goto found;
				return DECT_SFMT_MANDATORY_IE_MISSING;
			case DECT_SFMT_IE_NONE:
				if (desc->type == ie->id)
					return -1;
				break;
			case DECT_SFMT_IE_OPTIONAL:
				if (desc->type == ie->id)
					goto found;
				if (desc->type == S_DO_IE_SINGLE_DISPLAY &&
				    ie->id == S_VL_IE_MULTI_DISPLAY)
					goto found;
				if (desc->type == S_DO_IE_SINGLE_KEYPAD &&
				    ie->id == S_VL_IE_MULTI_KEYPAD)
					goto found;
				break;
			}

			dst = dect_next_ie(desc, dst);
			desc++;
			dect_msg_ie_init(desc, dst);
		}
found:
		/* Ignore corrupt optional IEs */
		if (dect_parse_sfmt_ie(dh, desc, dst, ie) < 0 &&
		    dect_rx_status(dh, desc) == DECT_SFMT_IE_MANDATORY)
			return DECT_SFMT_MANDATORY_IE_ERROR;

next:
		dect_mbuf_pull(mb, ie->len);

		dst = dect_next_ie(desc, dst);
		desc++;
		dect_msg_ie_init(desc, dst);
	}
out:
	while (!(desc->flags & DECT_SFMT_IE_END)) {
		dect_debug("clear missing IE: <%s>\n", dect_ie_handlers[desc->type].name);
		if (dect_rx_status(dh, desc) == DECT_SFMT_IE_MANDATORY)
			return DECT_SFMT_MANDATORY_IE_MISSING;
		dst = dect_next_ie(desc, dst);
		desc++;
		dect_msg_ie_init(desc, dst);
	}

	return DECT_SFMT_OK;
}

static enum dect_sfmt_error
dect_build_sfmt_ie(const struct dect_handle *dh,
		   const struct dect_sfmt_ie_desc *desc,
		   struct dect_msg_buf *mb,
		   struct dect_ie_common *ie)
{
	const struct dect_ie_handler *ieh;
	uint16_t type = desc->type;
	struct dect_sfmt_ie dst;
	enum dect_sfmt_error err = 0;

	if (dect_tx_status(dh, desc) == DECT_SFMT_IE_NONE)
		return DECT_SFMT_INVALID_IE;

	if (type == S_DO_IE_SINGLE_DISPLAY) {
		struct dect_ie_display *display = dect_ie_container(display, ie);
		if (display->len > 1)
			type = S_VL_IE_MULTI_DISPLAY;
	}
	if (type == S_DO_IE_SINGLE_KEYPAD) {
		struct dect_ie_keypad *keypad = dect_ie_container(keypad, ie);
		if (keypad->len > 1)
			type = S_VL_IE_MULTI_KEYPAD;
	}

	ieh = &dect_ie_handlers[type];
	if (ieh->build == NULL)
		goto err1;

	dect_debug("build IE: %s %p\n", ieh->name, ie);
	dst.data = mb->data + mb->len;
	dst.len = 0;
	err = ieh->build(&dst, ie);
	if (err < 0)
		goto err1;

	dect_build_sfmt_ie_header(&dst, type);
	mb->len += dst.len;
	return 0;

err1:
	return err;
}

enum dect_sfmt_error dect_build_sfmt_msg(const struct dect_handle *dh,
					 const struct dect_sfmt_msg_desc *mdesc,
					 const struct dect_msg_common *_src,
					 struct dect_msg_buf *mb)
{
	const struct dect_sfmt_ie_desc *desc = mdesc->ie;
	struct dect_ie_common * const *src = &_src->ie[0], **next, *rsrc;
	struct dect_ie_repeat_indicator *repeat_indicator;
	enum dect_sfmt_error err;

	while (!(desc->flags & DECT_SFMT_IE_END)) {
		next = dect_next_ie(desc, (struct dect_ie_common **)src);

		if (desc->type == S_SO_IE_REPEAT_INDICATOR) {
			repeat_indicator = (struct dect_ie_repeat_indicator *)src;
			if (repeat_indicator->list == NULL) {
				desc++;
				goto next;
			}

			/* Add repeat indicator if more than one element on the list */
			if (repeat_indicator->list->next != NULL)
				err = dect_build_sfmt_ie(dh, desc, mb, &repeat_indicator->common);
			desc++;

			assert(desc->flags & DECT_SFMT_IE_REPEAT);
			dect_foreach_ie(rsrc, repeat_indicator) {
				dect_debug("list elem %p\n", rsrc);
				err = dect_build_sfmt_ie(dh, desc, mb, rsrc);
			}
		} else {
			if (*src == NULL)
				goto next;
			err = dect_build_sfmt_ie(dh, desc, mb, *src);
		}
next:
		src = next;
		desc++;
	}

	return DECT_SFMT_OK;
}

void dect_msg_free(const struct dect_handle *dh,
		   const struct dect_sfmt_msg_desc *mdesc,
		   struct dect_msg_common *msg)
{
	const struct dect_sfmt_ie_desc *desc = mdesc->ie;
	struct dect_ie_common **ie = &msg->ie[0], **next;

	while (!(desc->flags & DECT_SFMT_IE_END)) {
		next = dect_next_ie(desc, ie);

		//dect_debug("free %s %p\n", dect_ie_handlers[desc->type].name, ie);
		if (desc->type == S_SO_IE_REPEAT_INDICATOR)
			desc++;
		else if (*ie != NULL && --(*ie)->refcnt == 0)
			dect_free(dh, *ie);

		ie = next;
		desc++;
	}
}
