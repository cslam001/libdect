/*
 * DECT S-Format Information Elements
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_DECT_IE_H
#define _LIBDECT_DECT_IE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup ie
 * @{
 */

#include <stdbool.h>
#include <string.h>

#include <dect/utils.h>
#include <dect/identities.h>

struct dect_handle;

/**
 * Common representation of a DECT IE collection
 *
 * IE collections are used to encapsulate sets of IEs, like parameters of libdect
 * primitives. IE collections passed upwards from libdect are allocated and
 * reference counted, the contained IEs will not be released while references to
 * the IE collection are held. IE collections passed to libdect primitives may be
 * allocated on the stack.
 *
 * The members of this structure may only be manipulated through the appropriate
 * helper functions.
 */
struct dect_ie_collection {
	unsigned int			refcnt;	/**< Reference count */
	unsigned int			size;	/**< Size if bytes of entire collection */
	struct dect_ie_common		*ie[];	/**< Dynamic amount of IEs/IE lists */
};

extern void *dect_ie_collection_alloc(const struct dect_handle *dh, unsigned int size);

extern struct dect_ie_collection *__dect_ie_collection_hold(struct dect_ie_collection *iec);

/**
 * Increase the reference count of an IE collection
 */
#define dect_ie_collection_hold(iec)	((void *)__dect_ie_collection_hold(&(iec)->common))

extern void __dect_ie_collection_put(const struct dect_handle *dh, struct dect_ie_collection *iec);

/**
 * Release a reference of an IE collection. If the reference count reaches zero,
 * the IE collection will be freed.
 */
#define dect_ie_collection_put(dh, iec)	__dect_ie_collection_put(dh, &(iec)->common)

/**
 * Common representation of a DECT IE
 *
 * Every IE contains a struct dect_ie_common to support operations common to all
 * IE types. IEs passed upwards from libdect are allocated and reference counted,
 * they will not be freed while references to the Information Element are held.
 * IEs Elements passed to libdect primitives may be allocated on the stack.
 *
 * The members of this structure may only be manipulated through the appropriate
 * helper functions.
 */
struct dect_ie_common {
	struct dect_ie_common		*next;	/**< IE list list node */
	unsigned int			refcnt;	/**< Reference count */
};

/**
 * Get a pointer to the containing IE from a struct dect_ie_common.
 */
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

/**
 * Clone an IE.
 */
#define dect_ie_clone(dh, ie)		dect_ie_container(ie, __dect_ie_clone(dh, &(ie)->common, sizeof(*(ie))))

extern struct dect_ie_common *__dect_ie_hold(struct dect_ie_common *ie);

/**
 * Increase the reference count of an IE.
 */
#define dect_ie_hold(ie)		dect_ie_container(ie, __dect_ie_hold(&(ie)->common))

extern void __dect_ie_put(const struct dect_handle *dh, struct dect_ie_common *ie);

/**
 * Release a reference of an IE. If the reference count drops to zero, the IE will be freed.
 */
#define dect_ie_put(dh, ie)		__dect_ie_put(dh, &(ie)->common)


/* Repeat indicator */

/**
 * Repeat indicator list types
 */
enum dect_ie_list_types {
	DECT_IE_LIST_NORMAL		= 0x1, /**< Non prioritized list */
	DECT_IE_LIST_PRIORITIZED	= 0x2, /**< Prioritized list */
};

/**
 * Repeat indicator
 *
 * A repeat indicator contains a list of Information Elements.
 */
struct dect_ie_list {
	struct dect_ie_common		common;
	enum dect_ie_list_types		type;
	struct dect_ie_common		*list;
};

extern void dect_ie_list_init(struct dect_ie_list *iel);
extern struct dect_ie_list *dect_ie_list_hold(struct dect_ie_list *iel);
extern void dect_ie_list_put(const struct dect_handle *dh, struct dect_ie_list *iel);

extern void __dect_ie_list_add(struct dect_ie_common *ie, struct dect_ie_list *iel);

/**
 * Add an IE to an IE list.
 */
#define dect_ie_list_add(ie, iel)	__dect_ie_list_add(&(ie)->common, iel)

/**
 * Iterate over all IE contained in an IE list.
 */
#define dect_foreach_ie(ie, iel) \
	for (ie = (void *)(iel)->list; ie != NULL; \
	     ie = (void *)((struct dect_ie_common *)ie)->next)

/**
 * @defgroup ie_cc_related Call Control related
 *
 * Call Control related IEs.
 *
 * @{
 * @defgroup ie_sending_complete Sending complete
 *
 * <<SENDING-COMPLETE>> IE specified in ETSI EN 300 175-5 section 7.6.2.
 *
 * The <<SENDING-COMPLETE>> IE indicates completion of the called party number.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.6.2
 * @{
 */

/** <<SENDING-COMPLETE>> IE */
struct dect_ie_sending_complete {
	struct dect_ie_common		common;
};

/**
 * @}
 * @defgroup ie_delimiter_request Delimiter request
 *
 * <<DELIMITER-REQUEST>> IE specified in ETSI EN 300 175-5 section 7.6.2.
 *
 * The <<DELIMITER-REQUEST>> IE requests the peer to return a <<SENDING-COMPLETE>>
 * IE when the called party number is complete.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.6.2
 * @{
 */

/** <<DELIMITER-REQUEST>> IE */
struct dect_ie_delimiter_request {
	struct dect_ie_common		common;
};

/**
 * @}@}
 * @defgroup ie_use_tpui Use TPUI
 *
 * <<USE-TPUI>> IE specified in ETSI EN 300 175-5 section 7.6.2.
 *
 * The <<USE-TPUI>> IE indicates that the PT may use its TPUI in the
 * <<PORTABLE-IDENTITY>> IE in the {CC-SETUP} and {LCE-PAGE-RESPONCE}
 * messages.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.6.2
 * @{
 */

/** <<USE-TPUI>> IE */
struct dect_ie_use_tpui {
	struct dect_ie_common		common;
};

/**
 * @}
 * @addtogroup ie_cc_related
 * @{
 * @defgroup ie_basic_service Basic service
 *
 * <<BASIC-SERVICE>> IE specified in ETSI EN 300 175-5 section 7.6.4.
 *
 * The <<BASIC-SERVICE>> IE indicates the basic aspects of the requested service.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.6.4
 * @{
 **/

#define DECT_BASIC_SERVICE_CALL_CLASS_MASK		0xf0
#define DECT_BASIC_SERVICE_CALL_CLASS_SHIFT		4

/**
 * Call classes specified in ETSI EN 300 175-5 section 7.6.4.
 */
enum dect_call_classes {
	DECT_CALL_CLASS_LIA_SERVICE_SETUP		= 0x2, /**< LiA service setup */
	DECT_CALL_CLASS_MESSAGE				= 0x4, /**< Message call setup */
	DECT_CALL_CLASS_DECT_ISDN			= 0x7, /**< DECT/ISDN IIP */
	DECT_CALL_CLASS_NORMAL				= 0x8, /**< Normal call setup */
	DECT_CALL_CLASS_INTERNAL			= 0x9, /**< Internal call setup */
	DECT_CALL_CLASS_EMERGENCY			= 0xa, /**< Emergency call setup */
	DECT_CALL_CLASS_SERVICE				= 0xb, /**< Service call setup */
	DECT_CALL_CLASS_EXTERNAL_HO			= 0xc, /**< External handover call setup */
	DECT_CALL_CLASS_SUPPLEMENTARY_SERVICE		= 0xd, /**< Supplementary service call setup */
	DECT_CALL_CLASS_QA_M				= 0xe, /**< OA&M call setup */
};
#define DECT_CALL_CLASS_MAX				0xf

#define DECT_BASIC_SERVICE_SERVICE_MASK			0x0f

/**
 * Basic service values specified in ETSI EN 300 175-5 section 7.6.4.
 */
enum dect_basic_service {
	DECT_SERVICE_BASIC_SPEECH_DEFAULT		= 0x0, /**< Basic speech default setup attributes */
	DECT_SERVICE_DECT_GSM_IWP			= 0x4, /**< DECT GSM IWP profile */
	DECT_SERVICE_UMTS_IWP				= 0x6, /**< DECT UMTS IWP */
	DECT_SERVICE_LRMS				= 0x5, /**< GSM IWP SMS */
	DECT_SERVICE_GSM_IWP_SMS			= 0x6, /**< LRMS (E-profile) service */
	DECT_SERVICE_WIDEBAND_SPEECH			= 0x8, /**< Wideband speech default setup attributes */
	DECT_SERVICE_SUOTA_CLASS_4_DPRS_MANAGEMENT	= 0x9, /**< SUOTA, Class 4 DPRS management, default setup attributes */
	DECT_SERVICE_SUOTA_CLASS_3_DPRS_MANAGEMENT	= 0xa, /**< SUOTA, Class 3 DPRS management, default setup attributes */
	DECT_SERVICE_OTHER				= 0xf, /**< Other */
};
#define DECT_SERVICE_MAX				0xf

/** <<BASIC-SERVICE>> IE */
struct dect_ie_basic_service {
	struct dect_ie_common		common;
	enum dect_call_classes		class;
	enum dect_basic_service		service;
};

/**
 * @}@}
 * @defgroup ie_generic Generic
 * @{
 * @defgroup ie_release_reason Release reason
 *
 * <<RELEASE-REASON>> IE specified in ETSI EN 300 175-5 section 7.6.7.
 *
 * The <<RELEASE-REASON>> IE conveys the cause of the release.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.6.7
 * @{
 */

/** Release reasons */
enum dect_release_reasons {
	/* general values */
	DECT_RELEASE_NORMAL				= 0x0, /**< Normal */
	DECT_RELEASE_UNEXPECTED_MESSAGE			= 0x1, /**< Unexpected Message */
	DECT_RELEASE_UNKNOWN_TRANSACTION_IDENTIFIER	= 0x2, /**< Unknown Transaction Identifier */
	DECT_RELEASE_MANDATORY_IE_MISSING		= 0x3, /**< Mandatory information element missing */
	DECT_RELEASE_INVALID_IE_CONTENTS		= 0x4, /**< Invalid information element contents */
	DECT_RELEASE_INCOMPATIBLE_SERVICE		= 0x5, /**< Incompatible service */
	DECT_RELEASE_SERVICE_NOT_IMPLEMENTED		= 0x6, /**< Service not implemented */
	DECT_RELEASE_NEGOTIATION_NOT_SUPPORTED		= 0x7, /**< Negotiation not supported */
	DECT_RELEASE_INVALID_IDENTITY			= 0x8, /**< Invalid identity */
	DECT_RELEASE_AUTHENTICATION_FAILED		= 0x9, /**< Authentication failed */
	DECT_RELEASE_UNKNOWN_IDENTITY			= 0xa, /**< Unknown identity */
	DECT_RELEASE_NEGOTIATION_FAILED			= 0xb, /**< Negotiation failed */
	DECT_RELEASE_TIMER_EXPIRY			= 0xd, /**< Timer expiry */
	DECT_RELEASE_PARTIAL_RELEASE			= 0xe, /**< Partial release */
	DECT_RELEASE_UNKNOWN				= 0xf, /**< Unknown */
	/* user values */
	DECT_RELEASE_USER_DETACHED			= 0x10, /**< User detached */
	DECT_RELEASE_USER_NOT_IN_RANGE			= 0x11, /**< User not in range */
	DECT_RELEASE_USER_UNKNOWN			= 0x12, /**< User unknown */
	DECT_RELEASE_USER_ALREADY_ACTIVE		= 0x13, /**< User already active */
	DECT_RELEASE_USER_BUSY				= 0x14, /**< User busy */
	DECT_RELEASE_USER_REJECTION			= 0x15, /**< User rejection */
	DECT_RELEASE_USER_CALL_MODIFY			= 0x16, /**< User call modify */
	/* external handover values */
	DECT_RELEASE_EXTERNAL_HANDOVER_NOT_SUPPORTED	= 0x21, /**< External Handover not supported */
	DECT_RELEASE_NETWORK_PARAMETERS_MISSING		= 0x22, /**< Network Parameters missing */
	DECT_RELEASE_EXTERNAL_HANDOVER_RELEASE		= 0x23, /**< External Handover release */
	/* temporary overload values */
	DECT_RELEASE_OVERLOAD				= 0x31, /**< Overload */
	DECT_RELEASE_INSUFFICIENT_RESOURCES		= 0x32, /**< Insufficient resources */
	DECT_RELEASE_INSUFFICIENT_BEARERS_AVAILABLE	= 0x33, /**< Insufficient bearers available */
	DECT_RELEASE_IWU_CONGESTION			= 0x34, /**< IWU congestion */
	/* security related values */
	DECT_RELEASE_SECURITY_ATTACK_ASSUMED		= 0x40, /**< Security attack assumed */
	DECT_RELEASE_ENCRYPTION_ACTIVATION_FAILED	= 0x41, /**< Encryption activation failed */
	DECT_RELEASE_REKEYING_FAILED			= 0x42, /**< Re-Keying failed */
};

