/*
 * DECT Connectionless Message Service (CLMS)
 *
 * Copyright (c) 2010 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_CLMS_H
#define _LIBDECT_CLMS_H

#include <dect/ie.h>

enum dect_clms_msg_types {
	CLMS_VARIABLE		= 0x1,
};

struct dect_clms_variable_msg {
	struct dect_msg_common			common;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_mms_generic_header	*mms_generic_header;
	struct dect_ie_mms_object_header	*mms_object_header;
	struct dect_ie_list			mms_extended_header;
	struct dect_ie_list			time_data;
	struct dect_ie_list			calling_party_number;
	struct dect_ie_calling_party_name	*calling_party_name;
	struct dect_ie_list			called_party_number;
	struct dect_ie_called_party_subaddr	*called_party_subaddr;
	struct dect_ie_segmented_info		*segmented_info;
	struct dect_ie_alphanumeric		*alphanumeric;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_iwu_packet		*iwu_packet;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_clms_fixed_addr_section {
	uint8_t		hdr;
	uint16_t	addr;
	uint8_t		pd;
	uint8_t		li;
} __packed;

#define DECT_CLMS_DATA_SIZE				4U

struct dect_clms_fixed_data_section {
	uint8_t		hdr;
	uint8_t		data[DECT_CLMS_DATA_SIZE];
} __packed;

enum dect_clms_section_types {
	DECT_CLMS_SECTION_DATA				= 0x0,
	DECT_CLMS_SECTION_ADDR				= 0x8,
};
#define DECT_CLMS_SECTION_TYPE_MASK			0x8

#define DECT_CLMS_SECTION_NUM_MASK			0x7

enum dect_clms_header_types {
	DECT_CLMS_HDR_STANDARD_ONE_SECTION		= 0x1,
	DECT_CLMS_HDR_STANDARD_MULTI_SECTION		= 0x2,
	DECT_CLMS_HDR_BITSTREAM_ONE_SECTION		= 0x3,
	DECT_CLMS_HDR_BITSTREAM_MULTI_SECTION		= 0x4,
	DECT_CLMS_HDR_ALPHANUMERIC_ONE_SECTION		= 0x5,
	DECT_CLMS_HDR_ALPHANUMERIC_MULTI_SECTION	= 0x6,
};
#define DECT_CLMS_HDR_MASK				0x7

enum dect_clms_protocol_discriminators {
	DECT_CLMS_PD_DECT_IE_CODING			= 0x1,
	DECT_CLMS_PD_DISTRIBUTED_COMMUNICATIONS		= 0x10,
};

extern void dect_clms_rcv_fixed(struct dect_handle *dh, struct dect_msg_buf *mb);

extern const struct dect_nwk_protocol dect_clms_protocol;

#endif /* _LIBDECT_CLMS_H */
