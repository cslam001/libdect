/*
 * DECT S-Format messages
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_DECT_S_FMT_H
#define _LIBDECT_DECT_S_FMT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup ie_sfmt
 * @{
 */

/*
 * Information elements
 */

#define DECT_SFMT_IE_FIXED_LEN			0x80
#define DECT_SFMT_IE_FIXED_ID_SHIFT		4
#define DECT_SFMT_IE_FIXED_VAL_MASK		0x0f

/**
 * Single octet Information Elements
 */
enum dect_sfmt_single_octet_ies {
	DECT_IE_SHIFT				= (0x01 << DECT_SFMT_IE_FIXED_ID_SHIFT) | DECT_SFMT_IE_FIXED_LEN,
	DECT_IE_EXT_PREFIX			= (0x02 << DECT_SFMT_IE_FIXED_ID_SHIFT) | DECT_SFMT_IE_FIXED_LEN,
	DECT_IE_REPEAT_INDICATOR		= (0x05 << DECT_SFMT_IE_FIXED_ID_SHIFT) | DECT_SFMT_IE_FIXED_LEN,
	DECT_IE_DOUBLE_OCTET_ELEMENT		= (0x06 << DECT_SFMT_IE_FIXED_ID_SHIFT) | DECT_SFMT_IE_FIXED_LEN,
};

/**
 * Single octet extended Information Elements
 */
enum dect_sfmt_single_octet_ext_ies {
	DECT_IE_SENDING_COMPLETE		= 0x1 | DECT_IE_EXT_PREFIX,
	DECT_IE_DELIMITER_REQUEST		= 0x2 | DECT_IE_EXT_PREFIX,
	DECT_IE_USE_TPUI			= 0x3 | DECT_IE_EXT_PREFIX,
};

/**
 * Double octet Information Elements
 */
enum dect_sfmt_double_octet_ies {
	DECT_IE_BASIC_SERVICE			= 0x0 | DECT_IE_DOUBLE_OCTET_ELEMENT,
	DECT_IE_RELEASE_REASON			= 0x2 | DECT_IE_DOUBLE_OCTET_ELEMENT,
	DECT_IE_SIGNAL				= 0x4 | DECT_IE_DOUBLE_OCTET_ELEMENT,
	DECT_IE_TIMER_RESTART			= 0x5 | DECT_IE_DOUBLE_OCTET_ELEMENT,
	DECT_IE_TEST_HOOK_CONTROL		= 0x6 | DECT_IE_DOUBLE_OCTET_ELEMENT,
	DECT_IE_SINGLE_DISPLAY			= 0x8 | DECT_IE_DOUBLE_OCTET_ELEMENT,
	DECT_IE_SINGLE_KEYPAD			= 0x9 | DECT_IE_DOUBLE_OCTET_ELEMENT,
};

/**
 * Variable length Information Elements
 */
