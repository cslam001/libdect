/*
 * DECT B-Format messages
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 */
#ifndef _LIBDECT_B_FMT_H
#define _LIBDECT_B_FMT_H

struct dect_short_page_msg {
	uint8_t		hdr;
	__be16		information;
} __packed;

/*
 * LCE request paging messages
 */

#define DECT_LCE_PAGE_W_FLAG	0x08
#define DECT_LCE_PAGE_HDR_MASK	0x07

/**
 * @DECT_LCE_PAGE_U_PLANE_NONE:		no U-plane
 * @DECT_LCE_PAGE_UNKNOWN_RINGING:	Unknown MAC service type and Ringing
 * @DECT_LCE_PAGE_ESCAPE:		Escape
 * @DECT_LCE_PAGE_GENERAL_PURPOSE:	General purpose code
 * @DECT_LCE_PAGE_GENERAL_VOICE:	General purpose code for voice service
 * @DECT_LCE_PAGE_AUXILIARY:		Auxiliary code
 * @DECT_LCE_PAGE_DPRS_INITIAL_SETUP:	DPRS initial set-up code
 * @DECT_LCE_PAGE_DPRS_INITIAL_SETUP:	DPRS resume code
 */
enum lce_request_page_hdr_codes {
	DECT_LCE_PAGE_U_PLANE_NONE,
	DECT_LCE_PAGE_UNKNOWN_RINGING,
	DECT_LCE_PAGE_ESCAPE,
	DECT_LCE_PAGE_GENERAL_PURPOSE,
	DECT_LCE_PAGE_GENERAL_VOICE,
	DECT_LCE_PAGE_AUXILIARY,
	DECT_LCE_PAGE_DPRS_INITIAL_SETUP,
	DECT_LCE_PAGE_DPRS_RESUME,
};

/* Short format message: group ringing request */

#define DECT_LCE_SHORT_PAGE_RING_PATTERN_MASK	0xf000
#define DECT_LCE_SHORT_PAGE_RING_PATTERN_SHIFT	12

#define DECT_LCE_SHORT_PAGE_GROUP_MASK		0x0fff

/* Short format message: other cases */

#define DECT_LCE_SHORT_PAGE_TPUI_MASK		0xffff

#endif /* _LIBDECT_B_FMT_H */
