/*
 * DECT Connectionless Message Service (CLMS) NWK <-> IWU interface
 *
 * Copyright (c) 2010 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_DECT_CLMS_H
#define _LIBDECT_DECT_CLMS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup clms
 * @{
 */

#include <dect/ie.h>

struct dect_mncl_unitdata_param {
	struct dect_ie_collection		common;
	struct dect_ie_alphanumeric		*alphanumeric;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_iwu_packet		*iwu_packet;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

enum dect_clms_message_types {
	DECT_CLMS_FIXED,
	DECT_CLMS_VARIABLE,
};

struct dect_clms_ops {
	void	(*mncl_unitdata_ind)(struct dect_handle *dh,
				     enum dect_clms_message_types type,
				     struct dect_mncl_unitdata_param *param,
				     struct dect_msg_buf *mb);
};

extern void dect_mncl_unitdata_req(struct dect_handle *dh,
				   enum dect_clms_message_types type,
				   const struct dect_mncl_unitdata_param *param,
				   const struct dect_msg_buf *mb);

/** @} */

#ifdef __cplusplus
}
#endif
#endif /* _LIBDECT_DECT_CLMS_H */
