/*
 * DECT S-Format Information Elements
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_DECT_IE_H
#define _LIBDECT_DECT_IE_H

/**
 * @addtogroup ie
 * @{
 */

#include <string.h>
#include <dect/utils.h>

struct dect_handle;

/**
 * struct dect_ie_collection - common representation of a DECT IE collection
 *
 * @refcnt:		reference count
 * @size:		size of entire collection
 * @ie:			dynamic amount of IEs/IE lists
 */
struct dect_ie_collection {
	unsigned int			refcnt;
	unsigned int			size;
	struct dect_ie_common		*ie[];
};

extern struct dect_ie_collection *__dect_ie_collection_hold(struct dect_ie_collection *iec);

#define dect_ie_collection_hold(iec)	((void *)__dect_ie_collection_hold(&(iec)->common))

extern void __dect_ie_collection_put(const struct dect_handle *dh, struct dect_ie_collection *iec);

#define dect_ie_collection_put(dh, iec)	__dect_ie_collection_put(dh, &(iec)->common)

/**
 * struct dect_ie_common - common representation of a DECT IE
 *
 * @next:		IE list list node
 * @refcnt:		reference count
 */
struct dect_ie_common {
	struct dect_ie_common		*next;
	unsigned int			refcnt;
};

#define dect_ie_container(res, ie)	container_of(ie, typeof(*res), common)

extern struct dect_ie_common *dect_ie_alloc(const struct dect_handle *dh, size_t size);
extern void dect_ie_destroy(const struct dect_handle *dh, struct dect_ie_common *ie);

static inline struct dect_ie_common *__dect_ie_init(struct dect_ie_common *ie)
{
	ie->refcnt = 1;
	ie->next   = NULL;
	return ie;
}

#define dect_ie_init(ie)		dect_ie_container(ie, __dect_ie_init(&(ie)->common))

extern struct dect_ie_common *__dect_ie_clone(const struct dect_handle *dh,
					      const struct dect_ie_common *ie, size_t size);

#define dect_ie_clone(dh, ie)		dect_ie_container(ie, __dect_ie_clone(dh, &(ie)->common, sizeof(*(ie))))

extern struct dect_ie_common *__dect_ie_hold(struct dect_ie_common *ie);

#define dect_ie_hold(ie)		dect_ie_container(ie, __dect_ie_hold(&(ie)->common))

extern void __dect_ie_put(const struct dect_handle *dh, struct dect_ie_common *ie);

#define dect_ie_put(dh, ie)		__dect_ie_put(dh, &(ie)->common)


/* Repeat indicator */

/**
 * enum dect_ie_list_types - Repeat indicator list types
 *
 * @DECT_IE_LIST_NORMAL:	Non priorized list
 * @DECT_IE_PRIORITIZED:	Priorized list
 */
enum dect_ie_list_types {
	DECT_IE_LIST_NORMAL		= 0x1,
	DECT_IE_LIST_PRIORITIZED	= 0x2,
};

struct dect_ie_list {
	struct dect_ie_common		common;
	enum dect_ie_list_types		type;
	struct dect_ie_common		*list;
};

extern void dect_ie_list_init(struct dect_ie_list *iel);
extern struct dect_ie_list *dect_ie_list_hold(struct dect_ie_list *iel);
extern void dect_ie_list_put(const struct dect_handle *dh, struct dect_ie_list *iel);

extern void __dect_ie_list_add(struct dect_ie_common *ie, struct dect_ie_list *iel);

#define dect_ie_list_add(ie, iel)	__dect_ie_list_add(&(ie)->common, iel)

#define dect_foreach_ie(ie, iel) \
	for (ie = (void *)(iel)->list; ie != NULL; \
	     ie = (void *)((struct dect_ie_common *)ie)->next)

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
	DECT_CALL_CLASS_LIA_SERVICE_SETUP		= 0x2,
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
	DECT_SERVICE_SUOTA_CLASS_4_DPRS_MANAGEMENT	= 0x9,
	DECT_SERVICE_SUOTA_CLASS_3_DPRS_MANAGEMENT	= 0xa,
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
	/* security related values */
	DECT_RELEASE_SECURITY_ATTACK_ASSUMED		= 0x40,
	DECT_RELEASE_ENCRYPTION_ACTIVATION_FAILED	= 0x41,
	DECT_RELEASE_REKEYING_FAILED			= 0x42,
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
	uint8_t				auth_id;
	uint8_t				auth_key_num;
	uint8_t				auth_code_num;
};

