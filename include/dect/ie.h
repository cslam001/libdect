/*
 * DECT S-Format Information Elements
 *
 * Copyright (c) 2009 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_DECT_IE_H
#define _LIBDECT_DECT_IE_H

#include <string.h>
#include <dect/utils.h>
#include <list.h>

/**
 * struct dect_ie_common - common representation of a DECT IE
 *
 * @list:		repeat indicator list node
 * @refcnt:		reference count
 */
struct dect_ie_common {
	struct list_head		list;
	unsigned int			refcnt;
};

#define dect_ie_container(res, ie)	container_of(ie, typeof(*res), common)

static inline struct dect_ie_common *__dect_ie_init(struct dect_ie_common *ie)
{
	ie->refcnt = 1;
	return ie;
}

#define dect_ie_init(ie)		dect_ie_container(ie, __dect_ie_init(&(ie)->common))

static inline struct dect_ie_common *__dect_ie_hold(struct dect_ie_common *ie)
{
	if (ie != NULL)
		ie->refcnt++;
	return ie;
}

#define dect_ie_hold(ie)		dect_ie_container(ie, __dect_ie_hold(&(ie)->common))

/* Repeat indicator */

/**
 * enum dect_ie_list_types - Repeat indicator list types
 *
 * @DECT_SFMT_IE_LIST_NORMAL:	Non priorized list
 * @DECT_SFMT_IE_PRIORITIZED:	Priorized list
 */
enum dect_ie_list_types {
	DECT_SFMT_IE_LIST_NORMAL	= 0x1,
	DECT_SFMT_IE_LIST_PRIORITIZED	= 0x2,
};

struct dect_ie_repeat_indicator {
	struct dect_ie_common		common;
	enum dect_ie_list_types		type;
	struct list_head		list;
};

static inline void dect_repeat_indicator_init(struct dect_ie_repeat_indicator *ie)
{
	dect_ie_init(ie);
	init_list_head(&ie->list);
}

#define dect_foreach_ie(ptr, repeat) \
	list_for_each_entry(ptr, &(repeat).list, common.list)

static inline void dect_ie_list_move(struct dect_ie_repeat_indicator *to,
				     struct dect_ie_repeat_indicator *from)
{
	list_splice_init(&from->list, &to->list);
}

/* Sending complete */

struct dect_ie_sending_complete {
	struct dect_ie_common		common;
};

/* Delimiter request */

struct dect_ie_delimiter_request {
	struct dect_ie_common		common;
};

/* Use TPUI */

struct dect_ie_use_tpui {
	struct dect_ie_common		common;
};

/* Basic service */

#define DECT_BASIC_SERVICE_CALL_CLASS_MASK		0xf0
#define DECT_BASIC_SERVICE_CALL_CLASS_SHIFT		4

enum dect_call_classes {
	DECT_CALL_CLASS_MESSAGE				= 0x4,
	DECT_CALL_CLASS_DECT_ISDN			= 0x7,
	DECT_CALL_CLASS_NORMAL				= 0x8,
	DECT_CALL_CLASS_INTERNAL			= 0x9,
	DECT_CALL_CLASS_EMERGENCY			= 0xa,
	DECT_CALL_CLASS_SERVICE				= 0xb,
	DECT_CALL_CLASS_EXTERNAL_HO			= 0xc,
	DECT_CALL_CLASS_SUPPLEMENTARY_SERVICE		= 0xd,
	DECT_CALL_CLASS_QA_M				= 0xe,
};
#define DECT_CALL_CLASS_MAX				0xf

#define DECT_BASIC_SERVICE_SERVICE_MASK			0x0f

enum dect_basic_service {
	DECT_SERVICE_BASIC_SPEECH_DEFAULT		= 0x0,
	DECT_SERVICE_DECT_GSM_IWP			= 0x4,
	DECT_SERVICE_UMTS_IWP				= 0x6,
	DECT_SERVICE_LRMS				= 0x5,
	DECT_SERVICE_GSM_IWP_SMS			= 0x6,
	DECT_SERVICE_WIDEBAND_SPEECH			= 0x8,
	DECT_SERVICE_OTHER				= 0xf,
};
#define DECT_SERVICE_MAX				0xf

struct dect_ie_basic_service {
	struct dect_ie_common		common;
	enum dect_call_classes		class;
	enum dect_basic_service		service;
};

