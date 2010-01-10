#ifndef _DECT_S_FMT_H
#define _DECT_S_FMT_H

#include <assert.h>
#include <identities.h>

/*
 * S-Format message header
 */

#define DECT_S_HDR_SIZE				2

/* Transaction Identifier (TI) element */
#define DECT_S_TI_F_FLAG			0x80

#define DECT_S_TI_TV_MASK			0x70
#define DECT_S_TI_TV_SHIFT			4

#define DECT_S_TI_TV_EXT_FLAG			0x08

/* Protocol Descriminator (PD) element */
#define DECT_S_PD_MASK				0x0f

/**
 * enum dect_pds - S-Fmt protocol discriminators
 *
 * @DECT_PD_LCE:	Link Control Entity (LCE) messages
 * @DECT_PD_CC:		Call Control (CC) messages
 * @DECT_PD_CISS:	Call Independant Supplementary Services (CISS) messages
 * @DECT_PD_MM:		Mobility Management (MM) messages
 * @DECT_PD_CLMS:	ConnectionLess Message Service (CLMS) messages
 * @DECT_PD_COMS:	Connection Orentied Message Service (COMS) messages
 * @DECT_PD_UNKNOWN:	Unknown protocol entity (bit 8)
 */
enum dect_pds {
	DECT_PD_LCE				= 0x0,
	DECT_PD_CC				= 0x3,
	DECT_PD_CISS				= 0x4,
	DECT_PD_MM				= 0x5,
	DECT_PD_CLMS				= 0x6,
	DECT_PD_COMS				= 0x7,
	DECT_PD_UNKNOWN				= 0x8,
	__DECT_PD_MAX
};
#define DECT_PD_MAX				(__DECT_PD_MAX - 1)

/* Message type element */
#define DECT_S_PD_MSG_TYPE_MASK			0x7f

/* CLMS message types */
enum dect_clms_msg_types {
	DECT_CLMS_VARIABLE			= 0x1,
};

/*
 * Information elements
 */

#define DECT_SFMT_IE_FIXED_LEN			0x80
#define DECT_SFMT_IE_FIXED_ID_MASK		0x70
#define DECT_SFMT_IE_FIXED_ID_SHIFT		4
#define DECT_SFMT_IE_FIXED_VAL_MASK		0x0f

enum dect_sfmt_single_octet_ies {
	S_SO_IE_RESERVED			= (0x00 << DECT_SFMT_IE_FIXED_ID_SHIFT) | DECT_SFMT_IE_FIXED_LEN,
	S_SO_IE_SHIFT				= (0x01 << DECT_SFMT_IE_FIXED_ID_SHIFT) | DECT_SFMT_IE_FIXED_LEN,
	S_SO_IE_EXT_PREFIX			= (0x02 << DECT_SFMT_IE_FIXED_ID_SHIFT) | DECT_SFMT_IE_FIXED_LEN,
	S_SO_IE_REPEAT_INDICATOR		= (0x05 << DECT_SFMT_IE_FIXED_ID_SHIFT) | DECT_SFMT_IE_FIXED_LEN,
	S_SO_IE_DOUBLE_OCTET_ELEMENT		= (0x06 << DECT_SFMT_IE_FIXED_ID_SHIFT) | DECT_SFMT_IE_FIXED_LEN,
};

enum dect_sfmt_single_octet_ext_ies {
	S_SE_IE_SENDING_COMPLETE		= 0x1 | S_SO_IE_EXT_PREFIX,
	S_SE_IE_DELIMITER_REQUEST		= 0x2 | S_SO_IE_EXT_PREFIX,
	S_SE_IE_USE_TPUI			= 0x3 | S_SO_IE_EXT_PREFIX,
};

