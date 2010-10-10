/*
 * DECT Mobility Management (MM)
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/**
 * @defgroup mm Mobility Management
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
#include <mm.h>
#include <dect/auth.h>

static DECT_SFMT_MSG_DESC(mm_access_rights_accept,
	DECT_SFMT_IE(DECT_IE_PORTABLE_IDENTITY,		IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_FIXED_IDENTITY,		IE_MANDATORY, IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_LOCATION_AREA,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_AUTH_TYPE,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_CIPHER_INFO,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_ZAP_FIELD,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_SERVICE_CLASS,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_SETUP_CAPABILITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_MODEL_IDENTIFIER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_CODEC_LIST,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_access_rights_request,
	DECT_SFMT_IE(DECT_IE_PORTABLE_IDENTITY,		IE_NONE,      IE_MANDATORY, 0),
	DECT_SFMT_IE(DECT_IE_AUTH_TYPE,			IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CIPHER_INFO,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SETUP_CAPABILITY,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_TERMINAL_CAPABILITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_MODEL_IDENTIFIER,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CODEC_LIST,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_access_rights_reject,
	DECT_SFMT_IE(DECT_IE_REJECT_REASON,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_access_rights_terminate_accept,
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_access_rights_terminate_reject,
	DECT_SFMT_IE(DECT_IE_REJECT_REASON,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_access_rights_terminate_request,
	DECT_SFMT_IE(DECT_IE_PORTABLE_IDENTITY,		IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_FIXED_IDENTITY,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_authentication_reject,
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_AUTH_TYPE,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_REJECT_REASON,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	//DECT_SFMT_IE(DECT_IE_AUTH_REJECT_PARAMETER,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_authentication_reply,
	DECT_SFMT_IE(DECT_IE_RES,			IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(DECT_IE_RS,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_ZAP_FIELD,			IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SERVICE_CLASS,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_KEY,			IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_authentication_request,
	DECT_SFMT_IE(DECT_IE_AUTH_TYPE,			IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(DECT_IE_RAND,			IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(DECT_IE_RES,			IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_RS,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_CIPHER_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_cipher_suggest,
	DECT_SFMT_IE(DECT_IE_CIPHER_INFO,		IE_NONE,      IE_MANDATORY, 0),
	DECT_SFMT_IE(DECT_IE_CALL_IDENTITY,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CONNECTION_IDENTITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_cipher_request,
	DECT_SFMT_IE(DECT_IE_CIPHER_INFO,		IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_CALL_IDENTITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_CONNECTION_IDENTITY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_cipher_reject,
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CIPHER_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_REJECT_REASON,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_detach,
	DECT_SFMT_IE(DECT_IE_PORTABLE_IDENTITY,		IE_NONE,      IE_MANDATORY, 0),
	DECT_SFMT_IE(DECT_IE_NWK_ASSIGNED_IDENTITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_NETWORK_PARAMETER,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_identity_reply,
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_PORTABLE_IDENTITY,		IE_NONE,      IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_FIXED_IDENTITY,		IE_NONE,      IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_NWK_ASSIGNED_IDENTITY,	IE_NONE,      IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_MODEL_IDENTIFIER,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_identity_request,
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_IDENTITY_TYPE,		IE_MANDATORY, IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_NETWORK_PARAMETER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_key_allocate,
	DECT_SFMT_IE(DECT_IE_ALLOCATION_TYPE,		IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_RAND,			IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_RS,			IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_locate_accept,
	DECT_SFMT_IE(DECT_IE_PORTABLE_IDENTITY,		IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_LOCATION_AREA,		IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_USE_TPUI,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_NWK_ASSIGNED_IDENTITY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_EXT_HO_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_SETUP_CAPABILITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_MODEL_IDENTIFIER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_CODEC_LIST,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_locate_reject,
	DECT_SFMT_IE(DECT_IE_REJECT_REASON,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_locate_request,
	DECT_SFMT_IE(DECT_IE_PORTABLE_IDENTITY,		IE_NONE,      IE_MANDATORY, 0),
	DECT_SFMT_IE(DECT_IE_FIXED_IDENTITY,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_LOCATION_AREA,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_NWK_ASSIGNED_IDENTITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CIPHER_INFO,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SETUP_CAPABILITY,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_TERMINAL_CAPABILITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_NETWORK_PARAMETER,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_MODEL_IDENTIFIER,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CODEC_LIST,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_info_accept,
	DECT_SFMT_IE(DECT_IE_INFO_TYPE,			IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_CALL_IDENTITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_FIXED_IDENTITY,		IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_LOCATION_AREA,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_NWK_ASSIGNED_IDENTITY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_NETWORK_PARAMETER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_info_reject,
	DECT_SFMT_IE(DECT_IE_CALL_IDENTITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_REJECT_REASON,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_info_request,
	DECT_SFMT_IE(DECT_IE_INFO_TYPE,			IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(DECT_IE_CALL_IDENTITY,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_PORTABLE_IDENTITY,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_FIXED_IDENTITY,		IE_NONE,      IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_LOCATION_AREA,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_NWK_ASSIGNED_IDENTITY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_NETWORK_PARAMETER,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_info_suggest,
	DECT_SFMT_IE(DECT_IE_INFO_TYPE,			IE_MANDATORY, IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_CALL_IDENTITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_FIXED_IDENTITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_LOCATION_AREA,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_NWK_ASSIGNED_IDENTITY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_NETWORK_PARAMETER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_EXT_HO_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_KEY,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_SETUP_CAPABILITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_temporary_identity_assign,
	DECT_SFMT_IE(DECT_IE_PORTABLE_IDENTITY,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_LOCATION_AREA,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_NWK_ASSIGNED_IDENTITY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_DURATION,			IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_NETWORK_PARAMETER,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_NONE,      DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_temporary_identity_assign_ack,
	DECT_SFMT_IE(DECT_IE_SEGMENTED_INFO,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_temporary_identity_assign_rej,
	DECT_SFMT_IE(DECT_IE_REJECT_REASON,		IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_NONE,      IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_iwu,
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_PACKET,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

static DECT_SFMT_MSG_DESC(mm_notify_msg,
	DECT_SFMT_IE(DECT_IE_TIMER_RESTART,		IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_NONE,      0),
	DECT_SFMT_IE_END_MSG
);

#define __mm_debug(mme, pfx, fmt, args...) \
	dect_debug(DECT_DEBUG_MM, "%sMM: link %d (%s): " fmt "\n", (pfx), \
		   (mme)->link && (mme)->link->dfd ? (mme)->link->dfd->fd : -1, \
		   (mme)->current ? dect_mm_proc[(mme)->current->type].name : "none", \
		   ## args)

#define mm_debug(mme, fmt, args...) \
	__mm_debug(mme, "", fmt, ## args)
#define mm_debug_entry(mme, fmt, args...) \
	__mm_debug(mme, "\n", fmt, ## args)

/*
 * MM Procedure Management
 */

struct dect_mm_proc {
	const char	*name;
	void		(*abort)(struct dect_handle *dh,
				 struct dect_mm_endpoint *mme,
				 struct dect_mm_procedure *mp);
	struct {
		uint8_t	priority;
		uint8_t	timeout;
	} param[2];
};

static const struct dect_mm_proc dect_mm_proc[DECT_MMP_MAX + 1];

static int dect_mm_procedure_initiate(struct dect_handle *dh,
				      struct dect_mm_endpoint *mme,
				      enum dect_mm_procedures type)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	const struct dect_mm_proc *proc = &dect_mm_proc[type];
	uint8_t priority;
	int err;

	priority = proc->param[dh->mode].priority;
	mm_debug(mme, "initiate procedure: %s priority: %u timeout: %us",
		 proc->name, priority, proc->param[dh->mode].timeout);

	/* Invalid procedure */
	if (priority == 0)
		return -1;

	/* Higher priority procedure active */
	if (mme->current != NULL &&
	    mme->current->priority <= priority)
		return -1;

	if (mp->type != DECT_MMP_NONE) {
		mm_debug(mme, "cancel procedure");
		if (dect_timer_running(mp->timer))
			dect_timer_stop(dh, mp->timer);
		dect_mm_proc[mp->type].abort(dh, mme, mp);
	} else {
		err = dect_ddl_transaction_open(dh, &mp->transaction, mme->link, DECT_PD_MM);
		if (err < 0)
			return err;
	}

	mp->type     = type;
	mp->priority = priority;
	mp->iec      = NULL;

	if (proc->param[dh->mode].timeout)
		dect_timer_start(dh, mp->timer, proc->param[dh->mode].timeout);

	mme->current = mp;
	return 0;
}

static int dect_mm_procedure_respond(struct dect_handle *dh,
				     struct dect_mm_endpoint *mme,
				     enum dect_mm_procedures type)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	const struct dect_mm_proc *proc = &dect_mm_proc[type];
	uint8_t priority;

	priority = proc->param[!dh->mode].priority;
	mm_debug(mme, "respond to procedure: %s priority: %u",
		 proc->name, priority);

	/* Invalid procedure */
	if (priority == 0)
		return -1;

	/* Higher priority procedure active */
	if (mme->current != NULL &&
	    mme->current->priority <= priority)
		return -1;

	if (mp->type != DECT_MMP_NONE)
		return -1;

	mp->type     = type;
	mp->priority = priority;
	mp->iec      = NULL;

	mme->current = mp;
	return 0;
}

static void dect_mm_procedure_complete(struct dect_handle *dh,
				       struct dect_mm_endpoint *mme)
{
	struct dect_mm_procedure *mp = mme->current;
	mm_debug(mme, "complete %s procedure", dect_mm_proc[mp->type].name);

	if (dect_timer_running(mp->timer))
		dect_timer_stop(dh, mp->timer);

	if (mp->iec != NULL)
		__dect_ie_collection_put(dh, mp->iec);

	dect_transaction_close(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
	mp->type = DECT_MMP_NONE;

	if (mme->procedure[!mp->role].type != DECT_MMP_NONE)
		mme->current = &mme->procedure[!mp->role];
	else
		mme->current = NULL;
}

#define dect_mm_procedure_cancel dect_mm_procedure_complete

static void dect_mm_procedure_timeout(struct dect_handle *dh,
				      struct dect_timer *timer)
{
	struct dect_mm_procedure *mp = timer->data;
	const struct dect_mm_proc *proc = &dect_mm_proc[mp->type];
	struct dect_mm_endpoint *mme;
	enum dect_mm_procedures type = mp->type;

	mme = container_of(mp, struct dect_mm_endpoint, procedure[mp->role]);
	dect_debug(DECT_DEBUG_MM, "\n");
	if (mp->retransmissions++ == 0) {
		mm_debug(mme, "timeout, retransmitting");
		dect_lce_retransmit(dh, &mp->transaction);
		dect_timer_start(dh, mp->timer, proc->param[dh->mode].timeout);
	} else {
		mm_debug(mme, "procedure timeout");
		dect_mm_procedure_complete(dh, mme);
		dect_mm_proc[type].abort(dh, mme, mp);
	}
}

static int dect_mm_procedure_init(struct dect_handle *dh,
				  struct dect_mm_endpoint *mme,
				  enum dect_transaction_role role)
{
	struct dect_mm_procedure *mp = &mme->procedure[role];

	mp->role  = role;
	mp->timer = dect_timer_alloc(dh);
	if (mp->timer == NULL)
		return -1;
	dect_timer_setup(mp->timer, dect_mm_procedure_timeout, mp);
	return 0;
}

/*
 * MM Endpoints
 */

/**
 * Get a pointer to the private data area from a Mobility Management Endpoint
 *
 * @param mme		Mobility Management Endpoint
 */
void *dect_mm_priv(struct dect_mm_endpoint *mme)
{
	return mme->priv;
}
EXPORT_SYMBOL(dect_mm_priv);

static struct dect_mm_endpoint *
dect_mm_endpoint_get_by_link(const struct dect_handle *dh,
			     const struct dect_data_link *link)
{
	struct dect_mm_endpoint *mme;

	list_for_each_entry(mme, &dh->mme_list, list) {
		if (mme->link == link)
			return mme;
	}
	return NULL;
}

struct dect_mm_endpoint *dect_mm_endpoint_get(struct dect_handle *dh,
					      const struct dect_ipui *ipui)
{
	struct dect_mm_endpoint *mme;

	list_for_each_entry(mme, &dh->mme_list, list) {
		if (!dect_ipui_cmp(&mme->link->ipui, ipui))
			return mme;
	}

	return dect_mm_endpoint_alloc(dh, ipui);
}
EXPORT_SYMBOL(dect_mm_endpoint_get);

struct dect_mm_endpoint *dect_mm_endpoint_alloc(struct dect_handle *dh,
						const struct dect_ipui *ipui)
{
	struct dect_mm_endpoint *mme;

	mme = dect_zalloc(dh, sizeof(*mme) + dh->ops->mm_ops->priv_size);
	if (mme == NULL)
		goto err1;
	if (dect_mm_procedure_init(dh, mme, DECT_TRANSACTION_INITIATOR) < 0)
		goto err2;
	if (dect_mm_procedure_init(dh, mme, DECT_TRANSACTION_RESPONDER) < 0)
		goto err3;

	if (ipui != NULL) {
		mme->link = dect_ddl_connect(dh, ipui);
		if (mme->link == NULL)
			goto err4;
	}

	list_add_tail(&mme->list, &dh->mme_list);
	return mme;

err4:
	dect_timer_free(dh, mme->procedure[DECT_TRANSACTION_RESPONDER].timer);
err3:
	dect_timer_free(dh, mme->procedure[DECT_TRANSACTION_INITIATOR].timer);
err2:
	dect_free(dh, mme);
err1:
	return NULL;
}
EXPORT_SYMBOL(dect_mm_endpoint_alloc);

void dect_mm_endpoint_destroy(struct dect_handle *dh,
			      struct dect_mm_endpoint *mme)
{
	list_del(&mme->list);
	dect_timer_free(dh, mme->procedure[DECT_TRANSACTION_RESPONDER].timer);
	dect_timer_free(dh, mme->procedure[DECT_TRANSACTION_INITIATOR].timer);
	dect_free(dh, mme);
}
EXPORT_SYMBOL(dect_mm_endpoint_destroy);

static struct dect_mm_endpoint *dect_mm_endpoint(struct dect_transaction *ta)
{
	return container_of(ta, struct dect_mm_endpoint, procedure[ta->role].transaction);
}

static int dect_mm_send_msg(const struct dect_handle *dh,
			    const struct dect_mm_endpoint *mme,
			    const struct dect_sfmt_msg_desc *desc,
			    const struct dect_msg_common *msg,
			    enum dect_mm_msg_types type)
{
	return dect_lce_send(dh, &mme->current->transaction, desc, msg, type);
}

#define dect_mm_send_reject(dh, mme, type, err)			\
({								\
	struct dect_ie_reject_reason reject_reason;		\
	struct dect_mm_ ## type ## _param reply = {		\
		.reject_reason = &reject_reason,		\
	};							\
								\
	reject_reason.reason = dect_sfmt_reject_reason(err);	\
	dect_mm_send_ ## type ## _reject(dh, mme, &reply);	\
	(void)0;						\
})