/** <<RELEASE-REASON>> IE */
struct dect_ie_release_reason {
	struct dect_ie_common		common;
	enum dect_release_reasons	reason;
};

/**
 * @}@}
 * @addtogroup ie_cc_related
 * @{
 * @defgroup ie_display Display IE (Single Display/Multi Display)
 *
 * <<SINGLE-DISPLAY>> and <<MULTI-DISPLAY>> IEs specified in ETSI EN 300 175-5
 * section 7.6.5 and 7.7.26.
 *
 * The <<SINGLE-DISPLAY>> and <<MULTI-DISPLAY>> IEs convey display information
 * that may be displayed to the PT.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.6.5
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.26
 * @{
 */

/** <<SINGLE-DISPLAY>>/<<MULTI-DISPLAY>> IEs */
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

/**
 * @}
 * @defgroup ie_keypad Keypad IE (Single Keypad/Multi Keypad)
 *
 * <<SINGLE-KEYPAD>> and <<MULTI-KEYPAD>> IEs specified in ETSI EN 300 175-6
 * section 7.6.5 and 7.7.27.
 *
 * The <<SINGLE-KEYPAD>> and <<MULTI-KEYPAD>> IEs convey DECT standard characters
 * to the FT.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.6.6
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.27
 * @{
 */

/** <<SINGLE-KEYPAD>>/<<MULTI-KEYPAD>> IE */
struct dect_ie_keypad {
	struct dect_ie_common		common;
	uint16_t			len;
	uint8_t				info[256];
};

/**
 * @}
 * @defgroup ie_signal Signal
 *
 * <<SIGNAL>> IE specified in ETSI EN 300 175-6 section 7.6.8.
 *
 * The <<SIGNAL>> IE conveys signal information to the PT.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.6.8
 * @{
 */

/** Ring patterns */
enum dect_alerting_patterns {
	DECT_ALERTING_PATTERN_0				= 0x0, /**< Alerting on - pattern 0 */
	DECT_ALERTING_PATTERN_1				= 0x1, /**< Alerting on - pattern 1 */
	DECT_ALERTING_PATTERN_2				= 0x2, /**< Alerting on - pattern 2 */
	DECT_ALERTING_PATTERN_3				= 0x3, /**< Alerting on - pattern 3 */
	DECT_ALERTING_PATTERN_4				= 0x4, /**< Alerting on - pattern 4 */
	DECT_ALERTING_PATTERN_5				= 0x5, /**< Alerting on - pattern 5 */
	DECT_ALERTING_PATTERN_6				= 0x6, /**< Alerting on - pattern 6 */
	DECT_ALERTING_PATTERN_7				= 0x7, /**< Alerting on - pattern 7 */
	DECT_ALERTING_CONTINUOUS			= 0x8, /**< Alerting on - continous */
	DECT_ALERTING_INCOMING_CALL_RELEASED		= 0xa, /**< Incoming call has been released by the FP */
	DECT_ALERTING_INCOMING_CALL_ANSWERED		= 0xb, /**< Incoming call has been answered */
	DECT_ALERTING_OFF				= 0xf, /**< Alerting off */
};

/** Signal codes */
enum dect_signal_codes {
	DECT_SIGNAL_DIAL_TONE_ON			= 0x0,	/**< Dial tone on */
	DECT_SIGNAL_RING_BACK_TONE_ON			= 0x1,	/**< Dial tone off */
	DECT_SIGNAL_INTERCEPT_TONE_ON			= 0x2,	/**< Intercept tone on */
	DECT_SIGNAL_NETWORK_CONGESTION_TONE_ON		= 0x3,	/**< Network congestion tone on */
	DECT_SIGNAL_BUSY_TONE_ON			= 0x4,	/**< Busy tone on */
	DECT_SIGNAL_CONFIRM_TONE_ON			= 0x5,	/**< Confirm tone on */
	DECT_SIGNAL_ANSWER_TONE_ON			= 0x6,	/**< Answer tone on */
	DECT_SIGNAL_CALL_WAITING_TONE_ON		= 0x7,	/**< Call waiting tone on */
	DECT_SIGNAL_OFF_HOOK_WARNING_TONE_ON		= 0x8,	/**< Off-hook warning tone on */
	DECT_SIGNAL_NEGATIVE_ACKNOWLEDGEMENT_TONE	= 0x9,	/**< Negative acknowledgement tone on */
	DECT_SIGNAL_TONES_OFF				= 0x3f,	/**< Tones off */
	DECT_SIGNAL_ALERTING_BASE			= 0x40,	/**< Base value for ring patterns */
};

/** <<SIGNAL>> IE */
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

/**
 * @}@}
 * @addtogroup ie_generic
 * @{
 * @defgroup ie_timer_restart Timer restart
 *
 * <<TIMER-RESTART>> IE specified in ETSI EN 300 175-5 section 7.6.9.
 *
 * The <<TIMER-RESTART>> IE is used to stop or restart a NWK-layer timer.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.6.9
 * @{
 */

/** Timer restart values */
enum dect_timer_restart_values {
	DECT_TIMER_RESTART		= 0x0, /**< Restart timer */
	DECT_TIMER_STOP			= 0x1, /**< Stop timer */
};

/** <<TIMER-RESTART>> IE */
struct dect_ie_timer_restart {
	struct dect_ie_common		common;
	enum dect_timer_restart_values	code;
};

/**
 * @}
 * @defgroup ie_test_hook_control Test hook control
 *
 * <<TEST-HOOK-CONTROL>> IE specified in ETSI EN 300 175-5 section 7.6.10.
 *
 * The <<TEST-HOOK-CONTROL>> IE conveys the remote control of the PT hook switch
 * for testing.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.6.10
 * @{
 */

/** Test hook control values */
enum dect_test_hook_values {
	DECT_TEST_HOOK_ON_HOOK		= 0x0, /**< On-hook */
	DECT_TEST_HOOK_OFF_HOOK		= 0x1, /**< Off-hook */
};

/** <<TEST-HOOK-CONTROL>> IE */
struct dect_ie_test_hook_control {
	struct dect_ie_common		common;
	enum dect_test_hook_values	hook;
};

/**
 * @}@}
 * @defgroup ie_auth Authentication related
 *
 * Authentication related IEs.
 *
 * @{
 */
/**
 * @defgroup ie_allocation_type Allocation type
 *
 * <<ALLOCATION-TYPE>> IE specified in ETSI EN 300 175-5 section 7.7.2.
 *
 * The <<ALLOCATION-TYPE>> IE defines the authentication parameters for the
 * key allocation procedure.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.2
 * @{
 */

/** <<ALLOCATION-TYPE>> IE */
struct dect_ie_allocation_type {
	struct dect_ie_common		common;
	uint8_t				auth_id;
	uint8_t				auth_key_num;
	uint8_t				auth_code_num;
};

/**
 * @}@}
 * @defgroup ie_alphanumeric Alphanumeric
 *
 * <<ALPHANUMERIC>> IE specified in ETSI EN 300 175-5 section 7.7.3.
 *
 * The <<ALPHANUMERIC>> IE provides a transport mechanism for alternative
 * character sets in both directions.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.3.
 * @{
 */

/** Alphanumeric character types */
enum dect_character_types {
	DECT_CHARACTER_TYPE_USER_SPECIFIC	= 0x0, /**< User specific */
	DECT_CHARACTER_TYPE_STANDARD_8BIT	= 0x1, /**< Standard 8-bit characters*/
	DECT_CHARACTER_TYPE_STANDARD_4BIT	= 0x2, /**< Standard 4-bit characters */
};

/** Alphanumeric charcter sets */
enum dect_character_sets {
	DECT_CHARSET_DECT_STANDARD		= 0x1, /**< DECT standard 8-bit characters */
	DECT_CHARSET_IA5			= 0x2, /**< IA5 characters */
	DECT_CHARSET_ERMES			= 0x4, /**< ERMES 7-bit characters */
	DECT_CHARSET_ASCII			= 0x6, /**< Standard ASCII 7-bit characters */
};

/** <<ALPHANUMERIC>> IE */
struct dect_ie_alphanumeric {
	struct dect_ie_common			common;
	enum dect_character_types		type;
	enum dect_character_sets		charset;
	uint8_t					len;
	uint8_t					data[64];
};

/**
 * @}
 * @addtogroup ie_auth
 * @{
 */

/** Authentication algorithm identifiers */
enum dect_auth_type_identifiers {
	DECT_AUTH_DSAA				= 0x1,  /**< DECT standard authentication algorithm 1 */
	DECT_AUTH_GSM				= 0x40, /**< GSM authentication algorithm */
	DECT_AUTH_UMTS				= 0x20, /**< UMTS authentication algorithm */
	DECT_AUTH_PROPRIETARY			= 0x7f, /**< Escape to proprietary algorithm identifier */
};

/** Authentication key types */
enum dect_auth_key_types {
	DECT_KEY_USER_AUTHENTICATION_KEY	= 0x1, /**< User authentication key */
	DECT_KEY_USER_PERSONAL_IDENTITY		= 0x3, /**< User personal identity */
	DECT_KEY_AUTHENTICATION_CODE		= 0x4, /**< Authentication code */
};

/** Authentication key flags - defines the relation of the key to identities. */
enum dect_auth_key_flags {
	DECT_AUTH_KEY_IPUI			= 0x0, /**< The key shall be related to the active IPUI */
	DECT_AUTH_KEY_IPUI_PARK			= 0x8, /**< The key shall be related to the active IPUI/PARK pair */
};

/**
 * @defgroup ie_auth_type Auth type
 *
 * <<AUTH-TYPE>> IE specified in ETSI EN 300 175-5 section 7.7.4.
 *
 * The <<AUTH-TYPE>> IE defines the authentication algorithm and the
 * authentication key. In addition it may be used to send a ZAP increment
 * command and/or to indicate if the cipher key shall be updated and/or
 * sent.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.4
 * @{
 */

/** Authentication flags repesenting the INC/DEF/TXC/UPC bits */
enum dect_auth_flags {
	DECT_AUTH_FLAG_INC			= 0x80, /**< INC bit */
	DECT_AUTH_FLAG_DEF			= 0x40, /**< DEF bit */
	DECT_AUTH_FLAG_TXC			= 0x20, /**< TXC bit */
	DECT_AUTH_FLAG_UPC			= 0x10, /**< UPC bit */
};

/** <<AUTH-TYPE>> IE */
struct dect_ie_auth_type {
	struct dect_ie_common		common;
	uint8_t				auth_id;
	uint8_t				proprietary_auth_id;
	uint8_t				auth_key_type;
	uint8_t				auth_key_num;
	uint8_t				flags;
	uint8_t				cipher_key_num;
	uint16_t			defck_index;
};

/**
 * @}@}
 * @addtogroup ie_cc_related
 * @{
 * @defgroup ie_call_attributes Call attributes
 * @{
 */

struct dect_ie_call_attributes {
	struct dect_ie_common		common;
};

/**
 * @}
 * @defgroup ie_call_identity Call identity
 * @{
 */

struct dect_ie_call_identity {
	struct dect_ie_common		common;
};