/* Release reason */

enum dect_release_reasons {
	/* general values */
	DECT_RELEASE_NORMAL				= 0x0,
	DECT_RELEASE_UNEXPECTED_MESSAGE			= 0x1,
	DECT_RELEASE_UNKNOWN_TRANSACTION_IDENTIFIER	= 0x2,
	DECT_RELEASE_MANDATORY_IE_MISSING		= 0x3,
	DECT_RELEASE_INVALID_IE_CONTENTS		= 0x4,
	DECT_RELEASE_INCOMPATIBLE_SERVICE		= 0x5,
	DECT_RELEASE_SERVICE_NOT_IMPLEMENTED		= 0x6,
	DECT_RELEASE_NEGOTIATION_NOT_SUPPORTED		= 0x7,
	DECT_RELEASE_INVALID_IDENTITY			= 0x8,
	DECT_RELEASE_AUTHENTICATION_FAILED		= 0x9,
	DECT_RELEASE_UNKNOWN_IDENTITY			= 0xa,
	DECT_RELEASE_NEGOTIATION_FAILED			= 0xb,
	DECT_RELEASE_TIMER_EXPIRY			= 0xd,
	DECT_RELEASE_PARTIAL_RELEASE			= 0xe,
	DECT_RELEASE_UNKNOWN				= 0xf,
	/* user values */
	DECT_RELEASE_USER_DETACHED			= 0x10,
	DECT_RELEASE_USER_NOT_IN_RANGE			= 0x11,
	DECT_RELEASE_USER_UNKNOWN			= 0x12,
	DECT_RELEASE_USER_ALREADY_ACTIVE		= 0x13,
	DECT_RELEASE_USER_BUSY				= 0x14,
	DECT_RELEASE_USER_REJECTION			= 0x15,
	DECT_RELEASE_USER_CALL_MODIFY			= 0x16,
	/* external handover values */
	DECT_RELEASE_EXTERNAL_HANDOVER_NOT_SUPPORTED	= 0x21,
	DECT_RELEASE_NETWORK_PARAMETERS_MISSING		= 0x22,
	DECT_RELEASE_EXTERNAL_HANDOVER_RELEASE		= 0x23,
	/* temporary overload values */
	DECT_RELEASE_OVERLOAD				= 0x31,
	DECT_RELEASE_INSUFFICIENT_RESOURCES		= 0x32,
	DECT_RELEASE_INSUFFICIENT_BEARERS_AVAILABLE	= 0x33,
	DECT_RELEASE_IWU_CONGESTION			= 0x34,
};

struct dect_ie_release_reason {
	struct dect_ie_common		common;
	enum dect_release_reasons	reason;
};

/* Display IE (used for both Single Display and Multi Display) */

struct dect_ie_display {
	struct dect_ie_common		common;
	uint8_t				len;
	uint8_t				info[256];
};

static inline void dect_display_init(struct dect_ie_display *display)
{
	dect_ie_init(display);
	display->len = 0;
}

static inline void
dect_display_append_char(struct dect_ie_display *display, char c)
{
	display->info[display->len] = c;
	display->len++;
}

static inline void dect_display_append(struct dect_ie_display *display,
				       const char *str, size_t len)
{
	memcpy(display->info + display->len, str, len);
	display->len += len;
}

/* Keypad IE (used for both Single Keypad and Multi Keypad) */

struct dect_ie_keypad {
	struct dect_ie_common		common;
	uint16_t			len;
	uint8_t				info[256];
};

/* Signal IE */

// FIXME: rename to alerting
enum dect_ring_patterns {
	DECT_RING_PATTERN_0				= 0x0,
	DECT_RING_PATTERN_1				= 0x1,
	DECT_RING_PATTERN_2				= 0x2,
	DECT_RING_PATTERN_3				= 0x3,
	DECT_RING_PATTERN_4				= 0x4,
	DECT_RING_PATTERN_5				= 0x5,
	DECT_RING_PATTERN_6				= 0x6,
	DECT_RING_PATTERN_7				= 0x7,
	DECT_RING_CONTINUOUS				= 0x8,
	DECT_RING_INCOMING_CALL_RELEASED		= 0xa,
	DECT_RING_INCOMING_CALL_ANSWERED		= 0xb,
	DECT_RING_OFF					= 0xf,
	__DECT_RING_MAX
};
#define DECT_RING_MAX					(__DECT_RING_MAX - 1)

