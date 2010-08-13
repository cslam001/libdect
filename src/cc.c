/*
 * DECT Call Control (CC)
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/**
 * @defgroup cc Call Control
 *
 * This module implements the call control processes specified in
 * ETSI EN 300 175-5, section 9.
 * @{
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
#include <timer.h>
#include <io.h>
#include <s_fmt.h>
#include <lce.h>
#include <cc.h>
#include <ss.h>

static DECT_SFMT_MSG_DESC(cc_setup,
	DECT_SFMT_IE(DECT_IE_PORTABLE_IDENTITY,		IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(DECT_IE_FIXED_IDENTITY,		IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(DECT_IE_NWK_ASSIGNED_IDENTITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_BASIC_SERVICE,		IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_ATTRIBUTES,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CALL_ATTRIBUTES,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CONNECTION_ATTRIBUTES,	IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_CIPHER_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CONNECTION_IDENTITY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_FACILITY,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_PROGRESS_INDICATOR,	IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_SINGLE_KEYPAD,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SIGNAL,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_FEATURE_ACTIVATE,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_FEATURE_INDICATE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_NETWORK_PARAMETER,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_EXT_HO_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_TERMINAL_CAPABILITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_END_TO_END_COMPATIBILITY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_RATE_PARAMETERS,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_TRANSIT_DELAY,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_WINDOW_SIZE,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CALLING_PARTY_NUMBER,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CALLED_PARTY_NUMBER,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CALLED_PARTY_SUBADDR,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SENDING_COMPLETE,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_PACKET,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CALLING_PARTY_NAME,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CODEC_LIST,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CALL_INFORMATION,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(cc_info,
	DECT_SFMT_IE(DECT_IE_LOCATION_AREA,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_NWK_ASSIGNED_IDENTITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_FACILITY,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_PROGRESS_INDICATOR,	IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_SINGLE_KEYPAD,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SIGNAL,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_FEATURE_ACTIVATE,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_FEATURE_INDICATE,      	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_NETWORK_PARAMETER,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_EXT_HO_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_CALLING_PARTY_NUMBER,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CALLED_PARTY_NUMBER,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CALLED_PARTY_SUBADDR,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SENDING_COMPLETE,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_TEST_HOOK_CONTROL,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_IWU_PACKET,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CALLING_PARTY_NAME,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CODEC_LIST,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CALL_INFORMATION,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(cc_setup_ack,
	DECT_SFMT_IE(DECT_IE_INFO_TYPE,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_PORTABLE_IDENTITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_FIXED_IDENTITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_LOCATION_AREA,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_IWU_ATTRIBUTES,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_CALL_ATTRIBUTES,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_CONNECTION_ATTRIBUTES,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_CONNECTION_IDENTITY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_FACILITY,			IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_PROGRESS_INDICATOR,	IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_SIGNAL,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_FEATURE_INDICATE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_NETWORK_PARAMETER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_EXT_HO_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_TRANSIT_DELAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_WINDOW_SIZE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_DELIMITER_REQUEST,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_IWU_PACKET,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_CODEC_LIST,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(cc_call_proc,
	DECT_SFMT_IE(DECT_IE_IWU_ATTRIBUTES,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_CALL_ATTRIBUTES,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_CONNECTION_ATTRIBUTES,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_CONNECTION_IDENTITY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_FACILITY,			IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_PROGRESS_INDICATOR,	IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_SIGNAL,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_FEATURE_INDICATE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_TRANSIT_DELAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_WINDOW_SIZE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_IWU_PACKET,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_CODEC_LIST,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(cc_alerting,
	DECT_SFMT_IE(DECT_IE_IWU_ATTRIBUTES,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CALL_ATTRIBUTES,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CONNECTION_ATTRIBUTES,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CONNECTION_IDENTITY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_FACILITY,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_PROGRESS_INDICATOR,	IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_SIGNAL,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_FEATURE_INDICATE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_TERMINAL_CAPABILITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_TRANSIT_DELAY,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_WINDOW_SIZE,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_IWU_PACKET,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CODEC_LIST,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(cc_connect,
	DECT_SFMT_IE(DECT_IE_IWU_ATTRIBUTES,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CALL_ATTRIBUTES,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CONNECTION_ATTRIBUTES,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CONNECTION_IDENTITY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_FACILITY,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_PROGRESS_INDICATOR,	IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_SIGNAL,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_FEATURE_INDICATE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_NETWORK_PARAMETER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_EXT_HO_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_TERMINAL_CAPABILITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_TRANSIT_DELAY,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_WINDOW_SIZE,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_PACKET,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CODEC_LIST,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(cc_connect_ack,
	DECT_SFMT_IE(DECT_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_FEATURE_INDICATE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_IWU_PACKET,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(cc_release,
	DECT_SFMT_IE(DECT_IE_RELEASE_REASON,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_FACILITY,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_PROGRESS_INDICATOR,	IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_FEATURE_INDICATE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_PACKET,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(cc_release_com,
	DECT_SFMT_IE(DECT_IE_RELEASE_REASON,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IDENTITY_TYPE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_LOCATION_AREA,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_IWU_ATTRIBUTES,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CONNECTION_ATTRIBUTES,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_FACILITY,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_FEATURE_INDICATE,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_NETWORK_PARAMETER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_PACKET,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(cc_service_change,
	DECT_SFMT_IE(DECT_IE_PORTABLE_IDENTITY,		IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(DECT_IE_IWU_ATTRIBUTES,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SERVICE_CHANGE_INFO,	IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(DECT_IE_CALL_ATTRIBUTES,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CONNECTION_ATTRIBUTES,	IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_CONNECTION_IDENTITY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CODEC_LIST,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(cc_service_accept,
	DECT_SFMT_IE(DECT_IE_IWU_ATTRIBUTES,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CONNECTION_IDENTITY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(cc_service_reject,
	DECT_SFMT_IE(DECT_IE_RELEASE_REASON,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_ATTRIBUTES,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CONNECTION_ATTRIBUTES,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(cc_notify,
	DECT_SFMT_IE(DECT_IE_TIMER_RESTART,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(cc_iwu_info,
	DECT_SFMT_IE(DECT_IE_PORTABLE_IDENTITY,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_MMS_GENERIC_HEADER,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_MMS_OBJECT_HEADER,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_MMS_EXTENDED_HEADER,	IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_TIME_DATE,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CALLING_PARTY_NUMBER,	IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_CALLING_PARTY_NAME,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CALLED_PARTY_NUMBER,	IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_CALLED_PARTY_SUBADDR,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ALPHANUMERIC,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_PACKET,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CODEC_LIST,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(crss_hold,
	DECT_SFMT_IE(DECT_IE_SINGLE_DISPLAY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

#define __cc_debug(call, pfx, fmt, args...) \
	dect_debug(DECT_DEBUG_CC, "%sCC: call %p (%s): " fmt "\n", pfx, \
		   (call), call_states[(call)->state], ## args)

#define cc_debug(call, fmt, args...) \
	__cc_debug(call, "", fmt, ## args)
#define cc_debug_entry(call, fmt, args...) \
	__cc_debug(call, "\n", fmt, ## args)

static const char * const call_states[DECT_CC_STATE_MAX + 1] = {
	[DECT_CC_NULL]				= "NULL",
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

/**
 * Get a pointer to the private data area from a Call Control Endpoint
 *
 * @param call		Call Control Endpoint
 */
