/*
 * DECT Supplementary Services (SS)
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_SS_H
#define _LIBDECT_SS_H

/**
 * Call Independant Supplementary Services messages types
 */
enum dect_ciss_msg_types {
	DECT_CISS_RELEASE_COM		= 0x5a,
	DECT_CISS_FACILITY		= 0x62,
	DECT_CISS_REGISTER		= 0x64,
};

struct dect_ciss_release_com_msg {
	struct dect_msg_common			common;
	struct dect_ie_release_reason		*release_reason;
	struct dect_ie_list			facility;
	struct dect_ie_display			*display;
	struct dect_ie_keypad			*keypad;
	struct dect_ie_feature_activate		*feature_activate;
	struct dect_ie_feature_indicate		*feature_indicate;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_ciss_facility_msg {
	struct dect_msg_common			common;
	struct dect_ie_list			facility;
	struct dect_ie_display			*display;
	struct dect_ie_keypad			*keypad;
	struct dect_ie_feature_activate		*feature_activate;
	struct dect_ie_feature_indicate		*feature_indicate;
	struct dect_ie_list			iwu_to_iwu;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
	struct dect_ie_time_date		*time_date;
	struct dect_ie_events_notification	*events_notification;
	struct dect_ie_call_information		*call_information;
};

struct dect_ciss_register_msg {
	struct dect_msg_common			common;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_list			facility;
	struct dect_ie_display			*display;
	struct dect_ie_keypad			*keypad;
	struct dect_ie_feature_activate		*feature_activate;
	struct dect_ie_feature_indicate		*feature_indicate;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

/**
 * struct dect_ss_endpoint - Supplementary Services Endpoint
 *
 * @transaction:	LCE link transaction
 * @priv:		libdect user private storage
 */
struct dect_ss_endpoint {
	struct dect_transaction			transaction;
	uint8_t					priv[] __aligned(__alignof__(uint64_t));
};

extern void dect_clss_rcv(struct dect_handle *dh, struct dect_msg_buf *mb);

/**
 * Call Related Supplementary Services messages types
 */
enum dect_crss_msg_types {
	CRSS_HOLD		= 0x24,
	CRSS_HOLD_ACK		= 0x28,
	CRSS_HOLD_REJECT	= 0x30,
	CRSS_RETRIEVE		= 0x31,
	CRSS_RETRIEVE_ACK	= 0x33,
	CRSS_RETRIEVE_REJECT	= 0x37,
	CRSS_FACILITY		= 0x62,
};

extern const struct dect_nwk_protocol dect_ciss_protocol;

#endif /* _LIBDECT_SS_H */