/**
 * @defgroup mm_key_allocation Key Allocation
 *
 * This module implements the key allocation procedure specified in
 * ETSI EN 300 175-5 section 13.6. The procedure works by performing
 * mutual authentication and storing the resulting session key as UAK:
 *
 * - The F-IWU invokes the @ref dect_mm_key_allocate_req() "MM_KEY_ALLOCATE-req"
 *   primitive, the F-MM entity sends a {KEY-ALLOCATE} message to the P-MM
 *   containing a @ref dect_ie_auth_value "<<RAND>>", @ref dect_ie_auth_value
 *   "<<RS>>" and @ref dect_ie_allocation_type "<<ALLOCATION-TYPE>>" information
 *   element.
 *
 * - The P-MM invokes the @ref dect_mm_ops::mm_key_allocate_ind()
 *   "MM_KEY_ALLOCATE-ind" primitive, the P-IWU responds with a
 *   @ref dect_mm_authenticate_req() "MM_AUTHENTICATE-req" primitive and sends
 *   a {AUTHENTICATION-REQUEST} message to the F-MM containing a
 *   @ref dect_ie_auth_value "<<RAND>>" and @ref dect_ie_auth_res "<<RES>>"
 *   information element.
 *
 * - The F-MM invokes the @ref dect_mm_ops::mm_authenticate_ind()
 *   "MM_AUTHENTICATE-ind" primitive. If the @ref dect_ie_auth_res "<<RES>>"
 *   value matches the expected value, the PT authentication is considered
 *   successful. The F-IWU responds with a @ref dect_mm_authenticate_res()
 *   "MM_AUTHENTICATE-res" primitive, the F-MM sends an {AUTHENTICATION-REPLY}
 *   message to the P-MM containing a @ref dect_ie_auth_res "<<RES>>"
 *   information element.
 *
 * - The P-MM invokes the @ref dect_mm_ops::mm_authenticate_cfm()
 *   "MM_AUTHENTICATE-cfm" primitive. If the @ref dect_ie_auth_res "<<RES>>"
 *   value matches the expected value, the FT authentication is considered
 *   successful. The P-IWU stores the reverse session key KS' as a new user
 *   authentication key under the UAK-number given in the
 *   @ref dect_ie_allocation_type "<<ALLOCATION-TYPE>>" information element.
 *
 * @msc
 *  "F-IWU", "F-MM", "P-MM", "P-IWU";
 *
 *  "F-IWU" => "F-MM"    [label="MM_KEY_ALLOCATE-req", URL="\ref dect_mm_key_allocate_req()"];
 *  "F-MM"  -> "P-MM"    [label="KEY-ALLOCATE"];
 *  "P-MM" =>> "P-IWU"   [label="MM_KEY_ALLOCATE-ind", URL="\ref dect_mm_ops::mm_key_allocate_ind"];
 *  "P-IWU" => "P-MM"    [label="MM_AUTHENTICATE-req", URL="\ref dect_mm_authenticate_req()"];
 *  "P-MM"  -> "F-MM"    [label="AUTHENTICATION-REQUEST"];
 *  "F-MM" =>> "F-IWU"   [label="MM_AUTHENTICATE-ind", URL="\ref dect_mm_ops::mm_authenticate_ind"];
 *  "F-IWU" => "F-MM"    [label="MM_AUTHENTICATE-res", URL="\ref dect_mm_authenticate_res()"];
 *  "F-MM"  -> "P-MM"    [label="AUTHENTICATION-REPLY"];
 *  "P-MM" =>> "P-IWU"   [label="MM_AUTHENTICATE-cfm", URL="\ref dect_mm_ops::mm_authenticate_cfm"];
 * @endmsc
 *
 * @sa @ref security "Security Features",
 * ETSI EN 300 175-7 (DECT Common Interface - Security Features)
 *
 * @{
 */

/**
 * MM_KEY_ALLOCATE-req primitive
 *
 * @param dh		libdect DECT handle
 * @param mme		Mobility Management Endpoint
 * @param param		key allocate request parameters
 *
 * Begin a key allocation procedure and send a {KEY-ALLOCATE} message to the PT.
 *
 * When the procedure is successfully accepted by the PT, it will respond by
 * requesting authentication, in which case the #dect_mm_ops::mm_authenticate_ind()
 * callback will be invoked. If the procedure is rejected or an error occurs,
 * the #dect_mm_ops::mm_authenticate_cfm() callback will be invoked with an 'accept'
 * parameter value of 'false'.
 *
 * The key allocation procedure may only be invoked by the FT.
 */
int dect_mm_key_allocate_req(struct dect_handle *dh,
			     struct dect_mm_endpoint *mme,
			     const struct dect_mm_key_allocate_param *param)
{
	struct dect_mm_key_allocate_msg msg;
	int err;

	mm_debug_entry(mme, "MM_KEY_ALLOCATE-req");
	err = dect_mm_procedure_initiate(dh, mme, DECT_MMP_KEY_ALLOCATION);
	if (err < 0)
		goto err1;

	memset(&msg, 0, sizeof(msg));
	msg.allocation_type		= param->allocation_type;
	msg.rand			= param->rand;
	msg.rs				= param->rs;
	msg.escape_to_proprietary	= param->escape_to_proprietary;

	err = dect_mm_send_msg(dh, mme, &mm_key_allocate_msg_desc,
			       &msg.common, DECT_MM_KEY_ALLOCATE);
	if (err < 0)
		goto err2;

	return 0;

err2:
	dect_mm_procedure_cancel(dh, mme);
err1:
	return err;
}
EXPORT_SYMBOL(dect_mm_key_allocate_req);

static void dect_mm_rcv_key_allocate(struct dect_handle *dh,
				     struct dect_mm_endpoint *mme,
				     struct dect_msg_buf *mb)
{
	struct dect_mm_key_allocate_msg msg;
	struct dect_mm_key_allocate_param *param;

	mm_debug(mme, "KEY-ALLOCATE");
	if (dect_mm_procedure_respond(dh, mme, DECT_MMP_KEY_ALLOCATION) < 0)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_key_allocate_msg_desc,
				&msg.common, mb) < 0)
		goto err1;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err2;

	param->allocation_type		= dect_ie_hold(msg.allocation_type);
	param->rand			= dect_ie_hold(msg.rand);
	param->rs			= dect_ie_hold(msg.rs);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	mm_debug(mme, "MM_KEY_ALLOCATE-ind");
	dh->ops->mm_ops->mm_key_allocate_ind(dh, mme, param);
	dect_ie_collection_put(dh, param);

	return dect_msg_free(dh, &mm_key_allocate_msg_desc, &msg.common);

err2:
	dect_msg_free(dh, &mm_key_allocate_msg_desc, &msg.common);
err1:
	dect_mm_procedure_cancel(dh, mme);
}

/**
 * @}
 * @defgroup mm_auth Authentication
 *
 * This module implements the authentication procedures specified in
 * ETSI EN 300 175-5 section 13.3. Authentication may be invoked by either
 * side, the procedure works as follows:
 *
 * - The IWU-1 invokes the @ref dect_mm_authenticate_req() "MM_AUTHENTICATE-req"
 *   primitive, the MM-1 sends a {AUTHENTICATION-REQUEST} message to the MM-2
 *   containing a @ref dect_ie_auth_value "<<RAND>>" and @ref dect_ie_auth_value
 *   "<<RS>>" information element.
 *
 * - The MM-2 invokes the @ref dect_mm_ops::mm_authenticate_ind()
 *   "MM_AUTHENTICATE-ind" primitive. The IWU-2 responds with a @ref dect_mm_authenticate_res()
 *   "MM_AUTHENTICATE-res" primitive, the MM-2 sends an {AUTHENTICATION-REPLY}
 *   message to the MM-1 containing a @ref dect_ie_auth_res "<<RES>>" information
 *   element.
 *
 * - The MM-1 invokes the @ref dect_mm_ops::mm_authenticate_cfm() "MM_AUTHENTICATE-cfm"
 *   primitive. If the @ref dect_ie_auth_res "<<RES>>" value matches the expected
 *   value, the authentication is considered successful.
 *
 * @msc
 *  "IWU-1", "MM-1", "MM-2", "IWU-2";
 *
 *  "IWU-1" => "MM-1"    [label="MM_AUTHENTICATE-req", URL="\ref dect_mm_authenticate_req()"];
 *  "MM-1"  -> "MM-2"    [label="AUTHENTICATION-REQUEST"];
 *  "MM-2" =>> "IWU-2"   [label="MM_AUTHENTICATE-ind", URL="\ref dect_mm_ops::mm_authenticate_ind"];
 *  "IWU-2" => "MM-2"    [label="MM_AUTHENTICATE-res", URL="\ref dect_mm_authenticate_res()"];
 *  "MM-2"  -> "MM-1"    [label="AUTHENTICATION-REPLY"];
 *  "MM-1" =>> "IWU-1"   [label="MM_AUTHENTICATE-cfm", URL="\ref dect_mm_ops::mm_authenticate_cfm"];
 * @endmsc
 *
 * @sa @ref security "Security Features",
 * ETSI EN 300 175-7 (DECT Common Interface - Security Features)
 *
 * @{
 */

/**
 * MM_AUTHENTICATE-req primitive
 *
 * @param dh		libdect DECT handle
 * @param mme		Mobility Management Endpoint
 * @param param		authenticate request parameters
 *
 * Begin an authentication procedure and send an {AUTHENTICATION-REQUEST}
 * message to the peer.
 *
 * When the procedure is successfully accepted by the peer, it will respond
 * with an {AUTHENTICATION-REPLY} message, in which case the
 * #dect_mm_ops::mm_authenticate_cfm() callback will be invoked with an 'accept'
 * parameter value of 'true'. If the procedure is rejected or an error occurs,
 * the dect_mm_ops::mm_authenticate_cfm() callback will be invoked with an
 * 'accept' parameter value of 'false'.
 */
int dect_mm_authenticate_req(struct dect_handle *dh,
			     struct dect_mm_endpoint *mme,
			     const struct dect_mm_authenticate_param *param)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_procedure *mpr = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_authentication_request_msg msg;
	int err;

	mm_debug_entry(mme, "MM_AUTHENTICATE-req");
	if (mpr->type == DECT_MMP_KEY_ALLOCATION)
		mp = mpr;
	else {
		err = dect_mm_procedure_initiate(dh, mme, DECT_MMP_AUTHENTICATE);
		if (err < 0)
			goto err1;
	}

	memset(&msg, 0, sizeof(msg));
	msg.auth_type			= param->auth_type;
	msg.rand			= param->rand;
	msg.res				= param->res;
	msg.rs				= param->rs;
	msg.cipher_info			= param->cipher_info;
	msg.iwu_to_iwu			= param->iwu_to_iwu;
	msg.escape_to_proprietary	= param->escape_to_proprietary;

	err = dect_mm_send_msg(dh, mme, &mm_authentication_request_msg_desc,
			       &msg.common, DECT_MM_AUTHENTICATION_REQUEST);
	if (err < 0)
		goto err2;

	return 0;

err2:
	dect_mm_procedure_cancel(dh, mme);
err1:
	return err;
}
EXPORT_SYMBOL(dect_mm_authenticate_req);

static int dect_mm_send_authenticate_reply(const struct dect_handle *dh,
					   const struct dect_mm_endpoint *mme,
					   const struct dect_mm_authenticate_param *param)
{
	struct dect_mm_authentication_reply_msg msg = {
		.res			= param->res,
		.rs			= param->rs,
		.zap_field		= param->zap_field,
		.service_class		= param->service_class,
		.key			= param->key,
		.iwu_to_iwu		= param->iwu_to_iwu,
		.escape_to_proprietary	= param->escape_to_proprietary,
	};

	return dect_mm_send_msg(dh, mme, &mm_authentication_reply_msg_desc,
				&msg.common, DECT_MM_AUTHENTICATION_REPLY);
}

static int dect_mm_send_authenticate_reject(const struct dect_handle *dh,
					    const struct dect_mm_endpoint *mme,
					    const struct dect_mm_authenticate_param *param)
{
	struct dect_mm_authentication_reject_msg msg = {
		//.auth_type		= param->auth_type,
		.reject_reason		= param->reject_reason,
		.iwu_to_iwu		= param->iwu_to_iwu,
		.escape_to_proprietary	= param->escape_to_proprietary,
	};

	return dect_mm_send_msg(dh, mme, &mm_authentication_reject_msg_desc,
				&msg.common, DECT_MM_AUTHENTICATION_REJECT);
}

/**
 * MM_AUTHENTICATE-res primitive
 *
 * @param dh		libdect DECT handle
 * @param mme		Mobility Management Endpoint
 * @param accept	accept/reject authentication
 * @param param		authenticate response parameters
 *
 * Respond to an authentication request and complete the authentication procedure.
 */
int dect_mm_authenticate_res(struct dect_handle *dh,
			     struct dect_mm_endpoint *mme, bool accept,
			     const struct dect_mm_authenticate_param *param)
{
	struct dect_mm_procedure *mp = mme->current;
	int err;

	mm_debug_entry(mme, "MM_AUTHENTICATE-res: accept: %u", accept);
	if (mp->type != DECT_MMP_AUTHENTICATE &&
	    mp->type != DECT_MMP_KEY_ALLOCATION)
		return -1;

	if (accept)
		err = dect_mm_send_authenticate_reply(dh, mme, param);
	else
		err = dect_mm_send_authenticate_reject(dh, mme, param);

	if (err < 0)
		return err;

	dect_mm_procedure_complete(dh, mme);
	return 0;
}
EXPORT_SYMBOL(dect_mm_authenticate_res);

