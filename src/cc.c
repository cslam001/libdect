/*
 * DECT Call Control (CC)
 *
 * Copyright (c) 2009 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/dect.h>

#include <libdect.h>
#include <utils.h>
#include <s_fmt.h>
#include <lce.h>
#include <cc.h>

static const struct dect_sfmt_ie_desc cc_setup_msg_desc[] = {
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(S_VL_IE_FIXED_IDENTITY,		IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(S_VL_IE_NWK_ASSIGNED_IDENTITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_DO_IE_BASIC_SERVICE,		IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_ATTRIBUTES,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CALL_ATTRIBUTES,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CONNECTION_ATTRIBUTES,	IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_CIPHER_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CONNECTION_IDENTITY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_FACILITY,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_PROGRESS_INDICATOR,	IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_DO_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_DO_IE_SINGLE_KEYPAD,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_DO_IE_SIGNAL,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_FEATURE_ACTIVATE,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_FEATURE_INDICATE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_NETWORK_PARAMETER,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_EXT_HO_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_TERMINAL_CAPABILITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_END_TO_END_COMPATIBILITY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_RATE_PARAMETERS,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_TRANSIT_DELAY,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_WINDOW_SIZE,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CALLING_PARTY_NUMBER,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CALLED_PARTY_NUMBER,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CALLED_PARTY_SUBADDR,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SE_IE_SENDING_COMPLETE,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_PACKET,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CALLING_PARTY_NAME,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CODEC_LIST,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CALL_INFORMATION,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
};

static const struct dect_sfmt_ie_desc cc_info_msg_desc[] = {
	DECT_SFMT_IE(S_VL_IE_LOCATION_AREA,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_NWK_ASSIGNED_IDENTITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_FACILITY,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_PROGRESS_INDICATOR,	IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_DO_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_DO_IE_SINGLE_KEYPAD,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_DO_IE_SIGNAL,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_FEATURE_ACTIVATE,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_FEATURE_INDICATE,      	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_NETWORK_PARAMETER,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_EXT_HO_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CALLING_PARTY_NUMBER,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CALLED_PARTY_NUMBER,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CALLED_PARTY_SUBADDR,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SE_IE_SENDING_COMPLETE,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_DO_IE_TEST_HOOK_CONTROL,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_IWU_PACKET,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CALLING_PARTY_NAME,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CODEC_LIST,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CALL_INFORMATION,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
};

static const struct dect_sfmt_ie_desc cc_setup_ack_msg_desc[] = {
	DECT_SFMT_IE(S_VL_IE_INFO_TYPE,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_PORTABLE_IDENTITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_FIXED_IDENTITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_LOCATION_AREA,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_IWU_ATTRIBUTES,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CALL_ATTRIBUTES,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CONNECTION_ATTRIBUTES,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CONNECTION_IDENTITY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_FACILITY,			IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_PROGRESS_INDICATOR,	IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_DO_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_DO_IE_SIGNAL,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_FEATURE_INDICATE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_NETWORK_PARAMETER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_EXT_HO_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_TRANSIT_DELAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_WINDOW_SIZE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_SE_IE_DELIMITER_REQUEST,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_IWU_PACKET,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CODEC_LIST,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
};

static const struct dect_sfmt_ie_desc cc_call_proc_msg_desc[] = {
	DECT_SFMT_IE(S_VL_IE_IWU_ATTRIBUTES,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CALL_ATTRIBUTES,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CONNECTION_ATTRIBUTES,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CONNECTION_IDENTITY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_FACILITY,			IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_PROGRESS_INDICATOR,	IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_DO_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_DO_IE_SIGNAL,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_FEATURE_INDICATE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_TRANSIT_DELAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_WINDOW_SIZE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_IWU_PACKET,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_CODEC_LIST,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
};

static const struct dect_sfmt_ie_desc cc_alerting_msg_desc[] = {
	DECT_SFMT_IE(S_VL_IE_IWU_ATTRIBUTES,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CALL_ATTRIBUTES,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CONNECTION_ATTRIBUTES,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CONNECTION_IDENTITY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_FACILITY,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_PROGRESS_INDICATOR,	IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_DO_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_DO_IE_SIGNAL,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_FEATURE_INDICATE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_TERMINAL_CAPABILITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_TRANSIT_DELAY,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_WINDOW_SIZE,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_IWU_PACKET,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CODEC_LIST,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
};

static const struct dect_sfmt_ie_desc cc_connect_msg_desc[] = {
	DECT_SFMT_IE(S_VL_IE_IWU_ATTRIBUTES,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CALL_ATTRIBUTES,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CONNECTION_ATTRIBUTES,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CONNECTION_IDENTITY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_FACILITY,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_PROGRESS_INDICATOR,	IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_DO_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_DO_IE_SIGNAL,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_FEATURE_INDICATE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_NETWORK_PARAMETER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_EXT_HO_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_TERMINAL_CAPABILITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_TRANSIT_DELAY,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_WINDOW_SIZE,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_PACKET,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CODEC_LIST,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
};

static const struct dect_sfmt_ie_desc cc_connect_ack_msg_desc[] = {
	DECT_SFMT_IE(S_DO_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_FEATURE_INDICATE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_IWU_PACKET,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
};

static const struct dect_sfmt_ie_desc cc_release_msg_desc[] = {
	DECT_SFMT_IE(S_DO_IE_RELEASE_REASON,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_FACILITY,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_PROGRESS_INDICATOR,	IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_FEATURE_INDICATE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_PACKET,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
};

static const struct dect_sfmt_ie_desc cc_release_com_msg_desc[] = {
	DECT_SFMT_IE(S_DO_IE_RELEASE_REASON,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IDENTITY_TYPE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_LOCATION_AREA,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_IWU_ATTRIBUTES,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_CONNECTION_ATTRIBUTES,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_FACILITY,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_DO_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_FEATURE_INDICATE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_VL_IE_NETWORK_PARAMETER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(S_SO_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(S_VL_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_IWU_PACKET,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(S_VL_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
};

static const struct dect_sfmt_ie_desc cc_service_change_msg_desc[] = {
	DECT_SFMT_IE_END_MSG
};

static const struct dect_sfmt_ie_desc cc_service_accept_msg_desc[] = {
	DECT_SFMT_IE_END_MSG
};

static const struct dect_sfmt_ie_desc cc_service_reject_msg_desc[] = {
	DECT_SFMT_IE_END_MSG
};

static const struct dect_sfmt_ie_desc cc_notify_msg_desc[] = {
	DECT_SFMT_IE_END_MSG
};

static const struct dect_sfmt_ie_desc cc_iwu_info_msg_desc[] = {
	DECT_SFMT_IE_END_MSG
};

#define cc_debug(call, fmt, args...) \
	dect_debug("CC: call %p (%s): " fmt "\n", \
		   (call), call_states[(call)->state], ## args)

static const char *call_states[DECT_CC_STATE_MAX + 1] = {
	[DECT_CC_CALL_INITIATED]		= "CALL INITIATED",
	[DECT_CC_OVERLAP_SENDING]		= "OVERLAP SENDING",
	[DECT_CC_CALL_PROCEEDING]		= "CALL PROCEEDING",
	[DECT_CC_CALL_DELIVERED]		= "CALL DELIVERED",
	[DECT_CC_CALL_PRESENT]			= "CALL PRESENT",
	[DECT_CC_CALL_RECEIVED]			= "CALL RECEIVED",
	[DECT_CC_CONNECT_PENDING]		= "CONNECT PENDING",
	[DECT_CC_ACTIVE]			= "ACTIVE",
	[DECT_CC_RELEASE_PENDING]		= "RELEASE PENDING",
	[DECT_CC_OVERLAP_RECEIVING]		= "OVERLAP RECEIVING",
	[DECT_CC_INCOMING_CALL_PROCEEDING]	= "INCOMING CALL PROCEEDING",
};

void *dect_call_priv(struct dect_call *call)
{
	return call->priv;
}

const struct dect_ipui *dect_call_portable_identity(const struct dect_call *call)
{
	return &call->pt_id->ipui;
}

int dect_dl_u_data_req(const struct dect_handle *dh, struct dect_call *call,
		       struct dect_msg_buf *mb)
{
	ssize_t size;

	if (call->lu_sap == NULL) {
		cc_debug(call, "U-Plane U_DATA-req, but still unconnected");
		return 0;
	}
	//cc_debug(call, "U-Plane U_DATA-req");
	//dect_mbuf_dump(mb, "LU1");
	size = send(call->lu_sap->fd, mb->data, mb->len, 0);
	if (size != ((ssize_t)mb->len))
		cc_debug(call, "sending %u bytes failed: err=%d\n",
			 mb->len, errno);
	return 0;
}

static void dect_cc_lu_event(struct dect_handle *dh, struct dect_fd *fd,
			     uint32_t event)
{
	struct dect_call *call = fd->data;
	struct dect_msg_buf *mb;
	ssize_t len;

	//cc_debug(call, "U-Plane U_DATA-ind");
	mb = dect_mbuf_alloc(dh);
	if (mb == NULL)
		return;

	len = recv(call->lu_sap->fd, mb->data, sizeof(mb->head), 0);
	if (len < 0)
		return;
	mb->len = len;

	//dect_mbuf_dump(mb, "LU1");
	dh->ops->cc_ops->dl_u_data_ind(dh, call, mb);
}

static int dect_call_connect_uplane(const struct dect_handle *dh,
				    struct dect_call *call)
{
	struct sockaddr_dect_lu addr;

	call->lu_sap = dect_socket(dh, SOCK_STREAM, DECT_LU1_SAP);
	if (call->lu_sap == NULL)
		goto err1;

	dect_transaction_get_ulei(&addr, &call->transaction);
	if (connect(call->lu_sap->fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		goto err2;

	dect_setup_fd(call->lu_sap, dect_cc_lu_event, call);
	if (dect_register_fd(dh, call->lu_sap, DECT_FD_READ) < 0)
		goto err2;
	cc_debug(call, "U-Plane connected");
	return 0;

err2:
	dect_close(dh, call->lu_sap);
	call->lu_sap = NULL;
err1:
	cc_debug(call, "U-Plane connect failed: %s", strerror(errno));
	return -1;
}

static void dect_call_disconnect_uplane(const struct dect_handle *dh,
					struct dect_call *call)
{
	dect_unregister_fd(dh, call->lu_sap);
	dect_close(dh, call->lu_sap);
	call->lu_sap = NULL;
	cc_debug(call, "U-Plane disconnected");
}

struct dect_call *dect_call_alloc(const struct dect_handle *dh)
{
	struct dect_call *call;

	call = dect_zalloc(dh, sizeof(*call) + dh->ops->cc_ops->priv_size);
	if (call == NULL)
		goto err1;

	call->setup_timer = dect_alloc_timer(dh);
	if (call->setup_timer == NULL)
		goto err2;
	call->setup_timer->data = call;

	call->state = DECT_CC_NULL;
	return call;

err2:
	dect_free(dh, call);
err1:
	return NULL;
}

static void dect_call_destroy(const struct dect_handle *dh,
			      struct dect_call *call)
{
	if (call->state == DECT_CC_CALL_PRESENT)
		dect_stop_timer(dh, call->setup_timer);
	dect_free(dh, call->setup_timer);
	dect_free(dh, call);
}

static int dect_cc_send_msg(struct dect_handle *dh, struct dect_call *call,
			    const struct dect_sfmt_ie_desc *desc,
			    const struct dect_msg_common *msg,
			    enum dect_cc_msg_types type, const char *prefix)
{
	return dect_lce_send(dh, &call->transaction, desc, msg, type, prefix);
}

static void dect_cc_setup_timer(struct dect_handle *dh, struct dect_timer *timer)
{
	struct dect_call *call = timer->data;
	struct dect_ie_release_reason release_reason;
	struct dect_mncc_release_param param = {
		.release_reason		= &release_reason,
	};

	cc_debug(call, "setup timer");
	// release-com

	dect_ie_init(&release_reason);
	release_reason.reason = DECT_RELEASE_TIMER_EXPIRY;
	dh->ops->cc_ops->mncc_reject_ind(dh, call, &param);

	dect_close_transaction(dh, &call->transaction);
	dect_call_destroy(dh, call);
}

int dect_mncc_setup_req(struct dect_handle *dh, struct dect_call *call,
			const struct dect_ipui *ipui,
			const struct dect_mncc_setup_param *param)
{
	struct dect_ie_portable_identity portable_identity;
	struct dect_ie_fixed_identity fixed_identity;
	struct dect_cc_setup_msg msg = {
		.portable_identity		= &portable_identity,
		.fixed_identity			= &fixed_identity,
		.basic_service			= param->basic_service,
		.iwu_attributes			= param->iwu_attributes,
		.cipher_info			= param->cipher_info,
		.facility			= param->facility,
		.progress_indicator		= param->progress_indicator,
		.display			= param->display,
		.keypad				= param->keypad,
		.signal				= param->signal,
		.feature_activate		= param->feature_activate,
		.feature_indicate		= param->feature_indicate,
		.network_parameter		= param->network_parameter,
		.terminal_capability		= param->terminal_capability,
		.end_to_end_compatibility	= param->end_to_end_compatibility,
		.rate_parameters		= param->rate_parameters,
		.transit_delay			= param->transit_delay,
		.window_size			= param->window_size,
		.calling_party_number		= param->calling_party_number,
		.called_party_number		= param->called_party_number,
		.called_party_subaddress	= param->called_party_subaddress,
		.calling_party_name		= param->calling_party_name,
		.sending_complete		= param->sending_complete,
		.iwu_to_iwu			= param->iwu_to_iwu,
		.iwu_packet			= param->iwu_packet,
	};

	cc_debug(call, "setup request");

	call->transaction.pd = DECT_S_PD_CC;
	if (dect_open_transaction(dh, &call->transaction, ipui) < 0)
		goto err1;

	fixed_identity.type = ID_TYPE_PARK;
	memcpy(&fixed_identity.ari, &dh->pari, sizeof(fixed_identity.ari));
	portable_identity.type = ID_TYPE_IPUI;
	portable_identity.ipui = *ipui;

	if (dect_cc_send_msg(dh, call, cc_setup_msg_desc, &msg.common,
			     CC_SETUP, "CC-SETUP") < 0)
		goto err2;
	call->state = DECT_CC_CALL_PRESENT;

	call->setup_timer->callback = dect_cc_setup_timer;
	dect_start_timer(dh, call->setup_timer, DECT_CC_SETUP_TIMEOUT);
	return 0;

err2:
	dect_close_transaction(dh, &call->transaction);
err1:
	return -1;
}

int dect_mncc_setup_ack_req(struct dect_handle *dh, struct dect_call *call,
			    const struct dect_mncc_setup_ack_param *param)
{
	struct dect_cc_setup_ack_msg msg = {
		.portable_identity	= call->pt_id,
		.fixed_identity		= call->ft_id,
		.info_type		= param->info_type,
		.location_area		= param->location_area,
		.display		= param->display,
		.signal			= param->signal,
		.feature_indicate	= param->feature_indicate,
		.transit_delay		= param->transit_delay,
		.window_size		= param->window_size,
		.delimiter_request	= param->delimiter_request,
		.iwu_to_iwu		= param->iwu_to_iwu,
		.iwu_packet		= param->iwu_packet,
	};

	dect_cc_send_msg(dh, call, cc_setup_ack_msg_desc, &msg.common,
			 CC_SETUP_ACK,  "CC-SETUP_ACK");
	return 0;
}

int dect_mncc_reject_req(struct dect_handle *dh, struct dect_call *call,
			 const struct dect_mncc_release_param *param)
{
	struct dect_cc_release_com_msg msg = {
		.release_reason			= param->release_reason,
		.identity_type			= param->identity_type,
		.location_area			= param->location_area,
		.iwu_attributes			= param->iwu_attributes,
		//.facility			= param->facility,
		.display			= param->display,
		.feature_indicate		= param->feature_indicate,
		.network_parameter		= param->network_parameter,
		.iwu_to_iwu			= param->iwu_to_iwu,
		.iwu_packet			= param->iwu_packet,
	};

	dect_cc_send_msg(dh, call, cc_release_com_msg_desc, &msg.common,
			 CC_RELEASE_COM, "CC-RELEASE_COM");

	dect_close_transaction(dh, &call->transaction);
	dect_call_destroy(dh, call);
	return 0;
}

int dect_mncc_call_proc_req(struct dect_handle *dh, struct dect_call *call,
			    const struct dect_mncc_call_proc_param *param)
{
	struct dect_cc_call_proc_msg msg = {
		.facility		= param->facility,
		.progress_indicator	= param->progress_indicator,
		.display		= param->display,
		.signal			= param->signal,
		.feature_indicate	= param->feature_indicate,
		.transit_delay		= param->transit_delay,
		.window_size		= param->window_size,
		.iwu_to_iwu		= param->iwu_to_iwu,
		.iwu_packet		= param->iwu_packet,
	};

	dect_cc_send_msg(dh, call, cc_call_proc_msg_desc, &msg.common,
			 CC_CALL_PROC, "CC-CALL_PROC");
	return 0;
}
int dect_mncc_alert_req(struct dect_handle *dh, struct dect_call *call,
			const struct dect_mncc_alert_param *param)
{
	struct dect_cc_alerting_msg msg = {
		.facility		= param->facility,
		//.progress_indicator	= param->progress_indicator,
		.display		= param->display,
		.signal			= param->signal,
		.feature_indicate	= param->feature_indicate,
		.terminal_capability	= param->terminal_capability,
		.transit_delay		= param->transit_delay,
		.window_size		= param->window_size,
		.iwu_to_iwu		= param->iwu_to_iwu,
		.iwu_packet		= param->iwu_packet,
	};

	// FIXME FIXME FIXME FIXME
	if (param->progress_indicator.list.next != NULL) {
		init_list_head(&msg.progress_indicator.list);
		dect_ie_list_move(&msg.progress_indicator,
				  (struct dect_ie_repeat_indicator *)&param->progress_indicator);
	}

	dect_cc_send_msg(dh, call, cc_alerting_msg_desc, &msg.common,
			 CC_ALERTING, "CC-ALERTING");
	return 0;
}

int dect_mncc_connect_req(struct dect_handle *dh, struct dect_call *call,
			  const struct dect_mncc_connect_param *param)
{
	struct dect_cc_connect_msg msg = {
		.facility		= param->facility,
		.progress_indicator	= param->progress_indicator,
		.display		= param->display,
		.signal			= param->signal,
		.feature_indicate	= param->feature_indicate,
		.terminal_capability	= param->terminal_capability,
		.transit_delay		= param->transit_delay,
		.window_size		= param->window_size,
		.iwu_to_iwu		= param->iwu_to_iwu,
		.iwu_packet		= param->iwu_packet,
	};

	dect_cc_send_msg(dh, call, cc_connect_msg_desc, &msg.common,
			 CC_CONNECT,  "CC-CONNECT");

	dect_call_connect_uplane(dh, call);
	return 0;
}

int dect_mncc_connect_res(struct dect_handle *dh, struct dect_call *call,
			  const struct dect_mncc_connect_param *param)
{
	struct dect_cc_connect_ack_msg msg = {
		.display		= param->display,
		.feature_indicate	= param->feature_indicate,
		//.iwu_to_iwu		= param->iwu_to_iwu,
		.iwu_packet		= param->iwu_packet,
	};

	dect_call_connect_uplane(dh, call);
	if (dect_cc_send_msg(dh, call, cc_connect_ack_msg_desc, &msg.common,
			     CC_CONNECT_ACK, "CC-CONNECT_ACK") < 0)
		goto err1;

	call->state = DECT_CC_ACTIVE;
	return 0;

err1:
	dect_call_disconnect_uplane(dh, call);
	return -1;
}

int dect_mncc_release_req(struct dect_handle *dh, struct dect_call *call,
			  const struct dect_mncc_release_param *param)
{
	struct dect_cc_release_msg msg = {
		.release_reason			= param->release_reason,
		.facility			= param->facility,
		.display			= param->display,
		.feature_indicate		= param->feature_indicate,
		.iwu_to_iwu			= param->iwu_to_iwu,
		.iwu_packet			= param->iwu_packet,
	};

	dect_cc_send_msg(dh, call, cc_release_msg_desc, &msg.common,
			 CC_RELEASE, "CC-RELEASE");
	call->state = DECT_CC_RELEASE_PENDING;
	return 0;
}

int dect_mncc_release_res(struct dect_handle *dh, struct dect_call *call,
			  const struct dect_mncc_release_param *param)
{
	struct dect_cc_release_com_msg msg = {
		.release_reason			= param->release_reason,
		.identity_type			= param->identity_type,
		.location_area			= param->location_area,
		.iwu_attributes			= param->iwu_attributes,
		//.facility			= param->facility,
		.display			= param->display,
		.feature_indicate		= param->feature_indicate,
		.network_parameter		= param->network_parameter,
		.iwu_to_iwu			= param->iwu_to_iwu,
		.iwu_packet			= param->iwu_packet,
	};

	dect_cc_send_msg(dh, call, cc_release_com_msg_desc, &msg.common,
			 CC_RELEASE_COM,  "CC-RELEASE_COM");

	dect_call_disconnect_uplane(dh, call);
	dect_close_transaction(dh, &call->transaction);
	dect_call_destroy(dh, call);
	return 0;
}

int dect_mncc_facility_req(struct dect_handle *dh, struct dect_call *call,
			  const struct dect_mncc_facility_param *param)
{
	return 0;
}

int dect_mncc_info_req(struct dect_handle *dh, struct dect_call *call,
		       const struct dect_mncc_info_param *param)
{
	struct dect_cc_info_msg msg = {
		.location_area			= param->location_area,
		.nwk_assigned_identity		= param->nwk_assigned_identity,
		.facility			= param->facility,
//		.progress_indicator		= param->progress_indicator,
		.display			= param->display,
		.keypad				= param->keypad,
		.signal				= param->signal,
		.feature_activate		= param->feature_activate,
		.feature_indicate		= param->feature_indicate,
		.network_parameter		= param->network_parameter,
		.called_party_number		= param->called_party_number,
		.called_party_subaddress	= param->called_party_subaddress,
		.calling_party_number		= param->calling_party_number,
		.calling_party_name		= param->calling_party_name,
		.sending_complete		= param->sending_complete,
		.iwu_to_iwu			= param->iwu_to_iwu,
		.iwu_packet			= param->iwu_packet,
	};

	// FIXME FIXME FIXME FIXME
	if (param->progress_indicator.list.next != NULL) {
		init_list_head(&msg.progress_indicator.list);
		dect_ie_list_move(&msg.progress_indicator,
				  (struct dect_ie_repeat_indicator *)&param->progress_indicator);
	}

	dect_cc_send_msg(dh, call, cc_info_msg_desc, &msg.common,
			 CC_INFO,  "CC-INFO");
	return 0;
}

int dect_mncc_modify_req(struct dect_handle *dh, struct dect_call *call,
			 const struct dect_mncc_modify_param *param)
{
	return 0;
}

int dect_mncc_modify_res(struct dect_handle *dh, struct dect_call *call,
			 const struct dect_mncc_modify_param *param)
{
	return 0;
}

int dect_mncc_hold_req(struct dect_handle *dh, struct dect_call *call,
		       const struct dect_mncc_hold_param *param)
{
	return 0;
}

int dect_mncc_hold_res(struct dect_handle *dh, struct dect_call *call,
		       const struct dect_mncc_hold_param *param)
{
	return 0;
}

int dect_mncc_retrieve_req(struct dect_handle *dh, struct dect_call *call,
			   const struct dect_mncc_hold_param *param)
{
	return 0;
}

int dect_mncc_retrieve_res(struct dect_handle *dh, struct dect_call *call,
			   const struct dect_mncc_hold_param *param)
{
	return 0;
}

int dect_mncc_iwu_info_req(struct dect_handle *dh, struct dect_call *call,
			   const struct dect_mncc_iwu_info_param *param)
{
	return 0;
}

static void dect_mncc_alert_ind(struct dect_handle *dh, struct dect_call *call,
				const struct dect_cc_alerting_msg *msg)
{
	struct dect_mncc_alert_param param = {
		.facility			= msg->facility,
		.progress_indicator		= msg->progress_indicator,
		.display			= msg->display,
		.signal				= msg->signal,
		.feature_indicate		= msg->feature_indicate,
		.terminal_capability		= msg->terminal_capability,
		.transit_delay			= msg->transit_delay,
		.window_size			= msg->window_size,
		.iwu_to_iwu			= msg->iwu_to_iwu,
		.iwu_packet			= msg->iwu_packet,
	};

	dh->ops->cc_ops->mncc_alert_ind(dh, call, &param);
}

static void dect_cc_rcv_alerting(struct dect_handle *dh, struct dect_call *call,
				 struct dect_msg_buf *mb)
{
	struct dect_cc_alerting_msg msg;

	dect_mbuf_dump(mb, "CC-ALERTING");
	if (call->state != DECT_CC_CALL_PRESENT)
		;

	if (dect_parse_sfmt_msg(dh, cc_alerting_msg_desc, &msg.common, mb) < 0)
		return;

	dect_mncc_alert_ind(dh, call, &msg);
	dect_msg_free(dh, cc_alerting_msg_desc, &msg.common);
	call->state = DECT_CC_CALL_RECEIVED;
}

static void dect_cc_rcv_call_proc(struct dect_handle *dh, struct dect_call *call,
				  struct dect_msg_buf *mb)
{
	struct dect_cc_call_proc_msg msg;

	dect_mbuf_dump(mb, "CC-CALL_PROC");
	if (dect_parse_sfmt_msg(dh, cc_call_proc_msg_desc, &msg.common, mb) < 0)
		return;
}

static void dect_mncc_connect_ind(struct dect_handle *dh, struct dect_call *call,
				  struct dect_cc_connect_msg *msg)
{
	struct dect_mncc_connect_param param = {
		.facility		= msg->facility,
		.progress_indicator	= msg->progress_indicator,
		.display		= msg->display,
		.signal			= msg->signal,
		.feature_indicate	= msg->feature_indicate,
		.terminal_capability	= msg->terminal_capability,
		.transit_delay		= msg->transit_delay,
		.window_size		= msg->window_size,
		.iwu_to_iwu		= msg->iwu_to_iwu,
		.iwu_packet		= msg->iwu_packet,
	};

	dh->ops->cc_ops->mncc_connect_ind(dh, call, &param);
}

static void dect_cc_rcv_connect(struct dect_handle *dh, struct dect_call *call,
				struct dect_msg_buf *mb)
{
	struct dect_cc_connect_msg msg;

	if (call->state != DECT_CC_CALL_PRESENT &&
	    call->state != DECT_CC_CALL_RECEIVED)
		;

	dect_mbuf_dump(mb, "CC-CONNECT");
	if (dect_parse_sfmt_msg(dh, cc_connect_msg_desc, &msg.common, mb) < 0)
		return;

	if (call->setup_timer != NULL) {
		dect_stop_timer(dh, call->setup_timer);
		dect_free(dh, call->setup_timer);
		call->setup_timer = NULL;
	}

	dect_mncc_connect_ind(dh, call, &msg);
	dect_msg_free(dh, cc_connect_msg_desc, &msg.common);
}

static void dect_cc_rcv_setup_ack(struct dect_handle *dh, struct dect_call *call,
				  struct dect_msg_buf *mb)
{
	struct dect_cc_setup_ack_msg msg;

	dect_mbuf_dump(mb, "CC-SETUP_ACK");
	if (dect_parse_sfmt_msg(dh, cc_setup_ack_msg_desc, &msg.common, mb) < 0)
		return;

	dect_msg_free(dh, cc_setup_ack_msg_desc, &msg.common);
}

static void dect_cc_rcv_connect_ack(struct dect_handle *dh, struct dect_call *call,
				    struct dect_msg_buf *mb)
{
	struct dect_cc_connect_ack_msg msg;

	dect_mbuf_dump(mb, "CC-CONNECT_ACK");
	if (dect_parse_sfmt_msg(dh, cc_connect_ack_msg_desc, &msg.common, mb) < 0)
		return;

	dect_msg_free(dh, cc_connect_ack_msg_desc, &msg.common);
}

static void dect_mncc_release_ind(struct dect_handle *dh, struct dect_call *call,
				  const struct dect_cc_release_msg *msg)
{
	struct dect_mncc_release_param param = {
		.release_reason			= msg->release_reason,
		.facility			= msg->facility,
		.display			= msg->display,
		.feature_indicate		= msg->feature_indicate,
		.iwu_to_iwu			= msg->iwu_to_iwu,
		.iwu_packet			= msg->iwu_packet,
	};

	dh->ops->cc_ops->mncc_release_ind(dh, call, &param);
}

static void dect_cc_rcv_release(struct dect_handle *dh, struct dect_call *call,
				struct dect_msg_buf *mb)
{
	struct dect_cc_release_msg msg;

	dect_mbuf_dump(mb, "CC-RELEASE");
	if (dect_parse_sfmt_msg(dh, cc_release_msg_desc, &msg.common, mb) < 0)
		return;

	dect_mncc_release_ind(dh, call, &msg);
	dect_msg_free(dh, cc_release_msg_desc, &msg.common);
}

static void dect_mncc_release_cfm(struct dect_handle *dh, struct dect_call *call,
				  const struct dect_cc_release_com_msg *msg)
{
	struct dect_mncc_release_param param = {
		.release_reason			= msg->release_reason,
		.identity_type			= msg->identity_type,
		.location_area			= msg->location_area,
		.iwu_attributes			= msg->iwu_attributes,
		.facility			= msg->facility,
		.display			= msg->display,
		.feature_indicate		= msg->feature_indicate,
		.network_parameter		= msg->network_parameter,
		.iwu_to_iwu			= msg->iwu_to_iwu,
		.iwu_packet			= msg->iwu_packet,

	};

	dh->ops->cc_ops->mncc_release_cfm(dh, call, &param);
}

static void dect_cc_rcv_release_com(struct dect_handle *dh, struct dect_call *call,
				    struct dect_msg_buf *mb)
{
	struct dect_cc_release_com_msg msg;

	dect_mbuf_dump(mb, "CC-RELEASE_COM");
	if (dect_parse_sfmt_msg(dh, cc_release_com_msg_desc, &msg.common, mb) < 0)
		return;

	if (call->state == DECT_CC_RELEASE_PENDING)
		dect_mncc_release_cfm(dh, call, &msg);
	else {
		struct dect_mncc_release_param param = {
			.release_reason		= msg.release_reason,
			.facility		= msg.facility,
			.iwu_to_iwu		= msg.iwu_to_iwu,
			.iwu_packet		= msg.iwu_packet,
		};
		dh->ops->cc_ops->mncc_release_ind(dh, call, &param);
	}

	dect_msg_free(dh, cc_release_com_msg_desc, &msg.common);

	if (call->lu_sap != NULL)
		dect_call_disconnect_uplane(dh, call);
	dect_close_transaction(dh, &call->transaction);
	dect_call_destroy(dh, call);
}

static void dect_cc_rcv_iwu_info(struct dect_handle *dh, struct dect_call *call,
				 struct dect_msg_buf *mb)
{
	struct dect_cc_iwu_info_msg msg;

	dect_mbuf_dump(mb, "CC-IWU_INFO");
	if (dect_parse_sfmt_msg(dh, cc_iwu_info_msg_desc, &msg.common, mb) < 0)
		return;

	dect_msg_free(dh, cc_iwu_info_msg_desc, &msg.common);
}

static void dect_cc_rcv_notify(struct dect_handle *dh, struct dect_call *call,
			       struct dect_msg_buf *mb)
{
	struct dect_cc_notify_msg msg;

	dect_mbuf_dump(mb, "CC-NOTIFY");
	if (dect_parse_sfmt_msg(dh, cc_notify_msg_desc, &msg.common, mb) < 0)
		return;

	dect_msg_free(dh, cc_notify_msg_desc, &msg.common);
}

static void dect_mncc_info_ind(struct dect_handle *dh, struct dect_call *call,
			       struct dect_cc_info_msg *msg)
{
	struct dect_mncc_info_param param = {
		.location_area			= msg->location_area,
		.nwk_assigned_identity		= msg->nwk_assigned_identity,
		.facility			= msg->facility,
		.progress_indicator		= msg->progress_indicator,
		.display			= msg->display,
		.keypad				= msg->keypad,
		.signal				= msg->signal,
		.feature_activate		= msg->feature_activate,
		.feature_indicate		= msg->feature_indicate,
		.network_parameter		= msg->network_parameter,
		.called_party_number		= msg->called_party_number,
		.called_party_subaddress	= msg->called_party_subaddress,
		.calling_party_number		= msg->calling_party_number,
		.calling_party_name		= msg->calling_party_name,
		.sending_complete		= msg->sending_complete,
		.iwu_to_iwu			= msg->iwu_to_iwu,
		.iwu_packet			= msg->iwu_packet,
	};

	dh->ops->cc_ops->mncc_info_ind(dh, call, &param);
}

static void dect_cc_rcv_info(struct dect_handle *dh, struct dect_call *call,
			     struct dect_msg_buf *mb)
{
	struct dect_cc_info_msg msg;

	dect_mbuf_dump(mb, "CC-INFO");
	if (call->state == DECT_CC_CALL_INITIATED ||
	    call->state == DECT_CC_CALL_PRESENT)
		;

	if (dect_parse_sfmt_msg(dh, cc_info_msg_desc, &msg.common, mb) < 0)
		return;

	dect_mncc_info_ind(dh, call, &msg);
	dect_msg_free(dh, cc_info_msg_desc, &msg.common);
}

static void dect_cc_rcv(struct dect_handle *dh, struct dect_transaction *ta,
			struct dect_msg_buf *mb)
{
	struct dect_call *call = container_of(ta, struct dect_call, transaction);

	cc_debug(call, "receive msg type %x", mb->type);
	switch (mb->type) {
	case CC_ALERTING:
		return dect_cc_rcv_alerting(dh, call, mb);
	case CC_CALL_PROC:
		return dect_cc_rcv_call_proc(dh, call, mb);
	case CC_CONNECT:
		return dect_cc_rcv_connect(dh, call, mb);
	case CC_SETUP_ACK:
		return dect_cc_rcv_setup_ack(dh, call, mb);
	case CC_CONNECT_ACK:
		return dect_cc_rcv_connect_ack(dh, call, mb);
	case CC_SERVICE_CHANGE:
	case CC_SERVICE_ACCEPT:
	case CC_SERVICE_REJECT:
	case CC_RELEASE:
		return dect_cc_rcv_release(dh, call, mb);
	case CC_RELEASE_COM:
		return dect_cc_rcv_release_com(dh, call, mb);
	case CC_IWU_INFO:
		return dect_cc_rcv_iwu_info(dh, call, mb);
	case CC_NOTIFY:
		return dect_cc_rcv_notify(dh, call, mb);
	case CC_INFO:
		return dect_cc_rcv_info(dh, call, mb);
	}
}

static void dect_mncc_setup_ind(struct dect_handle *dh,
				struct dect_call *call,
				struct dect_cc_setup_msg *msg)
{
	struct dect_mncc_setup_param param = {
		.basic_service			= msg->basic_service,
		.cipher_info			= msg->cipher_info,
		.display			= msg->display,
		.keypad				= msg->keypad,
		.signal				= msg->signal,
		.feature_activate       	= msg->feature_activate,
		.feature_indicate       	= msg->feature_indicate,
		.network_parameter		= msg->network_parameter,
		.terminal_capability		= msg->terminal_capability,
		.end_to_end_compatibility	= msg->end_to_end_compatibility,
		.rate_parameters		= msg->rate_parameters,
		.transit_delay			= msg->transit_delay,
		.window_size			= msg->window_size,
		.called_party_number		= msg->called_party_number,
		.called_party_subaddress	= msg->called_party_subaddress,
		.calling_party_number		= msg->calling_party_number,
		.calling_party_name		= msg->calling_party_name,
		.sending_complete		= msg->sending_complete,
		.iwu_to_iwu			= msg->iwu_to_iwu,
		.iwu_packet			= msg->iwu_packet,
	};

	dect_ie_list_move(&param.iwu_attributes, &msg->iwu_attributes);
	dect_ie_list_move(&param.facility, &msg->facility);
	dect_ie_list_move(&param.progress_indicator, &msg->progress_indicator);

	dh->ops->cc_ops->mncc_setup_ind(dh, call, &param);
}

static void dect_cc_rcv_setup(struct dect_handle *dh,
			      const struct dect_transaction *req,
			      struct dect_msg_buf *mb)
{
	struct dect_ie_connection_attributes *connection_attributes;
	struct dect_ie_call_attributes *call_attributes;
	struct dect_cc_setup_msg msg;
	struct dect_call *call;

	dect_mbuf_dump(mb, "CC-SETUP");
	if (dect_parse_sfmt_msg(dh, cc_setup_msg_desc, &msg.common, mb) < 0)
		return;

	dect_foreach_ie(call_attributes, msg.call_attributes)
		dect_debug("call attributes\n");

	dect_foreach_ie(connection_attributes, msg.connection_attributes)
		dect_debug("connection attributes\n");

	call = dect_call_alloc(dh);
	if (call == NULL)
		goto out;
	call->ft_id = dect_ie_hold(msg.fixed_identity);
	call->pt_id = dect_ie_hold(msg.portable_identity);
	call->state = DECT_CC_CALL_INITIATED;
	dect_confirm_transaction(dh, &call->transaction, req);
	cc_debug(call, "new call");

	dect_mncc_setup_ind(dh, call, &msg);
out:
	dect_msg_free(dh, cc_setup_msg_desc, &msg.common);
}

static void dect_cc_open(struct dect_handle *dh,
			 const struct dect_transaction *req,
			 struct dect_msg_buf *mb)
{
	dect_debug("CC: unknown transaction msg type: %x\n", mb->type);

	switch (mb->type) {
	case CC_SETUP:
		return dect_cc_rcv_setup(dh, req, mb);
	case CC_RELEASE:
	case CC_RELEASE_COM:
		break;
	default:
		// send release-com
		break;
	}
}

static void dect_cc_shutdown(struct dect_handle *dh,
			     struct dect_transaction *ta)
{
	struct dect_call *call = container_of(ta, struct dect_call, transaction);

	cc_debug(call, "shutdown");
	dh->ops->cc_ops->mncc_reject_ind(dh, call, NULL);
	dect_close_transaction(dh, &call->transaction);
	dect_call_destroy(dh, call);
}

static const struct dect_nwk_protocol cc_protocol = {
	.name			= "Call Control",
	.pd			= DECT_S_PD_CC,
	.max_transactions	= 7,
	.open			= dect_cc_open,
	.shutdown		= dect_cc_shutdown,
	.rcv			= dect_cc_rcv,
};

static void __init dect_cc_init(void)
{
	dect_lce_register_protocol(&cc_protocol);
}