/**
 * @}
 * @defgroup Party Identification
 * @{
 */

/**
 * Number types specified in ETSI EN 300 175-5 section 7.7.7 and 7.7.9.
 */
enum dect_number_type {
	DECT_NUMBER_TYPE_UNKNOWN		= 0x0, /**< Unknown */
	DECT_NUMBER_TYPE_INTERNATIONAL		= 0x1, /**< International number */
	DECT_NUMBER_TYPE_NATIONAL		= 0x2, /**< National number */
	DECT_NUMBER_TYPE_NETWORK_SPECIFIC	= 0x3, /**< Network specific number */
	DECT_NUMBER_TYPE_SUBSCRIBER		= 0x4, /**< Subscriber number */
	DECT_NUMBER_TYPE_ABBREVIATED		= 0x6, /**< Abbreviated number */
	DECT_NUMBER_TYPE_RESERVED		= 0x7, /**< Reserved for extension */
};

/**
 * Numbering plan identification values specified in ETSI EN 300 175-5
 * section 7.7.7 and 7.7.9.
 */
enum dect_numbering_plan_identification {
	DECT_NPI_UNKNOWN			= 0x0, /**< Unknown */
	DECT_NPI_ISDN_E164			= 0x1, /**< ISDN/telephony plan ITU-T Recommendations E.164 */
	DECT_NPI_DATA_PLAN_X121			= 0x3, /**< Data plan ITU-T Recommendation X.121 */
	DECT_NPI_TCP_IP				= 0x7, /**< TCP/IP address */
	DECT_NPI_NATIONAL_STANDARD		= 0x8, /**< National standard plan */
	DECT_NPI_PRIVATE			= 0x9, /**< Private plan */
	DECT_NPI_SIP				= 0xa, /**< SIP addressing scheme, "To:" field */
	DECT_NPI_INTERNET_CHARACTER_FORMAT	= 0xb, /**< Internet character format address */
	DECT_NPI_LAN_MAC_ADDRESS		= 0xc, /**< LAN MAC address */
	DECT_NPI_X400				= 0xd, /**< ITU-T Recommendation X.400 address */
	DECT_NPI_PROFILE_SPECIFIC		= 0xe, /**< Profile service specific alphanumeric identifier */
	DECT_NPI_RESERVED			= 0xf, /**< Reserved for extension */
};

/**
 * Presentation Indicators specified in ETSI EN 300 175-5, section 7.7.9
 * and 7.7.53.
 */
enum dect_presentation_indicators {
	DECT_PRESENTATION_ALLOWED			= 0x0, /**< Presentation allowed */
	DECT_PRESENTATION_RESTRICTED			= 0x1, /**< Presentation restricted */
	DECT_PRESENTATION_NOT_AVAILABLE			= 0x2, /**< Name/Number not available */
	DECT_PRESENTATION_HANDSET_LOCATOR		= 0x3, /**< Handset locator (Calling Party Name only) */
};

/**
 * Alphabets specified in ETSI EN 300 175-5, section 7.7.53.
 */
enum dect_alphabets {
	DECT_ALPHABET_STANDARD				= 0x0, /**< DECT standard */
	DECT_ALPHABET_UTF8				= 0x1, /**< UTF-8 */
	DECT_ALPHABET_NETWORK_SPECIFIC			= 0x7, /**< Network specific */
};

/**
 * Screening Indicators specified in ETSI EN 300 175-5, section 7.7.9
 * and 7.7.53.
 */
enum dect_screening_indicators {
	DECT_SCREENING_USER_PROVIDED_NOT_SCREENED	= 0x0, /**< User-provided, not screened */
	DECT_SCREENING_USER_PROVIDED_VERIFIED_PASSED	= 0x1, /**< User-provided, verified and passed */
	DECT_SCREENING_USER_PROVIDED_VERIFIED_FAILED	= 0x2, /**< User-provided, verified and failed */
	DECT_SCREENING_NETWORK_PROVIDED			= 0x3, /**< Network provided */
};

/**
 * @defgroup ie_called_party_number Called party number
 *
 * <<CALLED-PARTY-NUMBER>> IE specified in ETSI EN 300 175-5 section 7.7.7.
 *
 * The <<CALLED-PARTY-NUMBER>> IE identifies the called party of a call in an
 * en-bloc format.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.7
 * @{
 */

/** <<CALLED-PARTY-NUMBER>> IE */
struct dect_ie_called_party_number {
	struct dect_ie_common			common;
	enum dect_number_type			type;
	enum dect_numbering_plan_identification	npi;
	unsigned int				len;
	uint8_t					address[64];
};

/**
 * @}
 * @defgroup ie_called_party_subaddress Called party subaddress
 *
 * <<CALLED-PARTY-SUBADDRESS> IE specified in ETSI EN 300 175-5 section 7.7.8
 *
 * The <<CALLED-PARTY-SUBADDRESS>> IE identifies the subaddress of the called party
 * of a call.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.8.
 * @{
 */

/** <<CALLED-PARTY-SUBADDRESS> IE */
struct dect_ie_called_party_subaddress {
	struct dect_ie_common		common;
};

/**
 * @}
 * @defgroup ie_calling_party_number Calling party number
 *
 * <<CALLING-PARTY-NUMBER>> IE specified in ETSI EN 300 175-5 section 7.7.9.
 *
 * The <<CALLING-PARTY-NUMBER>> IE conveys the number of the calling party of a
 * call.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.9.
 * @{
 */

/** <<CALLING-PARTY-NUMBER>> IE */
struct dect_ie_calling_party_number {
	struct dect_ie_common			common;
	enum dect_number_type			type:8;
	enum dect_numbering_plan_identification	npi:8;
	enum dect_presentation_indicators	presentation:8;
	enum dect_screening_indicators		screening:8;
	uint8_t					len;
	uint8_t					address[64];
};

/**
 * @}
 * @defgroup ie_calling_party_name Calling party Name
 *
 * <<CALLING-PARTY-NAME>> IE specified in ETSI EN 300 175-5 section 7.7.53.
 *
 * The <<CALLING-PARTY-NAME>> IE conveys the name of the calling party of a
 * call.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.53
 * @{
 */

/** <<CALLING-PARTY-NAME>> IE */
struct dect_ie_calling_party_name {
	struct dect_ie_common			common;
	enum dect_presentation_indicators	presentation:8;
	enum dect_alphabets			alphabet:8;
	enum dect_screening_indicators		screening:8;
	uint8_t					len;
	uint8_t					name[64];
};

/**
 * @}@}@}
 * @defgroup ie_ciphering Ciphering related
 *
 * Ciphering related IEs.
 *
 * @{
 * @defgroup ie_cipher_info Cipher info
 *
 * <<CIPHER-INFO>> IE specified in ETSI EN 300 175-5 section 7.7.10.
 *
 * The <<CIPHER-INFO>> IE indicates if a call shall be ciphered or not.
 * In the case of ciphering it defines the cipher algorithm and the cipher key.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.10
 * @{
 */

/** Cipher algorithm identifiers */
enum dect_cipher_algs {
	DECT_CIPHER_STANDARD_1		= 0x1,  /**< DECT standard cipher algorithm 1 */
	DECT_CIPHER_GPRS_NO_CIPHERING	= 0x28, /**< GPRS ciphering not used */
	DECT_CIPHER_GRPS_GEA_1		= 0x29, /**< GPRS encryption algorithm GEA/1 */
	DECT_CIPHER_GRPS_GEA_2		= 0x2a, /**< GPRS encryption algorithm GEA/2 */
	DECT_CIPHER_GRPS_GEA_3		= 0x2b, /**< GPRS encryption algorithm GEA/3 */
	DECT_CIPHER_GRPS_GEA_4		= 0x2c, /**< GPRS encryption algorithm GEA/4 */
	DECT_CIPHER_GRPS_GEA_5		= 0x2d, /**< GPRS encryption algorithm GEA/5 */
	DECT_CIPHER_GRPS_GEA_6		= 0x2e, /**< GPRS encryption algorithm GEA/6 */
	DECT_CIPHER_GRPS_GEA_7		= 0x2f, /**< GPRS encryption algorithm GEA/7 */
	DECT_CIPHER_ESC_TO_PROPRIETARY	= 0x7f, /**< Escape to proprietary algorithm identifier */
};

/** Cipher key types */
enum dect_cipher_key_types {
	DECT_CIPHER_DERIVED_KEY		= 0x9, /**< Derived cipher key */
	DECT_CIPHER_STATIC_KEY		= 0xa, /**< Static cipher key */
};

/** <<CIPHER-INFO>> IE */
struct dect_ie_cipher_info {
	struct dect_ie_common		common;
	bool				enable;
	enum dect_cipher_algs		cipher_alg_id;
	enum dect_cipher_key_types	cipher_key_type;
	uint8_t				cipher_key_num;
};

/**
 * @}@}
 * @addtogroup ie_cc_related
 * @{
 * @defgroup ie_connection_attributes Connection attributes
 *
 * <<CONNECTION-ATTRIBUTES>> IE specified in ETSI EN 300 175-5 section 7.7.11.
 *
 * The <<CONNECTION-ATTRIBUTES>> IE describes the connections that are required
 * for the requested service.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.11
 * @{
 */

/** <<CONNECTION-ATTRIBUTES>> IE */
struct dect_ie_connection_attributes {
	struct dect_ie_common		common;
};

/**
 * @}@}
 * @defgroup ie_connection_identity Connection identity
 *
 * <<CONNECTION-IDENTITY>> IE specified in ETSI EN 300 175-5 section 7.7.12.
 *
 * The <<CONNECTION-IDENTITY>> IE associates one or more U-plane link with an
 * advanced connection or allocates ECN for connections associated with a
 * broadband data link.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.12
 * @{
 */

/** <<CONNECTION-IDENTITY>> IE */
struct dect_ie_connection_identity {
	struct dect_ie_common		common;
};

/**
 * @}
 * @defgroup ie_duration Duration
 *
 * <<DURATION>> IE specified in ETSI EN 300 175-5 section 7.7.13.
 *
 * The <<DURATION>> IE indicates a time duration. The time is defined
 * in units of MAC layer multiframes and depends on the time limit:
 *
 * - @ref DECT_TIME_LIMIT_DEFINED_TIME_LIMIT_1 "Defined time limit 1": 1 unit =  2^8 multiframes =        40.96s
 * - @ref DECT_TIME_LIMIT_DEFINED_TIME_LIMIT_2 "Defined time limit 2": 1 unit = 2^16 multiframes = 2h 54m 45.76s
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.13
 * @{
 */

/** Lock limits */
enum dect_lock_limits {
	DECT_LOCK_TEMPORARY_USER_LIMIT_1	= 0x6, /**< Temporary user limit */
	DECT_LOCK_NO_LIMITS			= 0x7, /**< No limits */
	DECT_LOCK_TEMPORARY_USER_LIMIT_2	= 0x5, /**< Temporary user limits 2 */
};

/** Time limits */
enum dect_time_limits {
	DECT_TIME_LIMIT_ERASE			= 0x0, /**< Erase (time limit zero) */
	DECT_TIME_LIMIT_DEFINED_TIME_LIMIT_1	= 0x1, /**< Defined time limit 1: 2^8 multiframes */
	DECT_TIME_LIMIT_DEFINED_TIME_LIMIT_2	= 0x2, /**< Defined time limit 2: 2^16 multiframes */
	DECT_TIME_LIMIT_STANDARD_TIME_LIMIT	= 0x4, /**< Standard time limit */
	DECT_TIME_LIMIT_INFINITE		= 0xf, /**< Infinite */
};

#define DECT_FRAMES_PER_SECOND			100
#define DECT_FRAMES_PER_MULTIFRAME		16

#define DECT_TIME_LIMIT_UNITS_1			(256   * DECT_FRAMES_PER_MULTIFRAME)
#define DECT_TIME_LIMIT_UNITS_2			(65536 * DECT_FRAMES_PER_MULTIFRAME)