static void dect_mm_rcv_authentication_request(struct dect_handle *dh,
					       struct dect_mm_endpoint *mme,
					       struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mpi = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_authentication_request_msg msg;
	struct dect_mm_authenticate_param *param;
	enum dect_sfmt_error err;

	mm_debug(mme, "AUTHENTICATION-REQUEST");
	if (mpi->type == DECT_MMP_KEY_ALLOCATION)
		mp = mpi;
	else if (dect_mm_procedure_respond(dh, mme, DECT_MMP_AUTHENTICATE) < 0)
		return;

	err = dect_parse_sfmt_msg(dh, &mm_authentication_request_msg_desc,
				  &msg.common, mb);
	if (err < 0)
		return dect_mm_send_reject(dh, mme, authenticate, err);

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->auth_type		= dect_ie_hold(msg.auth_type);
	param->rand			= dect_ie_hold(msg.rand);
	param->res			= dect_ie_hold(msg.res);
	param->rs			= dect_ie_hold(msg.rs);
	param->cipher_info		= dect_ie_hold(msg.cipher_info);
	param->iwu_to_iwu		= *dect_ie_list_hold(&msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	if (mp->type == DECT_MMP_NONE)
		mp->type = DECT_MMP_AUTHENTICATE;

	mm_debug(mme, "MM_AUTHENTICATE-ind");
	dh->ops->mm_ops->mm_authenticate_ind(dh, mme, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_authentication_request_msg_desc, &msg.common);
}

static void dect_mm_authentication_abort(struct dect_handle *dh,
					 struct dect_mm_endpoint *mme,
					 struct dect_mm_procedure *mp)

{
	struct dect_mm_authenticate_param param = {};

	mm_debug(mme, "MM_AUTHENTICATE-cfm: accept: 0");
	dh->ops->mm_ops->mm_authenticate_cfm(dh, mme, false, &param);
}

static void dect_mm_rcv_authentication_reply(struct dect_handle *dh,
					     struct dect_mm_endpoint *mme,
					     struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_procedure *mpr = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_authentication_reply_msg msg;
	struct dect_mm_authenticate_param *param;

	mm_debug(mme, "AUTHENTICATION-REPLY");
	if (mpr->type == DECT_MMP_KEY_ALLOCATION)
		mp = mpr;
	else if (mp->type != DECT_MMP_AUTHENTICATE)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_authentication_reply_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->res			= dect_ie_hold(msg.res);
	param->rs			= dect_ie_hold(msg.rs);
	param->zap_field		= dect_ie_hold(msg.zap_field);
	param->service_class		= dect_ie_hold(msg.service_class);
	param->key			= dect_ie_hold(msg.key);
	param->iwu_to_iwu		= *dect_ie_list_hold(&msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_mm_procedure_complete(dh, mme);

	mm_debug(mme, "MM_AUTHENTICATE-cfm: accept: 1");
	dh->ops->mm_ops->mm_authenticate_cfm(dh, mme, true, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_authentication_reply_msg_desc, &msg.common);
}

static void dect_mm_rcv_authentication_reject(struct dect_handle *dh,
					      struct dect_mm_endpoint *mme,
					      struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_procedure *mpr = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_authentication_reject_msg msg;
	struct dect_mm_authenticate_param *param;

	mm_debug(mme, "AUTHENTICATION-REJECT");
	if (mpr->type == DECT_MMP_KEY_ALLOCATION)
		mp = mpr;
	else if (mp->type != DECT_MMP_AUTHENTICATE)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_authentication_reject_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	//param->auth_type		= *dect_ie_list_hold(&msg.auth_type);
	param->reject_reason		= dect_ie_hold(msg.reject_reason);
	param->iwu_to_iwu		= *dect_ie_list_hold(&msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_mm_procedure_complete(dh, mme);

	mm_debug(mme, "MM_AUTHENTICATE-cfm: accept: 0");
	dh->ops->mm_ops->mm_authenticate_cfm(dh, mme, false, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_authentication_reject_msg_desc, &msg.common);
}

/**
 * @}
 * @defgroup mm_cipher Ciphering
 *
 * This module implements the ciphering related procedure specified in
 * ETSI EN 300 175-5 section 13.8.
 *
 * @msc
 *  "F-IWU", "F-MM", "F-DLC", "P-DLC", "P-MM", "P-IWU";
 *
 *  "F-IWU"  => "F-MM"   [label="MM_CIPHER-req", URL="\ref dect_mm_cipher_req()"];
 *  "F-MM"   => "F-DLC"  [label="DL_ENC_KEY-req"];
 *  "F-MM"   -> "P-MM"   [label="CIPHER-REQUEST"];
 *  "P-MM"  =>> "P-IWU"  [label="MM_CIPHER-ind", URL="\ref dect_mm_ops::mm_cipher_ind"];
 *  "P-IWU"  => "P-MM"   [label="MM_CIPHER-res", URL="\ref dect_mm_cipher_res()"];
 *  "P-MM"   => "P-DLC"  [label="DL_ENC_KEY-req"];
 *  "P-MM"   => "P-DLC"  [label="DL_ENCRYPT-req"];
 *  ...                  [label="Establish MAC bearer encryption"];
 *  "P-DLC" =>> "P-MM"   [label="DL_ENCRYPT-cfm"];
 *  "F-DLC" =>> "F-MM"   [label="DL_ENCRYPT-ind"];
 *  "F-MM"  =>> "F-IWU"  [label="MM_CIPHER-cfm", URL="\ref dect_mm_ops::mm_cipher_cfm"];
 * @endmsc
 *
 * @sa @ref security "Security Features",
 * ETSI EN 300 175-7 (DECT Common Interface - Security Features)
 *
 * @{
 */

/**
 * MM_CIPHER-req primitive
 *
 * @param dh		libdect DECT handle
 * @param mme		Mobility Management Endpoint
 * @param param		cipher request parameters
 * @param ck		cipher key
 */
int dect_mm_cipher_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
		       const struct dect_mm_cipher_param *param,
		       const uint8_t ck[])
{
	struct dect_mm_cipher_request_msg msg;
	int err;

	mm_debug_entry(mme, "MM_CIPHER-req");
	err = dect_mm_procedure_initiate(dh, mme, DECT_MMP_CIPHER);
	if (err < 0)
		goto err1;

	err = dect_ddl_set_cipher_key(mme->link, ck);
	if (err < 0)
		goto err2;

	memset(&msg, 0, sizeof(msg));
	msg.cipher_info			= param->cipher_info;
	msg.call_identity		= param->call_identity;
	msg.connection_identity		= param->connection_identity;
	msg.iwu_to_iwu			= param->iwu_to_iwu;
	msg.escape_to_proprietary	= param->escape_to_proprietary;

	/* cipher_request and cipher_suggest messages have identical layout */
	if (dh->mode == DECT_MODE_FP)
		err = dect_mm_send_msg(dh, mme, &mm_cipher_request_msg_desc,
				       &msg.common, DECT_MM_CIPHER_REQUEST);
	else
		err = dect_mm_send_msg(dh, mme, &mm_cipher_suggest_msg_desc,
				       &msg.common, DECT_MM_CIPHER_SUGGEST);

	if (err < 0)
		goto err2;

	return 0;

err2:
	dect_mm_procedure_cancel(dh, mme);
err1:
	return err;
}
EXPORT_SYMBOL(dect_mm_cipher_req);

static int dect_mm_send_cipher_reject(const struct dect_handle *dh,
				      const struct dect_mm_endpoint *mme,
				      const struct dect_mm_cipher_param *param)
{
	struct dect_mm_cipher_reject_msg msg = {
		//.cipher_info		= param->cipher_info,
		.reject_reason		= param->reject_reason,
		.escape_to_proprietary	= param->escape_to_proprietary,
	};

	return dect_mm_send_msg(dh, mme, &mm_cipher_reject_msg_desc,
				&msg.common, DECT_MM_CIPHER_REJECT);
}

/**
 * MM_CIPHER-res primitive
 *
 * @param dh		libdect DECT handle
 * @param mme		Mobility Management Endpoint
 * @param accept	accept/reject ciphering
 * @param param		cipher respond parameters
 * @param ck		cipher key
 */
int dect_mm_cipher_res(struct dect_handle *dh, struct dect_mm_endpoint *mme,
		       bool accept, const struct dect_mm_cipher_param *param,
		       const uint8_t ck[])
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	int err;

	mm_debug_entry(mme, "MM_CIPHER-res: accept: %u", accept);
	if (mp->type != DECT_MMP_CIPHER)
		return -1;

	if (accept) {
		err = dect_ddl_set_cipher_key(mme->link, ck);
		if (err < 0)
			goto err1;

		err = dect_ddl_encrypt_req(mme->link, DECT_CIPHER_ENABLED);
		if (err < 0)
			goto err1;
	} else
		err = dect_mm_send_cipher_reject(dh, mme, param);

	dect_mm_procedure_complete(dh, mme);
	return 0;

err1:
	return err;
}
EXPORT_SYMBOL(dect_mm_cipher_res);

static void dect_mm_rcv_cipher_request(struct dect_handle *dh,
				       struct dect_mm_endpoint *mme,
				       struct dect_msg_buf *mb)
{
	struct dect_mm_cipher_request_msg msg;
	struct dect_mm_cipher_param *param;
	enum dect_sfmt_error err;

	mm_debug(mme, "CIPHER-REQUEST");
	if (dect_mm_procedure_respond(dh, mme, DECT_MMP_CIPHER) < 0)
		return;

	err = dect_parse_sfmt_msg(dh, &mm_cipher_request_msg_desc,
				  &msg.common, mb);
	if (err < 0) {
		dect_mm_send_reject(dh, mme, cipher, err);
		goto err1;
	}

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err2;

	param->cipher_info		= dect_ie_hold(msg.cipher_info);
	param->call_identity		= dect_ie_hold(msg.call_identity);
	param->connection_identity	= dect_ie_hold(msg.connection_identity);
	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	mm_debug(mme, "MM_CIPHER-ind");
	dh->ops->mm_ops->mm_cipher_ind(dh, mme, param);
	dect_ie_collection_put(dh, param);

	return dect_msg_free(dh, &mm_cipher_request_msg_desc, &msg.common);

err2:
	dect_msg_free(dh, &mm_cipher_request_msg_desc, &msg.common);
err1:
	dect_mm_procedure_cancel(dh, mme);
}

static void dect_mm_rcv_cipher_suggest(struct dect_handle *dh,
				       struct dect_mm_endpoint *mme,
				       struct dect_msg_buf *mb)
{
	struct dect_mm_cipher_suggest_msg msg;
	struct dect_mm_cipher_param *param;

	mm_debug(mme, "CIPHER-SUGGEST");
	if (dect_mm_procedure_respond(dh, mme, DECT_MMP_CIPHER) < 0)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_cipher_suggest_msg_desc,
				&msg.common, mb) < 0)
		goto err1;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err2;

	param->cipher_info		= dect_ie_hold(msg.cipher_info);
	param->call_identity		= dect_ie_hold(msg.call_identity);
	param->connection_identity	= dect_ie_hold(msg.connection_identity);
	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	mm_debug(mme, "MM_CIPHER-ind");
	dh->ops->mm_ops->mm_cipher_ind(dh, mme, param);
	dect_ie_collection_put(dh, param);

	return dect_msg_free(dh, &mm_cipher_suggest_msg_desc, &msg.common);

err2:
	dect_msg_free(dh, &mm_cipher_suggest_msg_desc, &msg.common);
err1:
	dect_mm_procedure_cancel(dh, mme);
}

static void dect_mm_cipher_abort(struct dect_handle *dh,
				 struct dect_mm_endpoint *mme,
				 struct dect_mm_procedure *mp)
{
	struct dect_mm_cipher_param param = {};

	mm_debug(mme, "MM_CIPHER-cfm: accept: 0");
	dh->ops->mm_ops->mm_cipher_cfm(dh, mme, false, &param);
}

static void dect_mm_rcv_cipher_reject(struct dect_handle *dh,
				      struct dect_mm_endpoint *mme,
				      struct dect_msg_buf *mb)
{
	struct dect_mm_cipher_reject_msg msg;
	struct dect_mm_cipher_param *param;