void *dect_call_priv(struct dect_call *call)
{
	return call->priv;
}
EXPORT_SYMBOL(dect_call_priv);

const struct dect_ipui *dect_call_portable_identity(const struct dect_call *call)
{
	return &call->pt_id->ipui;
}
EXPORT_SYMBOL(dect_call_portable_identity);

/**
 * DL_U_DATA-req primitive
 *
 * @param dh		libdect DECT handle
 * @param call		Call Control Endpoint
 * @param mb		Message Buffer
 *
 * Pass U-plane data to the DLC.
 */
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
		cc_debug(call, "sending %u bytes failed: %s",
			 mb->len, strerror(errno));
	return 0;
}
EXPORT_SYMBOL(dect_dl_u_data_req);

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
	addr.dect_index = dh->index;
	if (connect(call->lu_sap->fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		goto err2;

	dect_fd_setup(call->lu_sap, dect_cc_lu_event, call);
	if (dect_fd_register(dh, call->lu_sap, DECT_FD_READ) < 0)
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
	if (call->lu_sap == NULL)
		return;

	dect_fd_unregister(dh, call->lu_sap);
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

	call->setup_timer = dect_timer_alloc(dh);
	if (call->setup_timer == NULL)
		goto err2;

	call->state = DECT_CC_NULL;
	return call;

err2:
	dect_free(dh, call);
err1:
	return NULL;
}
EXPORT_SYMBOL(dect_call_alloc);

static void dect_call_destroy(const struct dect_handle *dh,
			      struct dect_call *call)
{
	if (call->state == DECT_CC_CALL_PRESENT)
		dect_timer_stop(dh, call->setup_timer);
	dect_free(dh, call->setup_timer);
	dect_free(dh, call);
}

static int dect_cc_send_msg(struct dect_handle *dh, struct dect_call *call,
			    const struct dect_sfmt_msg_desc *desc,
			    const struct dect_msg_common *msg,
			    enum dect_cc_msg_types type)
{
	return dect_lce_send(dh, &call->transaction, desc, msg, type);
}

static void dect_cc_send_release_com(struct dect_handle *dh,
				     struct dect_transaction *ta,
				     enum dect_release_reasons reason)
{
	struct dect_ie_release_reason release_reason;
	struct dect_cc_release_com_msg msg = {
		.release_reason		= &release_reason,
	};

	release_reason.reason = reason;
	dect_lce_send(dh, ta, &cc_release_com_msg_desc,
		      &msg.common, CC_RELEASE_COM);
}

static void dect_cc_setup_timer(struct dect_handle *dh, struct dect_timer *timer)
{
	struct dect_call *call = timer->data;
	struct dect_mncc_release_param *param;

	cc_debug(call, "setup timer");
	dect_cc_send_release_com(dh, &call->transaction,
				 DECT_RELEASE_TIMER_EXPIRY);

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto out;

	cc_debug(call, "MNCC_REJECT-ind");
	dh->ops->cc_ops->mncc_reject_ind(dh, call, param);
	dect_ie_collection_put(dh, param);
out:
	dect_transaction_close(dh, &call->transaction, DECT_DDL_RELEASE_NORMAL);
	dect_call_destroy(dh, call);
}

/**
 * MNCC_SETUP-req primitive
 *
 * @param dh		libdect DECT handle
 * @param call		Call Control Endpoint
 * @param ipui		IPUI of PP
 * @param param		call setup parameters
 *
 * Initiate call establishment by sending a {CC-SETUP} message to the remote
 * side.
 *
 * @sa ETSI EN 300 175-5 (Network Layer), sections 9.3.1 and 9.3.2.
 */
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
		.escape_to_proprietary		= param->escape_to_proprietary,
		.codec_list			= param->codec_list,
	};

	cc_debug_entry(call, "MNCC_SETUP-req");
	if (dect_transaction_open(dh, &call->transaction, ipui, DECT_PD_CC) < 0)
		goto err1;

	fixed_identity.type = DECT_FIXED_ID_TYPE_PARK;
	memcpy(&fixed_identity.ari, &dh->pari, sizeof(fixed_identity.ari));
	portable_identity.type = DECT_PORTABLE_ID_TYPE_IPUI;
	portable_identity.ipui = *ipui;

	if (dect_cc_send_msg(dh, call, &cc_setup_msg_desc, &msg.common,
			     CC_SETUP) < 0)
		goto err2;
	call->state = DECT_CC_CALL_PRESENT;

	dect_timer_setup(call->setup_timer, dect_cc_setup_timer, call);
	dect_timer_start(dh, call->setup_timer, DECT_CC_SETUP_TIMEOUT);
	return 0;