/** <<DURATION>> IE */
struct dect_ie_duration {
	struct dect_ie_common		common;
	enum dect_lock_limits		lock;
	enum dect_time_limits		time;
	uint8_t				duration;
};

/**
 * @}
 * @addtogroup ie_cc_related
 * @{
 * @defgroup ie_end_to_end_compatibility End-to-end compatibility
 *
 * <<END-TO-END-COMPATIBILITY>> IE specified in ETSI EN 300 175-5 section 7.7.13.
 *
 * The <<END-TO-END-COMPATIBILITY>> IE exchanges some aspects of the end-to-end
 * data terminal capabilities during call establishment.
 *
 * @sa ETSI EN 300 175-5 section 7.7.13
 * @{
 */

/** <<END-TO-END-COMPATIBILITY>> IE */
struct dect_ie_end_to_end_compatibility {
	struct dect_ie_common		common;
};

/**
 * @}@}
 * @addtogroup ie_generic
 * @{
 * @defgroup ie_facility Facility
 *
 * <<FACILITY>> IE specified in ETSI EN 300 175-5 section 7.7.15.
 *
 * The <<FACILITY>> IE indicates the invocation and operates supplementary
 * services.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.15
 * @{
 */

enum dect_facility_discriminators {
	DECT_FACILITY_SS			= 0x11, /**< Supplementary service application */
};

/** <<FACILITY>> IE */
struct dect_ie_facility {
	struct dect_ie_common			common;
	enum dect_facility_discriminators	service;
	uint8_t					len;
	uint8_t					components[64];
};

/**
 * @}@}
 * @defgroup ie_feature_management Feature key management related
 *
 * Feature key management protocol related IEs.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 10.3 (Feature key management protocol)
 * @{
 */

/** Features specified in ETSI EN 300 175-5 section 7.7.16 and 7.7.17. */
enum dect_feature {
	DECT_FEATURE_REGISTER_RECALL			= 0x1,  /**< Register recall */
	DECT_FEATURE_EXTERNAL_HO_SWITCH			= 0xf,  /**< External handover switch */
	DECT_FEATURE_QUEUE_ENTRY_REQUEST		= 0x20, /**< Queue entry request */
	DECT_FEATURE_INDICATION_OF_SUBSCRIBER_NUMBER	= 0x30, /**< Indication of subscriber number */
	DECT_FEATURE_FEATURE_KEY			= 0x42, /**< Feature key */
	DECT_FEATURE_SPECIFIC_LINE_SELECTION		= 0x44, /**< Specific line selection */
	DECT_FEATURE_SPECIFIC_TRUNK_SELECTION		= 0x47, /**< Specific trunk carrier selection */
	DECT_FEATURE_ECHO_CONTROL			= 0x48, /**< Control of echo control functions */
	DECT_FEATURE_COST_INFORMATION			= 0x60, /**< Cost information */
};

/**
 * @defgroup ie_feature_activate Feature activate
 *
 * <<FEATURE-ACTIVATE>> IE specified in ETSI EN 300 175-5 section 7.7.16.
 *
 * The <<FEATURE-ACTIVATE>> IE requests activation of the feature specified in
 * the feature field.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.18
 * @{
 */

/** <<FEATURE-ACTIVATE>> IE */
struct dect_ie_feature_activate {
	struct dect_ie_common		common;
	enum dect_feature		feature;
};

/**
 * @}
 * @defgroup ie_feature_indicate Feature indicate
 *
 * <<FEATURE-INDICATE>> IE specified in ETSI EN 300 175-5 section 7.7.17.
 *
 * The <<FEATURE-INDICATE>> IE conveys FT feature indications to the user
 * regarding the status of an activated feature.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.17
 * @{
 */

/** Feature status indicators */
enum dect_feature_status {
	DECT_FEATURE_SERVICE_REQUEST_REJECTED		= 0x80, /**< Service request rejected */
	DECT_FEATURE_SERVICE_REQUEST_ACCEPTED		= 0x81, /**< Service request accepted, feature is activated */
	DECT_FEATURE_SERVICE_REQUEST_PENDING		= 0x83, /**< Service request accepted, feature is pending */
	DECT_FEATURE_SERVICE_BUSY			= 0x84, /**< Service busy */
	DECT_FEATURE_SERVICE_UNOBTAINABLE		= 0x86, /**< Service unobtainable */
};

/** <<FEATURE-INDICATE>> IE */
struct dect_ie_feature_indicate {
	struct dect_ie_common		common;
	enum dect_feature		feature;
	enum dect_feature_status	status;
};

/**
 * @}@}
 * @defgroup ie_identity_related Identity related
 *
 * Equipment Identity related IEs.
 *
 * @{
 * @defgroup ie_fixed_identity Fixed identity
 *
 * <<FIXED-IDENTITY>> IE specified in ETSI EN 300 175-5 section 7.7.18.
 *
 * The <<FIXED-IDENTITY>> IE transports a DECT fixed identity or a
 * Portable Access Rights Key (PARK).
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.18
 * @sa ETSI EN 300 175-6 (Identities and addressing)
 * @{
 */

/** Fixed identity types */
enum dect_fixed_identity_types {
	DECT_FIXED_ID_TYPE_ARI		= 0x00, /**< Access rights identity */
	DECT_FIXED_ID_TYPE_ARI_RPN	= 0x01, /**< Access rights identity plus radio fixed part number */
	DECT_FIXED_ID_TYPE_ARI_WRS	= 0x02, /**< Access rights identity plus radio fixed part number for WRS */
	DECT_FIXED_ID_TYPE_PARK		= 0x20, /**< Portable access rights key */
};

/** <<FIXED-IDENTITY>> IE */
struct dect_ie_fixed_identity {
	struct dect_ie_common		common;
	enum dect_fixed_identity_types	type;
	struct dect_ari			ari;
	uint8_t				rpn;
};

/**
 * @}
 * @defgroup ie_portable_identity Portable identity
 *
 * <<PORTABLE-IDENTITY>> IE specified in ETSI EN 300 175-5 section 7.7.30.
 *
 * The <<PORTABLE-IDENTITY>> IE transports a DECT portable identity.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.30
 * @sa ETSI EN 300 175-6 (Identities and addressing)
 * @{
 */

/** Portable identity types */
enum dect_portable_identity_types {
	DECT_PORTABLE_ID_TYPE_IPUI	= 0x0,  /**< International Portable User Identity (IPUI) */
	DECT_PORTABLE_ID_TYPE_IPEI	= 0x10, /**< International Portable Equipment Identity (IPEI) */
	DECT_PORTABLE_ID_TYPE_TPUI	= 0x20, /**< Temporary Portable User Identity (TPUI) */
};

/** <<PORTABLE-IDENTITY>> IE */
struct dect_ie_portable_identity {
	struct dect_ie_common			common;
	enum dect_portable_identity_types	type;
	union {
		struct dect_ipui		ipui;
		struct dect_tpui		tpui;
	};
};

/**
 * @}
 * @defgroup ie_nwk_assigned_identity NetWorK (NWK) assigned identity
 *
 * <<NWK-ASSIGNED-IDENTITY>> IE specified in ETSI EN 300 175-5 section 7.7.28.
 *
 * The <<NWK-ASSIGNED-IDENTITY>> IE transports a network assigned identity.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.28
 * @{
 */

/** Network assigned identity types */
enum dect_nwk_identity_types {
	DECT_NWK_ID_TYPE_TMSI		= 0x74, /**< Temporary mobile subscriber identity (TMSI) */
	DECT_NWK_ID_TYPE_PROPRIETARY	= 0x7f, /**< Proprietary (application specific) */
};

/** <<NWK-ASSIGNED-IDENTITY>> IE */
struct dect_ie_nwk_assigned_identity {
	struct dect_ie_common		common;
	enum dect_nwk_identity_types	type;
};

/**
 * @}
 * @defgroup ie_identity_type Identity type
 *
 * <<IDENTITY-TYPE>> IE specified in ETSI EN 300 175-5 section 7.7.19.
 *
 * The <<IDENTITY-TYPE>> IE indicates a specific identity type, e.g. used by
 * the FT when requesting for a specific PT identity.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.19
 * @sa ETSI EN 300 175-6 (Identities and addressing)
 * @{
 */

/** Identity groups specified in ETSI EN 300 175-5 section 7.7.19. */
enum dect_identity_groups {
	DECT_IDENTITY_PORTABLE_IDENTITY		= 0x0, /**< Portable identity */
	DECT_IDENTITY_NETWORK_ASSIGNED_IDENTITY	= 0x1, /**< Network assigned identity */
	DECT_IDENTITY_FIXED_IDENTITY		= 0x4, /**< Fixed identity */
	DECT_IDENTITY_APPLICATION_ASSIGNED	= 0x8, /**< Application assigned identity */
	DECT_IDENTITY_PROPRIETARY		= 0xf, /**< Proprietary identity (application specific) */
};

/** <<IDENTITY-TYPE>> IE */
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

/**
 * @}@}
 * @defgroup ie_info_type Info type
 *
 * <<INFO-TYPE>> IEs specified in ETSI EN 300 175-5 section 7.7.20.
 *
 * The <<INFO-TYPE>> IE indicates the type of requested or transmitted
 * information.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.20
 * @{
 */

/** Information parameter types */
enum dect_info_parameter_type {
	DECT_INFO_LOCATE_SUGGEST				= 0x00, /**< Locate suggest */
	DECT_INFO_ACCESS_RIGHTS_MODIFY_SUGGEST			= 0x01, /**< Access rights modify suggest */
	DECT_INFO_PP_AUTHENTICATION_FAILURE			= 0x04, /**< Authentication of PP failure */
	DECT_INFO_DYNAMIC_PARAMETERS_ALLOCATION			= 0x06, /**< Dynamic parameters allocation */
	DECT_INFO_EXTERNAL_HO_PARAMETERS			= 0x08, /**< External handover parameters */
	DECT_INFO_LOCATION_AREA					= 0x09, /**< Location area */
	DECT_INFO_HANDOVER_REFERENCE				= 0x0a, /**< Hand over reference */
	DECT_INFO_MF_PSCN_SYNCHRONIZED_HANDOVER_CANDIATE	= 0x0b, /**< Multiframe and PSCN synchronized external handover candidate */
	DECT_INFO_EXT_HANDOVER_CANDIDATE			= 0x0c, /**< External handover candidate */
	DECT_INFO_MF_SYNCHRONIZED_HANDOVER_CANDIATE		= 0x0d, /**< Multiframe synchronized external handover candidate */
	DECT_INFO_NON_SYNCHRONIZED_HANDOVER_CANDIDATE		= 0x0e, /**< Non synchronized external handover candidate */
	DECT_INFO_MF_PSCN_MFN_SYNCHRONIZED_HANDOVER_CANDIATE	= 0x0f, /**< Multiframe, PSCN and multiframe number synchronized external handover candidate */
	DECT_INFO_OLD_FIXED_PART_IDENTITY			= 0x10, /**< Old fixed part identity */
	DECT_INFO_OLD_NETWORK_ASSIGNED_IDENTITY			= 0x11, /**< Old network assigned identity */
	DECT_INFO_OLD_NETWORK_ASSIGNED_LOCATION_AREA		= 0x12, /**< Old network assigned location area */
	DECT_INFO_OLD_NETWORK_ASSIGNED_HANDOVER_REFERENCE	= 0x13, /**< Old network assigned handover reference */
	DECT_INFO_BILLING					= 0x20, /**< Billing */
	DECT_INFO_DEBITING					= 0x21, /**< Debiting */
	DECT_INFO_CK_TRANSFER					= 0x22, /**< CK transfer */
	DECT_INFO_HANDOVER_FAILED_REVERSION			= 0x23, /**< Handover failed, reversion to old channel */
	DECT_INFO_QA_M_CALL					= 0x24, /**< OA&M call */
	DECT_INFO_DISTRIBUTED_COMMUNICATION_DOWNLOAD		= 0x25, /**< Distributed Communication Download */
	DECT_INFO_ETHERNET_ADDRESS				= 0x30, /**< Ethernet (IEEE 802.3) Address */
	DECT_INFO_TOKEN_RING_ADDRESS				= 0x31, /**< Tocken Ring Address */
	DECT_INFO_IPV4_ADDRESS					= 0x32, /**< Internet Protocol Version 4 Address (IPv4) */
	DECT_INFO_IPV6_ADDRESS					= 0x33, /**< Internet Protocol Version 6 Address (IPv6) */
	DECT_INFO_IDENTITY_ALLOCATION				= 0x34, /**< Identity allocation */
};