	mm_debug(mme, "CIPHER-REJECT");
	if (dect_parse_sfmt_msg(dh, &mm_cipher_reject_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	//param->cipher_info		= dect_ie_hold(msg.cipher_info);
	param->reject_reason		= dect_ie_hold(msg.reject_reason);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_mm_procedure_complete(dh, mme);

	mm_debug(mme, "MM_CIPHER-cfm: accept: 0");
	dh->ops->mm_ops->mm_cipher_cfm(dh, mme, false, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_cipher_reject_msg_desc, &msg.common);
}

static void dect_mm_cipher_cfm(struct dect_handle *dh,
			       struct dect_mm_endpoint *mme)
{
	mm_debug(mme, "CIPHER-cfm");

	dect_mm_procedure_complete(dh, mme);

	mm_debug(mme, "MM_CIPHER-cfm: accept: 1");
	dh->ops->mm_ops->mm_cipher_cfm(dh, mme, true, NULL);
}

static void dect_mm_encrypt_ind(struct dect_handle *dh, struct dect_transaction *ta,
				enum dect_cipher_states state)
{
	struct dect_mm_endpoint *mme = dect_mm_endpoint(ta);
	struct dect_mm_procedure *mp = &mme->procedure[ta->role];

	if (mp->type == DECT_MMP_CIPHER)
		dect_mm_cipher_cfm(dh, mme);
}

/**
 * @}
 * @defgroup mm_access_rights Access Rights requests
 *
 * This module implements the access rights procedure specified in
 * ETSI EN 300 175-5 section 13.5.1.
 *
 * @msc
 *  "P-IWU", "P-MM", "F-MM", "F-IWU";
 *
 *  "P-IWU" => "P-MM"    [label="MM_ACCESS_RIGHTS-req", URL="\ref dect_mm_access_rights_req()"];
 *  "P-MM"  -> "F-MM"    [label="ACCESS-RIGHTS-REQUEST"];
 *  "F-MM" =>> "F-IWU"   [label="MM_ACCESS_RIGHTS-ind", URL="\ref dect_mm_ops::mm_access_rights_ind"];
 *  "F-IWU" => "F-MM"    [label="MM_ACCESS_RIGHTS-res", URL="\ref dect_mm_access_rights_res()"];
 *  "F-MM"  -> "P-MM"    [label="ACCESS-RIGHTS-ACCEPT"];
 *  "P-MM" =>> "P-IWU"   [label="MM_ACCESS_RIGHTS-cfm", URL="\ref dect_mm_ops::mm_access_rights_cfm"];
 * @endmsc
 *
 * @{
 */

/**
 * MM_ACCESS_RIGHTS-req primitive
 *
 * @param dh		libdect DECT handle
 * @param mme		Mobility Management Endpoint
 * @param param		access rights request parameters
 *
 * Begin an access rights procedure and send a {ACCESS-RIGHTS-REQUEST} message
 * to the FT.
 *
 * When the procedure is successfully accepted by the FT, it will by respond
 * with an {ACCESS-RIGHTS-ACCEPT} message, in which case the
 * #dect_mm_ops::mm_access_rights_cfm() callback will be invoked with an 'accept'
 * parameter value of 'true'. If the procedure is rejected or an error occurs, the
 * #dect_mm_ops::mm_access_rights_cfm() callback will be invoked with an 'accept'
 * parameter value of 'false'.
 *
 * The access rights procedure may only be invoked by the PT.
 */
int dect_mm_access_rights_req(struct dect_handle *dh,
			      struct dect_mm_endpoint *mme,
			      const struct dect_mm_access_rights_param *param)
{
	struct dect_mm_access_rights_request_msg msg;
	int err;

	mm_debug_entry(mme, "MM_ACCESS_RIGHTS-req");
	err = dect_mm_procedure_initiate(dh, mme, DECT_MMP_ACCESS_RIGHTS);
	if (err < 0)
		goto err1;

	memset(&msg, 0, sizeof(msg));
	msg.portable_identity		= param->portable_identity;
	msg.auth_type			= param->auth_type;
	msg.cipher_info			= param->cipher_info;
	msg.setup_capability		= param->setup_capability;
	msg.terminal_capability		= param->terminal_capability;
	msg.model_identifier		= param->model_identifier;
	msg.escape_to_proprietary	= param->escape_to_proprietary;
	msg.codec_list			= param->codec_list;

	err = dect_mm_send_msg(dh, mme, &mm_access_rights_request_msg_desc,
			       &msg.common, DECT_MM_ACCESS_RIGHTS_REQUEST);
	if (err < 0)
		goto err2;

	return 0;

err2:
	dect_mm_procedure_cancel(dh, mme);
err1:
	return err;
}
EXPORT_SYMBOL(dect_mm_access_rights_req);

static int dect_mm_send_access_rights_accept(const struct dect_handle *dh,
					     const struct dect_mm_endpoint *mme,
					     const struct dect_mm_access_rights_param *param)
{
	struct dect_ie_fixed_identity fixed_identity;
	struct dect_mm_access_rights_accept_msg msg = {
		.portable_identity	= param->portable_identity,
		.fixed_identity		= param->fixed_identity,
		.auth_type		= param->auth_type,
		.location_area		= param->location_area,
		.cipher_info		= param->cipher_info,
		.setup_capability	= param->setup_capability,
		.model_identifier	= param->model_identifier,
		//.iwu_to_iwu		= param->iwu_to_iwu,
		.escape_to_proprietary	= param->escape_to_proprietary,
		.codec_list		= param->codec_list,
	};

	if (param->fixed_identity.list == NULL) {
		fixed_identity.type = DECT_FIXED_ID_TYPE_PARK;
		fixed_identity.ari = dh->pari;
		fixed_identity.rpn = 0;
		dect_ie_list_add(&fixed_identity, &msg.fixed_identity);
	}

	return dect_mm_send_msg(dh, mme, &mm_access_rights_accept_msg_desc,
				&msg.common, DECT_MM_ACCESS_RIGHTS_ACCEPT);
}

static int dect_mm_send_access_rights_reject(const struct dect_handle *dh,
					     const struct dect_mm_endpoint *mme,
					     const struct dect_mm_access_rights_param *param)
{
	struct dect_mm_access_rights_reject_msg msg = {
		.reject_reason		= param->reject_reason,
		.duration		= param->duration,
		.iwu_to_iwu		= param->iwu_to_iwu,
		.escape_to_proprietary	= param->escape_to_proprietary,
	};

	return dect_mm_send_msg(dh, mme, &mm_access_rights_reject_msg_desc,
				&msg.common, DECT_MM_ACCESS_RIGHTS_REJECT);
}

/**
 * MM_ACCESS_RIGHTS-res primitive
 *
 * @param dh		libdect DECT handle
 * @param mme		Mobility Management Endpoint
 * @param accept	accept/reject access rights request
 * @param param		access rights response parameters
 *
 * Respond to an access rights request and complete the access rights procedure.
 */
int dect_mm_access_rights_res(struct dect_handle *dh,
			      struct dect_mm_endpoint *mme, bool accept,
			      const struct dect_mm_access_rights_param *param)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	int err;

	mm_debug_entry(mme, "MM_ACCESS_RIGHTS-res: accept: %u", accept);
	if (mp->type != DECT_MMP_ACCESS_RIGHTS)
		return -1;

	if (accept)
		err = dect_mm_send_access_rights_accept(dh, mme, param);
	else
		err = dect_mm_send_access_rights_reject(dh, mme, param);

	if (err < 0)
		return err;

	dect_mm_procedure_complete(dh, mme);
	return 0;
}
EXPORT_SYMBOL(dect_mm_access_rights_res);

static void dect_mm_rcv_access_rights_request(struct dect_handle *dh,
					      struct dect_mm_endpoint *mme,
					      struct dect_msg_buf *mb)
{
	struct dect_mm_access_rights_request_msg msg;
	struct dect_mm_access_rights_param *param;
	enum dect_sfmt_error err;

	mm_debug(mme, "ACCESS-RIGHTS-REQUEST");
	if (dect_mm_procedure_respond(dh, mme, DECT_MMP_ACCESS_RIGHTS) < 0)
		return;

	err = dect_parse_sfmt_msg(dh, &mm_access_rights_request_msg_desc,
				  &msg.common, mb);
	if (err < 0) {
		dect_mm_send_reject(dh, mme, access_rights, err);
		goto err1;
	}

	if (msg.portable_identity->type != DECT_PORTABLE_ID_TYPE_IPUI)
		goto err2;
	if (dect_ddl_set_ipui(dh, mme->link, &msg.portable_identity->ipui) < 0)
		goto err2;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err2;

	param->portable_identity	= dect_ie_hold(msg.portable_identity);
	param->auth_type		= dect_ie_hold(msg.auth_type);
	param->cipher_info		= dect_ie_hold(msg.cipher_info);
	param->setup_capability		= dect_ie_hold(msg.setup_capability);
	param->terminal_capability	= dect_ie_hold(msg.terminal_capability);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);
	param->codec_list		= dect_ie_hold(msg.codec_list);

	mm_debug(mme, "MM_ACCESS_RIGHTS-ind");
	dh->ops->mm_ops->mm_access_rights_ind(dh, mme, param);
	dect_ie_collection_put(dh, param);

	return dect_msg_free(dh, &mm_access_rights_request_msg_desc, &msg.common);

err2:
	dect_msg_free(dh, &mm_access_rights_request_msg_desc, &msg.common);
err1:
	dect_mm_procedure_complete(dh, mme);
}

static void dect_mm_access_rights_abort(struct dect_handle *dh,
					struct dect_mm_endpoint *mme,
					struct dect_mm_procedure *mp)
{
	struct dect_mm_access_rights_param param = {};

	mm_debug(mme, "MM_ACCESS_RIGHTS-cfm: accept: 0");
	dh->ops->mm_ops->mm_access_rights_cfm(dh, mme, false, &param);
}

static void dect_mm_rcv_access_rights_accept(struct dect_handle *dh,
					     struct dect_mm_endpoint *mme,
					     struct dect_msg_buf *mb)
{
	struct dect_mm_access_rights_accept_msg msg;
	struct dect_mm_access_rights_param *param;

	mm_debug(mme, "ACCESS-RIGHTS-ACCEPT");
	if (dect_parse_sfmt_msg(dh, &mm_access_rights_accept_msg_desc,
				&msg.common, mb) < 0)
		return;

	if (msg.portable_identity->type != DECT_PORTABLE_ID_TYPE_IPUI)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->portable_identity	= dect_ie_hold(msg.portable_identity);
	param->fixed_identity		= *dect_ie_list_hold(&msg.fixed_identity);
	param->location_area		= dect_ie_hold(msg.location_area);
	param->auth_type		= dect_ie_hold(msg.auth_type);
	param->cipher_info		= dect_ie_hold(msg.cipher_info);
	param->zap_field		= dect_ie_hold(msg.zap_field);
	param->service_class		= dect_ie_hold(msg.service_class);
	param->setup_capability		= dect_ie_hold(msg.setup_capability);
	param->model_identifier		= dect_ie_hold(msg.model_identifier);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);
	param->codec_list		= dect_ie_hold(msg.codec_list);

	dect_mm_procedure_complete(dh, mme);

	mm_debug(mme, "MM_ACCESS_RIGHTS-cfm: accept: 1");
	dh->ops->mm_ops->mm_access_rights_cfm(dh, mme, true, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_access_rights_accept_msg_desc, &msg.common);
}

static void dect_mm_rcv_access_rights_reject(struct dect_handle *dh,
					     struct dect_mm_endpoint *mme,
					     struct dect_msg_buf *mb)
{
	struct dect_mm_access_rights_reject_msg msg;
	struct dect_mm_access_rights_param *param;

	mm_debug(mme, "ACCESS-RIGHTS-REJECT");
	if (dect_parse_sfmt_msg(dh, &mm_access_rights_reject_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->reject_reason		= dect_ie_hold(msg.reject_reason);
	param->duration			= dect_ie_hold(msg.duration);
	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_mm_procedure_complete(dh, mme);

	mm_debug(mme, "MM_ACCESS_RIGHTS-cfm: accept: 0");
	dh->ops->mm_ops->mm_access_rights_cfm(dh, mme, false, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_access_rights_reject_msg_desc, &msg.common);
}

/**
 * @}
 * @defgroup mm_access_rights_terminate Access Rights termination
 *
 * This module implements the access rights termination procedure specified in
 * ETSI EN 300 175-5 section 13.5.2.
 *
 * @msc
 *  "IWU-1", "MM-1", "MM-2", "IWU-2";
 *
 *  "IWU-1" => "MM-1"    [label="MM_ACCESS_RIGHTS_TERMINATE-req", URL="\ref dect_mm_access_rights_terminate_req()"];
 *  "MM-1"  -> "MM-2"    [label="ACCESS-RIGHTS-TERMINATE-REQUEST"];
 *  "MM-2" =>> "IWU-2"   [label="MM_ACCESS_RIGHTS_TERMINATE-ind", URL="\ref dect_mm_ops::mm_access_rights_terminate_ind"];
 *  "IWU-2" => "MM-2"    [label="MM_ACCESS_RIGHTS_TERMINATE-res", URL="\ref dect_mm_access_rights_terminate_res()"];
 *  "MM-2"  -> "MM-1"    [label="ACCESS-RIGHTS-TERMINATE-ACCEPT"];
 *  "MM-1" =>> "IWU-1"   [label="MM_ACCESS_RIGHTS_TERMINATE-cfm", URL="\ref dect_mm_ops::mm_access_rights_terminate_cfm"];
 * @endmsc
 *
 * @{
 */

/**
 * MM_ACCESS_RIGHTS_TERMINATE-req primitive
 *
 * @param dh		libdect DECT handle
 * @param mme		Mobility Management Endpoint
 * @param param		access rights terminate request parameters
 *
 * Begin an access rights termination procedure and send a
 * {ACCESS-RIGHTS-TERMINATE-REQUEST} message.
 */
int dect_mm_access_rights_terminate_req(struct dect_handle *dh,
					struct dect_mm_endpoint *mme,
					const struct dect_mm_access_rights_terminate_param *param)
{
	struct dect_mm_access_rights_terminate_request_msg msg;
	struct dect_ie_fixed_identity fixed_identity;
	int err;

	mm_debug_entry(mme, "MM_ACCESS_RIGHTS_TERMINATE-req");
	err = dect_mm_procedure_initiate(dh, mme, DECT_MMP_ACCESS_RIGHTS_TERMINATE);
	if (err < 0)
		goto err1;

	memset(&msg, 0, sizeof(msg));
	msg.portable_identity		= param->portable_identity;
	msg.fixed_identity		= param->fixed_identity;
	msg.iwu_to_iwu			= param->iwu_to_iwu;
	msg.escape_to_proprietary	= param->escape_to_proprietary;

	if (param->fixed_identity.list == NULL) {
		fixed_identity.type = DECT_FIXED_ID_TYPE_PARK;
		fixed_identity.ari = dh->pari;
		fixed_identity.rpn = 0;
		dect_ie_list_add(&fixed_identity, &msg.fixed_identity);
	}

	err = dect_mm_send_msg(dh, mme, &mm_access_rights_terminate_request_msg_desc,
			       &msg.common, DECT_MM_ACCESS_RIGHTS_TERMINATE_REQUEST);
	if (err < 0)
		goto err2;

	return 0;

err2:
	dect_mm_procedure_cancel(dh, mme);
err1:
	return err;
}
EXPORT_SYMBOL(dect_mm_access_rights_terminate_req);

static int dect_mm_send_access_rights_terminate_accept(const struct dect_handle *dh,
						       const struct dect_mm_endpoint *mme,
						       const struct dect_mm_access_rights_terminate_param *param)
{
	struct dect_mm_access_rights_terminate_accept_msg msg = {
		.escape_to_proprietary	= param->escape_to_proprietary,
	};

	return dect_mm_send_msg(dh, mme, &mm_access_rights_terminate_accept_msg_desc,
				&msg.common, DECT_MM_ACCESS_RIGHTS_TERMINATE_ACCEPT);
}

static int dect_mm_send_access_rights_terminate_reject(const struct dect_handle *dh,
						       const struct dect_mm_endpoint *mme,
						       const struct dect_mm_access_rights_terminate_param *param)
{
	struct dect_mm_access_rights_terminate_reject_msg msg = {
		.reject_reason		= param->reject_reason,
		.duration		= param->duration,
		.escape_to_proprietary	= param->escape_to_proprietary,
	};

	return dect_mm_send_msg(dh, mme, &mm_access_rights_terminate_reject_msg_desc,
				&msg.common, DECT_MM_ACCESS_RIGHTS_TERMINATE_REJECT);
}

/**
 * MM_ACCESS_RIGHTS_TERMINATE-res primitive
 *
 * @param dh		libdect DECT handle
 * @param mme		Mobility Management Endpoint
 * @param accept	accept/reject access rights termination
 * @param param		access rights terminate response parameters
 *
 * Respond to an access rights termination request and complete the access
 * rights termination procedure.
 */
int dect_mm_access_rights_terminate_res(struct dect_handle *dh,
					struct dect_mm_endpoint *mme, bool accept,
					const struct dect_mm_access_rights_terminate_param *param)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	int err;

	mm_debug_entry(mme, "MM_ACCESS_RIGHTS_TERMINATE-res: accept: %u", accept);
	if (mp->type != DECT_MMP_ACCESS_RIGHTS_TERMINATE)
		return -1;

	if (accept)
		err = dect_mm_send_access_rights_terminate_accept(dh, mme, param);
	else
		err = dect_mm_send_access_rights_terminate_reject(dh, mme, param);

	if (err < 0)
		return err;

	dect_mm_procedure_complete(dh, mme);
	return 0;
}
EXPORT_SYMBOL(dect_mm_access_rights_terminate_res);

static void dect_mm_rcv_access_rights_terminate_request(struct dect_handle *dh,
							struct dect_mm_endpoint *mme,
							struct dect_msg_buf *mb)
{
	struct dect_mm_access_rights_terminate_request_msg msg;
	struct dect_mm_access_rights_terminate_param *param;
	enum dect_sfmt_error err;