enum dect_sfmt_double_octet_ies {
	S_DO_IE_BASIC_SERVICE			= 0x0 | S_SO_IE_DOUBLE_OCTET_ELEMENT,
	S_DO_IE_RELEASE_REASON			= 0x2 | S_SO_IE_DOUBLE_OCTET_ELEMENT,
	S_DO_IE_SIGNAL				= 0x4 | S_SO_IE_DOUBLE_OCTET_ELEMENT,
	S_DO_IE_TIMER_RESTART			= 0x5 | S_SO_IE_DOUBLE_OCTET_ELEMENT,
	S_DO_IE_TEST_HOOK_CONTROL		= 0x6 | S_SO_IE_DOUBLE_OCTET_ELEMENT,
	S_DO_IE_SINGLE_DISPLAY			= 0x8 | S_SO_IE_DOUBLE_OCTET_ELEMENT,
	S_DO_IE_SINGLE_KEYPAD			= 0x9 | S_SO_IE_DOUBLE_OCTET_ELEMENT,
	S_DO_IE_RESERVED			= 0xf | S_SO_IE_DOUBLE_OCTET_ELEMENT,
};

enum dect_sfmt_variable_length_ies {
	S_VL_IE_INFO_TYPE			= 0x01,
	S_VL_IE_IDENTITY_TYPE			= 0x02,
	S_VL_IE_PORTABLE_IDENTITY		= 0x05,
	S_VL_IE_FIXED_IDENTITY			= 0x06,
	S_VL_IE_LOCATION_AREA			= 0x07,
	S_VL_IE_NWK_ASSIGNED_IDENTITY		= 0x09,
	S_VL_IE_AUTH_TYPE			= 0x0a,
	S_VL_IE_ALLOCATION_TYPE			= 0x0b,
	S_VL_IE_RAND				= 0x0c,
	S_VL_IE_RES				= 0x0d,
	S_VL_IE_RS				= 0x0e,
	S_VL_IE_IWU_ATTRIBUTES			= 0x12,
	S_VL_IE_CALL_ATTRIBUTES			= 0x13,
	S_VL_IE_SERVICE_CHANGE_INFO		= 0x16,
	S_VL_IE_CONNECTION_ATTRIBUTES		= 0x17,
	S_VL_IE_CIPHER_INFO			= 0x19,
	S_VL_IE_CALL_IDENTITY			= 0x1a,
	S_VL_IE_CONNECTION_IDENTITY		= 0x1b,
	S_VL_IE_FACILITY			= 0x1c,
	S_VL_IE_PROGRESS_INDICATOR		= 0x1e,
	S_VL_IE_MMS_GENERIC_HEADER		= 0x20,
	S_VL_IE_MMS_OBJECT_HEADER		= 0x21,
	S_VL_IE_MMS_EXTENDED_HEADER		= 0x22,
	S_VL_IE_TIME_DATE			= 0x23,
	S_VL_IE_MULTI_DISPLAY			= 0x28,
	S_VL_IE_MULTI_KEYPAD			= 0x2c,
	S_VL_IE_FEATURE_ACTIVATE		= 0x38,
	S_VL_IE_FEATURE_INDICATE		= 0x39,
	S_VL_IE_NETWORK_PARAMETER		= 0x41,
	S_VL_IE_EXT_HO_INDICATOR		= 0x42,
	S_VL_IE_ZAP_FIELD			= 0x52,
	S_VL_IE_SERVICE_CLASS			= 0x54,
	S_VL_IE_KEY				= 0x56,
	S_VL_IE_REJECT_REASON			= 0x60,
	S_VL_IE_SETUP_CAPABILITY		= 0x62,
	S_VL_IE_TERMINAL_CAPABILITY		= 0x63,
	S_VL_IE_END_TO_END_COMPATIBILITY	= 0x64,
	S_VL_IE_RATE_PARAMETERS			= 0x65,
	S_VL_IE_TRANSIT_DELAY			= 0x66,
	S_VL_IE_WINDOW_SIZE			= 0x67,
	S_VL_IE_CALLING_PARTY_NUMBER		= 0x6c,
	S_VL_IE_CALLING_PARTY_NAME		= 0x6d,
	S_VL_IE_CALLED_PARTY_NUMBER		= 0x70,
	S_VL_IE_CALLED_PARTY_SUBADDR		= 0x71,
	S_VL_IE_DURATION			= 0x72,
	S_VL_IE_SEGMENTED_INFO			= 0x75,
	S_VL_IE_ALPHANUMERIC			= 0x76,
	S_VL_IE_IWU_TO_IWU			= 0x77,
	S_VL_IE_MODEL_IDENTIFIER		= 0x78,
	S_VL_IE_IWU_PACKET			= 0x7a,
	S_VL_IE_ESCAPE_TO_PROPRIETARY		= 0x7b,
	S_VL_IE_CODEC_LIST			= 0x7c,
	S_VL_IE_EVENTS_NOTIFICATION		= 0x7d,
	S_VL_IE_CALL_INFORMATION		= 0x7e,
	S_VL_IE_ESCAPE_FOR_EXTENSION		= 0x7f,
	__S_VL_IE_MAX
};

