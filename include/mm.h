/*
 * DECT Mobility Management
 *
 * Copyright (c) 2009 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_MM_H
#define _LIBDECT_MM_H

/**
 * MM message types
 */
enum dect_mm_msg_types {
	DECT_MM_AUTHENTICATION_REQUEST		= 0x40,
	DECT_MM_AUTHENTICATION_REPLY		= 0x41,
	DECT_MM_KEY_ALLOCATE			= 0x42,
	DECT_MM_AUTHENTICATION_REJECT		= 0x43,
	DECT_MM_ACCESS_RIGHTS_REQUEST		= 0x44,
	DECT_MM_ACCESS_RIGHTS_ACCEPT		= 0x45,
	DECT_MM_ACCESS_RIGHTS_REJECT		= 0x47,
	DECT_MM_ACCESS_RIGHTS_TERMINATE_REQUEST	= 0x48,
	DECT_MM_ACCESS_RIGHTS_TERMINATE_ACCEPT	= 0x49,
	DECT_MM_ACCESS_RIGHTS_TERMINATE_REJECT	= 0x4b,
	DECT_MM_CIPHER_REQUEST			= 0x4c,
	DECT_MM_CIPHER_SUGGEST			= 0x4e,
	DECT_MM_CIPHER_REJECT			= 0x4f,
	DECT_MM_INFO_REQUEST			= 0x50,
	DECT_MM_INFO_ACCEPT			= 0x51,
	DECT_MM_INFO_SUGGEST			= 0x52,
	DECT_MM_INFO_REJECT			= 0x53,
	DECT_MM_LOCATE_REQUEST			= 0x54,
	DECT_MM_LOCATE_ACCEPT			= 0x55,
	DECT_MM_DETACH				= 0x56,
	DECT_MM_LOCATE_REJECT			= 0x57,
	DECT_MM_IDENTITY_REQUEST		= 0x58,
	DECT_MM_IDENTITY_REPLY			= 0x59,
	DECT_MM_TEMPORARY_IDENTITY_ASSIGN	= 0x5c,
	DECT_MM_TEMPORARY_IDENTITY_ASSIGN_ACK	= 0x5d,
	DECT_MM_TEMPORARY_IDENTITY_ASSIGN_REJ	= 0x5f,
};

struct dect_mm_access_rights_accept_msg {
	struct dect_msg_common			common;
};

struct dect_mm_access_rights_reject_msg {
	struct dect_msg_common			common;
	struct dect_ie_reject_reason		*reject_reason;
	struct dect_ie_duration			*duration;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_mm_access_rights_request_msg {
	struct dect_msg_common			common;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_auth_type		*auth_type;
	struct dect_ie_cipher_info		*cipher_info;
	struct dect_ie_setup_capability		*setup_capability;
	struct dect_ie_terminal_capability	*terminal_capability;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_model_identifier		*model_identifier;
	struct dect_ie_codec_list		*codec_list;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_mm_access_rights_terminate_accept_msg {
	struct dect_msg_common			common;
};

struct dect_mm_access_rights_terminate_reject_msg {
	struct dect_msg_common			common;
};

struct dect_mm_access_rights_terminate_request_msg {
	struct dect_msg_common			common;
};

struct dect_mm_authentication_reject_msg {
	struct dect_msg_common			common;
};

struct dect_mm_authentication_reply_msg {
	struct dect_msg_common			common;
};

struct dect_mm_authentication_request_msg {
	struct dect_msg_common			common;
};

struct dect_mm_cipher_reject_msg {
	struct dect_msg_common			common;
};

struct dect_mm_cipher_request_msg {
	struct dect_msg_common			common;
};

struct dect_mm_cipher_suggest_msg {
	struct dect_msg_common			common;
};

struct dect_mm_detach_msg {
	struct dect_msg_common			common;
};

struct dect_mm_identity_reply_msg {
	struct dect_msg_common			common;
};

struct dect_mm_identity_request_msg {
	struct dect_msg_common			common;
};

struct dect_mm_key_allocate_msg {
	struct dect_msg_common			common;
};

struct dect_mm_locate_accept_msg {
	struct dect_msg_common			common;
};

struct dect_mm_locate_reject_msg {
	struct dect_msg_common			common;
};

struct dect_mm_locate_request_msg {
	struct dect_msg_common			common;
};

struct dect_mm_info_accept_msg {
	struct dect_msg_common			common;
};

struct dect_mm_info_reject_msg {
	struct dect_msg_common			common;
};

struct dect_mm_info_request_msg {
	struct dect_msg_common			common;
};

struct dect_mm_info_suggest_msg {
	struct dect_msg_common			common;
};

struct dect_mm_temporary_identity_assign_msg {
	struct dect_msg_common			common;
};

struct dect_mm_temporary_identity_assign_ack_msg {
	struct dect_msg_common			common;
};

struct dect_mm_temporary_identity_assign_rej_msg {
	struct dect_msg_common			common;
};

struct dect_mm_iwu_msg {
	struct dect_msg_common			common;
};

struct dect_mm_notify_msg {
	struct dect_msg_common			common;
};

#endif /* _LIBDECT_MM_H */