	mm_debug(mme, "ACCESS-RIGHTS-TERMINATE-REQUEST");
	if (dect_mm_procedure_respond(dh, mme, DECT_MMP_ACCESS_RIGHTS_TERMINATE) < 0)
		return;

	err = dect_parse_sfmt_msg(dh, &mm_access_rights_terminate_request_msg_desc,
				  &msg.common, mb);
	if (err < 0) {
		dect_mm_send_reject(dh, mme, access_rights_terminate, err);
		goto err1;
	}

	if (msg.portable_identity->type != DECT_PORTABLE_ID_TYPE_IPUI)
		goto err2;
	if (dect_ddl_set_ipui(dh, mme->link, &msg.portable_identity->ipui) < 0)
		goto err2;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err2;

	param->portable_identity	= dect_ie_hold(msg.portable_identity);
	param->fixed_identity		= *dect_ie_list_hold(&msg.fixed_identity);
	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	mm_debug(mme, "MM_ACCESS_RIGHTS_TERMINATE-ind");
	dh->ops->mm_ops->mm_access_rights_terminate_ind(dh, mme, param);
	dect_ie_collection_put(dh, param);

	return dect_msg_free(dh, &mm_access_rights_terminate_request_msg_desc, &msg.common);

err2:
	dect_msg_free(dh, &mm_access_rights_terminate_request_msg_desc, &msg.common);
err1:
	dect_mm_procedure_complete(dh, mme);
}

static void dect_mm_access_rights_terminate_abort(struct dect_handle *dh,
						  struct dect_mm_endpoint *mme,
						  struct dect_mm_procedure *mp)
{
	struct dect_mm_access_rights_terminate_param param = {};

	mm_debug(mme, "MM_ACCESS_RIGHTS_TERMINATE-cfm: accept: 0");
	dh->ops->mm_ops->mm_access_rights_terminate_cfm(dh, mme, false, &param);
}

static void dect_mm_rcv_access_rights_terminate_accept(struct dect_handle *dh,
						       struct dect_mm_endpoint *mme,
						       struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_access_rights_terminate_accept_msg msg;
	struct dect_mm_access_rights_terminate_param *param;

	mm_debug(mme, "ACCESS-RIGHTS-TERMINATE-ACCEPT");
	if (mp->type != DECT_MMP_ACCESS_RIGHTS_TERMINATE)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_access_rights_terminate_accept_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_mm_procedure_complete(dh, mme);

	mm_debug(mme, "MM_ACCESS_RIGHTS_TERMINATE-cfm: accept: 1");
	dh->ops->mm_ops->mm_access_rights_terminate_cfm(dh, mme, true, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_access_rights_terminate_accept_msg_desc, &msg.common);
}

static void dect_mm_rcv_access_rights_terminate_reject(struct dect_handle *dh,
						       struct dect_mm_endpoint *mme,
						       struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_access_rights_terminate_reject_msg msg;
	struct dect_mm_access_rights_terminate_param *param;

	mm_debug(mme, "ACCESS-RIGHTS-TERMINATE-REJECT");
	if (mp->type != DECT_MMP_ACCESS_RIGHTS_TERMINATE)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_access_rights_terminate_reject_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->reject_reason		= dect_ie_hold(msg.reject_reason);
	param->duration			= dect_ie_hold(msg.duration);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_mm_procedure_complete(dh, mme);

	mm_debug(mme, "MM_ACCESS_RIGHTS_TERMINATE-cfm: accept: 0");
	dh->ops->mm_ops->mm_access_rights_terminate_cfm(dh, mme, false, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_access_rights_terminate_reject_msg_desc, &msg.common);
}

/**
 * @}
 * @defgroup mm_location_registration Location registration
 *
 * This module implements the location registration procedure specified in
 * ETSI EN 300 175-5 section 13.4.1.
 *
 * - The P-IWU invokes the @ref dect_mm_locate_req() "MM_LOCATE-req" primitive,
 *   the P-MM entity sends a {LOCATE-REQUEST} message to the F-MM containing a
 *   @ref dect_ie_portable_identity "<<PORTABLE-IDENTITY>>" and zero or more
 *   optional information elements.
 *
 * - The F-MM invokes the @ref dect_mm_ops::mm_locate_ind() "MM_LOCATE-ind"
 *   primitive, the F-IWU responds with a @ref dect_mm_locate_res()
 *   "MM_LOCATE-res" primitive and sends a {LOCATE-ACCEPT} message to the
 *   P-MM containing a @ref dect_ie_portable_identity "<<PORTABLE-IDENTITY>>",
 *   a @ref dect_ie_location_area "<<LOCATION-AREA>" and zero or more optional
 *   information elements.
 *
 * - The P-MM invokes the @ref dect_mm_ops::mm_locate_cfm() "MM_LOCATE-cfm"
 *   primitive. The P-IWU responds with a @ref dect_mm_locate_res "MM_LOCATE-res"
 *   primitive, the P-MM sends a {LOCATE-ACCEPT} message to the F-MM.
 *
 * - The F-MM invokes the @ref dect_mm_ops::mm_locate_cfm() "MM_LOCATE-cfm" primitive.
 *
 * @msc
 *  "P-IWU", "P-MM", "F-MM", "F-IWU";
 *
 *  "P-IWU" => "P-MM"    [label="MM_LOCATE-req", URL="\ref dect_mm_locate_req()"];
 *  "P-MM"  -> "F-MM"    [label="LOCATE-REQUEST"];
 *  "F-MM" =>> "F-IWU"   [label="MM_LOCATE-ind", URL="\ref dect_mm_ops::mm_locate_ind"];
 *  "F-IWU" => "F-MM"    [label="MM_LOCATE-res", URL="\ref dect_mm_locate_res()"];
 *  "F-MM"  -> "P-MM"    [label="LOCATE-ACCEPT"];
 *  "P-MM" =>> "P-IWU"   [label="MM_LOCATE-cfm", URL="\ref dect_mm_ops::mm_locate_cfm"];
 * @endmsc
 *
 * @{
 */

/**
 * MM_LOCATE-req primitive
 *
 * @param dh		libdect DECT handle
 * @param mme		Mobility Management Endpoint
 * @param param		locate request parameters
 */
int dect_mm_locate_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
		       const struct dect_mm_locate_param *param)
{
	struct dect_ie_fixed_identity fixed_identity;
	struct dect_mm_locate_request_msg msg;
	int err;

	mm_debug_entry(mme, "MM_LOCATE-req");
	err = dect_mm_procedure_initiate(dh, mme, DECT_MMP_LOCATION_REGISTRATION);
	if (err < 0)
		goto err1;

	fixed_identity.type = DECT_FIXED_ID_TYPE_PARK;
	memcpy(&fixed_identity.ari, &dh->pari, sizeof(fixed_identity.ari));

	memset(&msg, 0, sizeof(msg));
	msg.portable_identity		= param->portable_identity;
	msg.fixed_identity		= &fixed_identity;
	msg.location_area		= param->location_area;
	msg.nwk_assigned_identity	= param->nwk_assigned_identity;
	msg.cipher_info			= param->cipher_info;
	msg.setup_capability		= param->setup_capability;
	msg.terminal_capability		= param->terminal_capability;
	msg.iwu_to_iwu			= param->iwu_to_iwu;
	msg.model_identifier		= param->model_identifier;
	msg.escape_to_proprietary	= param->escape_to_proprietary;
	msg.codec_list			= param->codec_list;

	err = dect_mm_send_msg(dh, mme, &mm_locate_request_msg_desc,
			       &msg.common, DECT_MM_LOCATE_REQUEST);
	if (err < 0)
		goto err2;

	return 0;

err2:
	dect_mm_procedure_cancel(dh, mme);
err1:
	return err;
}
EXPORT_SYMBOL(dect_mm_locate_req);

static int dect_mm_send_locate_accept(const struct dect_handle *dh,
				      const struct dect_mm_endpoint *mme,
				      const struct dect_mm_locate_param *param)
{
	struct dect_mm_locate_accept_msg msg = {
		.portable_identity	= param->portable_identity,
		.location_area		= param->location_area,
		.nwk_assigned_identity	= param->nwk_assigned_identity,
		.duration		= param->duration,
		.iwu_to_iwu		= param->iwu_to_iwu,
		.escape_to_proprietary	= param->escape_to_proprietary,
		.codec_list		= param->codec_list,
		.model_identifier	= param->model_identifier,
	};

	return dect_mm_send_msg(dh, mme, &mm_locate_accept_msg_desc,
				&msg.common, DECT_MM_LOCATE_ACCEPT);
}

static int dect_mm_send_locate_reject(const struct dect_handle *dh,
				      const struct dect_mm_endpoint *mme,
				      const struct dect_mm_locate_param *param)
{
	struct dect_mm_locate_reject_msg msg = {
		.reject_reason		= param->reject_reason,
		.duration		= param->duration,
		.segmented_info		= {},
		.iwu_to_iwu		= param->iwu_to_iwu,
		.escape_to_proprietary	= param->escape_to_proprietary,
	};

	return dect_mm_send_msg(dh, mme, &mm_locate_reject_msg_desc,
				&msg.common, DECT_MM_LOCATE_REJECT);
}

/**
 * MM_LOCATE-res primitive
 *
 * @param dh		libdect DECT handle
 * @param mme		obility Management Endpoint
 * @param accept	accept/reject location registration/update
 * @param param		ccess rights response parameters
 */
int dect_mm_locate_res(struct dect_handle *dh, struct dect_mm_endpoint *mme,
		       bool accept, const struct dect_mm_locate_param *param)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	int err;

	mm_debug_entry(mme, "MM_LOCATE-res: accept: %u", accept);
	if (mp->type != DECT_MMP_LOCATION_REGISTRATION)
		return -1;

	if (accept)
		err = dect_mm_send_locate_accept(dh, mme, param);
	else
		err = dect_mm_send_locate_reject(dh, mme, param);

	if (err < 0)
		return err;

	/*
	 * TPUI or NWK identity assignment begins an identity assignment
	 * procedure.
	 */
	if (accept && param->portable_identity &&
	    (param->portable_identity->type == DECT_PORTABLE_ID_TYPE_TPUI ||
	     param->nwk_assigned_identity)) {
		mp->type = DECT_MMP_TEMPORARY_IDENTITY_ASSIGNMENT;
		return 0;
	}

	dect_mm_procedure_complete(dh, mme);
	return 0;
}
EXPORT_SYMBOL(dect_mm_locate_res);

static void dect_mm_rcv_locate_request(struct dect_handle *dh,
				       struct dect_mm_endpoint *mme,
				       struct dect_msg_buf *mb)
{
	struct dect_mm_locate_request_msg msg;
	struct dect_mm_locate_param *param;
	enum dect_sfmt_error err;

	mm_debug(mme, "LOCATE-REQUEST");
	if (dect_mm_procedure_respond(dh, mme, DECT_MMP_LOCATION_REGISTRATION) < 0)
		return;

	err = dect_parse_sfmt_msg(dh, &mm_locate_request_msg_desc,
				  &msg.common, mb);
	if (err < 0) {
		dect_mm_send_reject(dh, mme, locate, err);
		goto err1;
	}

	if (msg.portable_identity->type != DECT_PORTABLE_ID_TYPE_IPUI)
		goto err2;
	if (dect_ddl_set_ipui(dh, mme->link, &msg.portable_identity->ipui) < 0)
		goto err2;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err2;

	param->portable_identity	= dect_ie_hold(msg.portable_identity);
	param->fixed_identity		= dect_ie_hold(msg.fixed_identity);
	param->location_area		= dect_ie_hold(msg.location_area);
	param->nwk_assigned_identity	= dect_ie_hold(msg.nwk_assigned_identity);
	param->cipher_info		= dect_ie_hold(msg.cipher_info);
	param->setup_capability		= dect_ie_hold(msg.setup_capability);
	param->terminal_capability	= dect_ie_hold(msg.terminal_capability);
	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->model_identifier		= dect_ie_hold(msg.model_identifier);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);
	param->codec_list		= dect_ie_hold(msg.codec_list);

	mm_debug(mme, "MM_LOCATE-ind");
	dh->ops->mm_ops->mm_locate_ind(dh, mme, param);
	dect_ie_collection_put(dh, param);

	return dect_msg_free(dh, &mm_locate_request_msg_desc, &msg.common);

err2:
	dect_msg_free(dh, &mm_locate_request_msg_desc, &msg.common);