#define DECT_OCTET_GROUP_END			0x80

/* Repeat indicator */

/* Basic service */

#define DECT_BASIC_SERVICE_CALL_CLASS_MASK	0xf0
#define DECT_BASIC_SERVICE_CALL_CLASS_SHIFT	4

#define DECT_BASIC_SERVICE_SERVICE_MASK		0x0f

/* Single display IE */

/* Single keypad IE */

/* Release reason */

/* Allocation type IE */

/* Alphanumeric IE */

/* Auth type IE */

/* Call attributes IE */

/* Call identity IE */

/* Called party number IE */

/* Called party subaddress IE */

/* Calling party number IE */

/* Cipher info IE */

/* Connection attributes IE */

/* Connection identity IE */

/* Duration IE */

/* End-to-end compatibility IE */

/* Facility IE */

/* Feature activate IE */

/* Feature indicate IE */

/* Fixed identity IE */

#define S_VL_IE_FIXED_IDENTITY_MIN_SIZE		2

/* Identity type IE */

/* Info type IE */

/* InterWorking Unit (IWU) attributes IE */

/* IWU packet IE */

/* IWU to IWU IE */

/* Key IE */

/* Location area IE */

#define DECT_LOCATION_AREA_TYPE_MASK			0xc0
#define DECT_LOCATION_AREA_TYPE_SHIFT			6

#define DECT_LOCATION_LEVEL_MASK			0x3f

/* Multi-display IE */

/* Multi-keypad IE */

/* NetWorK (NWK) assigned identity IE */

/* Network parameter IE */

/* Portable identity IE */

#define S_VL_IE_PORTABLE_IDENTITY_MIN_SIZE		2

/* IPUI */

#define IPUI_BITS_PER_DIGIT			4
#define IPUI_PUT_PSTN_ISDN_NUMBER_MAX_DIGITS	60

#define IPUI_PUT_PRIVATE_NUMBER_MAX_LEN		60

/* Progress indicator IE */

#define DECT_SFMT_IE_PROGRESS_INDICATOR_LOCATION_MASK	0x0f

/* RAND IE */

/* Rate parameters IE */

/* Reject reason IE */

/* RES IE */

/* RS IE */

/* Segmented info IE */

/* Service change info IE */

/* Service class IE */

/* Setup capability IE */

/* Terminal capability IE */

#define DECT_TERMINAL_CAPABILITY_DISPLAY_MASK	0x0f

#define DECT_TERMINAL_CAPABILITY_TONE_MASK	0x70
#define DECT_TERMINAL_CAPABILITY_TONE_SHIFT	4

#define DECT_TERMINAL_CAPABILITY_ECHO_MASK	0x70
#define DECT_TERMINAL_CAPABILITY_ECHO_SHIFT	4

#define DECT_TERMINAL_CAPABILITY_NOISE_MASK	0x0c
#define DECT_TERMINAL_CAPABILITY_NOISE_SHIFT	2

#define DECT_TERMINAL_CAPABILITY_VOLUME_MASK	0x03

/* Transit delay IE */

/* Window size IE */

/* ZAP field IE */

/* Escape to proprietary IE */

#define DECT_ESC_TO_PROPRIETARY_IE_DESC_TYPE_MASK	0x7f
#define DECT_ESC_TO_PROPRIETARY_IE_DESC_EMC		1

/* Model identifier IE */

/* MMS Generic Header IE */

/* MMS Object Header IE */

/* MMS Extended Header IE */

/* Time-Data IE */

/* Ext h/o indicator IE */

/* Authentication Reject Parameter IE */

/* Calling party Name IE */

