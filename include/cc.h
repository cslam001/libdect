/*
 * DECT Call Control
 *
 * Copyright (c) 2009 Patrick McHardy <kaber@trash.net>
 */

#ifndef _DECT_CC_H
#define _DECT_CC_H

/**
 * Call Control message types
 */
enum dect_cc_msg_types {
	CC_RESERVED				= 0x0,
	CC_ALERTING				= 0x1,
	CC_CALL_PROC				= 0x2,
	CC_SETUP				= 0x5,
	CC_CONNECT				= 0x7,
	CC_SETUP_ACK				= 0xd,
	CC_CONNECT_ACK				= 0xf,
	CC_SERVICE_CHANGE			= 0x20,
	CC_SERVICE_ACCEPT			= 0x21,
	CC_SERVICE_REJECT			= 0x22,
	CC_RELEASE				= 0x4d,
	CC_RELEASE_COM				= 0x5a,
	CC_IWU_INFO				= 0x60,
	CC_NOTIFY				= 0x6e,
	CC_INFO					= 0x7b,
};

struct dect_cc_setup_msg {
	struct dect_msg_common			common;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_fixed_identity		*fixed_identity;
	struct dect_ie_nwk_assigned_identity	*nwk_assigned_identity;
	struct dect_ie_basic_service		*basic_service;
	struct dect_ie_list			iwu_attributes;
	struct dect_ie_list			call_attributes;
	struct dect_ie_list			connection_attributes;
	struct dect_ie_cipher_info		*cipher_info;
	struct dect_ie_connection_identity	*connection_identity;
	struct dect_ie_list			facility;
	struct dect_ie_list			progress_indicator;
	struct dect_ie_display			*display;
	struct dect_ie_keypad			*keypad;
	struct dect_ie_signal			*signal;
	struct dect_ie_feature_activate		*feature_activate;
	struct dect_ie_feature_indicate		*feature_indicate;
	struct dect_ie_network_parameter	*network_parameter;
	struct dect_ie_ext_ho_indicator		*ext_ho_indicator;
	struct dect_ie_terminal_capability	*terminal_capability;
	struct dect_ie_end_to_end_compatibility	*end_to_end_compatibility;
	struct dect_ie_rate_parameters		*rate_parameters;
	struct dect_ie_transit_delay		*transit_delay;
	struct dect_ie_window_size		*window_size;
	struct dect_ie_calling_party_number	*calling_party_number;
	struct dect_ie_called_party_number	*called_party_number;
	struct dect_ie_called_party_subaddress	*called_party_subaddress;
	struct dect_ie_sending_complete		*sending_complete;
	struct dect_ie_list			segmented_info;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_iwu_packet		*iwu_packet;
	struct dect_ie_calling_party_name	*calling_party_name;
	struct dect_ie_codec_list		*codec_list;
	struct dect_ie_call_information		*call_information;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_cc_info_msg {
	struct dect_msg_common			common;
	struct dect_ie_location_area		*location_area;
	struct dect_ie_nwk_assigned_identity	*nwk_assigned_identity;
	struct dect_ie_list			facility;
	struct dect_ie_list			progress_indicator;
	struct dect_ie_display			*display;
	struct dect_ie_keypad			*keypad;
	struct dect_ie_signal			*signal;
	struct dect_ie_feature_activate		*feature_activate;
	struct dect_ie_feature_indicate		*feature_indicate;
	struct dect_ie_network_parameter	*network_parameter;
	struct dect_ie_ext_ho_indicator		*ext_ho_indicator;
	struct dect_ie_calling_party_number	*calling_party_number;
	struct dect_ie_called_party_number	*called_party_number;
	struct dect_ie_called_party_subaddress	*called_party_subaddress;
	struct dect_ie_sending_complete		*sending_complete;
	struct dect_ie_test_hook_control	*test_hook_control;
	struct dect_ie_list			iwu_to_iwu;
	struct dect_ie_iwu_packet		*iwu_packet;
	struct dect_ie_calling_party_name	*calling_party_name;
	struct dect_ie_codec_list		*codec_list;
	struct dect_ie_call_information		*call_information;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_cc_setup_ack_msg {
	struct dect_msg_common			common;
	struct dect_ie_info_type		*info_type;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_fixed_identity		*fixed_identity;
	struct dect_ie_location_area		*location_area;
	struct dect_ie_iwu_attributes		*iwu_attributes;
	struct dect_ie_call_attributes		*call_attributes;
	struct dect_ie_connection_attributes	*connection_attributes;
	struct dect_ie_connection_identity	*connection_identity;
	struct dect_ie_list			facility;
	struct dect_ie_list			progress_indicator;
	struct dect_ie_display			*display;
	struct dect_ie_signal			*signal;
	struct dect_ie_feature_indicate		*feature_indicate;
	struct dect_ie_network_parameter	*network_parameter;
	struct dect_ie_ext_ho_indicator		*ext_ho_indicator;
	struct dect_ie_transit_delay		*transit_delay;
	struct dect_ie_window_size		*window_size;
	struct dect_ie_delimiter_request	*delimiter_request;
	struct dect_ie_list			iwu_to_iwu;
	struct dect_ie_iwu_packet		*iwu_packet;
	struct dect_ie_codec_list		*codec_list;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_cc_call_proc_msg {
	struct dect_msg_common			common;
	struct dect_ie_iwu_attributes		*iwu_attributes;
	struct dect_ie_call_attributes		*call_attributes;
	struct dect_ie_connection_attributes	*connection_attributes;
	struct dect_ie_connection_identity	*connection_identity;
	struct dect_ie_list			facility;
	struct dect_ie_list			progress_indicator;
	struct dect_ie_display			*display;
	struct dect_ie_signal			*signal;
	struct dect_ie_feature_indicate		*feature_indicate;
	struct dect_ie_transit_delay		*transit_delay;
	struct dect_ie_window_size		*window_size;
	struct dect_ie_list			iwu_to_iwu;
	struct dect_ie_iwu_packet		*iwu_packet;
	struct dect_ie_codec_list		*codec_list;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_cc_alerting_msg {
	struct dect_msg_common			common;
	struct dect_ie_iwu_attributes		*iwu_attributes;
	struct dect_ie_call_attributes		*call_attributes;
	struct dect_ie_connection_attributes	*connection_attributes;
	struct dect_ie_connection_identity	*connection_identity;
	struct dect_ie_list			facility;
	struct dect_ie_list			progress_indicator;
	struct dect_ie_display			*display;
	struct dect_ie_signal			*signal;
	struct dect_ie_feature_indicate		*feature_indicate;
	struct dect_ie_terminal_capability	*terminal_capability;
	struct dect_ie_transit_delay		*transit_delay;
	struct dect_ie_window_size		*window_size;
	struct dect_ie_list			iwu_to_iwu;
	struct dect_ie_iwu_packet		*iwu_packet;
	struct dect_ie_codec_list		*codec_list;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_cc_connect_msg {
	struct dect_msg_common			common;
	struct dect_ie_iwu_attributes		*iwu_attributes;
	struct dect_ie_call_attributes		*call_attributes;
	struct dect_ie_connection_attributes	*connection_attributes;
	struct dect_ie_connection_identity	*connection_identity;
	struct dect_ie_list			facility;
	struct dect_ie_list			progress_indicator;
	struct dect_ie_display			*display;
	struct dect_ie_signal			*signal;
	struct dect_ie_feature_indicate		*feature_indicate;
	struct dect_ie_network_parameter	*network_parameter;
	struct dect_ie_ext_ho_indicator		*ext_ho_indicator;
	struct dect_ie_terminal_capability	*terminal_capability;
	struct dect_ie_transit_delay		*transit_delay;
	struct dect_ie_window_size		*window_size;
	struct dect_ie_list			segmented_info;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_iwu_packet		*iwu_packet;
	struct dect_ie_codec_list		*codec_list;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_cc_connect_ack_msg {
	struct dect_msg_common			common;
	struct dect_ie_display			*display;
	struct dect_ie_feature_indicate		*feature_indicate;
	struct dect_ie_list			iwu_to_iwu;
	struct dect_ie_iwu_packet		*iwu_packet;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_cc_release_msg {
	struct dect_msg_common			common;
	struct dect_ie_release_reason		*release_reason;
	struct dect_ie_list			facility;
	struct dect_ie_list			progress_indicator;
	struct dect_ie_display			*display;
	struct dect_ie_feature_indicate		*feature_indicate;
	struct dect_ie_list			segmented_info;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_iwu_packet		*iwu_packet;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_cc_release_com_msg {
	struct dect_msg_common			common;
	struct dect_ie_release_reason		*release_reason;
	struct dect_ie_identity_type		*identity_type;
	struct dect_ie_location_area		*location_area;
	struct dect_ie_iwu_attributes		*iwu_attributes;
	struct dect_ie_connection_attributes	*connection_attributes;
	struct dect_ie_list			facility;
	struct dect_ie_display			*display;
	struct dect_ie_feature_indicate		*feature_indicate;
	struct dect_ie_network_parameter	*network_parameter;
	struct dect_ie_list			segmented_info;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_iwu_packet		*iwu_packet;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_cc_service_change_msg {
	struct dect_msg_common			common;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_iwu_attributes		*iwu_attributes;
	struct dect_ie_service_change_info	*service_change_info;
	struct dect_ie_call_attributes		*call_attributes;
	struct dect_ie_list			connection_attributes;
	struct dect_ie_connection_identity	*connection_identity;
	struct dect_ie_list			segmented_info;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_codec_list		*codec_list;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_cc_service_accept_msg {
	struct dect_msg_common			common;
	struct dect_ie_iwu_attributes		*iwu_attributes;
	struct dect_ie_connection_identity	*connection_identity;
	struct dect_ie_list			segmented_info;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_cc_service_reject_msg {
	struct dect_msg_common			common;
	struct dect_ie_release_reason		*release_reason;
	struct dect_ie_iwu_attributes		*iwu_attributes;
	struct dect_ie_connection_attributes	*connection_attributes;
	struct dect_ie_list			segmented_info;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_cc_notify_msg {
	struct dect_msg_common			common;
	struct dect_ie_timer_restart		*timer_restart;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_cc_iwu_info_msg {
	struct dect_msg_common			common;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_mms_generic_header	*mms_generic_header;
	struct dect_ie_mms_object_header	*mms_object_header;
	struct dect_ie_list			mms_extended_header;
	struct dect_ie_list			time_date;
	struct dect_ie_list			calling_party_number;
	struct dect_ie_list			called_party_number;
	struct dect_ie_called_party_subaddress	*called_party_subaddress;
	struct dect_ie_segmented_info		*segmented_info;
	struct dect_ie_alphanumeric		*alphanumeric;
	struct dect_ie_list			segmented_info2;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_segmented_info		*segmented_info3;
	struct dect_ie_iwu_packet		*iwu_packet;
	struct dect_ie_codec_list		*codec_list;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

/**
 * enum dect_cc_states - CC call states
 *
 * @DECT_CC_NULL:			T-00/F-00
 * @DECT_CC_CALL_INITIATED:		T-01/F-01
 * @DECT_CC_OVERLAP_SENDING:		T-02/F-02
 * @DECT_CC_CALL_PROCEEDING:		T-03/F-03
 * @DECT_CC_CALL_DELIVERED:		T-04/F-04
 * @DECT_CC_CALL_PRESENT:		T-06/F-06
 * @DECT_CC_CALL_RECEIVED:		T-07/F-07
 * @DECT_CC_CONNECT_PENDING:		T-08
 * @DECT_CC_ACTIVE:			T-10/F-10
 * @DECT_CC_RELEASE_PENDING:		T-19/F-19
 * @DECT_CC_OVERLAP_RECEIVING:		T-22/F-22
 * @DECT_CC_INCOMING_CALL_PROCEEDING:	T-23/F-23
 */
enum dect_cc_states {
	DECT_CC_NULL,
	DECT_CC_CALL_INITIATED,
	DECT_CC_OVERLAP_SENDING,
	DECT_CC_CALL_PROCEEDING,
	DECT_CC_CALL_DELIVERED,
	DECT_CC_CALL_PRESENT,
	DECT_CC_CALL_RECEIVED,
	DECT_CC_CONNECT_PENDING,
	DECT_CC_ACTIVE,
	DECT_CC_RELEASE_PENDING,
	DECT_CC_OVERLAP_RECEIVING,
	DECT_CC_INCOMING_CALL_PROCEEDING,
	__DECT_CC_STATE_MAX
};
#define DECT_CC_STATE_MAX		(__DECT_CC_STATE_MAX - 1)

/**
 * @transaction:	LCE link transaction
 * @state:		call state
 * @setup_timer:	call setup watchdog timer
 * @lu_sap:		U-Plane file descriptor
 * @priv:		libdect user private storage
 */
struct dect_call {
	struct dect_transaction			transaction;
	struct dect_ie_fixed_identity		*ft_id;
	struct dect_ie_portable_identity	*pt_id;
	enum dect_cc_states			state;
	struct dect_timer			*setup_timer;
	struct dect_fd				*lu_sap;
	uint8_t					priv[];
};

#define DECT_CC_SETUP_TIMEOUT		20 /* seconds */

#endif /* _DECT_CC_H */