/** <<INFO-TYPE>> IE */
struct dect_ie_info_type {
	struct dect_ie_common		common;
	unsigned int			num;
	enum dect_info_parameter_type	type[8];
};

/**
 * @}
 * @addtogroup ie_cc_related
 * @{
 * @defgroup ie_iwu_attributes InterWorking Unit (IWU) attributes
 *
 * <<IWU-ATTRIBUTES>> IEs specified in ETSI EN 300 175-5 section 7.7.21.
 *
 * The <<IWU-ATTRIBUTES>> IE provides a means to transparently exchange service
 * compatibility information.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.21
 * @{
 */

/** <<IWU-ATTRIBUTES>> IE */
struct dect_ie_iwu_attributes {
	struct dect_ie_common		common;
};

/**
 * @}@}
 * @addtogroup ie_generic
 * @{
 * @defgroup ie_iwu_packet IWU packet
 *
 * <<IWU-PACKET>> IEs specified in ETSI EN 300 175-5 section 7.7.22.
 *
 * The <<IWU-PACKET>> IE encapsulates external frame or unstructured data such
 * that is can be transported inside one or more CC, MM, COMS or CLMS messages.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.22
 * @{
 */

/** <<IWU-PACKET>> IE */
struct dect_ie_iwu_packet {
	struct dect_ie_common		common;
};

/**
 * @}
 * @defgroup ie_iwu_to_iwu IWU to IWU
 *
 * <<IWU-TO-IWU>> IEs specified in ETSI EN 300 175-5 section 7.7.23.
 *
 * The <<IWU-TO-IWU>> IE is used to exchange information between the
 * interworking units.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.23
 * @{
 */

/** IWU-TO-IWU protocol discriminators */
enum dect_iwu_to_iwu_discriminators {
	DECT_IWU_TO_IWU_PD_USER_SPECIFIC			= 0x00, /**< User Specific */
	DECT_IWU_TO_IWU_PD_OSI_HIGHER_LAYER			= 0x01, /**< OSI high layer protocols */
	DECT_IWU_TO_IWU_PD_ITU_T_X263				= 0x02, /**< ITU-T Recommendation X.263 */
	DECT_IWU_TO_IWU_PD_LIST_ACCESS				= 0x03, /**< List Access */
	DECT_IWU_TO_IWU_PD_IA5_CHARACTERS			= 0x04, /**< IA5 characters */
	DECt_IWU_TO_IWU_PD_LDS_SUOTA				= 0x06, /**< Light data service, Software Upgrade Over The Air (SUOTA) */
	DECT_IWU_TO_IWU_PD_ITU_T_V120				= 0x07, /**< ITU-T Recommendation V.120 Rate adaption */
	DECT_IWU_TO_IWU_PD_ITU_T_Q931_MESSAGE			= 0x08, /**< ITU-T Recommendation Q.931 (I.451), message */
	DECT_IWU_TO_IWU_PD_ITU_T_Q931_IE			= 0x09, /**< ITU-T Recommendation Q.931 (I.451), information element(s) */
	DECT_IWU_TO_IWU_PD_ITU_T_Q931_PARTIAL_MESSAGE		= 0x0a, /**< ITU-T Recommendation Q.931 (I.451), partial message. */
	DECT_IWU_TO_IWU_PD_GSM_MESSAGE				= 0x10, /**< GSM, message */
	DECT_IWU_TO_IWU_PD_GSM_IE				= 0x11, /**< GSM, information element(s) */
	DECT_IWU_TO_IWU_PD_UMTS_GPRS_IE				= 0x12, /**< UMTS/GPRS information element(s) */
	DECT_IWU_TO_IWU_PD_UMTS_GPRS_MESSAGE			= 0x13, /**< UMTS/GPRS, messages */
	DECT_IWU_TO_IWU_PD_LRMS					= 0x14, /**< LRMS */
	DECT_IWU_TO_IWU_PD_RLL_ACCESS_PROFILE			= 0x15, /**< RLL Access Profile */
	DECT_IWU_TO_IWU_PD_WRS					= 0x16, /**< WRS */
	DECT_IWU_TO_IWU_PD_DECT_ISDN_C_PLANE_SPECIFIC		= 0x20, /**< DECT/ISDN Intermediate System C-plane specific */
	DECT_IWU_TO_IWU_PD_DECT_ISDN_U_PLANE_SPECIFIC		= 0x21, /**< DECT/ISDN Intermediate System U-plane specific */
	DECT_IWU_TO_IWU_PD_DECT_ISDN_OPERATION_AND_MAINTENANCE	= 0x22, /**< DECT/ISDN Intermediate System Operation and Maintenance */
	DECT_IWU_TO_IWU_PD_TERMINAL_DATA			= 0x23, /**< Terminal Data */
	DECT_IWU_TO_IWU_PD_DECT_IP_NETWORK_ACCESS_SPECIFIC	= 0x24, /**< DECT access to IP Networks specific */
	DECT_IWU_TO_IWU_PD_MPEG4_ER_AAL_LD_CONFIGURATION	= 0x25, /**< MPEG-4 ER AAC-LD Configuration Description */
	DECT_IWU_TO_IWU_PD_UNKNOWN				= 0x3f, /**< Unknown */
};

/** <<IWU-TO-IWU>> IE */
struct dect_ie_iwu_to_iwu {
	struct dect_ie_common		common;
	bool				sr;
	uint8_t				pd;
	uint8_t				len;
	uint8_t				data[256];
};

/**
 * @}@}
 * @addtogroup ie_auth
 * @{
 * @defgroup ie_key Key
 *
 * <<KEY>> IEs specified in ETSI EN 300 175-5 section 7.7.24.
 *
 * The <<KEY>> IE transfers a key.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.24
 * @{
 */

/** <<KEY>> IE */
struct dect_ie_key {
	struct dect_ie_common		common;
};

/**
 * @}@}
 * @defgroup ie_location_area Location area
 *
 * <<LOCATION_AREA>> IE specified in ETSI EN 300 175-5 section 7.7.25.
 *
 * The <<LOCATION-AREA>> IE provides an identification of the location area.
 *
 * @sa ETSI EN 300 175-5 section 7.7.25.
 * @{
 */

enum dect_location_area_types {
	DECT_LOCATION_AREA_LEVEL		= 0x1 << 0, /**<  */
	DECT_LOCATION_AREA_EXTENDED		= 0x1 << 1, /**<  */
};

/** <<LOCATION-AREA>> IE */
struct dect_ie_location_area {
	struct dect_ie_common		common;
	uint8_t				type;
	uint8_t				level;
};

/**
 * @}
 * @defgroup ie_network_parameter Network parameter
 *
 * <<NETWORK-PARAMETER>> IE specified in ETSI EN 300 175-5 section 7.7.29.
 *
 * The <<NETWORK-PARAMETER>> IE conveys a network parameter.
 *
 * @sa ETSI EN 300 175-5 section 7.7.29.
 * @{
 */

/** Network parameter discriminators */
enum dect_network_parameter_discriminators {
	DECT_NETWORK_PARAMETER_APPLICATION_ASSIGNED		= 0x08,	/**< Application assigned */
	DECT_NETWORK_PARAMETER_DEVICE_NAME			= 0x10,	/**< Device name */
	DECT_NETWORK_PARAMETER_HO_REFERENCE_NOT_ACQUIRED	= 0x68, /**< Handover reference not required */
	DECT_NETWORK_PARAMETER_HO_REFERENCE_PRIVATE_NETWORK	= 0x69, /**< Handover reference, private network */
	DECT_NETWORK_PARAMETER_HO_REFERENCE_GSM_NETWORK		= 0x6a, /**< Handover reference, GSM network */
	DECT_NETWORK_PARAMETER_HO_REFERENCE_PUBLIC_NETWORK	= 0x6b, /**< Handover reference, public network */
	DECT_NETWORK_PARAMETER_PROPRIETARY			= 0x7f, /**< Proprietary */
	DECT_NETWORK_PARAMETER_HO_REFERENCE_REQUEST_GSM_NETWORK	= 0xea, /**< Handover reference request, GSM network */
	DECT_NETWORK_PARAMETER_HO_REFERENCE_UMTS_NETWORK	= 0xeb, /**< Handover reference, UMTS network */
	DECT_NETWORK_PARAMETER_HO_REFERENCE_REQUEST_UMTS_NETWORK= 0xec, /**< Handover reference request, UMTS network */
};

/** <<NETWORK-PARAMETER>> IE */
struct dect_ie_network_parameter {
	struct dect_ie_common				common;
	enum dect_network_parameter_discriminators	discriminator;
	uint8_t						len;
	uint8_t						data[64];
};

/**
 * @}
 * @addtogroup ie_cc_related
 * @{
 * @defgroup ie_progress_indicator Progress indicator
 *
 * <<PROGRESS-INDICATOR>> IEs specified in ETSI EN 300 175-5 section 7.7.31.
 *
 * The <<PROGRESS-INDICATOR>> IE describes an event which has occured during the
 * life of a call.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.31
 * @{
 */

/** Event location */
enum dect_location {
	DECT_LOCATION_USER					= 0x0, /**< user */
	DECT_LOCATION_PRIVATE_NETWORK_SERVING_LOCAL_USER	= 0x1, /**< private network serving the local user */
	DECT_LOCATION_PUBLIC_NETWORK_SERVING_LOCAL_USER		= 0x2, /**< public network serving the local user */
	DECT_LOCATION_PUBLIC_NETWORK_SERVING_REMOTE_USER	= 0x4, /**< public network serving the remote user */
	DECT_LOCATION_PRIVATE_NETWORK_SERVING_REMOTE_USER	= 0x5, /**< private network serving the remote user */
	DECT_LOCATION_INTERNATIONAL_NETWORK			= 0x7, /**< international network */
	DECT_LOCATION_NETWORK_BEYONG_INTERWORKING_POINT		= 0xa, /**< network beyond interworking point */
	DECT_LOCATION_NOT_APPLICABLE				= 0xf, /**< not applicable */
};

/** Progress description */
enum dect_progress_description {
	DECT_PROGRESS_NOT_END_TO_END_ISDN			= 0x00, /**< Call is not end-to-end ISDN, further call progress info may be available in-band */
	DECT_PROGRESS_DESTINATION_ADDRESS_NON_ISDN		= 0x02, /**< Destination address is non-ISDN */
	DECT_PROGRESS_ORIGINATION_ADDRESS_NON_ISDN		= 0x03, /**< Origination address is non-ISDN */
	DECT_PROGRESS_CALL_RETURNED_TO_ISDN			= 0x04, /**< Call has returned to the ISDN */
	DECT_PROGRESS_SERVICE_CHANGE				= 0x05, /**< Service change has occurred */
	DECT_PROGRESS_INBAND_INFORMATION_NOW_AVAILABLE		= 0x08, /**< In-band information or appropriate pattern now available */
	DECT_PROGRESS_INBAND_INFORMATION_NOT_AVAILABLE		= 0x09, /**< In-band information not available */
	DECT_PROGRESS_END_TO_END_ISDN				= 0x40, /**< Call is end-to-end PLMN/ISDN */
};

/** <<PROGRESS-INDICATOR>> IE */
struct dect_ie_progress_indicator {
	struct dect_ie_common		common;
	enum dect_location		location;
	enum dect_progress_description	progress;
};

/**
 * @}@}
 * @addtogroup ie_auth
 * @{
 * @defgroup ie_auth_value RAND/RS
 *
 * <<RAND>> and <<RS>> IEs specified in ETSI EN 300 175-5 section 7.7.32 and 7.7.36.
 *
 * The <<RAND>> IE provides a non-predictable number to by used for to calculate
 * the authentication response.
 *
 * The <RS> IE provides a number to be used together with <<RAND>> and the
 * authentication key to calculate the authentication response.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.32
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.36
 * @{
 */

