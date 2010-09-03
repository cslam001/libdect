/*
 * DECT Connetionless Message Service (CLMS)
 *
 * Copyright (c) 2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/**
 * @defgroup clms ConnectionLess Message Service
 * @{
 */

#include <stdint.h>
#include <linux/byteorder/little_endian.h>

#include <libdect.h>
#include <clms.h>
#include <lce.h>

static DECT_SFMT_MSG_DESC(clms_variable,
	DECT_SFMT_IE(DECT_IE_PORTABLE_IDENTITY,		IE_MANDATORY, IE_MANDATORY, 0),
	DECT_SFMT_IE(DECT_IE_MMS_GENERIC_HEADER,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_MMS_OBJECT_HEADER,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_MMS_EXTENDED_HEADER,	IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_TIME_DATE,			IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CALLING_PARTY_NUMBER,	IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_CALLING_PARTY_NAME,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_REPEAT_INDICATOR,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_CALLED_PARTY_NUMBER,	IE_OPTIONAL,  IE_OPTIONAL,  DECT_SFMT_IE_REPEAT),
	DECT_SFMT_IE(DECT_IE_CALLED_PARTY_SUBADDR,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_SEGMENTED_INFO,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ALPHANUMERIC,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_TO_IWU,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_IWU_PACKET,		IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE(DECT_IE_ESCAPE_TO_PROPRIETARY,	IE_OPTIONAL,  IE_OPTIONAL,  0),
	DECT_SFMT_IE_END_MSG
);

#define __clms_debug(pfx, fmt, args...) \
	dect_debug(DECT_DEBUG_CLMS, "%sCLMS: " fmt "\n", pfx, ## args)

#define clms_debug(fmt, args...) \
	__clms_debug("", fmt, ##args)
#define clms_debug_entry(fmt, args...) \
	__clms_debug("\n", fmt, ##args)

void dect_clms_rcv_fixed(struct dect_handle *dh, struct dect_msg_buf *mb)
{
	DECT_DEFINE_MSG_BUF_ONSTACK(_mb), *mbr = &_mb;
	struct dect_clms_fixed_addr_section *as;
	struct dect_clms_fixed_data_section *ds;
	unsigned int n, len, section, sections;

	clms_debug("parse {CLMS-FIXED} message");
	assert(mb->len % 5 == 0);

	as = (void *)mb->data;
	if ((as->hdr & DECT_CLMS_SECTION_TYPE_MASK) != DECT_CLMS_SECTION_ADDR)
		return;

	dect_debug(DECT_DEBUG_CLMS, "  address section:\n");
	dect_debug(DECT_DEBUG_CLMS, "\tAddress: %04x\n", __be16_to_cpu(as->addr));
	dect_debug(DECT_DEBUG_CLMS, "\tProtocol Discriminator: %02x\n", as->pd);
	dect_debug(DECT_DEBUG_CLMS, "\tLength Indicator: %02x\n", as->li);

	if (as->pd != DECT_CLMS_PD_DECT_IE_CODING)
		return;

	switch (as->hdr & DECT_CLMS_HDR_MASK) {
	case DECT_CLMS_HDR_STANDARD_ONE_SECTION:
	case DECT_CLMS_HDR_BITSTREAM_ONE_SECTION:
	case DECT_CLMS_HDR_ALPHANUMERIC_ONE_SECTION:
		memcpy(dect_mbuf_put(mbr, 1), &as->li, 1);
		goto deliver;
	case DECT_CLMS_HDR_STANDARD_MULTI_SECTION:
	case DECT_CLMS_HDR_BITSTREAM_MULTI_SECTION:
	case DECT_CLMS_HDR_ALPHANUMERIC_MULTI_SECTION:
		if (as->li == 0 || as->li % 8)
			return;
		len = as->li / 8;
		break;
	default:
		return;
	}

	dect_mbuf_pull(mb, sizeof(*as));

	sections = 0;
	while (mb->len > 0) {
		ds = (void *)mb->data;
		if ((ds->hdr & DECT_CLMS_SECTION_TYPE_MASK) !=
		    DECT_CLMS_SECTION_DATA)
			return;

		section = ds->hdr & DECT_CLMS_SECTION_NUM_MASK;
		dect_debug(DECT_DEBUG_CLMS, "  data section %u:\n", section);
		dect_debug(DECT_DEBUG_CLMS, "\tData: %02x%02x%02x%02x\n",
			   ds->data[0], ds->data[1], ds->data[2], ds->data[3]);

		if (section >= 5)
			return;
		if (section != sections)
			return;
		sections++;

		n = min(len, DECT_CLMS_DATA_SIZE);
		memcpy(dect_mbuf_put(mbr, n), ds->data, n);
		len -= n;

		dect_mbuf_pull(mb, sizeof(*ds));
	}

	if (len > 0)
		return;
deliver:
	dect_mbuf_dump(DECT_DEBUG_CLMS, mbr, "CLMS");

	clms_debug("MNCL_UNITDATA-ind: type: %u", DECT_CLMS_FIXED);
	dh->ops->clms_ops->mncl_unitdata_ind(dh, DECT_CLMS_FIXED, NULL, mbr);
}

static void dect_clms_send_fixed(struct dect_handle *dh,
				 const void *data, unsigned int len)
{
	DECT_DEFINE_MSG_BUF_ONSTACK(_mb), *mb = &_mb;
	struct dect_clms_fixed_addr_section *as;
	struct dect_clms_fixed_data_section *ds;
	unsigned int n, section;

	as = dect_mbuf_put(mb, sizeof(*as));
	as->hdr   = DECT_CLMS_SECTION_ADDR;
	as->addr  = __cpu_to_be16(DECT_TPUI_CBI);
	as->pd    = DECT_CLMS_PD_DECT_IE_CODING;
	if (len > 1) {
		as->hdr |= DECT_CLMS_HDR_STANDARD_MULTI_SECTION;
		as->li   = len * 8;
	} else {
		as->hdr |= DECT_CLMS_HDR_STANDARD_ONE_SECTION;
		as->li   = *(uint8_t *)data;
		goto deliver;
	}

	section = 0;
	while (len > 0) {
		ds = dect_mbuf_put(mb, sizeof(*ds));
		ds->hdr  = DECT_CLMS_SECTION_DATA;
		ds->hdr |= section;

		n = min(len, DECT_CLMS_DATA_SIZE);
		memcpy(ds->data, data, n);
		if (n < DECT_CLMS_DATA_SIZE)
			memset(ds->data + n, 0, DECT_CLMS_DATA_SIZE - n);
		data += n;
		len  -= n;

		section++;
	}
deliver:
	dect_lce_broadcast(dh, mb, true);
}

/**
 * DECT_MNCL_UNITDATA-req primitive
 *
 * @param dh		libdect DECT handle
 * @param type		message type (fixed/variable) to use
 * @param param		unitdata parameters
 * @param mb		message buffer for {CLMS-FIXED} messages
 */
void dect_mncl_unitdata_req(struct dect_handle *dh,
			    enum dect_clms_message_types type,
			    const struct dect_mncl_unitdata_param *param,
			    const struct dect_msg_buf *mb)
{
	clms_debug_entry("MNCL_UNITDATA-req");
	dect_clms_send_fixed(dh, mb->data, mb->len);
}
EXPORT_SYMBOL(dect_mncl_unitdata_req);

static void dect_clms_rcv_variable(struct dect_handle *dh,
				   struct dect_transaction *ta,
				   struct dect_msg_buf *mb)
{
	struct dect_clms_variable_msg msg;

	clms_debug("CLMS-VARIABLE");
	if (dect_parse_sfmt_msg(dh, &clms_variable_msg_desc,
				&msg.common, mb) < 0)
		return;

	if (msg.portable_identity->type != DECT_PORTABLE_ID_TYPE_IPUI)
		goto out;
	if (dect_ddl_set_ipui(dh, ta->link, &msg.portable_identity->ipui) < 0)
		goto out;
out:
	dect_msg_free(dh, &clms_variable_msg_desc, &msg.common);
}

static void dect_clms_open(struct dect_handle *dh,
			   struct dect_transaction *req,
			   struct dect_msg_buf *mb)
{
	clms_debug("msg type: %x", mb->type);

	switch (mb->type) {
	case CLMS_VARIABLE:
		return dect_clms_rcv_variable(dh, req, mb);
	default:
		return;
	}
}

const struct dect_nwk_protocol dect_clms_protocol = {
	.name			= "ConnectionLess Message Service",
	.pd			= DECT_PD_CLMS,
	.max_transactions	= 1,
	.open			= dect_clms_open,
};

/** @} */