/* Alphanumeric IE */

struct dect_ie_alphanumeric {
	struct dect_ie_common		common;
};

/* Auth type IE */

enum dect_auth_type_identifiers {
	DECT_AUTH_DSAA				= 0x1,
	DECT_AUTH_GSM				= 0x40,
	DECT_AUTH_UMTS				= 0x20,
	DECT_AUTH_PROPRIETARY			= 0x7f,
};

enum dect_auth_key_types {
	DECT_KEY_USER_AUTHENTICATION_KEY	= 0x1,
	DECT_KEY_USER_PERSONAL_IDENTITY		= 0x3,
	DECT_KEY_AUTHENTICATION_CODE		= 0x4,
};

enum dect_auth_key_flags {
	DECT_AUTH_KEY_IPUI			= 0x0,
	DECT_AUTH_KEY_IPUI_PARK			= 0x8,
};

enum dect_auth_flags {
	DECT_AUTH_FLAG_INC			= 0x80,
	DECT_AUTH_FLAG_DEF			= 0x40,
	DECT_AUTH_FLAG_TXC			= 0x20,
	DECT_AUTH_FLAG_UPC			= 0x10,
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

enum dect_number_type {
	DECT_NUMBER_TYPE_UNKNOWN		= 0x0,
	DECT_NUMBER_TYPE_INTERNATIONAL		= 0x1,
	DECT_NUMBER_TYPE_NATIONAL		= 0x2,
	DECT_NUMBER_TYPE_NETWORK_SPECIFIC	= 0x3,
	DECT_NUMBER_TYPE_SUBSCRIBER		= 0x4,
	DECT_NUMBER_TYPE_ABBREVIATED		= 0x6,
	DECT_NUMBER_TYPE_RESERVED		= 0x7,
};

enum dect_numbering_plan_identification {
	DECT_NPI_UNKNOWN			= 0x0,
	DECT_NPI_ISDN_E164			= 0x1,
	DECT_NPI_DATA_PLAN_X121			= 0x3,
	DECT_NPI_TCP_IP				= 0x7,
	DECT_NPI_NATIONAL_STANDARD		= 0x8,
	DECT_NPI_PRIVATE			= 0x9,
	DECT_NPI_SIP				= 0xa,
	DECT_NPI_INTERNET_CHARACTER_FORMAT	= 0xb,
	DECT_NPI_LAN_MAC_ADDRESS		= 0xc,
	DECT_NPI_X400				= 0xd,
	DECT_NPI_PROFILE_SPECIFIC		= 0xe,
	DECT_NPI_RESERVED			= 0xf,
};

struct dect_ie_called_party_number {
	struct dect_ie_common			common;
	enum dect_number_type			type;
	enum dect_numbering_plan_identification	npi;
	unsigned int				len;
	uint8_t					address[64];
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

enum dect_cipher_algs {
	DECT_CIPHER_STANDARD_1		= 0x1,
	DECT_CIPHER_GRPS_GEA_1		= 0x29,
	DECT_CIPHER_GRPS_GEA_2		= 0x2a,
	DECT_CIPHER_GRPS_GEA_3		= 0x2b,
	DECT_CIPHER_GRPS_GEA_4		= 0x2c,
	DECT_CIPHER_GRPS_GEA_5		= 0x2d,
	DECT_CIPHER_GRPS_GEA_6		= 0x2e,
	DECT_CIPHER_GRPS_GEA_7		= 0x2f,
	DECT_CIPHER_ESC_TO_PROPRIETARY	= 0x7f,
};

enum dect_cipher_key_types {
	DECT_CIPHER_DERIVED_KEY		= 0x9,
	DECT_CIPHER_STATIC_KEY		= 0xa,
};

struct dect_ie_cipher_info {
	struct dect_ie_common		common;
	bool				enable;
	enum dect_cipher_algs		cipher_alg_id;
	enum dect_cipher_key_types	cipher_key_type;
	uint8_t				cipher_key_num;
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

enum dect_lock_limits {
	DECT_LOCK_TEMPORARY_USER_LIMIT_1	= 0x6,
	DECT_LOCK_NO_LIMITS			= 0x7,
	DECT_LOCK_TEMPORARY_USER_LIMIT_2	= 0x5,
};

enum dect_time_limits {
	DECT_TIME_LIMIT_ERASE			= 0x0,
	DECT_TIME_LIMIT_DEFINED_TIME_LIMIT_1	= 0x1,
	DECT_TIME_LIMIT_DEFINED_TIME_LIMIT_2	= 0x2,
	DECT_TIME_LIMIT_STANDARD_TIME_LIMIT	= 0x4,
	DECT_TIME_LIMIT_INFINITE		= 0xf,
};

struct dect_ie_duration {
	struct dect_ie_common		common;
	enum dect_lock_limits		lock;
	enum dect_time_limits		time;
	uint8_t				duration;
};

/* End-to-end compatibility IE */

struct dect_ie_end_to_end_compatibility {
	struct dect_ie_common		common;
};

/* Facility IE */

enum dect_facility_discriminators {
	DECT_FACILITY_SS			= 0x17,
};

struct dect_ie_facility {
	struct dect_ie_common			common;
	enum dect_facility_discriminators	service;
};

/* Feature activate IE */

enum dect_feature {
	DECT_FEATURE_REGISTER_RECALL			= 0x1,
	DECT_FEATURE_EXTERNAL_HO_SWITCH			= 0xf,
	DECT_FEATURE_QUEUE_ENTRY_REQUEST		= 0x20,
	DECT_FEATURE_INDICATION_OF_SUBSCRIBER_NUMBER	= 0x30,
	DECT_FEATURE_FEATURE_KEY			= 0x42,
	DECT_FEATURE_SPECIFIC_LINE_SELECTION		= 0x44,
	DECT_FEATURE_SPECIFIC_TRUNK_SELECTION		= 0x47,
	DECT_FEATURE_ECHO_CONTROL			= 0x48,
	DECT_FEATURE_COST_INFORMATION			= 0x60,
};

struct dect_ie_feature_activate {
	struct dect_ie_common		common;
	enum dect_feature		feature;
};

/* Feature indicate IE */

enum dect_feature_status {
	DECT_FEATURE_SERVICE_REQUEST_REJECTED		= 0x80,
	DECT_FEATURE_SERVICE_REQUEST_ACCEPTED		= 0x81,
	DECT_FEATURE_SERVICE_REQUEST_PENDING		= 0x83,
	DECT_FEATURE_SERVICE_BUSY			= 0x84,
	DECT_FEATURE_SERVICE_UNOBTAINABLE		= 0x86,
};

struct dect_ie_feature_indicate {
	struct dect_ie_common		common;
	enum dect_feature		feature;
	enum dect_feature_status	status;
};

/* Fixed identity IE */

/**
 * @DECT_FIXED_ID_TYPE_ARI:	Access rights identity
 * @DECT_FIXED_ID_TYPE_ARI_RPN:	Access rights identity plus radio fixed part number
 * @DECT_FIXED_ID_TYPE_ARI_WRS:	Access rights identity plus radio fixed part number for WRS
 * @DECT_FIXED_ID_TYPE_PARK:	Portable access rights key
 */
enum dect_fixed_identity_types {
	DECT_FIXED_ID_TYPE_ARI			= 0x00,
	DECT_FIXED_ID_TYPE_ARI_RPN		= 0x01,
	DECT_FIXED_ID_TYPE_ARI_WRS		= 0x02,
	DECT_FIXED_ID_TYPE_PARK			= 0x20,
};

#define S_VL_IE_FIXED_IDENTITY_MIN_SIZE		2

#define S_VL_IE_FIXED_IDENTITY_TYPE_MASK	0x7f
#define S_VL_IE_FIXED_IDENTITY_LENGTH_MASK	0x7f

struct dect_ie_fixed_identity {
	struct dect_ie_common		common;
	enum dect_fixed_identity_types	type;
	struct dect_ari			ari;
	uint8_t				rpn;
};

/* Portable identity IE */

/**
 * @DECT_PORTABLE_ID_TYPE_IPUI:	International Portable User Identity (IPUI)
 * @DECT_PORTABLE_ID_TYPE_IPEI:	International Portable Equipment Identity (IPEI)
 * @DECT_PORTABLE_ID_TYPE_TPUI:	Temporary Portable User Identity (TPUI)
 */
enum dect_portable_identity_types {
	DECT_PORTABLE_ID_TYPE_IPUI		= 0x0,
	DECT_PORTABLE_ID_TYPE_IPEI		= 0x10,
	DECT_PORTABLE_ID_TYPE_TPUI		= 0x20,
};

struct dect_ie_portable_identity {
	struct dect_ie_common			common;
	enum dect_portable_identity_types	type;
	union {
		struct dect_ipui		ipui;
		struct dect_tpui		tpui;
	};
};

/* NetWorK (NWK) assigned identity IE */

/**
 * @DECT_NWK_ID_TMSI:		Temporary mobile subscriber identity (TMSI)
 * @DECT_NWK_ID_PROPRIETARY:	Proprietary (application specific)
 */
enum dect_nwk_identity_types {
	DECT_NWK_ID_TYPE_TMSI			= 0x74,
	DECT_NWK_ID_TYPE_PROPRIETARY		= 0x7f,
};

struct dect_ie_nwk_assigned_identity {
	struct dect_ie_common		common;
	enum dect_nwk_identity_types	type;
};

/* Identity type IE */

/**
 * @DECT_IDENTITY_PORTABLE_IDENTITY:		Portable identity
 * @DECT_IDENTITY_NETWORK_ASSIGNED_IDENTITY:	Network assigned identity
 * @DECT_IDENTITY_FIXED_IDENTITY:		Fixed identity
 * @DECT_IDENTITY_APPLICATION_ASSIGNED:		Application assigned identity
 * @DECT_IDENTITY_PROPRIETARY:			Proprietary identity (application specific)
 */
enum dect_identity_groups {
	DECT_IDENTITY_PORTABLE_IDENTITY		= 0x0,
	DECT_IDENTITY_NETWORK_ASSIGNED_IDENTITY	= 0x1,
	DECT_IDENTITY_FIXED_IDENTITY		= 0x4,
	DECT_IDENTITY_APPLICATION_ASSIGNED	= 0x8,
	DECT_IDENTITY_PROPRIETARY		= 0xf,
};

struct dect_ie_identity_type {
	struct dect_ie_common			common;
	enum dect_identity_groups		group;
	union {
		unsigned int				type;
		enum dect_portable_identity_types	portable;
		enum dect_fixed_identity_types		fixed;
		enum dect_nwk_identity_types		nwk;
	};
};

/* Info type IE */

enum dect_info_parameter_type {
	DECT_INFO_LOCATE_SUGGEST				= 0x0,
	DECT_INFO_ACCESS_RIGHTS_MODIFY_SUGGEST			= 0x1,
	DECT_INFO_PP_AUTHENTICATION_FAILURE			= 0x4,
	DECT_INFO_DYNAMIC_PARAMETERS_ALLOCATION			= 0x6,
	DECT_INFO_EXTERNAL_HO_PARAMETERS			= 0x8,
	DECT_INFO_LOCATION_AREA					= 0x9,
	DECT_INFO_HANDOVER_REFERENCE				= 0xa,
	DECT_INFO_MF_PSCN_SYNCHRONIZED_HANDOVER_CANDIATE	= 0xb,
	DECT_INFO_EXT_HANDOVER_CANDIDATE			= 0xc,
	DECT_INFO_MF_SYNCHRONIZED_HANDOVER_CANDIATE		= 0xd,
	DECT_INFO_MF_PSCN_MFN_SYNCHRONIZED_HANDOVER_CANDIATE	= 0xe,
	DECT_INFO_NON_SYNCHRONIZED_HANDOVER_CANDIDATE		= 0xf,
	DECT_INFO_OLD_FIXED_PART_IDENTITY			= 0x10,
	DECT_INFO_OLD_NETWORK_ASSIGNED_IDENTITY			= 0x11,
	DECT_INFO_OLD_NETWORK_ASSIGNED_LOCATION_AREA		= 0x12,
	DECT_INFO_OLD_NETWORK_ASSIGNED_HANDOVER_REFERENCE	= 0x13,
	DECT_INFO_BILLING					= 0x20,
	DECT_INFO_DEBITING					= 0x21,
	DECT_INFO_CK_TRANSFER					= 0x22,
	DECT_INFO_HANDOVER_FAILED_REVERSION			= 0x23,
	DECT_INFO_QA_M_CALL					= 0x24,
	DECT_INFO_DISTRIBUTED_COMMUNICATION_DOWNLOAD		= 0x25,
	DECT_INFO_ETHERNET_ADDRESS				= 0x30,
	DECT_INFO_TOKEN_RING_ADDRESS				= 0x31,
	DECT_INFO_IPV4_ADDRESS					= 0x32,
	DECT_INFO_IPV6_ADDRESS					= 0x33,
	DECT_INFO_IDENTITY_ALLOCATION				= 0x34,
};

struct dect_ie_info_type {
	struct dect_ie_common		common;
	unsigned int			num;
	enum dect_info_parameter_type	type[8];
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

enum dect_iwu_to_iwu_discriminators {
	DECT_IWU_TO_IWU_PD_USER_SPECIFIC			= 0x0,
	DECT_IWU_TO_IWU_PD_OSI_HIGHER_LAYER			= 0x1,
	DECT_IWU_TO_IWU_PD_ITU_T_X263				= 0x2,
	DECT_IWU_TO_IWU_PD_LIST_ACCESS				= 0x3,
	DECT_IWU_TO_IWU_PD_IA5_CHARACTERS			= 0x4,
	DECt_IWU_TO_IWU_PD_LDS_SUOTA				= 0x6,
	DECT_IWU_TO_IWU_PD_ITU_T_V120				= 0x7,
	DECT_IWU_TO_IWU_PD_ITU_T_Q931_MESSAGE			= 0x8,
	DECT_IWU_TO_IWU_PD_ITU_T_Q931_IE			= 0x9,
	DECT_IWU_TO_IWU_PD_ITU_T_Q931_PARTIAL_MESSAGE		= 0xa,
	DECT_IWU_TO_IWU_PD_GSM_MESSAGE				= 0x10,
	DECT_IWU_TO_IWU_PD_GSM_IE				= 0x11,
	DECT_IWU_TO_IWU_PD_UMTS_GPRS_IE				= 0x12,
	DECT_IWU_TO_IWU_PD_UMTS_GPRS_MESSAGE			= 0x13,
	DECT_IWU_TO_IWU_PD_LRMS					= 0x14,
	DECT_IWU_TO_IWU_PD_RLL_ACCESS_PROFILE			= 0x15,
	DECT_IWU_TO_IWU_PD_WRS					= 0x16,
	DECT_IWU_TO_IWU_PD_DECT_ISDN_C_PLANE_SPECIFIC		= 0x20,
	DECT_IWU_TO_IWU_PD_DECT_ISDN_U_PLANE_SPECIFIC		= 0x21,
	DECT_IWU_TO_IWU_PD_DECT_ISDN_OPERATION_AND_MAINTENANCE	= 0x22,
	DECT_IWU_TO_IWU_PD_TERMINAL_DATA			= 0x23,
	DECT_IWU_TO_IWU_PD_DECT_IP_NETWORK_ACCESS_SPECIFIC	= 0x24,
	DECT_IWU_TO_IWU_PD_MPEG4_ER_AAL_LD_CONFIGURATION	= 0x25,
	DECT_IWU_TO_IWU_PD_UNKNOWN				= 0x3f,
};

struct dect_ie_iwu_to_iwu {
	struct dect_ie_common		common;
	bool				sr;
	uint8_t				pd;
	uint8_t				len;
	uint8_t				data[256];
};

/* Key IE */

struct dect_ie_key {
	struct dect_ie_common		common;
};

/* Location area IE */

enum dect_location_area_types {
	DECT_LOCATION_AREA_LEVEL		= 0x1 << 0,
	DECT_LOCATION_AREA_EXTENDED		= 0x1 << 1,
};

struct dect_ie_location_area {
	struct dect_ie_common		common;
	uint8_t				type;
	uint8_t				level;
};

/* Network parameter IE */

struct dect_ie_network_parameter {
	struct dect_ie_common		common;
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

/* RAND/RS IE */

struct dect_ie_auth_value {
	struct dect_ie_common		common;
	uint64_t			value;
};

/* RES IE */

struct dect_ie_auth_res {
	struct dect_ie_common		common;
	uint32_t			value;
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

enum dect_page_capabilities {
	DECT_PAGE_CAPABILITY_NORMAL_PAGING			= 0x1,
	DECT_PAGE_CAPABILITY_FAST_AND_NORMAL_PAGING		= 0x2,
};

enum dect_setup_capabilities {
	DECT_SETUP_SELECTIVE_FAST_SETUP				= 0x0,
	DECT_SETUP_NO_FAST_SETUP				= 0x1,
	DECT_SETUP_COMPLETE_FAST_SETUP				= 0x2,
	DECT_SETUP_COMPLETE_AND_SELECTIVE_FAST_SETUP		= 0x3,
};

struct dect_ie_setup_capability {
	struct dect_ie_common		common;
	enum dect_page_capabilities	page_capability;
	enum dect_setup_capabilities	setup_capability;
};

/* Terminal capability IE */

enum dect_display_capabilities {
	DECT_DISPLAY_CAPABILITY_NOT_APPLICABLE			= 0x0,
	DECT_DISPLAY_CAPABILITY_NO_DISPLAY			= 0x1,
	DECT_DISPLAY_CAPABILITY_NUMERIC				= 0x2,
	DECT_DISPLAY_CAPABILITY_NUMERIC_PLUS			= 0x3,
	DECT_DISPLAY_CAPABILITY_ALPHANUMERIC			= 0x4,
	DECT_DISPLAY_CAPABILITY_FULL_DISPLAY			= 0x5,
};

enum dect_tone_capabilities {
	DECT_TONE_CAPABILITY_NOT_APPLICABLE			= 0x0,
	DECT_TONE_CAPABILITY_NO_TONE				= 0x1,
	DECT_TONE_CAPABILITY_DIAL_TONE_ONLY			= 0x2,
	DECT_TONE_CAPABILITY_ITU_T_E182_TONES			= 0x3,
	DECT_TONE_CAPABILITY_COMPLETE_DECT_TONES		= 0x4,
};

enum dect_echo_parameters {
	DECT_ECHO_PARAMETER_NOT_APPLICABLE			= 0x0,
	DECT_ECHO_PARAMETER_MINIMUM_TCLW			= 0x1,
	DECT_ECHO_PARAMETER_FULL_TCLW				= 0x2,
	DECT_ECHO_PARAMETER_VOIP_COMPATIBLE_TLCW		= 0x3,
};

enum dect_noise_rejection_capabilities {
	DECT_NOISE_REJECTION_NOT_APPLICABLE			= 0x0,
	DECT_NOISE_REJECTION_NONE				= 0x1,
	DECT_NOISE_REJECTION_PROVIDED				= 0x2,
};

enum dect_adaptive_volume_control_provision {
	DECT_ADAPTIVE_VOLUME_NOT_APPLICABLE			= 0x0,
	DECT_ADAPTIVE_VOLUME_PP_CONTROL_NONE			= 0x1,
	DECT_ADAPTIVE_VOLUME_PP_CONTROL_USED			= 0x2,
	DECT_ADAPTIVE_VOLUME_FP_CONTROL_DISABLE			= 0x3,
};

enum dect_slot_capabilities {
	DECT_SLOT_CAPABILITY_HALF_SLOT				= 1 << 0,
	DECT_SLOT_CAPABILITY_LONG_SLOT_640			= 1 << 1,
	DECT_SLOT_CAPABILITY_LONG_SLOT_672			= 1 << 2,
	DECT_SLOT_CAPABILITY_FULL_SLOT				= 1 << 3,
	DECT_SLOT_CAPABILITY_DOUBLE_SLOT			= 1 << 4,
};

enum dect_scrolling_behaviours {
	DECT_SCROLLING_NOT_SPECIFIED				= 0x0,
	DECT_SCROLLING_TYPE_1					= 0x1,
	DECT_SCROLLING_TYPE_2					= 0x2,
};

enum dect_profile_indicators {
	DECT_PROFILE_DPRS_ASYMETRIC_BEARERS_SUPPORTED		= 0x4000000000000000ULL,
	DECT_PROFILE_DPRS_STREAM_SUPPORTED			= 0x2000000000000000ULL,
	DECT_PROFILE_LRMS_SUPPORTED				= 0x1000000000000000ULL,
	DECT_PROFILE_ISDN_END_SYSTEM_SUPPORTED			= 0x0800000000000000ULL,
	DECT_PROFILE_DECT_GSM_INTERWORKING_PROFILE_SUPPORTED	= 0x0400000000000000ULL,
	DECT_PROFILE_GAP_SUPPORTED				= 0x0200000000000000ULL,
	DECT_PROFILE_CAP_SUPPORTED				= 0x0100000000000000ULL,
	DECT_PROFILE_RAP_1_PROFILE_SUPPORTED			= 0x0040000000000000ULL,
	DECT_PROFILE_UMTS_GSM_FACSIMILE_SUPPORTED		= 0x0020000000000000ULL,
	DECT_PROFILE_UMTS_GSM_SMS_SERVICE_SUPPORTED		= 0x0010000000000000ULL,
	DECT_PROFILE_UMTS_GSM_BEARER_SERVICE			= 0x0008000000000000ULL,
	DECT_PROFILE_ISDN_IAP_SUPPORTED				= 0x0004000000000000ULL,
	DECT_PROFILE_DATA_SERVICES_PROFILE_D			= 0x0002000000000000ULL,
	DECT_PROFILE_DPRS_FREL_SUPPORTED			= 0x0001000000000000ULL,
	DECT_PROFILE_TOKEN_RING_SUPPORTED			= 0x0000400000000000ULL,
	DECT_PROFILE_ETHERNET_SUPPORTED				= 0x0000200000000000ULL,
	DECT_PROFILE_MULTIPORT_CTA				= 0x0000100000000000ULL,
	DECT_PROFILE_DMAP_SUPPORTED				= 0x0000080000000000ULL,
	DECT_PROFILE_SMS_OVER_LRMS_SUPPORTED			= 0x0000040000000000ULL,
	DECT_PROFILE_WRS_SUPPORTED				= 0x0000020000000000ULL,
	DECT_PROFILE_DECT_GSM_DUAL_MODE_TERMINAL		= 0x0000010000000000ULL,
	DECT_PROFILE_DPRS_SUPPORTED				= 0x0000004000000000ULL,
	DECT_PROFILE_RAP_2_PROFILE_SUPPORTED			= 0x0000002000000000ULL,
	DECT_PROFILE_I_PQ_SERVICES_SUPPORTED			= 0x0000001000000000ULL,
	DECT_PROFILE_C_F_CHANNEL_SUPPORTED			= 0x0000000800000000ULL,
	DECT_PROFILE_V_24_SUPPORTED				= 0x0000000400000000ULL,
	DECT_PROFILE_PPP_SUPPORTED				= 0x0000000200000000ULL,
	DECT_PROFILE_IP_SUPPORTED				= 0x0000000100000000ULL,
	DECT_PROFILE_8_LEVEL_A_FIELD_MODULATION			= 0x0000000040000000ULL,
	DECT_PROFILE_4_LEVEL_A_FIELD_MODULATION			= 0x0000000020000000ULL,
	DECT_PROFILE_2_LEVEL_A_FIELD_MODULATION			= 0x0000000010000000ULL,
	DECT_PROFILE_16_LEVEL_BZ_FIELD_MODULATION		= 0x0000000008000000ULL,
	DECT_PROFILE_8_LEVEL_BZ_FIELD_MODULATION		= 0x0000000004000000ULL,
	DECT_PROFILE_4_LEVEL_BZ_FIELD_MODULATION		= 0x0000000002000000ULL,
	DECT_PROFILE_2_LEVEL_BZ_FIELD_MODULATION		= 0x0000000001000000ULL,
	DECT_PROFILE_NO_EMISSION_MODE_SUPPORTED			= 0x0000000000400000ULL,
	DECT_PROFILE_PT_WITH_FAST_HOPPING_RADIO			= 0x0000000000200000ULL,
	DECT_PROFILE_G_F_CHANNEL_SUPPORTED			= 0x0000000000100000ULL,
	DECT_PROFILE_F_MMS_INTERWORKING_PROFILE_SUPPORTED	= 0x0000000000080000ULL,
	DECT_PROFILE_BASIC_ODAP_SUPPORTED			= 0x0000000000040000ULL,
	DECT_PROFILE_DECT_UMTS_INTERWORKING_GPRS_SUPPORTED	= 0x0000000000020000ULL,
	DECT_PROFILE_DECT_UMTS_INTERWORKING_PROFILE_SUPPORTED	= 0x0000000000010000ULL,
	DECT_PROFILE_REKEYING_EARLY_ENCRYPTION_SUPPORTED	= 0x0000000000001000ULL,
	DECT_PROFILE_HEADSET_MANAGEMENT_SUPPORTED		= 0x0000000000000800ULL,
	DECT_PROFILE_NG_DECT_PART_3				= 0x0000000000000400ULL,
	DECT_PROFILE_NG_DECT_PART_1				= 0x0000000000000200ULL,
	DECT_PROFILE_64_LEVEL_BZ_FIELD_MODULATION		= 0x0000000000000100ULL,
};

enum dect_display_control_codes {
	DECT_DISPLAY_CONTROL_CODE_NOT_SPECIFIED			= 0x0,
	DECT_DISPLAY_CONTROL_CODE_CLEAR_DISPLAY			= 0x1,
	DECT_DISPLAY_CONTROL_CODE_CODING_1			= 0x2,
	DECT_DISPLAY_CONTROL_CODE_CODING_2			= 0x3,
	DECT_DISPLAY_CONTROL_CODE_CODING_3			= 0x4,
};

enum dect_display_character_sets {
	DECT_DISPLAY_CHARACTER_SET_ISO_8859_1			= 1 << 0,
	DECT_DISPLAY_CHARACTER_SET_ISO_8859_15			= 1 << 1,
	DECT_DISPLAY_CHARACTER_SET_ISO_8859_9			= 1 << 2,
	DECT_DISPLAY_CHARACTER_SET_ISO_8859_7			= 1 << 3,
};

struct dect_ie_terminal_capability {
	struct dect_ie_common				common;