/** <<RAND>>/<<RS>> IE */
struct dect_ie_auth_value {
	struct dect_ie_common		common;
	uint64_t			value;
};

/**
 * @}
 * @defgroup ie_res RES
 *
 * <<RES>> IE specified in ETSI EN 300 175-5 section 7.7.35.
 *
 * The <<RES>> IE provides the calculated authentication responde.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.35
 * @{
 **/

/** <<RES>> IE */
struct dect_ie_auth_res {
	struct dect_ie_common		common;
	uint32_t			value;
};

/**
 * @}@}
 * @defgroup ie_rate_parameter Rate parameters
 *
 * <<RATE-PARAMETERS>> IE specified in ETSI EN 300 175-5 section 7.7.33.
 *
 * The <<RATE-PARAMETERS>> IE indicates the attributes for the Basic Rate
 * Adaption Service (BRAT).
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.33
 * @{
 */

/** <<RATE-PARAMETERS>> IE */
struct dect_ie_rate_parameters {
	struct dect_ie_common		common;
};

/**
 * @}
 * @addtogroup ie_generic
 * @{
 * @defgroup ie_reject_reason Reject reason
 *
 * <<REJECT-REASON>> IE specified in ETSI EN 300 175-5 section 7.6.34.
 *
 * The <<REJECT-REASON>> IE conveys the reason why a request is rejected by
 * the FT or PT.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.6.34
 * @{
 */

/** Reject reasons */
enum dect_reject_reasons {
	DECT_REJECT_TPUI_UNKNOWN				= 0x01, /**< TPUI unknown */
	DECT_REJECT_IPUI_UNKNOWN				= 0x02, /**< IPUI unknown */
	DECT_REJECT_NETWORK_ASSIGNED_IDENTITY_UNKNOWN		= 0x03, /**< network assigned identity unknown */
	DECT_REJECT_IPEI_NOT_ACCEPTED				= 0x05, /**< IPEI not accepted */
	DECT_REJECT_IPUI_NOT_ACCEPTED				= 0x06, /**< IPUI not accepted */
	DECT_REJECT_AUTHENTICATION_FAILED			= 0x10, /**< authentication failed */
	DECT_REJECT_NO_AUTHENTICATION_ALGORITHM			= 0x11, /**< no authentication algorithm */
	DECT_REJECT_AUTHENTICATION_ALGORITHM_NOT_SUPPORTED	= 0x12, /**< authentication algorithm not supported */
	DECT_REJECT_AUTHENTICATION_KEY_NOT_SUPPORTED		= 0x13, /**< authentication key not supported */
	DECT_REJECT_UPI_NOT_ENTERED			 	= 0x14, /**< UPI not entered */
	DECT_REJECT_NO_CIPHER_ALGORITHM				= 0x17, /**< no cipher algorithm */
	DECT_REJECT_CIPHER_ALGORITHM_NOT_SUPPORTED		= 0x18, /**< cipher algorithm not supported */
	DECT_REJECT_CIPHER_KEY_NOT_SUPPORTED			= 0x19, /**< cipher key not supported */
	DECT_REJECT_INCOMPATIBLE_SERVICE			= 0x20, /**< incompatible service */
	DECT_REJECT_FALSE_LCE_REPLY				= 0x21, /**< false LCE reply (no corresponding service) */
	DECT_REJECT_LATE_LCE_REPLY				= 0x22, /**< late LCE reply (service already taken) */
	DECT_REJECT_INVALID_TPUI				= 0x23, /**< invalid TPUI */
	DECT_REJECT_TPUI_ASSIGNMENT_LIMITS_UNACCEPTABLE		= 0x24, /**< TPUI assignment limits unacceptable */
	DECT_REJECT_INSUFFICIENT_MEMORY			 	= 0x2f, /**< insufficient memory */
	DECT_REJECT_OVERLOAD					= 0x30, /**< overload */
	DECT_REJECT_TEST_CALL_BACK_NORMAL_EN_BLOC		= 0x40, /**< test call back: normal, en-bloc */
	DECT_REJECT_TEST_CALL_BACK_NORMAL_PIECEWISE		= 0x41, /**< test call back: normal, piecewise */
	DECT_REJECT_TEST_CALL_BACK_EMERGENCY_EN_BLOC		= 0x42, /**< test call back: emergency, en-bloc */
	DECT_REJECT_TEST_CALL_BACK_EMERGENCY_PIECEWISE		= 0x43, /**< test call back: emergency, piecewise */
	DECT_REJECT_INVALID_MESSAGE				= 0x5f, /**< invalid message */
	DECT_REJECT_INFORMATION_ELEMENT_ERROR			= 0x60, /**< information element error */
	DECT_REJECT_INVALID_INFORMATION_ELEMENT_CONTENTS	= 0x64, /**< invalid information element contents */
	DECT_REJECT_TIMER_EXPIRY				= 0x70, /**< timer expiry */
	DECT_REJECT_PLMN_NOT_ALLOWED				= 0x76, /**< PLMN not allowed */
	DECT_REJECT_LOCATION_AREA_NOT_ALLOWED			= 0x80, /**< Location area not allowed */
	DECT_REJECT_LOCATION_NATIONAL_ROAMING_NOT_ALLOWED	= 0x81, /**< National roaming not allowed in this location area */
};

/** <<REJECT-REASON>> IE */
struct dect_ie_reject_reason {
	struct dect_ie_common		common;
	enum dect_reject_reasons	reason;
};

/**
 * @}@}
 * @defgroup ie_segmented_info Segmented info
 *
 * <<SEGMENTED-INFO>> IE specified in ETSI EN 300 175-5 section 7.6.37.
 *
 * The <<SEGEMENTED-INFO>> IE indicates that a message contains only part of
 * segmented information element. It is only used internally by libdect, the
 * upper layers always receive reassembled and parsed information elements.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.6.37
 * @{
 */

/** <<SEGMENTED-INFO>> IE */
struct dect_ie_segmented_info {
	struct dect_ie_common		common;
};

/**
 * @}
 * @defgroup ie_service_change_info Service change info
 *
 * <<SERVICE-CHANGE-INFO>> IE specified in ETSI EN 300 175-5 section 7.6.38.
 *
 * The <<SERVICE-CHANGE-INFO>> IE indicates the attributes of the proposed
 * service change.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.6.38
 * @{
 */

enum dect_service_change_modes {
	DECT_SERVICE_CHANGE_NONE				= 0x0,
	DECT_SERVICE_CHANGE_CONNECTION_REVERSAL			= 0x1,
	DECT_SERVICE_CHANGE_BANDWIDTH				= 0x2,
	DECT_SERVICE_CHANGE_MODULATION				= 0x3,
	DECT_SERVICE_CHANGE_REROUTING				= 0x4,
	DECT_SERVICE_CHANGE_BANDWIDTH_PLUS_MODULATION		= 0x5,
	DECT_SERVICE_CHANGE_REROUTING_PLUS_BANDWIDTH		= 0x6,
	DECT_SERVICE_CHANGE_BANDWIDTH_OR_MODULATION		= 0x7,
	DECT_SERVICE_CHANGE_SUSPEND				= 0x8,
	DECT_SERVICE_CHANGE_RESUME				= 0x9,
	DECT_SERVICE_CHANGE_VOICE_DATA_TO_DATA			= 0xa,
	DECT_SERVICE_CHANGE_VOICE_DATA_TO_VOICE			= 0xb,
	DECT_SERVICE_CHANGE_IWU_ATTRIBUTES			= 0xc,
	DECT_SERVICE_CHANGE_AUDIO_CODEC				= 0xd,
	DECT_SERVICE_CHANGE_BASIC_SERVICE_AND_IWU_ATTRIBUTES	= 0xe,
};

/** <<SERVICE-CHANGE-INFO>> IE */
struct dect_ie_service_change_info {
	struct dect_ie_common		common;
	bool				master;
	enum dect_service_change_modes	mode;
};

/**
 * @}
 * @defgroup ie_service_class Service class
 *
 * <<SERVICE-CLASS>> IE specified in ETSI EN 300 175-5 section 7.6.39.
 *
 * The <<SERVICE-CLASS>> IE identifies the services which the PT is allowed
 * to use.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.6.39
 * @{
 */

/** <<SERVICE-CLASS>> IE */
struct dect_ie_service_class {
	struct dect_ie_common		common;
};

/**
 * @}
 * @defgroup ie_pt_capabilities PT capability related
 * @{
 * @defgroup ie_setup_capability Setup capability
 *
 * <<SETUP-CAPABILITY>> IE specified in ETSI EN 300 175-5 section 7.7.40.
 *
 * The <<SETUP-CAPABILITY>> IE conveys some aspects of the PP call setup
 * capabilities.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.40
 * @{
 */

/**
 * Paging capabilities
 *
 * @sa ETSI EN 300 175-3 (Medium Access Control (MAC) layer), section 11.3.3.1,
 *     Page detection in Idle_Locked state
 */
enum dect_page_capabilities {
	DECT_PAGE_CAPABILITY_NORMAL_PAGING			= 0x1, /**< Normal paging only (normal duty cycle paging detection) */
	DECT_PAGE_CAPABILITY_FAST_AND_NORMAL_PAGING		= 0x2, /**< paging and normal paging (high duty cycle paging detection) */
};

/**
 * Setup capabilities
 *
 * @sa ETSI EN 300 175-3 (Medium Access Control (MAC) layer), section 11.3.3.2,
 *     Setup detection in Idle_Locked state
 */
enum dect_setup_capabilities {
	DECT_SETUP_SELECTIVE_FAST_SETUP				= 0x0, /**< Selective fast setup (SEL1, SEL2 or SEL2b) supported */
	DECT_SETUP_NO_FAST_SETUP				= 0x1, /**< No fast setup supported (only indirect setup is supported) */
	DECT_SETUP_COMPLETE_FAST_SETUP				= 0x2, /**< Complete fast setup mode supported */
	DECT_SETUP_COMPLETE_AND_SELECTIVE_FAST_SETUP		= 0x3, /**< Complete and selective (SEL1, SEL2 or SEL2b) fast setup modes supported */
};

/** <<SETUP-CAPABILITY>> IE */
struct dect_ie_setup_capability {
	struct dect_ie_common		common;
	enum dect_page_capabilities	page_capability;
	enum dect_setup_capabilities	setup_capability;
};

/**
 * @}
 * @defgroup ie_terminal_capability Terminal capability
 *
 * <<TERMINAL-CAPABILITY>> IE specified in ETSI EN 300 175-5 section 7.7.41.
 *
 * The <<TERMINAL-CAPABILITY>> IE conveys some aspects of the PP capabilities.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.41
 * @{
 */

/** Display capabilities */
enum dect_display_capabilities {
	DECT_DISPLAY_CAPABILITY_NOT_APPLICABLE			= 0x0, /**< Not applicable */
	DECT_DISPLAY_CAPABILITY_NO_DISPLAY			= 0x1, /**< No display */
	DECT_DISPLAY_CAPABILITY_NUMERIC				= 0x2, /**< Numeric */
	DECT_DISPLAY_CAPABILITY_NUMERIC_PLUS			= 0x3, /**< Numeric-plus */
	DECT_DISPLAY_CAPABILITY_ALPHANUMERIC			= 0x4, /**< Alphanumeric */
	DECT_DISPLAY_CAPABILITY_FULL_DISPLAY			= 0x5, /**< Full display */
};

/** Tone capabilities */
enum dect_tone_capabilities {
	DECT_TONE_CAPABILITY_NOT_APPLICABLE			= 0x0, /**< Not applicable */
	DECT_TONE_CAPABILITY_NO_TONE				= 0x1, /**< No tone capability */
	DECT_TONE_CAPABILITY_DIAL_TONE_ONLY			= 0x2, /**< dial tone only */
	DECT_TONE_CAPABILITY_ITU_T_E182_TONES			= 0x3, /**< ITU-T Recommendation E.182 tones supported */
	DECT_TONE_CAPABILITY_COMPLETE_DECT_TONES		= 0x4, /**< Complete DECT tones supported */
};