err2:
	dect_transaction_close(dh, &call->transaction, DECT_DDL_RELEASE_NORMAL);
err1:
	return -1;
}
EXPORT_SYMBOL(dect_mncc_setup_req);

/**
 * MNCC_SETUP_ACK-req primitve
 *
 * @param dh		libdect DECT handle
 * @param call		Call Control Endpoint
 * @param param		call setup ACK parameters
 *
 * Accept call setup by sending a {CC-SETUP-ACK} message and enter "OVERLAP
 * RECEIVING" state.
 *
 * @sa ETSI EN 300 175-5 (Network Layer), sections 9.3.1.5 and 9.3.2.5.
 */
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
		.escape_to_proprietary	= param->escape_to_proprietary,
		.codec_list		= param->codec_list,
	};

	cc_debug_entry(call, "MNCC_SETUP_ACK-req");
	return dect_cc_send_msg(dh, call, &cc_setup_ack_msg_desc,
				&msg.common, CC_SETUP_ACK);
}
EXPORT_SYMBOL(dect_mncc_setup_ack_req);

/**
 * MNCC_REJECT-req primitive
 *
 * @param dh		libdect DECT handle
 * @param call		Call Control Endpoint
 * @param param		call release parameters
 *
 * Reject call setup by sending a {CC-RELEASE-COM} message and release the call.
 *
 * @sa ETSI EN 300 175-5 (Network Layer), sections 9.3.1.2 and 9.3.2.2.
 */
int dect_mncc_reject_req(struct dect_handle *dh, struct dect_call *call,
			 const struct dect_mncc_release_param *param)
{
	struct dect_cc_release_com_msg msg = {
		.release_reason			= param->release_reason,
		.identity_type			= param->identity_type,
		.location_area			= param->location_area,
		.iwu_attributes			= param->iwu_attributes,
		.facility			= param->facility,
		.display			= param->display,
		.feature_indicate		= param->feature_indicate,
		.network_parameter		= param->network_parameter,
		.iwu_to_iwu			= param->iwu_to_iwu,
		.iwu_packet			= param->iwu_packet,
		.escape_to_proprietary		= param->escape_to_proprietary,
	};

	cc_debug_entry(call, "MNCC_REJECT-req");
	dect_cc_send_msg(dh, call, &cc_release_com_msg_desc,
			 &msg.common, CC_RELEASE_COM);

	dect_transaction_close(dh, &call->transaction, DECT_DDL_RELEASE_NORMAL);
	dect_call_destroy(dh, call);
	return 0;
}
EXPORT_SYMBOL(dect_mncc_reject_req);

/**
 * MNCC_CALL_PROC-req primitive
 *
 * @param dh		libdect DECT handle
 * @param call		Call Control Endpoint
 * @param param		call proceeding parameters
 *
 * Accept call setup by sending a {CC-CALL-PROC} message and enter "CALL
 * PROCEEDING" state.
 *
 * @sa ETSI EN 300 175-5 (Network Layer), sections 9.3.1.6 and 9.3.2.6.
 */
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
		.escape_to_proprietary	= param->escape_to_proprietary,
		.codec_list		= param->codec_list,
	};

	cc_debug_entry(call, "MNCC_CALL_PROC-req");
	return dect_cc_send_msg(dh, call, &cc_call_proc_msg_desc,
				&msg.common, CC_CALL_PROC);
}
EXPORT_SYMBOL(dect_mncc_call_proc_req);

/**
 * MNCC_ALERT-req primitive
 *
 * @param dh		libdect DECT handle
 * @param call		Call Control Endpoint
 * @param param		call alerting parameters
 *
 * Accept a call by sending a {CC-ALERTING} message and enter "CALL DELIVERED"
 * state.
 *
 * @sa ETSI EN 300 175-5 (Network Layer), sections 9.3.1.7 and 9.3.2.7.
 */
int dect_mncc_alert_req(struct dect_handle *dh, struct dect_call *call,
			const struct dect_mncc_alert_param *param)
{
	struct dect_cc_alerting_msg msg = {
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
		.escape_to_proprietary	= param->escape_to_proprietary,
		.codec_list		= param->codec_list,
	};

	cc_debug_entry(call, "MNCC_ALERT-req");
	return dect_cc_send_msg(dh, call, &cc_alerting_msg_desc,
				&msg.common, CC_ALERTING);
}
EXPORT_SYMBOL(dect_mncc_alert_req);

/**
 * MNCC_CONNECT-req primitive
 *
 * @param dh		libdect DECT handle
 * @param call		Call Control Endpoint
 * @param param		call connection parameters
 *
 * Connect a call by sending a {CC-CONNECT} message and enter "ACTIVE" state.
 *
 * @sa ETSI EN 300 175-5 (Network Layer), sections 9.3.1.8 and 9.3.2.8.
 */
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
		.escape_to_proprietary	= param->escape_to_proprietary,
		.codec_list		= param->codec_list,
	};

	cc_debug_entry(call, "MNCC_CONNECT-req");
	dect_cc_send_msg(dh, call, &cc_connect_msg_desc,
			 &msg.common, CC_CONNECT);
	dect_call_connect_uplane(dh, call);
	return 0;
}
EXPORT_SYMBOL(dect_mncc_connect_req);

/**
 * MNCC_CONNECT-res primitive
 *
 * @param dh		libdect DECT handle
 * @param call		Call Control Endpoint
 * @param param		call connection parameters
 */
int dect_mncc_connect_res(struct dect_handle *dh, struct dect_call *call,
			  const struct dect_mncc_connect_param *param)
{
	struct dect_cc_connect_ack_msg msg = {
		.display		= param->display,
		.feature_indicate	= param->feature_indicate,
		//.iwu_to_iwu		= param->iwu_to_iwu,
		.iwu_packet		= param->iwu_packet,
		.escape_to_proprietary	= param->escape_to_proprietary,
	};

	cc_debug_entry(call, "MNCC_CONNECT-res");
	dect_call_connect_uplane(dh, call);
	if (dect_cc_send_msg(dh, call, &cc_connect_ack_msg_desc,
			     &msg.common, CC_CONNECT_ACK) < 0)
		goto err1;

	call->state = DECT_CC_ACTIVE;
	return 0;

err1:
	dect_call_disconnect_uplane(dh, call);
	return -1;
}
EXPORT_SYMBOL(dect_mncc_connect_res);

