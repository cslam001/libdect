/*
 * DECT S-Format messages
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_S_FMT_H
#define _LIBDECT_S_FMT_H

#include <dect/ie.h>
#include <dect/s_fmt.h>

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

/*
 * Information elements
 */

#define DECT_SFMT_IE_FIXED_ID_MASK		0x70
#define DECT_SFMT_IE_FIXED_VAL_MASK		0x0f

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

#define DECT_IE_FIXED_IDENTITY_MIN_SIZE		2

#define DECT_IE_FIXED_IDENTITY_TYPE_MASK	0x7f
#define DECT_IE_FIXED_IDENTITY_LENGTH_MASK	0x7f

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

#define DECT_IE_PORTABLE_IDENTITY_MIN_SIZE		2

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
	DECT_SFMT_IE_NONE		= 0,
	/* -1 indicates generic errors */
	DECT_SFMT_IE_OPTIONAL		= -2,
	DECT_SFMT_IE_MANDATORY		= -3,
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
	uint8_t				type;
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

#endif /* _LIBDECT_S_FMT_H */