/* Codec List IE */

/* Events notification IE */

/* Call information IE */

enum dect_sfmt_ie_status {
	DECT_SFMT_IE_NONE,
	DECT_SFMT_IE_OPTIONAL,
	DECT_SFMT_IE_MANDATORY
};

enum dect_sfmt_ie_flags {
	DECT_SFMT_IE_REPEAT		= 0x1,
	DECT_SFMT_IE_EITHER		= 0x2,
	DECT_SFMT_IE_END		= 0x4,
};

/**
 * struct dect_sfmt_ie_desc - S-Format IE description
 *
 * @offset:	offset of corresponding S-Format IE storage
 * @type:	IE type
 * @fp_pp:	Status in direction FP->PP
 * @pp_fp:	Status in direction PP->FP
 * @flags:	Global flags
 */
struct dect_sfmt_ie_desc {
	uint16_t			type;
	enum dect_sfmt_ie_status	fp_pp:8;
	enum dect_sfmt_ie_status	pp_fp:8;
	uint8_t				flags;
};

#define DECT_SFMT_IE(_type, _fp_pp, _pp_fp, _flags) {	\
	.type	= (_type),				\
	.fp_pp	= DECT_SFMT_ ## _fp_pp,			\
	.pp_fp	= DECT_SFMT_ ## _pp_fp,			\
	.flags	= (_flags),				\
}

#define DECT_SFMT_IE_END_MSG {				\
	.flags	= DECT_SFMT_IE_END,			\
}

struct dect_sfmt_ie {
	uint8_t			*data;
	uint16_t		id;
	uint8_t			len;
};

struct dect_sfmt_msg_desc {
	const char			*name;
	struct dect_sfmt_ie_desc	ie[];
};

#define DECT_SFMT_MSG_DESC(_name, _init...)			\
	const struct dect_sfmt_msg_desc _name ## _msg_desc = {	\
		.name	= # _name,				\
		.ie	= {					\
			_init,					\
		},						\
	}

/**
 * enum dect_sfmt_error - S-Format message parsing/construction state
 *
 * @DECT_SFMT_OK:			No error
 * @DECT_SFMT_MANDATORY_IE_MISSING:	A mandatory IE was missing
 * @DECT_SFMT_MANDATORY_IE_ERROR:	A mandatory IE had an internal structural error
 * @DECT_SFMT_INVALID_IE:		An invalid IE was passed to message construction
 */
enum dect_sfmt_error {
	DECT_SFMT_OK			= 0,
	DECT_SFMT_MANDATORY_IE_MISSING	= -1,
	DECT_SFMT_MANDATORY_IE_ERROR	= -2,
	DECT_SFMT_INVALID_IE		= -3,
};

static inline enum dect_reject_reasons dect_sfmt_reject_reason(enum dect_sfmt_error err)
{
	switch (err) {
	case DECT_SFMT_MANDATORY_IE_MISSING:
		return DECT_REJECT_INFORMATION_ELEMENT_ERROR;
	case DECT_SFMT_MANDATORY_IE_ERROR:
		return DECT_REJECT_INVALID_INFORMATION_ELEMENT_CONTENTS;
	default:
		BUG();
	}
}

/**
 * struct dect_msg_common - Common dummy msg structure to avoid casts
 *
 * @ie:		First IE
 */
struct dect_msg_common {
	struct dect_ie_common		*ie[0];
};

extern void *dect_ie_collection_alloc(const struct dect_handle *dh, unsigned int size);

struct dect_msg_buf;
extern enum dect_sfmt_error dect_parse_sfmt_msg(const struct dect_handle *dh,
						const struct dect_sfmt_msg_desc *desc,
						struct dect_msg_common *dst,
						struct dect_msg_buf *mb);
extern enum dect_sfmt_error dect_build_sfmt_msg(const struct dect_handle *dh,
						const struct dect_sfmt_msg_desc *desc,
						const struct dect_msg_common *src,
						struct dect_msg_buf *mb);

extern void dect_msg_free(const struct dect_handle *dh,
			  const struct dect_sfmt_msg_desc *desc,
			  struct dect_msg_common *msg);

#endif /* _DECT_S_FMT_H */