/**
 * MNCC_RELEASE-req primitive
 *
 * @param dh		libdect DECT handle
 * @param call		Call Control Endpoint
 * @param param		call release parameters
 */
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
		.escape_to_proprietary	= param->escape_to_proprietary,
	};

	cc_debug_entry(call, "MNCC_RELEASE-req");
	dect_cc_send_msg(dh, call, &cc_release_msg_desc,
			 &msg.common, CC_RELEASE);
	call->state = DECT_CC_RELEASE_PENDING;
	return 0;
}
EXPORT_SYMBOL(dect_mncc_release_req);

/**
 * MNCC_RELEASE-res primitive
 *
 * @param dh		libdect DECT handle
 * @param call		Call Control Endpoint
 * @param param		call release parameters
 */
int dect_mncc_release_res(struct dect_handle *dh, struct dect_call *call,
			  const struct dect_mncc_release_param *param)
{
	struct dect_cc_release_com_msg msg = {
		.release_reason			= param->release_reason,
		.identity_type			= param->identity_type,
		.location_area			= param->location_area,
		.iwu_attributes			= param->iwu_attributes,
		.facility			= param->facility,
		.display			= param->display,
		.feature_indicate		= param->feature_indicate,
		.network_parameter		= param->network_parameter,
		.iwu_to_iwu			= param->iwu_to_iwu,
		.iwu_packet			= param->iwu_packet,
		.escape_to_proprietary		= param->escape_to_proprietary,
	};

	cc_debug_entry(call, "MNCC_RELEASE-res");
	dect_cc_send_msg(dh, call, &cc_release_com_msg_desc,
			 &msg.common, CC_RELEASE_COM);

	dect_call_disconnect_uplane(dh, call);
	dect_transaction_close(dh, &call->transaction, DECT_DDL_RELEASE_NORMAL);
	dect_call_destroy(dh, call);
	return 0;
}
EXPORT_SYMBOL(dect_mncc_release_res);

/**
 * MNCC_FACILITY-req primitive
 *
 * @param dh		libdect DECT handle
 * @param call		Call Control Endpoint
 * @param param
 */
int dect_mncc_facility_req(struct dect_handle *dh, struct dect_call *call,
			  const struct dect_mncc_facility_param *param)
{
	cc_debug_entry(call, "MNCC_FACILITY-req");
	return 0;
}
EXPORT_SYMBOL(dect_mncc_facility_req);

/**
 * MNCC_INFO-req primitive
 *
 * @param dh		libdect DECT handle
 * @param call		Call Control Endpoint
 * @param param
 */
int dect_mncc_info_req(struct dect_handle *dh, struct dect_call *call,
		       const struct dect_mncc_info_param *param)
{
	struct dect_cc_info_msg msg = {
		.location_area			= param->location_area,
		.nwk_assigned_identity		= param->nwk_assigned_identity,
		.facility			= param->facility,
		.progress_indicator		= param->progress_indicator,
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
		.escape_to_proprietary		= param->escape_to_proprietary,
		.codec_list			= param->codec_list,
	};

	cc_debug_entry(call, "MNCC_INFO-req");
	dect_cc_send_msg(dh, call, &cc_info_msg_desc, &msg.common, CC_INFO);
	return 0;
}
EXPORT_SYMBOL(dect_mncc_info_req);

/**
 * MNCC_MODIFY-req primitive
 *
 * @param dh		libdect DECT handle
 * @param call		Call Control Endpoint
 * @param param		call modification parameters
 */
int dect_mncc_modify_req(struct dect_handle *dh, struct dect_call *call,
			 const struct dect_mncc_modify_param *param)
{
	cc_debug_entry(call, "MNCC_MODIFY-req");
	return 0;
}
EXPORT_SYMBOL(dect_mncc_modify_req);

/**
 * MNCC_MODIFY-res primitive
 *
 * @param dh		libdect DECT handle
 * @param call		Call Control Endpoint
 * @param param		call modification parameters
 */
int dect_mncc_modify_res(struct dect_handle *dh, struct dect_call *call,
			 const struct dect_mncc_modify_param *param)
{
	cc_debug_entry(call, "MNCC_MODIFY-res");
	return 0;
}
EXPORT_SYMBOL(dect_mncc_modify_res);

/**
 * MNCC_HOLD-req primitive
 *
 * @param dh		libdect DECT handle
 * @param call		Call Control Endpoint
 * @param param		call hold parameters
 */
int dect_mncc_hold_req(struct dect_handle *dh, struct dect_call *call,
		       const struct dect_mncc_hold_param *param)
{
	struct dect_crss_hold_msg msg = {
		.display			= param->display,
		.escape_to_proprietary		= param->escape_to_proprietary,
	};

	cc_debug_entry(call, "MNCC_HOLD-req");
	dect_cc_send_msg(dh, call, &crss_hold_msg_desc, &msg.common, CRSS_HOLD);
	return 0;
}
EXPORT_SYMBOL(dect_mncc_hold_req);

/**
 * MNCC_HOLD-res primitive
 *
 * @param dh		libdect DECT handle
 * @param call		Call Control Endpoint
 * @param param		call hold parameters
 */
int dect_mncc_hold_res(struct dect_handle *dh, struct dect_call *call,
		       const struct dect_mncc_hold_param *param)
{
	cc_debug_entry(call, "MNCC_HOLD-res");
	return 0;
}
EXPORT_SYMBOL(dect_mncc_hold_res);

/**
 * MNCC_RETRIEVE-req primitive
 *
 * @param dh		libdect DECT handle
 * @param call		Call Control Endpoint
 * @param param		call retrieve parameters
 */