enum dect_signal_codes {
	DECT_SIGNAL_DIAL_TONE_ON			= 0x0,
	DECT_SIGNAL_RING_BACK_TONE_ON			= 0x1,
	DECT_SIGNAL_INTERCEPT_TONE_ON			= 0x2,
	DECT_SIGNAL_NETWORK_CONGESTION_TONE_ON		= 0x3,
	DECT_SIGNAL_BUSY_TONE_ON			= 0x4,
	DECT_SIGNAL_CONFIRM_TONE_ON			= 0x5,
	DECT_SIGNAL_ANSWER_TONE_ON			= 0x6,
	DECT_SIGNAL_CALL_WAITING_TONE_ON		= 0x7,
	DECT_SIGNAL_OFF_HOOK_WARNING_TONE_ON		= 0x8,
	DECT_SIGNAL_NEGATIVE_ACKNOWLEDGEMENT_TONE	= 0x9,
	DECT_SIGNAL_TONES_OFF				= 0xf,
	DECT_SIGNAL_ALERTING_BASE			= 0x40,
};

struct dect_ie_signal {
	struct dect_ie_common		common;
	enum dect_signal_codes		code;
};

static inline struct dect_ie_signal *
dect_signal_init(struct dect_ie_signal *signal, enum dect_signal_codes code)
{
	dect_ie_init(signal);
	signal->code = code;
	return signal;
}

/* Timer restart IE */

enum dect_timer_restart_codes {
	DECT_TIMER_RESTART		= 0x0,
	DECT_TIMER_STOP			= 0x1,
};

struct dect_ie_timer_restart {
	struct dect_ie_common		common;
	enum dect_timer_restart_codes	code;
};

/* Test hook control */

enum dect_test_hook_ctrls {
	DECT_TEST_HOOK_ON_HOOK		= 0x0,
	DECT_TEST_HOOK_OFF_HOOK		= 0x1,
};

struct dect_ie_test_hook_control {
	struct dect_ie_common		common;
	enum dect_test_hook_ctrls	hook;
};

/* Allocation type IE */

struct dect_ie_allocation_type {
	struct dect_ie_common		common;
};

/* Alphanumeric IE */

struct dect_ie_alphanumeric {
	struct dect_ie_common		common;
};

/* Auth type IE */

enum dect_ie_auth_type_identifiers {
	AUTH_DSAA			= 0x1,
	AUTH_GSM			= 0x40,
	AUTH_UMTS			= 0x20,
	AUTH_PROPRIETARY		= 0x7f,
};

enum dect_ie_auth_key_types {
	KEY_USER_AUTHENTICATION_KEY	= 0x1,
	KEY_USER_PERSONAL_IDENTITY	= 0x3,
	KEY_AUTHENTICATION_CODE		= 0x4,
};

struct dect_ie_auth_type {
	struct dect_ie_common		common;
	uint8_t				auth_id;
	uint8_t				proprietary_auth_id;
	uint8_t				auth_key_type;
	uint8_t				auth_key_num;
	uint8_t				flags;
	uint8_t				cipher_key_num;
};

/* Call attributes IE */

struct dect_ie_call_attributes {
	struct dect_ie_common		common;
};

/* Call identity IE */

struct dect_ie_call_identity {
	struct dect_ie_common		common;
};

/* Called party number IE */

enum number_type {
	NUMBER_TYPE_UNKNOWN		= 0x0,
	NUMBER_TYPE_INTERNATIONAL	= 0x1,
	NUMBER_TYPE_NATIONAL		= 0x2,
	NUMBER_TYPE_NETWORK_SPECIFIC	= 0x3,
	NUMBER_TYPE_SUBSCRIBER		= 0x4,
	NUMBER_TYPE_ABBREVIATED		= 0x6,
	NUMBER_TYPE_RESERVED		= 0x7,
};