enum dect_sfmt_variable_length_ies {
	DECT_IE_INFO_TYPE			= 0x01,
	DECT_IE_IDENTITY_TYPE			= 0x02,
	DECT_IE_PORTABLE_IDENTITY		= 0x05,
	DECT_IE_FIXED_IDENTITY			= 0x06,
	DECT_IE_LOCATION_AREA			= 0x07,
	DECT_IE_NWK_ASSIGNED_IDENTITY		= 0x09,
	DECT_IE_AUTH_TYPE			= 0x0a,
	DECT_IE_ALLOCATION_TYPE			= 0x0b,
	DECT_IE_RAND				= 0x0c,
	DECT_IE_RES				= 0x0d,
	DECT_IE_RS				= 0x0e,
	DECT_IE_IWU_ATTRIBUTES			= 0x12,
	DECT_IE_CALL_ATTRIBUTES			= 0x13,
	DECT_IE_SERVICE_CHANGE_INFO		= 0x16,
	DECT_IE_CONNECTION_ATTRIBUTES		= 0x17,
	DECT_IE_CIPHER_INFO			= 0x19,
	DECT_IE_CALL_IDENTITY			= 0x1a,
	DECT_IE_CONNECTION_IDENTITY		= 0x1b,
	DECT_IE_FACILITY			= 0x1c,
	DECT_IE_PROGRESS_INDICATOR		= 0x1e,
	DECT_IE_MMS_GENERIC_HEADER		= 0x20,
	DECT_IE_MMS_OBJECT_HEADER		= 0x21,
	DECT_IE_MMS_EXTENDED_HEADER		= 0x22,
	DECT_IE_TIME_DATE			= 0x23,
	DECT_IE_MULTI_DISPLAY			= 0x28,
	DECT_IE_MULTI_KEYPAD			= 0x2c,
	DECT_IE_FEATURE_ACTIVATE		= 0x38,
	DECT_IE_FEATURE_INDICATE		= 0x39,
	DECT_IE_NETWORK_PARAMETER		= 0x41,
	DECT_IE_EXT_HO_INDICATOR		= 0x42,
	DECT_IE_ZAP_FIELD			= 0x52,
	DECT_IE_SERVICE_CLASS			= 0x54,
	DECT_IE_KEY				= 0x56,
	DECT_IE_REJECT_REASON			= 0x60,
	DECT_IE_SETUP_CAPABILITY		= 0x62,
	DECT_IE_TERMINAL_CAPABILITY		= 0x63,
	DECT_IE_END_TO_END_COMPATIBILITY	= 0x64,
	DECT_IE_RATE_PARAMETERS			= 0x65,
	DECT_IE_TRANSIT_DELAY			= 0x66,
	DECT_IE_WINDOW_SIZE			= 0x67,
	DECT_IE_CALLING_PARTY_NUMBER		= 0x6c,
	DECT_IE_CALLING_PARTY_NAME		= 0x6d,
	DECT_IE_CALLED_PARTY_NUMBER		= 0x70,
	DECT_IE_CALLED_PARTY_SUBADDR		= 0x71,
	DECT_IE_DURATION			= 0x72,
	DECT_IE_SEGMENTED_INFO			= 0x75,
	DECT_IE_ALPHANUMERIC			= 0x76,
	DECT_IE_IWU_TO_IWU			= 0x77,
	DECT_IE_MODEL_IDENTIFIER		= 0x78,
	DECT_IE_IWU_PACKET			= 0x7a,
	DECT_IE_ESCAPE_TO_PROPRIETARY		= 0x7b,
	DECT_IE_CODEC_LIST			= 0x7c,
	DECT_IE_EVENTS_NOTIFICATION		= 0x7d,
	DECT_IE_CALL_INFORMATION		= 0x7e,
	DECT_IE_ESCAPE_FOR_EXTENSION		= 0x7f,
};

struct dect_sfmt_ie {
	uint8_t			*data;
	uint8_t			id;
	uint8_t			len;
};

/**
 * S-Format message parsing/construction state
 */
enum dect_sfmt_error {
	DECT_SFMT_OK			= 0,	/**< No Error */
	DECT_SFMT_MANDATORY_IE_MISSING	= -1,	/**< A mandatory IE was missing */
	DECT_SFMT_MANDATORY_IE_ERROR	= -2,	/**< A mandatory IE had an internal structural error */
	DECT_SFMT_INVALID_IE		= -3,	/**< An invalid IE was passed to message construction */
};

extern enum dect_sfmt_error dect_build_sfmt_ie(const struct dect_handle *dh, uint8_t type,
					       struct dect_msg_buf *mb,
					       const struct dect_ie_common *ie);

extern enum dect_sfmt_error dect_parse_sfmt_ie_header(struct dect_sfmt_ie *ie,
						      const struct dect_msg_buf *mb);
extern enum dect_sfmt_error dect_parse_sfmt_ie(const struct dect_handle *dh, uint8_t type,
					       struct dect_ie_common **dst,
					       const struct dect_sfmt_ie *ie);
/** @} */

#ifdef __cplusplus
}
#endif
#endif /* _LIBDECT_DECT_S_FMT_H */