int dect_mncc_retrieve_req(struct dect_handle *dh, struct dect_call *call,
			   const struct dect_mncc_hold_param *param)
{
	cc_debug_entry(call, "MNCC_RETRIEVE-req");
	return 0;
}
EXPORT_SYMBOL(dect_mncc_retrieve_req);

/**
 * MNCC_RETRIEVE-res primitive
 *
 * @param dh		libdect DECT handle
 * @param call		Call Control Endpoint
 * @param param		call retrieve parameters
 */
int dect_mncc_retrieve_res(struct dect_handle *dh, struct dect_call *call,
			   const struct dect_mncc_hold_param *param)
{
	cc_debug_entry(call, "MNCC_RETRIEVE-res");
	return 0;
}
EXPORT_SYMBOL(dect_mncc_retrieve_res);

/**
 * MNCC_IWU_INFO-req primitive
 *
 * @param dh		libdect DECT handle
 * @param call		Call Control Endpoint
 * @param param
 */
int dect_mncc_iwu_info_req(struct dect_handle *dh, struct dect_call *call,
			   const struct dect_mncc_iwu_info_param *param)
{
	struct dect_cc_iwu_info_msg msg = {
		.alphanumeric		= param->alphanumeric,
		.iwu_to_iwu		= param->iwu_to_iwu,
		.iwu_packet		= param->iwu_packet,
		.escape_to_proprietary	= param->escape_to_proprietary,
	};

	cc_debug_entry(call, "MNCC_IWU_INFO-req");
	dect_cc_send_msg(dh, call, &cc_iwu_info_msg_desc, &msg.common, CC_IWU_INFO);
	return 0;
}
EXPORT_SYMBOL(dect_mncc_iwu_info_req);

static void dect_mncc_alert_ind(struct dect_handle *dh, struct dect_call *call,
				struct dect_cc_alerting_msg *msg)
{
	struct dect_mncc_alert_param *param;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		return;

	param->facility			= *dect_ie_list_hold(&msg->facility);
	param->progress_indicator	= *dect_ie_list_hold(&msg->progress_indicator);
	param->display			= dect_ie_hold(msg->display);
	param->signal			= dect_ie_hold(msg->signal);
	param->feature_indicate		= dect_ie_hold(msg->feature_indicate);
	param->terminal_capability	= dect_ie_hold(msg->terminal_capability);
	param->transit_delay		= dect_ie_hold(msg->transit_delay);
	param->window_size		= dect_ie_hold(msg->window_size);
	param->iwu_to_iwu		= *dect_ie_list_hold(&msg->iwu_to_iwu);
	param->iwu_packet		= dect_ie_hold(msg->iwu_packet);
	param->escape_to_proprietary	= dect_ie_hold(msg->escape_to_proprietary);
	param->codec_list		= dect_ie_hold(msg->codec_list);

	cc_debug(call, "MNCC_ALERT-ind");
	dh->ops->cc_ops->mncc_alert_ind(dh, call, param);
	dect_ie_collection_put(dh, param);
}

static void dect_cc_rcv_alerting(struct dect_handle *dh, struct dect_call *call,
				 struct dect_msg_buf *mb)
{
	struct dect_cc_alerting_msg msg;

	cc_debug(call, "CC-ALERTING");
	if (call->state != DECT_CC_CALL_PRESENT)
		;

	if (dect_parse_sfmt_msg(dh, &cc_alerting_msg_desc, &msg.common, mb) < 0)
		return;

	if (call->setup_timer != NULL) {
		dect_timer_stop(dh, call->setup_timer);
		dect_free(dh, call->setup_timer);
		call->setup_timer = NULL;
	}

	dect_mncc_alert_ind(dh, call, &msg);
	dect_msg_free(dh, &cc_alerting_msg_desc, &msg.common);
	call->state = DECT_CC_CALL_RECEIVED;
}

static void dect_mncc_call_proc_ind(struct dect_handle *dh, struct dect_call *call,
				    struct dect_cc_call_proc_msg *msg)
{
	struct dect_mncc_call_proc_param *param;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		return;

	param->facility			= *dect_ie_list_hold(&msg->facility);
	param->progress_indicator	= *dect_ie_list_hold(&msg->progress_indicator);
	param->display			= dect_ie_hold(msg->display);
	param->signal			= dect_ie_hold(msg->signal);
	param->feature_indicate		= dect_ie_hold(msg->feature_indicate);
	param->transit_delay		= dect_ie_hold(msg->transit_delay);
	param->window_size		= dect_ie_hold(msg->window_size);
	param->iwu_to_iwu		= *dect_ie_list_hold(&msg->iwu_to_iwu);
	param->iwu_packet		= dect_ie_hold(msg->iwu_packet);
	param->escape_to_proprietary	= dect_ie_hold(msg->escape_to_proprietary);
	param->codec_list		= dect_ie_hold(msg->codec_list);

	cc_debug(call, "MNCC_CALL_PROC-ind");
	dh->ops->cc_ops->mncc_call_proc_ind(dh, call, param);
	dect_ie_collection_put(dh, param);
}

static void dect_cc_rcv_call_proc(struct dect_handle *dh, struct dect_call *call,
				  struct dect_msg_buf *mb)
{
	struct dect_cc_call_proc_msg msg;

	cc_debug(call, "CC-CALL_PROC");
	if (dect_parse_sfmt_msg(dh, &cc_call_proc_msg_desc, &msg.common, mb) < 0)
		return;

	if (call->setup_timer != NULL) {
		dect_timer_stop(dh, call->setup_timer);
		dect_free(dh, call->setup_timer);
		call->setup_timer = NULL;
	}

	dect_mncc_call_proc_ind(dh, call, &msg);
	dect_msg_free(dh, &cc_call_proc_msg_desc, &msg.common);
	call->state = DECT_CC_CALL_PROCEEDING;
}

static void dect_mncc_connect_ind(struct dect_handle *dh, struct dect_call *call,
				  struct dect_cc_connect_msg *msg)
{
	struct dect_mncc_connect_param *param;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		return;