enum numbering_plan_identification {
	NPI_UNKNOWN			= 0x0,
	NPI_ISDN_E164			= 0x1,
	NPI_DATA_PLAN_X121		= 0x3,
	NPI_TCP_IP			= 0x7,
	NPI_NATIONAL_STANDARD		= 0x8,
	NPI_PRIVATE			= 0x9,
	NPI_SIP				= 0xa,
	NPI_INTERNET_CHARACTER_FORMAT	= 0xb,
	NPI_LAN_MAC_ADDRESS		= 0xc,
	NPI_X400			= 0xd,
	NPI_PROFILE_SPECIFIC		= 0xe,
	NPI_RESERVED			= 0xf,
};

struct dect_ie_called_party_number {
	struct dect_ie_common		common;
};

/* Called party subaddress IE */

struct dect_ie_called_party_subaddress {
	struct dect_ie_common		common;
};

/* Calling party number IE */

struct dect_ie_calling_party_number {
	struct dect_ie_common		common;
};

/* Cipher info IE */

struct dect_ie_cipher_info {
	struct dect_ie_common		common;
};

/* Connection attributes IE */

struct dect_ie_connection_attributes {
	struct dect_ie_common		common;
};

/* Connection identity IE */

struct dect_ie_connection_identity {
	struct dect_ie_common		common;
};

/* Duration IE */

struct dect_ie_duration {
	struct dect_ie_common		common;
};

/* End-to-end compatibility IE */

struct dect_ie_end_to_end_compatibility {
	struct dect_ie_common		common;
};

/* Facility IE */

struct dect_ie_facility {
	struct dect_ie_common		common;
};

/* Feature activate IE */

struct dect_ie_feature_activate {
	struct dect_ie_common		common;
};

/* Feature indicate IE */

struct dect_ie_feature_indicate {
	struct dect_ie_common		common;
};

/* Fixed identity IE */

/**
 * @ID_TYPE_ARI:	Access rights identity
 * @ID_TYPE_ARI_RPN:	Access rights identity plus radio fixed part number
 * @ID_TYPE_ARI_WRS:	Access rights identity plus radio fixed part number for WRS
 * @ID_TYPE_PARK:	Portable access rights key
 */
enum fixed_identity_types {
	ID_TYPE_ARI		= 0x00,
	ID_TYPE_ARI_RPN		= 0x01,
	ID_TYPE_ARI_WRS		= 0x02,
	ID_TYPE_PARK		= 0x20,
};

#define S_VL_IE_FIXED_IDENTITY_MIN_SIZE		2

#define S_VL_IE_FIXED_IDENTITY_TYPE_MASK	0x7f
#define S_VL_IE_FIXED_IDENTITY_LENGTH_MASK	0x7f

struct dect_ie_fixed_identity {
	struct dect_ie_common		common;
	enum fixed_identity_types	type;
	struct dect_ari			ari;
	uint8_t				rpn;
};

/* Identity type IE */

struct dect_ie_identity_type {
	struct dect_ie_common		common;
};

/* Info type IE */

struct dect_ie_info_type {
	struct dect_ie_common		common;
};

/* InterWorking Unit (IWU) attributes IE */

struct dect_ie_iwu_attributes {
	struct dect_ie_common		common;
};

/* IWU packet IE */

struct dect_ie_iwu_packet {
	struct dect_ie_common		common;
};

/* IWU to IWU IE */

struct dect_ie_iwu_to_iwu {
	struct dect_ie_common		common;
};

/* Key IE */

struct dect_ie_key {
	struct dect_ie_common		common;
};

/* Location area IE */

struct dect_ie_location_area {
	struct dect_ie_common		common;
};

/* NetWorK (NWK) assigned identity IE */

struct dect_ie_nwk_assigned_identity {
	struct dect_ie_common		common;
};

/* Network parameter IE */

struct dect_ie_network_parameter {
	struct dect_ie_common		common;
};

/* Portable identity IE */

/**
 * @ID_TYPE_IPUI:	International Portable User Identity (IPUI)
 * @ID_TYPE_IPEI:	International Portable Equipment Identity (IPEI)
 * @ID_TYPE_TPUI:	Temporary Portable User Identity (TPUI)
 */
enum portable_identity_types {
	ID_TYPE_IPUI		= 0x0,
	ID_TYPE_IPEI		= 0x10,
	ID_TYPE_TPUI		= 0x20,
};

struct dect_ie_portable_identity {
	struct dect_ie_common		common;
	enum portable_identity_types	type;
	union {
		struct dect_ipui	ipui;
	};
};

/* Progress indicator IE */