err1:
	dect_mm_procedure_complete(dh, mme);
}

static void dect_mm_locate_abort(struct dect_handle *dh,
				 struct dect_mm_endpoint *mme,
				 struct dect_mm_procedure *mp)
{
	struct dect_mm_locate_param param = {};

	mm_debug(mme, "MM_LOCATE-cfm: accept: 0");
	dh->ops->mm_ops->mm_locate_cfm(dh, mme, false, &param);
}

static void dect_mm_rcv_locate_accept(struct dect_handle *dh,
				      struct dect_mm_endpoint *mme,
				      struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_locate_accept_msg msg;
	struct dect_mm_locate_param *param;

	mm_debug(mme, "LOCATE-ACCEPT");
	if (mp->type != DECT_MMP_LOCATION_REGISTRATION)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_locate_accept_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->portable_identity	= dect_ie_hold(msg.portable_identity);
	param->location_area		= dect_ie_hold(msg.location_area);
	param->nwk_assigned_identity	= dect_ie_hold(msg.nwk_assigned_identity);
	param->setup_capability		= dect_ie_hold(msg.setup_capability);
	param->duration			= dect_ie_hold(msg.duration);
	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->model_identifier		= dect_ie_hold(msg.model_identifier);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);
	param->codec_list		= dect_ie_hold(msg.codec_list);

	/*
	 * TPUI or NWK identity assignment begins an identity assignment
	 * procedure.
	 */
	if ((param->portable_identity &&
	     param->portable_identity->type == DECT_PORTABLE_ID_TYPE_TPUI) ||
	    param->nwk_assigned_identity) {
		mp->iec  = dect_ie_collection_hold(param);
		mp->type = DECT_MMP_TEMPORARY_IDENTITY_ASSIGNMENT;
	} else
		dect_mm_procedure_complete(dh, mme);

	mm_debug(mme, "MM_LOCATE-cfm: accept: 1");
	dh->ops->mm_ops->mm_locate_cfm(dh, mme, true, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_locate_accept_msg_desc, &msg.common);
}

static void dect_mm_rcv_locate_reject(struct dect_handle *dh,
				      struct dect_mm_endpoint *mme,
				      struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_locate_reject_msg msg;
	struct dect_mm_locate_param *param;

	mm_debug(mme, "LOCATE-REJECT");
	if (mp->type != DECT_MMP_LOCATION_REGISTRATION)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_locate_reject_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->reject_reason		= dect_ie_hold(msg.reject_reason);
	param->duration			= dect_ie_hold(msg.duration);
	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_mm_procedure_complete(dh, mme);

	mm_debug(mme, "MM_LOCATE-cfm: accept: 0");
	dh->ops->mm_ops->mm_locate_cfm(dh, mme, false, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_locate_reject_msg_desc, &msg.common);
}

/**
 * @}
 * @defgroup mm_detach Detach
 *
 * This module implements the detach procedure specified in ETSI EN 300 175-5
 * section 13.4.2.
 *
 * @{
 */

/**
 * MM_DETACH-req primitive
 *
 * @param dh		libdect DECT handle
 * @param mme		Mobility Management Endpoint
 * @param param		detach parameters
 */
int dect_mm_detach_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
		       struct dect_mm_detach_param *param)
{
	struct dect_mm_detach_msg msg;
	int err;

	mm_debug_entry(mme, "MM_DETACH-req");
	err = dect_mm_procedure_initiate(dh, mme, DECT_MMP_DETACH);
	if (err < 0)
		goto err1;

	memset(&msg, 0, sizeof(msg));
	msg.portable_identity		= param->portable_identity;
	msg.nwk_assigned_identity	= param->nwk_assigned_identity;
	msg.iwu_to_iwu			= param->iwu_to_iwu;
	msg.escape_to_proprietary	= param->escape_to_proprietary;

	err = dect_mm_send_msg(dh, mme, &mm_detach_msg_desc,
			       &msg.common, DECT_MM_DETACH);

	dect_mm_procedure_complete(dh, mme);
err1:
	return err;
}
EXPORT_SYMBOL(dect_mm_detach_req);

static void dect_mm_rcv_detach(struct dect_handle *dh,
			       struct dect_mm_endpoint *mme,
			       struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_detach_param *param;
	struct dect_mm_detach_msg msg;

	mm_debug(mme, "MM_DETACH");
	if (dect_mm_procedure_respond(dh, mme, DECT_MMP_DETACH) < 0)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_detach_msg_desc,
				&msg.common, mb) < 0)
		goto err1;

	if (msg.portable_identity->type != DECT_PORTABLE_ID_TYPE_IPUI)
		goto err2;
	if (dect_ddl_set_ipui(dh, mme->link, &msg.portable_identity->ipui) < 0)
		goto err2;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err2;

	param->portable_identity	= dect_ie_hold(msg.portable_identity);
	param->nwk_assigned_identity	= dect_ie_hold(msg.nwk_assigned_identity);
	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_transaction_close(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);

	mm_debug(mme, "MM_DETACH-ind");
	dh->ops->mm_ops->mm_detach_ind(dh, mme, param);

	dect_ie_collection_put(dh, param);
err2:
	dect_msg_free(dh, &mm_detach_msg_desc, &msg.common);
err1:
	dect_mm_procedure_complete(dh, mme);
}

/**
 * @}
 * @defgroup mm_identification Identification
 *
 * This module implements the identification procedure specified in
 * ETSI EN 300 175-5 section 13.2.1.
 *
 * @msc
 *  "F-IWU", "F-MM", "P-MM", "P-IWU";
 *
 *  "F-IWU" => "F-MM"    [label="MM_IDENTITY-req", URL="\ref dect_mm_identity_req()"];
 *  "F-MM"  -> "P-MM"    [label="IDENTITY-REQUEST"];
 *  "P-MM" =>> "P-IWU"   [label="MM_IDENTITY-ind", URL="\ref dect_mm_ops::mm_identity_ind"];
 *  "P-IWU" => "P-MM"    [label="MM_IDENTITY-res", URL="\ref dect_mm_identity_res()"];
 *  "P-MM"  -> "F-MM"    [label="IDENTITY-ACCEPT"];
 *  "F-MM" =>> "F-IWU"   [label="MM_IDENTITY-cfm", URL="\ref dect_mm_ops::mm_identity_cfm"];
 * @endmsc
 *
 * @{
 */

/**
 * MM_IDENTITY-req primitive
 *
 * @param dh		libdect DECT handle
 * @param mme		Mobility Management Endpoint
 * @param param		identity request parameters
 */
int dect_mm_identity_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
			 const struct dect_mm_identity_param *param)
{
	struct dect_mm_identity_request_msg msg;
	int err;

	mm_debug_entry(mme, "MM_IDENTITY-req");
	err = dect_mm_procedure_initiate(dh, mme, DECT_MMP_IDENTIFICATION);
	if (err < 0)
		goto err1;

	memset(&msg, 0, sizeof(msg));
	msg.identity_type		= param->identity_type;
	msg.network_parameter		= param->network_parameter;
	msg.iwu_to_iwu			= param->iwu_to_iwu;
	msg.escape_to_proprietary	= param->escape_to_proprietary;

	err = dect_mm_send_msg(dh, mme, &mm_identity_request_msg_desc,
			       &msg.common, DECT_MM_IDENTITY_REQUEST);
	if (err < 0)
		goto err2;

	return 0;

err2:
	dect_mm_procedure_cancel(dh, mme);
err1:
	return err;
}
EXPORT_SYMBOL(dect_mm_identity_req);

/**
 * MM_IDENTITY_res primitive
 *
 * @param dh		libdect DECT handle
 * @param mme		Mobility Management Endpoint
 * @param param		identity response parameters
 */
int dect_mm_identity_res(struct dect_handle *dh,
			 struct dect_mm_endpoint *mme,
			 const struct dect_mm_identity_param *param)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_identity_reply_msg msg;
	int err;

	mm_debug_entry(mme, "MM_IDENTITY-res");
	if (mp->type != DECT_MMP_IDENTIFICATION)
		return -1;

	memset(&msg, 0, sizeof(msg));
	msg.portable_identity		= param->portable_identity;
	msg.fixed_identity		= param->fixed_identity;
	msg.nwk_assigned_identity	= param->nwk_assigned_identity;
	//msg.iwu_to_iwu		= param->iwu_to_iwu;
	msg.model_identifier		= param->model_identifier;
	msg.escape_to_proprietary	= param->escape_to_proprietary;

	err = dect_mm_send_msg(dh, mme, &mm_identity_reply_msg_desc,
			       &msg.common, DECT_MM_IDENTITY_REPLY);
	if (err < 0)
		return err;

	dect_mm_procedure_complete(dh, mme);
	return 0;
}
EXPORT_SYMBOL(dect_mm_identity_res);

static void dect_mm_rcv_identity_request(struct dect_handle *dh,
					 struct dect_mm_endpoint *mme,
					 struct dect_msg_buf *mb)
{
	struct dect_mm_identity_param *param;
	struct dect_mm_identity_request_msg msg;
	enum dect_sfmt_error err;

	mm_debug(mme, "IDENTITY-REQUEST");
	if (dect_mm_procedure_respond(dh, mme, DECT_MMP_IDENTIFICATION) < 0)
		return;

	err = dect_parse_sfmt_msg(dh, &mm_identity_request_msg_desc,
				  &msg.common, mb);
	if (err < 0)
		goto err1;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err2;

	param->identity_type		= *dect_ie_list_hold(&msg.identity_type);
	param->network_parameter	= dect_ie_hold(msg.network_parameter);
	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	mm_debug(mme, "MM_IDENTIY-ind");
	dh->ops->mm_ops->mm_identity_ind(dh, mme, param);
	dect_ie_collection_put(dh, param);

	return dect_msg_free(dh, &mm_identity_request_msg_desc, &msg.common);

err2:
	dect_msg_free(dh, &mm_identity_request_msg_desc, &msg.common);
err1:
	dect_mm_procedure_complete(dh, mme);
}

static void dect_mm_identity_abort(struct dect_handle *dh,
				   struct dect_mm_endpoint *mme,
				   struct dect_mm_procedure *mp)
{
	struct dect_mm_identity_param param = {};

	mm_debug(mme, "MM_IDENTITY-cfm");
	dh->ops->mm_ops->mm_identity_cfm(dh, mme, &param);
}

static void dect_mm_rcv_identity_reply(struct dect_handle *dh,
				       struct dect_mm_endpoint *mme,
				       struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_identity_param *param;
	struct dect_mm_identity_reply_msg msg;

	mm_debug(mme, "IDENTITY-REPLY");
	if (mp->type != DECT_MMP_IDENTIFICATION)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_identity_reply_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->portable_identity	= *dect_ie_list_hold(&msg.portable_identity);
	param->fixed_identity		= *dect_ie_list_hold(&msg.fixed_identity);
	param->nwk_assigned_identity	= *dect_ie_list_hold(&msg.nwk_assigned_identity);
	param->model_identifier		= dect_ie_hold(msg.model_identifier);
	//param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_mm_procedure_complete(dh, mme);

	mm_debug(mme, "MM_IDENTITY-cfm");
	dh->ops->mm_ops->mm_identity_cfm(dh, mme, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_identity_reply_msg_desc, &msg.common);
}

/**
 * @}
 * @defgroup mm_temporary_identity_assignment Temporary Identity assigment
 *
 * This module implements the temporary identity assigment procedure specified
 * in ETSI EN 300 175-5 section 13.2.2.
 *
 * @{
 */

/**
 * MM_IDENTITY_ASSIGN-req primitive
 *
 * @param dh		libdect DECT handle
 * @param mme		Mobility Management Endpoint
 * @param param		identity request parameters
 */
int dect_mm_identity_assign_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
				const struct dect_mm_identity_assign_param *param)
{
	struct dect_mm_temporary_identity_assign_msg msg;
	int err;

	mm_debug_entry(mme, "MM_IDENTITY_ASSIGN-req");
	err = dect_mm_procedure_initiate(dh, mme, DECT_MMP_TEMPORARY_IDENTITY_ASSIGNMENT);
	if (err < 0)
		goto err1;

	memset(&msg, 0, sizeof(msg));
	msg.portable_identity		= param->portable_identity;
	msg.location_area		= param->location_area;
	msg.nwk_assigned_identity	= param->nwk_assigned_identity;
	msg.duration			= param->duration;
	msg.network_parameter		= param->network_parameter;
	//msg.iwu_to_iwu		= param->iwu_to_iwu;
	msg.escape_to_proprietary	= param->escape_to_proprietary;

	err = dect_mm_send_msg(dh, mme, &mm_temporary_identity_assign_msg_desc,
			       &msg.common, DECT_MM_TEMPORARY_IDENTITY_ASSIGN);
	if (err < 0)
		goto err2;

	return 0;

err2:
	dect_mm_procedure_cancel(dh, mme);
err1:
	return err;
}
EXPORT_SYMBOL(dect_mm_identity_req);

static int dect_mm_send_temporary_identity_assign_ack(const struct dect_handle *dh,
						      const struct dect_mm_endpoint *mme,
						      const struct dect_mm_identity_assign_param *param)
{
	struct dect_mm_temporary_identity_assign_ack_msg msg = {
		.iwu_to_iwu		= param->iwu_to_iwu,
		.escape_to_proprietary	= param->escape_to_proprietary,
	};

	return dect_mm_send_msg(dh, mme, &mm_temporary_identity_assign_ack_msg_desc,
				&msg.common, DECT_MM_TEMPORARY_IDENTITY_ASSIGN_ACK);
}

static int dect_mm_send_temporary_identity_assign_rej(const struct dect_handle *dh,
						      const struct dect_mm_endpoint *mme,
						      const struct dect_mm_identity_assign_param *param)
{
	struct dect_mm_temporary_identity_assign_rej_msg msg = {
		.reject_reason		= param->reject_reason,
		.escape_to_proprietary	= param->escape_to_proprietary,
	};

	return dect_mm_send_msg(dh, mme, &mm_temporary_identity_assign_rej_msg_desc,
				&msg.common, DECT_MM_TEMPORARY_IDENTITY_ASSIGN_REJ);
}

/**
 * MM_IDENTITY_ASSIGN-res primitive
 *
 * @param dh		libdect DECT handle
 * @param mme		Mobility Management Endpoint
 * @param accept	accept/reject identity assignment
 * @param param		identity assigment response parameters
 */