	param->facility			= *dect_ie_list_hold(&msg->facility);
	param->progress_indicator	= *dect_ie_list_hold(&msg->progress_indicator);
	param->display			= dect_ie_hold(msg->display);
	param->signal			= dect_ie_hold(msg->signal);
	param->feature_indicate		= dect_ie_hold(msg->feature_indicate);
	param->terminal_capability	= dect_ie_hold(msg->terminal_capability);
	param->transit_delay		= dect_ie_hold(msg->transit_delay);
	param->window_size		= dect_ie_hold(msg->window_size);
	param->iwu_to_iwu		= dect_ie_hold(msg->iwu_to_iwu);
	param->iwu_packet		= dect_ie_hold(msg->iwu_packet);
	param->escape_to_proprietary	= dect_ie_hold(msg->escape_to_proprietary);
	param->codec_list		= dect_ie_hold(msg->codec_list);

	cc_debug(call, "MNCC_CONNECT-ind");
	dh->ops->cc_ops->mncc_connect_ind(dh, call, param);
	dect_ie_collection_put(dh, param);
}

static void dect_cc_rcv_connect(struct dect_handle *dh, struct dect_call *call,
				struct dect_msg_buf *mb)
{
	struct dect_cc_connect_msg msg;

	if (call->state != DECT_CC_CALL_PRESENT &&
	    call->state != DECT_CC_CALL_RECEIVED)
		;

	cc_debug(call, "CC-CONNECT");
	if (dect_parse_sfmt_msg(dh, &cc_connect_msg_desc, &msg.common, mb) < 0)
		return;

	if (call->setup_timer != NULL) {
		dect_timer_stop(dh, call->setup_timer);
		dect_free(dh, call->setup_timer);
		call->setup_timer = NULL;
	}

	dect_mncc_connect_ind(dh, call, &msg);
	dect_msg_free(dh, &cc_connect_msg_desc, &msg.common);
}

static void dect_cc_rcv_setup_ack(struct dect_handle *dh, struct dect_call *call,
				  struct dect_msg_buf *mb)
{
	struct dect_cc_setup_ack_msg msg;

	cc_debug(call, "CC-SETUP_ACK");
	if (dect_parse_sfmt_msg(dh, &cc_setup_ack_msg_desc, &msg.common, mb) < 0)
		return;

	dect_msg_free(dh, &cc_setup_ack_msg_desc, &msg.common);
}

static void dect_cc_rcv_connect_ack(struct dect_handle *dh, struct dect_call *call,
				    struct dect_msg_buf *mb)
{
	struct dect_cc_connect_ack_msg msg;

	cc_debug(call, "CC-CONNECT_ACK");
	if (dect_parse_sfmt_msg(dh, &cc_connect_ack_msg_desc, &msg.common, mb) < 0)
		return;

	dect_msg_free(dh, &cc_connect_ack_msg_desc, &msg.common);
}

static void dect_cc_rcv_service_change(struct dect_handle *dh, struct dect_call *call,
				       struct dect_msg_buf *mb)
{
	struct dect_cc_service_change_msg msg;

	cc_debug(call, "CC-SERVICE_CHANGE");
	if (dect_parse_sfmt_msg(dh, &cc_service_change_msg_desc,
				&msg.common, mb) < 0)
		return;

	dect_msg_free(dh, &cc_connect_ack_msg_desc, &msg.common);
}

static void dect_cc_rcv_service_accept(struct dect_handle *dh, struct dect_call *call,
				       struct dect_msg_buf *mb)
{
	struct dect_cc_service_accept_msg msg;

	cc_debug(call, "CC-SERVICE_ACCEPT");
	if (dect_parse_sfmt_msg(dh, &cc_service_accept_msg_desc,
				&msg.common, mb) < 0)
		return;

	dect_msg_free(dh, &cc_connect_ack_msg_desc, &msg.common);
}

static void dect_cc_rcv_service_reject(struct dect_handle *dh, struct dect_call *call,
				       struct dect_msg_buf *mb)
{
	struct dect_cc_service_reject_msg msg;

	cc_debug(call, "CC-SERVICE_REJECT");
	if (dect_parse_sfmt_msg(dh, &cc_service_reject_msg_desc,
				&msg.common, mb) < 0)
		return;

	dect_msg_free(dh, &cc_connect_ack_msg_desc, &msg.common);
}

static void dect_mncc_release_ind(struct dect_handle *dh, struct dect_call *call,
				  struct dect_cc_release_msg *msg)
{
	struct dect_mncc_release_param *param;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		return;

	param->release_reason		= dect_ie_hold(msg->release_reason);
	param->facility			= *dect_ie_list_hold(&msg->facility);
	param->display			= dect_ie_hold(msg->display);
	param->feature_indicate		= dect_ie_hold(msg->feature_indicate);
	param->iwu_to_iwu		= dect_ie_hold(msg->iwu_to_iwu);
	param->iwu_packet		= dect_ie_hold(msg->iwu_packet);
	param->escape_to_proprietary	= dect_ie_hold(msg->escape_to_proprietary);

	cc_debug(call, "MNCC_RELEASE-ind");
	dh->ops->cc_ops->mncc_release_ind(dh, call, param);
	dect_ie_collection_put(dh, param);
}

static void dect_cc_rcv_release(struct dect_handle *dh, struct dect_call *call,
				struct dect_msg_buf *mb)
{
	struct dect_cc_release_msg msg;

	cc_debug(call, "CC-RELEASE");
	if (dect_parse_sfmt_msg(dh, &cc_release_msg_desc, &msg.common, mb) < 0)
		return;

	dect_mncc_release_ind(dh, call, &msg);
	dect_msg_free(dh, &cc_release_msg_desc, &msg.common);
}

static void dect_mncc_release_cfm(struct dect_handle *dh, struct dect_call *call,
				  struct dect_cc_release_com_msg *msg)
{
	struct dect_mncc_release_param *param;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		return;