enum dect_location {
	DECT_LOCATION_USER					= 0x0,
	DECT_LOCATION_PRIVATE_NETWORK_SERVING_LOCAL_USER	= 0x1,
	DECT_LOCATION_PUBLIC_NETWORK_SERVING_LOCAL_USER		= 0x2,
 	DECT_LOCATION_PRIVATE_NETWORK_SERVING_REMOTE_USER	= 0x4,
 	DECT_LOCATION_PUBLIC_NETWORK_SERVING_REMOTE_USER	= 0x5,
	DECT_LOCATION_INTERNATIONAL_NETWORK			= 0x7,
	DECT_LOCATION_NETWORK_BEYONG_INTERWORKING_POINT		= 0xa,
	DECT_LOCATION_NOT_APPLICABLE				= 0xf,
};

enum dect_progress_description {
	DECT_PROGRESS_NOT_END_TO_END_ISDN			= 0x0,
	DECT_PROGRESS_DESTINATION_ADDRESS_NON_ISDN		= 0x2,
	DECT_PROGRESS_ORIGINATION_ADDRESS_NON_ISDN		= 0x3,
	DECT_PROGRESS_CALL_RETURNED_TO_ISDN			= 0x4,
	DECT_PROGRESS_SERVICE_CHANGE				= 0x5,
	DECT_PROGRESS_INBAND_INFORMATION_NOW_AVAILABLE		= 0x8,
	DECT_PROGRESS_INBAND_INFORMATION_NOT_AVAILABLE		= 0x9,
	DECT_PROGRESS_END_TO_END_ISDN				= 0x40,
};

struct dect_ie_progress_indicator {
	struct dect_ie_common		common;
	enum dect_location		location;
	enum dect_progress_description	progress;
};

/* RAND IE */

struct dect_ie_rand {
	struct dect_ie_common		common;
};

/* Rate parameters IE */

struct dect_ie_rate_parameters {
	struct dect_ie_common		common;
};

/* Reject reason IE */

enum dect_reject_reasons {
	DECT_REJECT_TPUI_UNKNOWN				= 0x1,
	DECT_REJECT_IPUI_UNKNOWN				= 0x2,
	DECT_REJECT_NETWORK_ASSIGNED_IDENTITY_UNKNOWN		= 0x3,
	DECT_REJECT_IPEI_NOT_ACCEPTED				= 0x5,
	DECT_REJECT_IPUI_NOT_ACCEPTED				= 0x6,
	DECT_REJECT_AUTHENTICATION_FAILED			= 0x10,
	DECT_REJECT_NO_AUTHENTICATION_ALGORITHM			= 0x11,
	DECT_REJECT_AUTHENTICATION_ALGORITHM_NOT_SUPPORTED	= 0x12,
	DECT_REJECT_AUTHENTICATION_KEY_NOT_SUPPORTED		= 0x13,
	DECT_REJECT_UPI_NOT_ENTERED			 	= 0x14,
	DECT_REJECT_NO_CIPHER_ALGORITHM				= 0x17,
	DECT_REJECT_CIPHER_ALGORITHM_NOT_SUPPORTED		= 0x18,
	DECT_REJECT_CIPHER_KEY_NOT_SUPPORTED			= 0x19,
	DECT_REJECT_INCOMPATIBLE_SERVICE			= 0x20,
	DECT_REJECT_FALSE_LCE_REPLY				= 0x21,
	DECT_REJECT_LATE_LCE_REPLY				= 0x22,
	DECT_REJECT_INVALID_TPUI				= 0x23,
	DECT_REJECT_TPUI_ASSIGNMENT_LIMITS_UNACCEPTABLE		= 0x24,
	DECT_REJECT_INSUFFICIENT_MEMORY			 	= 0x2f,
	DECT_REJECT_OVERLOAD					= 0x30,
	DECT_REJECT_TEST_CALL_BACK_NORMAL_EN_BLOC		= 0x40,
	DECT_REJECT_TEST_CALL_BACK_NORMAL_PIECEWISE		= 0x41,
	DECT_REJECT_TEST_CALL_BACK_EMERGENCY_EN_BLOC		= 0x42,
	DECT_REJECT_TEST_CALL_BACK_EMERGENCY_PIECEWISE		= 0x43,
	DECT_REJECT_INVALID_MESSAGE				= 0x5f,
	DECT_REJECT_INFORMATION_ELEMENT_ERROR			= 0x60,
	DECT_REJECT_INVALID_INFORMATION_ELEMENT_CONTENTS	= 0x64,
	DECT_REJECT_TIMER_EXPIRY				= 0x70,
	DECT_REJECT_PLMN_NOT_ALLOWED				= 0x76,
	DECT_REJECT_LOCATION_AREA_NOT_ALLOWED			= 0x80,
	DECT_REJECT_LOCATION_NATIONAL_ROAMING_NOT_ALLOWED	= 0x81,
};