int dect_mm_identity_assign_res(struct dect_handle *dh,
				struct dect_mm_endpoint *mme, bool accept,
				const struct dect_mm_identity_assign_param *param)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	int err;

	mm_debug_entry(mme, "MM_IDENTITY_ASSIGN-res: accept: %u", accept);
	if (mp->type != DECT_MMP_TEMPORARY_IDENTITY_ASSIGNMENT)
		return -1;

	if (accept) {
		struct dect_mm_locate_param *lp = (struct dect_mm_locate_param *)mp->iec;

		dect_assert(lp->portable_identity->type == DECT_PORTABLE_ID_TYPE_TPUI);
		dect_pp_set_tpui(dh, &lp->portable_identity->tpui);
		err = dect_mm_send_temporary_identity_assign_ack(dh, mme, param);
	} else
		err = dect_mm_send_temporary_identity_assign_rej(dh, mme, param);

	if (err < 0)
		return err;

	dect_mm_procedure_complete(dh, mme);
	return 0;
}
EXPORT_SYMBOL(dect_mm_identity_assign_res);

#define dect_mm_send_identity_assign_reject \
	dect_mm_send_temporary_identity_assign_rej

static void dect_mm_rcv_temporary_identity_assign(struct dect_handle *dh,
						  struct dect_mm_endpoint *mme,
						  struct dect_msg_buf *mb)
{
	struct dect_mm_temporary_identity_assign_msg msg;
	struct dect_mm_identity_assign_param *param;
	enum dect_sfmt_error err;

	mm_debug(mme, "TEMPORARY_IDENTITY_ASSIGN");
	if (dect_mm_procedure_respond(dh, mme, DECT_MMP_TEMPORARY_IDENTITY_ASSIGNMENT) < 0)
		return;

	err = dect_parse_sfmt_msg(dh, &mm_temporary_identity_assign_msg_desc,
				  &msg.common, mb);
	if (err < 0) {
		dect_mm_send_reject(dh, mme, identity_assign, err);
		goto err1;
	}

	if (msg.portable_identity) {
		if (msg.portable_identity->type != DECT_PORTABLE_ID_TYPE_IPUI)
			goto err2;
		if (dect_ddl_set_ipui(dh, mme->link, &msg.portable_identity->ipui) < 0)
			goto err2;
	}

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err2;

	param->portable_identity	= dect_ie_hold(msg.portable_identity);
	param->location_area		= dect_ie_hold(msg.location_area);
	param->nwk_assigned_identity	= dect_ie_hold(msg.nwk_assigned_identity);
	param->duration			= dect_ie_hold(msg.duration);
	param->network_parameter	= dect_ie_hold(msg.network_parameter);
	//param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	mm_debug(mme, "MM_IDENTITY_ASSIGN-ind");
	dh->ops->mm_ops->mm_identity_assign_ind(dh, mme, param);
	dect_ie_collection_put(dh, param);

	return dect_msg_free(dh, &mm_temporary_identity_assign_msg_desc, &msg.common);

err2:
	dect_msg_free(dh, &mm_temporary_identity_assign_msg_desc, &msg.common);
err1:
	dect_mm_procedure_complete(dh, mme);
}

static void dect_mm_temporary_identity_assign_abort(struct dect_handle *dh,
						    struct dect_mm_endpoint *mme,
						    struct dect_mm_procedure *mp)
{
	struct dect_mm_identity_assign_param param = {};

	mm_debug(mme, "MM_IDENTITY_ASSIGN-cfm: accept: 0");
	dh->ops->mm_ops->mm_identity_assign_cfm(dh, mme, false, &param);
}

static void dect_mm_rcv_temporary_identity_assign_ack(struct dect_handle *dh,
						      struct dect_mm_endpoint *mme,
						      struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_temporary_identity_assign_ack_msg msg;
	struct dect_mm_identity_assign_param *param;

	mm_debug(mme, "TEMPORARY-IDENTITY-ASSIGN-ACK");
	if (mp->type != DECT_MMP_TEMPORARY_IDENTITY_ASSIGNMENT)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_temporary_identity_assign_ack_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_mm_procedure_complete(dh, mme);

	mm_debug(mme, "MM_IDENTITY_ASSIGN-cfm: accept: 1");
	dh->ops->mm_ops->mm_identity_assign_cfm(dh, mme, true, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_temporary_identity_assign_ack_msg_desc, &msg.common);
}

static void dect_mm_rcv_temporary_identity_assign_rej(struct dect_handle *dh,
						      struct dect_mm_endpoint *mme,
						      struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_temporary_identity_assign_rej_msg msg;
	struct dect_mm_identity_assign_param *param;

	mm_debug(mme, "TEMPORARY-IDENTITY-ASSIGN-REJ");
	if (mp->type != DECT_MMP_TEMPORARY_IDENTITY_ASSIGNMENT)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_temporary_identity_assign_rej_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->reject_reason 		= dect_ie_hold(msg.reject_reason);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_mm_procedure_complete(dh, mme);

	mm_debug(mme, "MM_IDENTITY_ASSIGN-cfm: accept: 0");
	dh->ops->mm_ops->mm_identity_assign_cfm(dh, mme, false, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_temporary_identity_assign_rej_msg_desc, &msg.common);
}

/**
 * @}
 * @defgroup mm_parameter_retrieval Parameter retrieval
 *
 * This module implements the parameter retrieval procedure specified in
 * ETSI EN 300 175-5 section 13.7.
 *
 * @{
 */

/**
 * MM_INFO-req primitive
 *
 * @param dh		libdect DECT handle
 * @param mme		Mobility Management Endpoint
 * @param param		info parameters
 */
int dect_mm_info_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
		     struct dect_mm_info_param *param)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	int err;

	mm_debug_entry(mme, "MM_INFO-req");
	if (mp->type != DECT_MMP_NONE)
		return -1;

	err = dect_mm_procedure_initiate(dh, mme, DECT_MMP_PARAMETER_RETRIEVAL);
	if (err < 0)
		goto err1;

	if (dh->mode == DECT_MODE_FP) {
		struct dect_mm_info_suggest_msg msg;

		memset(&msg, 0, sizeof(msg));
		msg.info_type			= param->info_type;
		msg.call_identity		= param->call_identity;
		//msg.fixed_identity		= param->fixed_identity;
		msg.location_area		= param->location_area;
		msg.nwk_assigned_identity	= param->nwk_assigned_identity;
		msg.network_parameter		= param->network_parameter;
		msg.iwu_to_iwu			= param->iwu_to_iwu;
		msg.escape_to_proprietary	= param->escape_to_proprietary;

		err = dect_mm_send_msg(dh, mme, &mm_info_suggest_msg_desc,
				       &msg.common, DECT_MM_INFO_SUGGEST);
		if (err < 0)
			goto err2;

		dect_mm_procedure_complete(dh, mme);
	} else {
		struct dect_mm_info_request_msg msg;

		memset(&msg, 0, sizeof(msg));
		msg.info_type			= param->info_type;
		msg.call_identity		= param->call_identity;
		msg.portable_identity		= param->portable_identity;
		msg.fixed_identity		= param->fixed_identity;
		msg.location_area		= param->location_area;
		msg.nwk_assigned_identity	= param->nwk_assigned_identity;
		msg.network_parameter		= param->network_parameter;
		msg.iwu_to_iwu			= param->iwu_to_iwu;
		msg.escape_to_proprietary	= param->escape_to_proprietary;

		err = dect_mm_send_msg(dh, mme, &mm_info_request_msg_desc,
				       &msg.common, DECT_MM_INFO_REQUEST);
		if (err < 0)
			goto err2;
	}

	return 0;

err2:
	dect_mm_procedure_cancel(dh, mme);
err1:
	return err;
}
EXPORT_SYMBOL(dect_mm_info_req);

static int dect_mm_send_info_accept(const struct dect_handle *dh,
				    const struct dect_mm_endpoint *mme,
				    const struct dect_mm_info_param *param)
{
	struct dect_mm_info_accept_msg msg = {
		.info_type		= param->info_type,
		.call_identity		= param->call_identity,
		.fixed_identity		= param->fixed_identity,
		.location_area		= param->location_area,
		.nwk_assigned_identity	= param->nwk_assigned_identity,
		.network_parameter	= param->network_parameter,
		.iwu_to_iwu		= param->iwu_to_iwu,
		.escape_to_proprietary	= param->escape_to_proprietary,
	};

	return dect_mm_send_msg(dh, mme, &mm_info_accept_msg_desc,
				&msg.common, DECT_MM_INFO_ACCEPT);
}

static int dect_mm_send_info_reject(const struct dect_handle *dh,
				    const struct dect_mm_endpoint *mme,
				    const struct dect_mm_info_param *param)
{
	struct dect_mm_info_reject_msg msg = {
		.call_identity		= param->call_identity,
		.reject_reason		= param->reject_reason,
		.iwu_to_iwu		= param->iwu_to_iwu,
		.escape_to_proprietary	= param->escape_to_proprietary,
	};

	return dect_mm_send_msg(dh, mme, &mm_info_reject_msg_desc,
				&msg.common, DECT_MM_INFO_REJECT);
}

/**
 * MM_INFO-res primitive
 *
 * @param dh		libdect DECT handle
 * @param mme		Mobility Management Endpoint
 * @param accept	accept/reject info request
 * @param param		info parameters
 */
int dect_mm_info_res(struct dect_handle *dh, struct dect_mm_endpoint *mme,
		     bool accept, struct dect_mm_info_param *param)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	int err;

	mm_debug_entry(mme, "MM_INFO-res: accept: %u", accept);
	if (mp->type != DECT_MMP_PARAMETER_RETRIEVAL)
		return -1;

	if (accept)
		err = dect_mm_send_info_accept(dh, mme, param);
	else
		err = dect_mm_send_info_reject(dh, mme, param);

	if (err < 0)
		return err;

	dect_mm_procedure_complete(dh, mme);
	return 0;
}
EXPORT_SYMBOL(dect_mm_info_res);

static void dect_mm_rcv_info_request(struct dect_handle *dh,
				     struct dect_mm_endpoint *mme,
				     struct dect_msg_buf *mb)
{
	struct dect_mm_info_request_msg msg;
	struct dect_mm_info_param *param;
	enum dect_sfmt_error err;

	mm_debug(mme, "INFO-REQUEST");
	if (dect_mm_procedure_respond(dh, mme, DECT_MMP_PARAMETER_RETRIEVAL) < 0)
		return;

	err = dect_parse_sfmt_msg(dh, &mm_info_request_msg_desc,
				  &msg.common, mb);
	if (err < 0) {
		dect_mm_send_reject(dh, mme, info, err);
		goto err1;
	}

	if (msg.portable_identity != NULL) {
		if (msg.portable_identity->type != DECT_PORTABLE_ID_TYPE_IPUI)
			goto err2;
		if (dect_ddl_set_ipui(dh, mme->link, &msg.portable_identity->ipui) < 0)
			goto err2;
	}

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err2;

	param->info_type		= dect_ie_hold(msg.info_type);
	param->call_identity		= dect_ie_hold(msg.call_identity);
	param->portable_identity	= dect_ie_hold(msg.portable_identity);
	param->fixed_identity		= *dect_ie_list_hold(&msg.fixed_identity);
	param->location_area		= dect_ie_hold(msg.location_area);
	param->nwk_assigned_identity	= dect_ie_hold(msg.nwk_assigned_identity);
	param->network_parameter	= dect_ie_hold(msg.network_parameter);
	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	mm_debug(mme, "MM_INFO-ind");
	dh->ops->mm_ops->mm_info_ind(dh, mme, param);
	dect_ie_collection_put(dh, param);

	return dect_msg_free(dh, &mm_info_request_msg_desc, &msg.common);

err2:
	dect_msg_free(dh, &mm_info_request_msg_desc, &msg.common);
err1:
	dect_mm_procedure_complete(dh, mme);
}

static void dect_mm_info_abort(struct dect_handle *dh,
			       struct dect_mm_endpoint *mme,
			       struct dect_mm_procedure *mp)
{
	struct dect_mm_info_param param = {};

	mm_debug(mme, "MM_INFO-cfm: accept: 0");
	dh->ops->mm_ops->mm_info_cfm(dh, mme, false, &param);
}

static void dect_mm_rcv_info_accept(struct dect_handle *dh,
				    struct dect_mm_endpoint *mme,
				    struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_info_accept_msg msg;
	struct dect_mm_info_param *param;

	mm_debug(mme, "INFO-ACCEPT");
	if (mp->type != DECT_MMP_PARAMETER_RETRIEVAL)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_info_accept_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->info_type		= dect_ie_hold(msg.info_type);
	param->call_identity		= dect_ie_hold(msg.call_identity);
	param->fixed_identity		= *dect_ie_list_hold(&msg.fixed_identity);
	param->location_area		= dect_ie_hold(msg.location_area);
	param->nwk_assigned_identity	= dect_ie_hold(msg.nwk_assigned_identity);
	param->network_parameter	= dect_ie_hold(msg.network_parameter);
	param->duration			= dect_ie_hold(msg.duration);
	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_mm_procedure_complete(dh, mme);

	mm_debug(mme, "MM_INFO-cfm: accept: 1");
	dh->ops->mm_ops->mm_info_cfm(dh, mme, true, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_info_accept_msg_desc, &msg.common);
}

static void dect_mm_rcv_info_reject(struct dect_handle *dh,
				    struct dect_mm_endpoint *mme,
				    struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_info_reject_msg msg;
	struct dect_mm_info_param *param;

	mm_debug(mme, "INFO-REJECT");
	if (mp->type != DECT_MMP_PARAMETER_RETRIEVAL)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_info_reject_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->call_identity		= dect_ie_hold(msg.call_identity);
	param->reject_reason		= dect_ie_hold(msg.reject_reason);
	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_mm_procedure_complete(dh, mme);

	mm_debug(mme, "MM_INFO-cfm: accept: 0");
	dh->ops->mm_ops->mm_info_cfm(dh, mme, false, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_info_reject_msg_desc, &msg.common);
}