/** Echo capabilities */
enum dect_echo_parameters {
	DECT_ECHO_PARAMETER_NOT_APPLICABLE			= 0x0, /**< Not applicable */
	DECT_ECHO_PARAMETER_MINIMUM_TCLW			= 0x1, /**< Minimum TCLw */
	DECT_ECHO_PARAMETER_FULL_TCLW				= 0x2, /**< TCLw > 46 dB (Full TCLw) */
	DECT_ECHO_PARAMETER_VOIP_COMPATIBLE_TLCW		= 0x3, /**< TCLw > 55 dB (VoIP compatible TCLw) */
};

/** Noise rejection capabilities */
enum dect_noise_rejection_capabilities {
	DECT_NOISE_REJECTION_NOT_APPLICABLE			= 0x0, /**< Not applicable */
	DECT_NOISE_REJECTION_NONE				= 0x1, /**< No noise rejection */
	DECT_NOISE_REJECTION_PROVIDED				= 0x2, /**< Noise rejection provided */
};

/** Volume control provision */
enum dect_adaptive_volume_control_provision {
	DECT_ADAPTIVE_VOLUME_NOT_APPLICABLE			= 0x0, /**< Not applicable */
	DECT_ADAPTIVE_VOLUME_PP_CONTROL_NONE			= 0x1, /**< No PP adaptive volume control */
	DECT_ADAPTIVE_VOLUME_PP_CONTROL_USED			= 0x2, /**< PP adaptive volume control used */
	DECT_ADAPTIVE_VOLUME_FP_CONTROL_DISABLE			= 0x3, /**< Disable FP adaptive volume control */
};

/* Slot capabilities */
enum dect_slot_capabilities {
	DECT_SLOT_CAPABILITY_HALF_SLOT				= 1 << 0, /**< Half slot; j = 80 */
	DECT_SLOT_CAPABILITY_LONG_SLOT_640			= 1 << 1, /**< Long slot; j = 640 */
	DECT_SLOT_CAPABILITY_LONG_SLOT_672			= 1 << 2, /**< Long slot; j = 672 */
	DECT_SLOT_CAPABILITY_FULL_SLOT				= 1 << 3, /**< Full slot */
	DECT_SLOT_CAPABILITY_DOUBLE_SLOT			= 1 << 4, /**< Double slot */
};

/** Scrolling behaviours */
enum dect_scrolling_behaviours {
	DECT_SCROLLING_NOT_SPECIFIED				= 0x0, /**< Not specified */
	DECT_SCROLLING_TYPE_1					= 0x1, /**< Scrolling behaviour type 1 */
	DECT_SCROLLING_TYPE_2					= 0x2, /**< Scrolling behaviour type 2 */
};

/** Profile indicators */
enum dect_profile_indicators {
	DECT_PROFILE_DPRS_ASYMETRIC_BEARERS_SUPPORTED		= 0x4000000000000000ULL, /**< DPRS asymmetric bearers supported (EN 301 649) */
	DECT_PROFILE_DPRS_STREAM_SUPPORTED			= 0x2000000000000000ULL, /**< DPRS Stream supported (EN 301 649) */
	DECT_PROFILE_LRMS_SUPPORTED				= 0x1000000000000000ULL, /**< LRMS (EN 300 757) supported */
	DECT_PROFILE_ISDN_END_SYSTEM_SUPPORTED			= 0x0800000000000000ULL, /**< ISDN End-system supported */
	DECT_PROFILE_DECT_GSM_INTERWORKING_PROFILE_SUPPORTED	= 0x0400000000000000ULL, /**< DECT/GSM interworking profile supported */
	DECT_PROFILE_GAP_SUPPORTED				= 0x0200000000000000ULL, /**< GAP supported (EN 300 444) */
	DECT_PROFILE_CAP_SUPPORTED				= 0x0100000000000000ULL, /**< CAP supported */
	DECT_PROFILE_RAP_1_PROFILE_SUPPORTED			= 0x0040000000000000ULL, /**< RAP 1 Profile (EN 300 765-1 [50]) supported */
	DECT_PROFILE_UMTS_GSM_FACSIMILE_SUPPORTED		= 0x0020000000000000ULL, /**< DECT/UMTS-GSM interworking - UMTS-GSM Facsimile service supported */
	DECT_PROFILE_UMTS_GSM_SMS_SERVICE_SUPPORTED		= 0x0010000000000000ULL, /**< DECT/UMTS-GSM interworking - UMTS-GSM SMS service supported */
	DECT_PROFILE_UMTS_GSM_BEARER_SERVICE			= 0x0008000000000000ULL, /**< DECT/UMTS-GSM interworking - UMTS-GSM Bearer service supported */
	DECT_PROFILE_ISDN_IAP_SUPPORTED				= 0x0004000000000000ULL, /**< ISDN Intermediate Access Profile supported */
	DECT_PROFILE_DATA_SERVICES_PROFILE_D			= 0x0002000000000000ULL, /**< Data Services Profile D, Class 2 supported (EN 301 238) */
	DECT_PROFILE_DPRS_FREL_SUPPORTED			= 0x0001000000000000ULL, /**< DPRS Class 2 management and B-field procedures (DPRS-M.5) supported (EN 301 649) */
	DECT_PROFILE_TOKEN_RING_SUPPORTED			= 0x0000400000000000ULL, /**< Token Ring supported */
	DECT_PROFILE_ETHERNET_SUPPORTED				= 0x0000200000000000ULL, /**< Ethernet supported */
	DECT_PROFILE_MULTIPORT_CTA				= 0x0000100000000000ULL, /**< Multiport CTA (EN 300 765-1) */
	DECT_PROFILE_DMAP_SUPPORTED				= 0x0000080000000000ULL, /**< DMAP supported */
	DECT_PROFILE_SMS_OVER_LRMS_SUPPORTED			= 0x0000040000000000ULL, /**< SMS over LRMS(EN 300 757) supported */
	DECT_PROFILE_WRS_SUPPORTED				= 0x0000020000000000ULL, /**< WRS supported */
	DECT_PROFILE_DECT_GSM_DUAL_MODE_TERMINAL		= 0x0000010000000000ULL, /**< DECT/GSM dual mode terminal */
	DECT_PROFILE_DPRS_SUPPORTED				= 0x0000004000000000ULL, /**< Generic Media Encapsulation transport (DPRS) supported (EN 301 649) */
	DECT_PROFILE_RAP_2_PROFILE_SUPPORTED			= 0x0000002000000000ULL, /**< RAP 2 Profile supported (EN 300 765-2) */
	DECT_PROFILE_I_PQ_SERVICES_SUPPORTED			= 0x0000001000000000ULL, /**< I_PQ services supported */
	DECT_PROFILE_C_F_CHANNEL_SUPPORTED			= 0x0000000800000000ULL, /**< C_F supported */
	DECT_PROFILE_V_24_SUPPORTED				= 0x0000000400000000ULL, /**< V.24 supported */
	DECT_PROFILE_PPP_SUPPORTED				= 0x0000000200000000ULL, /**< PPP supported */
	DECT_PROFILE_IP_SUPPORTED				= 0x0000000100000000ULL, /**< IP supported */
	DECT_PROFILE_8_LEVEL_A_FIELD_MODULATION			= 0x0000000040000000ULL, /**< 8-level modulation scheme supported (A field) */
	DECT_PROFILE_4_LEVEL_A_FIELD_MODULATION			= 0x0000000020000000ULL, /**< 4-level modulation scheme supported (A field) */
	DECT_PROFILE_2_LEVEL_A_FIELD_MODULATION			= 0x0000000010000000ULL, /**< 2-level modulation scheme supported (A field) */
	DECT_PROFILE_16_LEVEL_BZ_FIELD_MODULATION		= 0x0000000008000000ULL, /**< 16-level modulation scheme supported (B+Z field) */
	DECT_PROFILE_8_LEVEL_BZ_FIELD_MODULATION		= 0x0000000004000000ULL, /**< 8-level modulation scheme supported (B+Z field) */
	DECT_PROFILE_4_LEVEL_BZ_FIELD_MODULATION		= 0x0000000002000000ULL, /**< 4-level modulation scheme supported (B+Z field) */
	DECT_PROFILE_2_LEVEL_BZ_FIELD_MODULATION		= 0x0000000001000000ULL, /**< 2-level modulation scheme supported (B+Z field) */
	DECT_PROFILE_NO_EMISSION_MODE_SUPPORTED			= 0x0000000000400000ULL, /**< Capability to support "no emission" mode (EN 300 175-3) */
	DECT_PROFILE_PT_WITH_FAST_HOPPING_RADIO			= 0x0000000000200000ULL, /**< PT with fast hopping radio */
	DECT_PROFILE_G_F_CHANNEL_SUPPORTED			= 0x0000000000100000ULL, /**< Channel G_F supported */
	DECT_PROFILE_F_MMS_INTERWORKING_PROFILE_SUPPORTED	= 0x0000000000080000ULL, /**< F-MMS Interworking profile supported (TS 102 379) */
	DECT_PROFILE_BASIC_ODAP_SUPPORTED			= 0x0000000000040000ULL, /**< Basic ODAP supported (TS 102 342) */
	DECT_PROFILE_DECT_UMTS_INTERWORKING_GPRS_SUPPORTED	= 0x0000000000020000ULL, /**< DECT/UMTS interworking - GPRS services supported (TS 101 863) */
	DECT_PROFILE_DECT_UMTS_INTERWORKING_PROFILE_SUPPORTED	= 0x0000000000010000ULL, /**< DECT/UMTS interworking profile supported (TS 101 863) */
	DECT_PROFILE_REKEYING_EARLY_ENCRYPTION_SUPPORTED	= 0x0000000000001000ULL, /**< Support of "Re-keying" and "default cipher key mechanism early encryption" */
	DECT_PROFILE_HEADSET_MANAGEMENT_SUPPORTED		= 0x0000000000000800ULL, /**< Support of the "Headset management" feature */
	DECT_PROFILE_NG_DECT_PART_3				= 0x0000000000000400ULL, /**< Support of NG-DECT Part 3 (TS 102 527-3) */
	DECT_PROFILE_NG_DECT_PART_1				= 0x0000000000000200ULL, /**< NG-DECT Part 1: Wideband voice supported (TS 102 527-1) */
	DECT_PROFILE_64_LEVEL_BZ_FIELD_MODULATION		= 0x0000000000000100ULL, /**< 64-level modulation scheme supported (B+Z field) */
};

/** Display control capabilities */
enum dect_display_control_codes {
	DECT_DISPLAY_CONTROL_CODE_NOT_SPECIFIED			= 0x0, /**< Not specified */
	DECT_DISPLAY_CONTROL_CODE_CLEAR_DISPLAY			= 0x1, /**< 0CH (clear display) */
	DECT_DISPLAY_CONTROL_CODE_CODING_1			= 0x2, /**< Coding 001 plus 08H to 0BH and 0DH. */
	DECT_DISPLAY_CONTROL_CODE_CODING_2			= 0x3, /**< Coding 001 plus 08H to 0BH and 0DH. */
	DECT_DISPLAY_CONTROL_CODE_CODING_3			= 0x4, /**< Coding 001 plus 08H to 0BH and 0DH. */
};

/** Supported character sets */
enum dect_display_character_sets {
	DECT_DISPLAY_CHARACTER_SET_ISO_8859_1			= 1 << 0, /**< ISO/IEC 8859-1 */
	DECT_DISPLAY_CHARACTER_SET_ISO_8859_15			= 1 << 1, /**< ISO/IEC 8859-1 */
	DECT_DISPLAY_CHARACTER_SET_ISO_8859_9			= 1 << 2, /**< ISO/IEC 8859-9 */
	DECT_DISPLAY_CHARACTER_SET_ISO_8859_7			= 1 << 3, /**< ISO/IEC 8859-7 */
};