	enum dect_tone_capabilities			tone;
	enum dect_echo_parameters			echo;
	enum dect_noise_rejection_capabilities		noise_rejection;
	enum dect_adaptive_volume_control_provision	volume_ctrl;
	uint8_t						slot;

	enum dect_display_capabilities			display;
	uint16_t					display_memory;
	uint8_t						display_lines;
	uint8_t						display_columns;
	enum dect_display_control_codes			display_control;
	enum dect_display_character_sets		display_charsets;
	enum dect_scrolling_behaviours			scrolling;

	uint64_t					profile_indicator;
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
	uint8_t				len;
	uint8_t				content[64];
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

enum dect_negotiation_indicator {
	DECT_NEGOTIATION_NOT_POSSIBLE		= 0x0,
	DECT_NEGOTIATION_CODEC			= 0x1,
};

enum dect_codec_identifier {
	DECT_CODEC_USER_SPECIFIC_32KBIT		= 0x1,
	DECT_CODEC_G726_32KBIT			= 0x2,
	DECT_CODEC_G722_64KBIT			= 0x3,
	DECT_CODEC_G711_ALAW_64KBIT		= 0x4,
	DECT_CODEC_G711_ULAW_64KBIT		= 0x5,
	DECT_CODEC_G729_1_32KBIT		= 0x6,
	DECT_CODEC_MPEG4_ER_AAC_LD_32KBIT	= 0x7,
	DECT_CODEC_MPEG4_ER_AAC_LD_64KBIT	= 0x8,
	DECT_CODEC_USER_SPECIFIC_64KBIT		= 0x9,
};

enum dect_mac_dlc_service {
	DECT_MAC_DLC_SERVICE_LU1_INA		= 0x0,
	DECT_MAC_DLC_SERVICE_LU1_INB		= 0x1,
	DECT_MAC_DLC_SERVICE_LU1_IPM		= 0x2,
	DECT_MAC_DLC_SERVICE_LU1_IPQ		= 0x3,
	DECT_MAC_DLC_SERVICE_LU7_INB		= 0x4,
	DECT_MAC_DLC_SERVICE_LU12_INB		= 0x5,
};

enum dect_slot_size {
	DECT_HALF_SLOT				= 0x0,
	DECT_LONG_SLOT_640			= 0x1,
	DECT_LONG_SLOT_672			= 0x2,
	DECT_FULL_SLOT				= 0x4,
	DECT_DOUBLE_SLOT			= 0x5,
};

enum dect_cplane_routing {
	DECT_CPLANE_CS_ONLY			= 0x0,
	DECT_CPLANE_CS_PREFERRED		= 0x1,
	DECT_CPLANE_CF_PREFERRED		= 0x2,
	DECT_CPLANE_CF_ONLY			= 0x4,
};

struct dect_ie_codec_list {
	struct dect_ie_common		common;
	enum dect_negotiation_indicator	negotiation;
	unsigned int			num;
	struct {
		enum dect_codec_identifier	codec;
		enum dect_mac_dlc_service	service;
		enum dect_slot_size		slot;
		enum dect_cplane_routing	cplane;
	} entry[8];
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

/* @} */

#endif /* _LIBDECT_DECT_IE_H */