static void dect_mm_rcv_info_suggest(struct dect_handle *dh,
				     struct dect_mm_endpoint *mme,
				     struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_info_suggest_msg msg;
	struct dect_mm_info_param *param;

	mm_debug(mme, "INFO-SUGGEST");
	if (mp->type != DECT_MMP_NONE)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_info_suggest_msg_desc,
				&msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->info_type		= dect_ie_hold(msg.info_type);
	param->call_identity		= dect_ie_hold(msg.call_identity);
	//param->fixed_identity		= dect_ie_hold(msg.fixed_identity);
	param->location_area		= dect_ie_hold(msg.location_area);
	param->nwk_assigned_identity	= dect_ie_hold(msg.nwk_assigned_identity);
	param->network_parameter	= dect_ie_hold(msg.network_parameter);
	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_transaction_close(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);

	mm_debug(mme, "MM_INFO-ind");
	dh->ops->mm_ops->mm_info_ind(dh, mme, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_info_suggest_msg_desc, &msg.common);
}

/**
 * @}
 * @defgroup mm_external_protocol_information External protocol information
 *
 * This module implements the external protocol information procedure specified
 * in ETSI EN 300 175-5 section 13.9.
 *
 * @{
 */

/**
 * MM_IWU-req primitive
 *
 * @param dh		libdect DECT handle
 * @param mme		Mobility Management Endpoint
 * @param param		IWU request parameters
 */
int dect_mm_iwu_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
		    const struct dect_mm_iwu_param *param)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_INITIATOR];
	struct dect_mm_iwu_msg msg;
	int err;

	mm_debug_entry(mme, "MM_IWU-req");
	if (mp->type != DECT_MMP_NONE)
		return -1;

	err = dect_ddl_transaction_open(dh, &mp->transaction, mme->link,
					DECT_PD_MM);
	if (err < 0)
		goto err1;

	memset(&msg, 0, sizeof(msg));
	msg.iwu_to_iwu			= param->iwu_to_iwu;
	msg.iwu_packet			= param->iwu_packet;
	msg.escape_to_proprietary	= param->escape_to_proprietary;

	err = dect_mm_send_msg(dh, mme, &mm_iwu_msg_desc,
			       &msg.common, DECT_MM_IWU);

	dect_transaction_close(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);
err1:
	return err;
}
EXPORT_SYMBOL(dect_mm_iwu_req);

static void dect_mm_rcv_iwu(struct dect_handle *dh,
			    struct dect_mm_endpoint *mme,
			    struct dect_msg_buf *mb)
{
	struct dect_mm_procedure *mp = &mme->procedure[DECT_TRANSACTION_RESPONDER];
	struct dect_mm_iwu_param *param;
	struct dect_mm_iwu_msg msg;

	mm_debug(mme, "MM_IWU");
	if (mp->type != DECT_MMP_NONE)
		return;

	if (dect_parse_sfmt_msg(dh, &mm_iwu_msg_desc, &msg.common, mb) < 0)
		return;

	param = dect_ie_collection_alloc(dh, sizeof(*param));
	if (param == NULL)
		goto err1;

	param->iwu_to_iwu		= dect_ie_hold(msg.iwu_to_iwu);
	param->iwu_packet		= dect_ie_hold(msg.iwu_packet);
	param->escape_to_proprietary	= dect_ie_hold(msg.escape_to_proprietary);

	dect_transaction_close(dh, &mp->transaction, DECT_DDL_RELEASE_PARTIAL);

	mm_debug(mme, "MM_IWU-ind");
	dh->ops->mm_ops->mm_iwu_ind(dh, mme, param);
	dect_ie_collection_put(dh, param);
err1:
	dect_msg_free(dh, &mm_iwu_msg_desc, &msg.common);
}

/** @} */

static const struct dect_mm_proc dect_mm_proc[DECT_MMP_MAX + 1] = {
	[DECT_MMP_ACCESS_RIGHTS] = {
		.name	= "access rights",
		.abort	= dect_mm_access_rights_abort,
		.param	= {
			[DECT_MODE_PP] = {
				.priority	= 3,
				.timeout	= 60,
			},
		},
	},
	[DECT_MMP_ACCESS_RIGHTS_TERMINATE] = {
		.name	= "access rights terminate",
		.abort	= dect_mm_access_rights_terminate_abort,
		.param	= {
			[DECT_MODE_FP] = {
				.priority	= 2,
				.timeout	= 10,
			},
			[DECT_MODE_PP] = {
				.priority	= 3,
				.timeout	= 20,
			},
		},
	},
	[DECT_MMP_AUTHENTICATE] = {
		.name	= "authentication",
		.abort	= dect_mm_authentication_abort,
		.param	= {
			[DECT_MODE_FP] = {
				.priority	= 2,
				.timeout	= 10,
			},
			[DECT_MODE_PP] = {
				.priority	= 1,
				.timeout	= 10,
			},
		},
	},
	[DECT_MMP_AUTHENTICATE_USER] = {
		.name	= "user authentication",
		.abort	= dect_mm_authentication_abort,
		.param	= {
			[DECT_MODE_FP] = {
				.priority	= 2,
				.timeout	= 100,
			},
		},
	},
	[DECT_MMP_KEY_ALLOCATION]  {
		.name	= "key allocation",
		.abort	= dect_mm_authentication_abort,
		.param	= {
			[DECT_MODE_FP] = {
				.priority	= 2,
				.timeout	= 10,
			},
		},
	},
	[DECT_MMP_LOCATION_REGISTRATION] = {
		.name	= "location registration",
		.abort	= dect_mm_locate_abort,
		.param	= {
			[DECT_MODE_PP] = {
				.priority	= 3,
				.timeout	= 20,
			},
		},
	},
	[DECT_MMP_TEMPORARY_IDENTITY_ASSIGNMENT] = {
		.name	= "temporary identity assignment",
		.abort	= dect_mm_temporary_identity_assign_abort,
		.param	= {
			[DECT_MODE_FP] = {
				.priority	= 2,
				.timeout	= 10,
			},
		},
	},
	[DECT_MMP_IDENTIFICATION] = {
		.name	= "identification",
		.abort	= dect_mm_identity_abort,
		.param	= {
			[DECT_MODE_FP] = {
				.priority	= 2,
				.timeout	= 10,
			},
		},
	},
	[DECT_MMP_CIPHER] = {
		.name	= "ciphering",
		.abort	= dect_mm_cipher_abort,
		.param	= {
			[DECT_MODE_FP] = {
				.priority	= 2,
				.timeout	= 10,
			},
			[DECT_MODE_PP] = {
				.priority	= 3,
				.timeout	= 10,
			},
		},
	},
	[DECT_MMP_PARAMETER_RETRIEVAL] = {
		.name	= "parameter retrieval",
		.abort	= dect_mm_info_abort,
		.param	= {
			[DECT_MODE_FP] = {
				.priority	= 2,
				.timeout	= 10,
			},
			[DECT_MODE_PP] = {
				.priority	= 3,
				.timeout	= 20,
			},
		},
	},
	[DECT_MMP_DETACH] = {
		.name	= "detach",
		.param	= {
			[DECT_MODE_PP] = {
				.priority	= 3,
			},
		},
	},
};

static void dect_mm_rcv(struct dect_handle *dh, struct dect_transaction *ta,
			struct dect_msg_buf *mb)
{
	struct dect_mm_endpoint *mme = dect_mm_endpoint(ta);

	switch (mb->type) {
	case DECT_MM_AUTHENTICATION_REQUEST:
		return dect_mm_rcv_authentication_request(dh, mme, mb);
	case DECT_MM_AUTHENTICATION_REPLY:
		return dect_mm_rcv_authentication_reply(dh, mme, mb);
	case DECT_MM_AUTHENTICATION_REJECT:
		return dect_mm_rcv_authentication_reject(dh, mme, mb);
	case DECT_MM_ACCESS_RIGHTS_ACCEPT:
		return dect_mm_rcv_access_rights_accept(dh, mme, mb);
	case DECT_MM_ACCESS_RIGHTS_REJECT:
		return dect_mm_rcv_access_rights_reject(dh, mme, mb);
	case DECT_MM_ACCESS_RIGHTS_TERMINATE_ACCEPT:
		return dect_mm_rcv_access_rights_terminate_accept(dh, mme, mb);
	case DECT_MM_ACCESS_RIGHTS_TERMINATE_REJECT:
		return dect_mm_rcv_access_rights_terminate_reject(dh, mme, mb);
	case DECT_MM_CIPHER_REQUEST:
		return dect_mm_rcv_cipher_request(dh, mme, mb);
	case DECT_MM_CIPHER_SUGGEST:
		return dect_mm_rcv_cipher_suggest(dh, mme, mb);
	case DECT_MM_CIPHER_REJECT:
		return dect_mm_rcv_cipher_reject(dh, mme, mb);
	case DECT_MM_INFO_ACCEPT:
		return dect_mm_rcv_info_accept(dh, mme, mb);
	case DECT_MM_INFO_REJECT:
		return dect_mm_rcv_info_reject(dh, mme, mb);
	case DECT_MM_LOCATE_ACCEPT:
		return dect_mm_rcv_locate_accept(dh, mme, mb);
	case DECT_MM_LOCATE_REJECT:
		return dect_mm_rcv_locate_reject(dh, mme, mb);
	case DECT_MM_IDENTITY_REPLY:
		return dect_mm_rcv_identity_reply(dh, mme, mb);
	case DECT_MM_TEMPORARY_IDENTITY_ASSIGN_ACK:
		return dect_mm_rcv_temporary_identity_assign_ack(dh, mme, mb);
	case DECT_MM_TEMPORARY_IDENTITY_ASSIGN_REJ:
		return dect_mm_rcv_temporary_identity_assign_rej(dh, mme, mb);
	}

	mm_debug(mme, "receive unknown msg type %x", mb->type);
}

static void dect_mm_open(struct dect_handle *dh,
			 struct dect_transaction *req,
			 struct dect_msg_buf *mb)
{
	struct dect_mm_endpoint *mme;
	struct dect_transaction *ta;

	dect_debug(DECT_DEBUG_MM, "MM: unknown transaction: msg type: %x\n", mb->type);
	switch (mb->type) {
	case DECT_MM_AUTHENTICATION_REQUEST:
	case DECT_MM_CIPHER_REQUEST:
	case DECT_MM_CIPHER_SUGGEST:
	case DECT_MM_ACCESS_RIGHTS_REQUEST:
	case DECT_MM_ACCESS_RIGHTS_TERMINATE_REQUEST:
	case DECT_MM_LOCATE_REQUEST:
	case DECT_MM_KEY_ALLOCATE:
	case DECT_MM_INFO_REQUEST:
	case DECT_MM_INFO_SUGGEST:
	case DECT_MM_IDENTITY_REQUEST:
	case DECT_MM_TEMPORARY_IDENTITY_ASSIGN:
	case DECT_MM_DETACH:
		break;
	default:
		return;
	}

	mme = dect_mm_endpoint_get_by_link(dh, req->link);
	if (mme == NULL) {
		mme = dect_mm_endpoint_alloc(dh, NULL);
		if (mme == NULL)
			return;
		mme->link = req->link;
	}

	ta = &mme->procedure[DECT_TRANSACTION_RESPONDER].transaction;
	dect_transaction_confirm(dh, ta, req);

	switch (mb->type) {
	case DECT_MM_AUTHENTICATION_REQUEST:
		return dect_mm_rcv_authentication_request(dh, mme, mb);
	case DECT_MM_CIPHER_REQUEST:
		return dect_mm_rcv_cipher_request(dh, mme, mb);
	case DECT_MM_CIPHER_SUGGEST:
		return dect_mm_rcv_cipher_suggest(dh, mme, mb);
	case DECT_MM_ACCESS_RIGHTS_REQUEST:
		return dect_mm_rcv_access_rights_request(dh, mme, mb);
	case DECT_MM_ACCESS_RIGHTS_TERMINATE_REQUEST:
		return dect_mm_rcv_access_rights_terminate_request(dh, mme, mb);
	case DECT_MM_LOCATE_REQUEST:
		return dect_mm_rcv_locate_request(dh, mme, mb);
	case DECT_MM_KEY_ALLOCATE:
		return dect_mm_rcv_key_allocate(dh, mme, mb);
	case DECT_MM_INFO_REQUEST:
		return dect_mm_rcv_info_request(dh, mme, mb);
	case DECT_MM_INFO_SUGGEST:
		return dect_mm_rcv_info_suggest(dh, mme, mb);
	case DECT_MM_IDENTITY_REQUEST:
		return dect_mm_rcv_identity_request(dh, mme, mb);
	case DECT_MM_TEMPORARY_IDENTITY_ASSIGN:
		return dect_mm_rcv_temporary_identity_assign(dh, mme, mb);
	case DECT_MM_DETACH:
		return dect_mm_rcv_detach(dh, mme, mb);
	case DECT_MM_IWU:
		return dect_mm_rcv_iwu(dh, mme, mb);
	default:
		BUG();
	}
}

static void dect_mm_shutdown(struct dect_handle *dh,
			     struct dect_transaction *ta)
{
	struct dect_mm_procedure *mp = container_of(ta, struct dect_mm_procedure, transaction);
	struct dect_mm_endpoint *mme = dect_mm_endpoint(ta);
	const struct dect_mm_proc *proc = &dect_mm_proc[mp->type];

	mm_debug(mme, "shutdown");
	dect_mm_procedure_complete(dh, mme);
	if (mme->current == NULL)
		mme->link = NULL;
	if (mp->role == DECT_TRANSACTION_INITIATOR)
		proc->abort(dh, mme, mp);
}

static void dect_mm_link_rebind(struct dect_handle *dh,
				struct dect_data_link *from,
				struct dect_data_link *to)
{
	struct dect_mm_endpoint *mme;

	mme = dect_mm_endpoint_get_by_link(dh, from);
	if (mme == NULL)
		return;

	if (to != NULL)
		mme->link = to;
	else
		dect_mm_endpoint_destroy(dh, mme);
}

const struct dect_nwk_protocol dect_mm_protocol = {
	.name			= "Mobility Management",
	.pd			= DECT_PD_MM,
	.max_transactions	= 1,
	.open			= dect_mm_open,
	.shutdown		= dect_mm_shutdown,
	.rcv			= dect_mm_rcv,
	.encrypt_ind		= dect_mm_encrypt_ind,
	.rebind			= dect_mm_link_rebind,
};

/** @} */