	param->release_reason		= dect_ie_hold(msg->release_reason);
	param->identity_type		= dect_ie_hold(msg->identity_type);
	param->location_area		= dect_ie_hold(msg->location_area);
	param->iwu_attributes		= dect_ie_hold(msg->iwu_attributes);
	param->facility			= *dect_ie_list_hold(&msg->facility);
	param->display			= dect_ie_hold(msg->display);
	param->feature_indicate		= dect_ie_hold(msg->feature_indicate);
	param->network_parameter	= dect_ie_hold(msg->network_parameter);
	param->iwu_to_iwu		= dect_ie_hold(msg->iwu_to_iwu);
	param->iwu_packet		= dect_ie_hold(msg->iwu_packet);
	param->escape_to_proprietary	= dect_ie_hold(msg->escape_to_proprietary);

	cc_debug(call, "MNCC_RELEASE-cfm");
	dh->ops->cc_ops->mncc_release_cfm(dh, call, param);
	dect_ie_collection_put(dh, param);
}

static void dect_cc_rcv_release_com(struct dect_handle *dh, struct dect_call *call,
				    struct dect_msg_buf *mb)
{
	struct dect_cc_release_com_msg msg;

	cc_debug(call, "CC-RELEASE_COM");
	if (dect_parse_sfmt_msg(dh, &cc_release_com_msg_desc, &msg.common, mb) < 0)
		return;

	if (call->state == DECT_CC_RELEASE_PENDING)
		dect_mncc_release_cfm(dh, call, &msg);
	else {
		struct dect_mncc_release_param *param;

		param = dect_ie_collection_alloc(dh, sizeof(*param));
		if (param == NULL)
			goto out;

		param->release_reason		= dect_ie_hold(msg.release_reason);
		param->facility			= *dect_ie_list_hold(&msg.facility);
		param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
		param->iwu_packet		= dect_ie_hold(msg.iwu_packet);
		param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

		cc_debug(call, "MNCC_RELEASE-ind");
		dh->ops->cc_ops->mncc_release_ind(dh, call, param);
		dect_ie_collection_put(dh, param);
	}
out:
	dect_msg_free(dh, &cc_release_com_msg_desc, &msg.common);

	dect_call_disconnect_uplane(dh, call);
	dect_transaction_close(dh, &call->transaction, DECT_DDL_RELEASE_NORMAL);
	dect_call_destroy(dh, call);
}

static void dect_mncc_iwu_info_ind(struct dect_handle *dh, struct dect_call *call,
				   struct dect_cc_iwu_info_msg *msg)
{
	struct dect_mncc_iwu_info_param *param;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		return;

	param->alphanumeric		= dect_ie_hold(msg->alphanumeric);
	param->iwu_to_iwu		= dect_ie_hold(msg->iwu_to_iwu);
	param->iwu_packet		= dect_ie_hold(msg->iwu_packet);
	param->escape_to_proprietary	= dect_ie_hold(msg->escape_to_proprietary);

	cc_debug(call, "MNCC_IWU_INFO-ind");
	dh->ops->cc_ops->mncc_iwu_info_ind(dh, call, param);
	dect_ie_collection_put(dh, param);
}

static void dect_cc_rcv_iwu_info(struct dect_handle *dh, struct dect_call *call,
				 struct dect_msg_buf *mb)
{
	struct dect_cc_iwu_info_msg msg;

	cc_debug(call, "CC-IWU_INFO");
	if (dect_parse_sfmt_msg(dh, &cc_iwu_info_msg_desc, &msg.common, mb) < 0)
		return;

	dect_mncc_iwu_info_ind(dh, call, &msg);
	dect_msg_free(dh, &cc_iwu_info_msg_desc, &msg.common);
}

static void dect_cc_rcv_notify(struct dect_handle *dh, struct dect_call *call,
			       struct dect_msg_buf *mb)
{
	struct dect_cc_notify_msg msg;

	cc_debug(call, "CC-NOTIFY");
	if (dect_parse_sfmt_msg(dh, &cc_notify_msg_desc, &msg.common, mb) < 0)
		return;

	dect_msg_free(dh, &cc_notify_msg_desc, &msg.common);
}

static void dect_mncc_info_ind(struct dect_handle *dh, struct dect_call *call,
			       struct dect_cc_info_msg *msg)
{
	struct dect_mncc_info_param *param;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		return;

	param->location_area		= dect_ie_hold(msg->location_area);
	param->nwk_assigned_identity	= dect_ie_hold(msg->nwk_assigned_identity);
	param->facility			= *dect_ie_list_hold(&msg->facility);
	param->progress_indicator	= *dect_ie_list_hold(&msg->progress_indicator);
	param->display			= dect_ie_hold(msg->display);
	param->keypad			= dect_ie_hold(msg->keypad);
	param->signal			= dect_ie_hold(msg->signal);
	param->feature_activate		= dect_ie_hold(msg->feature_activate);
	param->feature_indicate		= dect_ie_hold(msg->feature_indicate);
	param->network_parameter	= dect_ie_hold(msg->network_parameter);
	param->called_party_number	= dect_ie_hold(msg->called_party_number);
	param->called_party_subaddress	= dect_ie_hold(msg->called_party_subaddress);
	param->calling_party_number	= dect_ie_hold(msg->calling_party_number);
	param->calling_party_name	= dect_ie_hold(msg->calling_party_name);
	param->sending_complete		= dect_ie_hold(msg->sending_complete);
	param->iwu_to_iwu		= *dect_ie_list_hold(&msg->iwu_to_iwu);
	param->iwu_packet		= dect_ie_hold(msg->iwu_packet);
	param->escape_to_proprietary	= dect_ie_hold(msg->escape_to_proprietary);
	param->codec_list		= dect_ie_hold(msg->codec_list);

	cc_debug(call, "MNCC_INFO-ind");
	dh->ops->cc_ops->mncc_info_ind(dh, call, param);
	dect_ie_collection_put(dh, param);
}