/** <<TERMINAL-CAPABILITY>> IE */
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

/**
 * @}@}
 * @defgroup ie_transit_delay Transit delay
 *
 * <<TRANSIT-DELAY>> IE specified in ETSI EN 300 175-5 section 7.7.42.
 *
 * The <<TRANSIT-DELAY>> IE indicates the allowable delay for data transmitted
 * through the DECT subnetwork.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.42.
 * @{
 */

/** <<TRANSIT-DELAY>> IE */
struct dect_ie_transit_delay {
	struct dect_ie_common		common;
};

/**
 * @}
 * @defgroup ie_window_size Window size
 *
 * <<WINDOW-SIZE>> IE specified in ETSI EN 300 175-5 section 7.7.43.
 *
 * The <<WINDOW-SIZE>> IE indicates and optionally negotiates the window size
 * to be used for frame transmission.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.43.
 * @{
 */

/** <<WINDOW-SIZE>> IE */
struct dect_ie_window_size {
	struct dect_ie_common		common;
};

/**
 * @}
 * @addtogroup ie_ciphering
 * @{
 * @defgroup ie_zap_field ZAP field
 *
 * <<ZAP>> IE specified in ETSI EN 300 175-5 section 7.7.44.
 *
 * The <<ZAP>> IE provides the PT with a ZAP value.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.44.
 * @{
 */

struct dect_ie_zap_field {
	struct dect_ie_common		common;
};

/**
 * @}@}
 * @addtogroup ie_generic
 * @{
 * @defgroup ie_escape_to_proprietary Escape to proprietary
 *
 * <<ESCAPE-TO-PROPRIETARY>> IE specified in ETSI EN 300 175-5 section 7.7.45.
 *
 * The <<ESCAPE-TO-PROPRIETARY>> IE is used to send non-standardized information
 * inside a DECT message.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.45
 * @{
 */

/** <<ESCAPE-TO-PROPRIETARY>> IE */
struct dect_ie_escape_to_proprietary {
	struct dect_ie_common		common;
	uint16_t			emc;
	uint8_t				len;
	uint8_t				content[64];
};

/**
 * @}@}
 * @defgroup ie_model_identifier Model identifier
 *
 * <<MODEL-IDENTIFIER>> IE specified in ETSI EN 300 175-5 section 7.7.46.
 *
 * The <<MODEL-IDENTIFIER>> IE identifies the model version of the PT to the FT
 * or vice versa.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.46
 * @{
 */

/** <<MODEL-IDENTIFIER>> IE */
struct dect_ie_model_identifier {
	struct dect_ie_common		common;
};

/**
 * @}
 * @addtogroup ie_cc_related
 * @{
 * @defgroup ie_mms_generic_header MMS Generic Header
 * @{
 */

struct dect_ie_mms_generic_header {
	struct dect_ie_common		common;
};

/**
 * @}
 * @defgroup ie_mms_object_header MMS Object Header
 * @{
 */

struct dect_ie_mms_object_header {
	struct dect_ie_common		common;
};

/**
 * @}
 * @defgroup ie_mms_extended_header MMS Extended Header
 * @{
 */

struct dect_ie_mms_extended_header {
	struct dect_ie_common		common;
};

/**
 * @}@}
 * @defgroup ie_time_date Time-Date
 *
 * <<TIME-DATE>> IE specified in ETSI EN 300 175-5 section 7.7.50.
 *
 * The <<TIME-DATE>> IE is used to provide a time and/or date.
 *
 * @sa ETSI EN 300 175-5 (Network (NWK) layer), section 7.7.50
 * @{
 */

enum dect_time_date_coding {
	DECT_TIME_DATE_TIME			= 0x1, /**< Time */
	DECT_TIME_DATE_DATE			= 0x2, /**< Date */
	DECT_TIME_DATE_TIME_AND_DATE		= 0x3, /**< Time and Date */
};

enum dect_time_date_interpretation {
	DECT_TIME_DATE_CURRENT			= 0x0, /**< The current time/date */
	DECT_TIME_DATE_DURATION			= 0x1, /**< Time duration */
};

/** <<TIME-DATE>> IE */
struct dect_ie_time_date {
	struct dect_ie_common			common;
	enum dect_time_date_coding		coding;
	enum dect_time_date_interpretation	interpretation;
	uint8_t					year;
	uint8_t					month;
	uint8_t					day;
	uint8_t					hour;
	uint8_t					minute;
	uint8_t					second;
	uint8_t					timezone;
};

/**
 * @}
 * @defgroup ie_ext_ho_indicator Ext h/o indicator
 * @{
 */

struct dect_ie_ext_ho_indicator {
	struct dect_ie_common		common;
};

/**
 * @}
 * @addtogroup ie_auth
 * @{
 * @defgroup ie_auth_reject_parameter Authentication Reject Parameter
 *
 * <<AUTHENTICATION-REJECT-PARAMETER>> IE specified in ETSI EN 300 175-5 section 7.7.52.
 *
 * The <<AUTHENTICATION-REJECT-PARAMETER>> IE's function is defined in the application
 * profiles.
 *
 * @sa ETSI EN 300 175-5 section 7.7.52.
 * @{
 */

/** <<AUTHENTICATION-REJECT-PARAMETER>> IE */
struct dect_ie_auth_reject_parameter {
	struct dect_ie_common		common;
};

/**
 * @}@}
 * @addtogroup ie_cc_related
 * @{
 * @defgroup ie_codec_list Codec List
 *
 * <<CODEC-LIST>> IE specified in ETSI EN 300 175-5 section 7.7.54.
 *
 * The <<CODEC-LIST>> IE is used for codec negotiation in speech services.
 *
 * @sa ETSI EN 300 175-5 section 7.7.54.
 * @{
 */

/** Codec negotiation indicators */
enum dect_negotiation_indicator {
	DECT_NEGOTIATION_NOT_POSSIBLE		= 0x0, /**< Negotiation not possible */
	DECT_NEGOTIATION_CODEC			= 0x1, /**< Codec negotiation */
};

/** Codec identifiers */
enum dect_codec_identifier {
	DECT_CODEC_USER_SPECIFIC_32KBIT		= 0x1, /**< user specific, information transfer rate 32 kbit/s */
	DECT_CODEC_G726_32KBIT			= 0x2, /**< G.726 ADPCM, information transfer rate 32 kbit/s */
	DECT_CODEC_G722_64KBIT			= 0x3, /**< G.722, information transfer rate 64 kbit/s */
	DECT_CODEC_G711_ALAW_64KBIT		= 0x4, /**< G.711 A law PCM, information transfer rate 64 kbit/s */
	DECT_CODEC_G711_ULAW_64KBIT		= 0x5, /**< G.711 μ law PCM, information transfer rate 64 kbit/s */
	DECT_CODEC_G729_1_32KBIT		= 0x6, /**< G.729.1, information transfer rate 32 kbit/s */
	DECT_CODEC_MPEG4_ER_AAC_LD_32KBIT	= 0x7, /**< MPEG-4 ER AAC-LD, information transfer rate 32 kbit/s */
	DECT_CODEC_MPEG4_ER_AAC_LD_64KBIT	= 0x8, /**< MPEG-4 ER AAC-LD, information transfer rate 64 kbit/s */
	DECT_CODEC_USER_SPECIFIC_64KBIT		= 0x9, /**< user specific, information transfer rate 64 kbit/s */
};

/** MAC and DLC-layer services */
enum dect_mac_dlc_service {
	DECT_MAC_DLC_SERVICE_LU1_INA		= 0x0, /**< DLC service LU1, MAC service: INA (IN_minimum_delay) */
	DECT_MAC_DLC_SERVICE_LU1_INB		= 0x1, /**< DLC service LU1, MAC service: INB (IN_normal_delay) */
	DECT_MAC_DLC_SERVICE_LU1_IPM		= 0x2, /**< DLC service LU1, MAC service: IPM (IP_error_detect) */
	DECT_MAC_DLC_SERVICE_LU1_IPQ		= 0x3, /**< DLC service LU1, MAC service: IPQ (IPQ_error_detect) */
	DECT_MAC_DLC_SERVICE_LU7_INB		= 0x4, /**< DLC service LU7, MAC service: INB (IN_normal_delay) */
	DECT_MAC_DLC_SERVICE_LU12_INB		= 0x5, /**< DLC service LU12, MAC service: INB (IN_normal_delay), encapsulation as EN 300 175-4, clause E.1 */
};

/** Slot sizes */
enum dect_slot_size {
	DECT_HALF_SLOT				= 0x0, /**< Half slot; j = 0. */
	DECT_LONG_SLOT_640			= 0x1, /**< Long slot; j = 640 */
	DECT_LONG_SLOT_672			= 0x2, /**< Long slot; j = 640 */
	DECT_FULL_SLOT				= 0x4, /**< Full slot */
	DECT_DOUBLE_SLOT			= 0x5, /**< Double slot */
};

/** C-Plane routing options */
enum dect_cplane_routing {
	DECT_CPLANE_CS_ONLY			= 0x0, /**< CS only */
	DECT_CPLANE_CS_PREFERRED		= 0x1, /**< CS preferred/CF accepted */
	DECT_CPLANE_CF_PREFERRED		= 0x2, /**< CS preferred/CF accepted */
	DECT_CPLANE_CF_ONLY			= 0x4, /**< CF only */
};

/** <<CODEC-LIST>> IE */
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

/**
 * @}@}
 * @defgroup ie_events_notification Events notification
 *
 * <<EVENTS-NOTIFICATION>> IE specified in ETSI EN 300 175-5 section 7.7.55.
 *
 * The <<EVENTS-NOTIFICATION>> IE conveys events notifications to the user.
 *
 * @sa ETSI EN 300 175-5 section 7.7.55.
 * @{
 */

enum dect_event_types {
	DECT_EVENT_MESSAGE_WAITING		= 0x0, /**< Message waiting */
	DECT_EVENT_MISSED_CALL			= 0x1, /**< Missed call */
	DECT_EVENT_WEB_CONTENT			= 0x2, /**< Web content */
	DECT_EVENT_LIST_CHANGE_INDICATION	= 0x3, /**< List change indication */
};

enum dect_event_message_waiting_subtypes {
	DECT_EVENT_MESSAGE_WAITING_UNKNOWN	= 0x0, /**< Unknown */
	DECT_EVENT_MESSAGE_WAITING_VOICE	= 0x1, /**< Voice message */
	DECT_EVENT_MESSAGE_WAITING_SMS		= 0x2, /**< SMS message */
	DECT_EVENT_MESSAGE_WAITING_EMAIL	= 0x3, /**< E-mail message */
};

enum dect_event_missed_call_subtypes {
	DECT_EVENT_MISSED_CALL_UNKNOWN		= 0x0, /**< Unknown */
	DECT_EVENT_MISSED_CALL_VOICE		= 0x1, /**< A new external missed voice call just arrived */
};

enum dect_event_web_content_subtypes {
	DECT_EVENT_WEB_CONTENT_UNKNOWN		= 0x0, /**< Unknown */
	DECT_EVENT_WEB_CONTENT_RSS		= 0x1, /**< RSS description */
};

/** <<EVENTS-NOTIFICATION>> IE */
struct dect_ie_events_notification {
	struct dect_ie_common		common;
	unsigned int			num;
	struct {
		enum dect_event_types	type;
		uint8_t			subtype;
		uint8_t			multiplicity;
	} events[8];
};

/**
 * @}
 * @addtogroup ie_cc_related
 * @{
 * @defgroup ie_call_information Call Information
 *
 * <<CALL-INFORMATION>> IE specified in ETSI EN 300 175-5 section 7.7.56.
 *
 * The <<CALL-INFORMATION>> IE is used to convey call and line identifiers.
 *
 * @sa ETSI EN 300 175-5 section 7.7.56.
 * @{
 */

/** <<CALL-INFORMATION>> IE */
struct dect_ie_call_information {
	struct dect_ie_common		common;
};

/** @} */
/** @} */
/** @} */

#ifdef __cplusplus
}
#endif
#endif /* _LIBDECT_DECT_IE_H */