struct dect_ie_reject_reason {
	struct dect_ie_common		common;
	enum dect_reject_reasons	reason;
};

/* RES IE */

struct dect_ie_res {
	struct dect_ie_common		common;
};

/* RS IE */

struct dect_ie_rs {
	struct dect_ie_common		common;
};

/* Segmented info IE */

struct dect_ie_segmented_info {
	struct dect_ie_common		common;
};

/* Service change info IE */

struct dect_ie_service_change_info {
	struct dect_ie_common		common;
};

/* Service class IE */

struct dect_ie_service_class {
	struct dect_ie_common		common;
};

/* Setup capability IE */

struct dect_ie_setup_capability {
	struct dect_ie_common		common;
};

/* Terminal capability IE */

struct dect_ie_terminal_capability {
	struct dect_ie_common		common;
};

/* Transit delay IE */

struct dect_ie_transit_delay {
	struct dect_ie_common		common;
};

/* Window size IE */

struct dect_ie_window_size {
	struct dect_ie_common		common;
};

/* ZAP field IE */

struct dect_ie_zap_field {
	struct dect_ie_common		common;
};

/* Escape to proprietary IE */

struct dect_ie_escape_to_proprietary {
	struct dect_ie_common		common;
	uint16_t			emc;
	uint8_t				content[];
};

/* Model identifier IE */

struct dect_ie_model_identifier {
	struct dect_ie_common		common;
};

/* MMS Generic Header IE */

struct dect_ie_mms_generic_header {
	struct dect_ie_common		common;
};

/* MMS Object Header IE */

struct dect_ie_mms_object_header {
	struct dect_ie_common		common;
};

/* MMS Extended Header IE */

struct dect_ie_mms_extended_header {
	struct dect_ie_common		common;
};

/* Time-Date IE */

struct dect_ie_time_date {
	struct dect_ie_common		common;
};

/* Ext h/o indicator IE */

struct dect_ie_ext_ho_indicator {
	struct dect_ie_common		common;
};

/* Authentication Reject Parameter IE */

struct dect_ie_auth_reject_parameter {
	struct dect_ie_common		common;
};

/* Calling party Name IE */

struct dect_ie_calling_party_name {
	struct dect_ie_common		common;
};

/* Codec List IE */

struct dect_ie_codec_list {
	struct dect_ie_common		common;
};

/* Events notification IE */

enum dect_event_types {
	DECT_EVENT_MESSAGE_WAITING		= 0x0,
	DECT_EVENT_MISSED_CALL			= 0x1,
	DECT_EVENT_WEB_CONTENT			= 0x2,
	DECT_EVENT_LIST_CHANGE_INDICATION	= 0x3,
};

enum dect_event_message_waiting_subtypes {
	DECT_EVENT_MESSAGE_WAITING_UNKNOWN	= 0x0,
	DECT_EVENT_MESSAGE_WAITING_VOICE	= 0x1,
	DECT_EVENT_MESSAGE_WAITING_SMS		= 0x2,
	DECT_EVENT_MESSAGE_WAITING_EMAIL	= 0x3,
};

enum dect_event_missed_call_subtypes {
	DECT_EVENT_MISSED_CALL_UNKNOWN		= 0x0,
	DECT_EVENT_MISSED_CALL_VOICE		= 0x1,
};

enum dect_event_web_content_subtypes {
	DECT_EVENT_WEB_CONTENT_UNKNOWN		= 0x0,
	DECT_EVENT_WEB_CONTENT_RSS		= 0x1,
};

struct dect_ie_events_notification {
	struct dect_ie_common		common;
};

/* Call information IE */

struct dect_ie_call_information {
	struct dect_ie_common		common;
};

#endif /* _LIBDECT_DECT_IE_H */