static void dect_cc_rcv_info(struct dect_handle *dh, struct dect_call *call,
			     struct dect_msg_buf *mb)
{
	struct dect_cc_info_msg msg;

	cc_debug(call, "CC-INFO");
	if (call->state == DECT_CC_CALL_INITIATED ||
	    call->state == DECT_CC_CALL_PRESENT)
		;

	if (dect_parse_sfmt_msg(dh, &cc_info_msg_desc, &msg.common, mb) < 0)
		return;

	dect_mncc_info_ind(dh, call, &msg);
	dect_msg_free(dh, &cc_info_msg_desc, &msg.common);
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
		return dect_cc_rcv_service_change(dh, call, mb);
	case CC_SERVICE_ACCEPT:
		return dect_cc_rcv_service_accept(dh, call, mb);
	case CC_SERVICE_REJECT:
		return dect_cc_rcv_service_reject(dh, call, mb);
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
	struct dect_mncc_setup_param *param;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		return;

	param->basic_service		= dect_ie_hold(msg->basic_service);
	param->iwu_attributes		= *dect_ie_list_hold(&msg->iwu_attributes);
	param->cipher_info		= dect_ie_hold(msg->cipher_info);
	param->facility			= *dect_ie_list_hold(&msg->facility);
	param->progress_indicator	= *dect_ie_list_hold(&msg->progress_indicator);
	param->display			= dect_ie_hold(msg->display);
	param->keypad			= dect_ie_hold(msg->keypad);
	param->signal			= dect_ie_hold(msg->signal);
	param->feature_activate       	= dect_ie_hold(msg->feature_activate);
	param->feature_indicate       	= dect_ie_hold(msg->feature_indicate);
	param->network_parameter	= dect_ie_hold(msg->network_parameter);
	param->terminal_capability	= dect_ie_hold(msg->terminal_capability);
	param->end_to_end_compatibility	= dect_ie_hold(msg->end_to_end_compatibility);
	param->rate_parameters		= dect_ie_hold(msg->rate_parameters);
	param->transit_delay		= dect_ie_hold(msg->transit_delay);
	param->window_size		= dect_ie_hold(msg->window_size);
	param->called_party_number	= dect_ie_hold(msg->called_party_number);
	param->called_party_subaddress	= dect_ie_hold(msg->called_party_subaddress);
	param->calling_party_number	= dect_ie_hold(msg->calling_party_number);
	param->calling_party_name	= dect_ie_hold(msg->calling_party_name);
	param->sending_complete		= dect_ie_hold(msg->sending_complete);
	param->iwu_to_iwu		= dect_ie_hold(msg->iwu_to_iwu);
	param->iwu_packet		= dect_ie_hold(msg->iwu_packet);
	param->escape_to_proprietary	= dect_ie_hold(msg->escape_to_proprietary);
	param->codec_list		= dect_ie_hold(msg->codec_list);

	cc_debug(call, "MNCC_SETUP-ind");
	dh->ops->cc_ops->mncc_setup_ind(dh, call, param);
	dect_ie_collection_put(dh, param);
}

static void dect_cc_rcv_setup(struct dect_handle *dh,
			      const struct dect_transaction *req,
			      struct dect_msg_buf *mb)
{
	struct dect_ie_connection_attributes *connection_attributes;
	struct dect_ie_call_attributes *call_attributes;
	struct dect_cc_setup_msg msg;
	struct dect_call *call;

	dect_debug(DECT_DEBUG_CC, "CC-SETUP\n");
	if (dect_parse_sfmt_msg(dh, &cc_setup_msg_desc, &msg.common, mb) < 0)
		return;

	/* Validate portable_identity type */
	if (msg.portable_identity->type != DECT_PORTABLE_ID_TYPE_IPUI)
		goto out;

	dect_foreach_ie(call_attributes, &msg.call_attributes)
		dect_debug(DECT_DEBUG_CC, "call attributes\n");

	dect_foreach_ie(connection_attributes, &msg.connection_attributes)
		dect_debug(DECT_DEBUG_CC, "connection attributes\n");

	call = dect_call_alloc(dh);
	if (call == NULL)
		goto out;
	call->ft_id = dect_ie_hold(msg.fixed_identity);
	call->pt_id = dect_ie_hold(msg.portable_identity);
	call->state = DECT_CC_CALL_INITIATED;
	dect_transaction_confirm(dh, &call->transaction, req);
	cc_debug(call, "new call");

	dect_mncc_setup_ind(dh, call, &msg);
out:
	dect_msg_free(dh, &cc_setup_msg_desc, &msg.common);
}

static void dect_cc_open(struct dect_handle *dh,
			 struct dect_transaction *req,
			 struct dect_msg_buf *mb)
{
	dect_debug(DECT_DEBUG_CC, "CC: unknown transaction: msg type: %x\n",
		   mb->type);

	switch (mb->type) {
	case CC_SETUP:
		return dect_cc_rcv_setup(dh, req, mb);
	case CC_RELEASE:
	case CC_RELEASE_COM:
		break;
	default:
		return dect_cc_send_release_com(dh, req,
				DECT_RELEASE_UNKNOWN_TRANSACTION_IDENTIFIER);
	}
}

static void dect_cc_shutdown(struct dect_handle *dh,
			     struct dect_transaction *ta)
{
	struct dect_call *call = container_of(ta, struct dect_call, transaction);

	cc_debug(call, "shutdown");
	dh->ops->cc_ops->mncc_reject_ind(dh, call, NULL);
	dect_transaction_close(dh, &call->transaction, DECT_DDL_RELEASE_NORMAL);
	dect_call_destroy(dh, call);
}

static const struct dect_nwk_protocol cc_protocol = {
	.name			= "Call Control",
	.pd			= DECT_PD_CC,
	.max_transactions	= 7,
	.open			= dect_cc_open,
	.shutdown		= dect_cc_shutdown,
	.rcv			= dect_cc_rcv,
};

static void __init dect_cc_init(void)
{
	dect_lce_register_protocol(&cc_protocol);
}

/** @} */
