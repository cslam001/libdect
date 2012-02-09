/*
 * DECT Link Control Entity (LCE) NWK <-> IWU interface
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_DECT_LCE_H
#define _LIBDECT_DECT_LCE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup lce
 * @{
 */

#include <dect/ie.h>

/** LCE_PAGE_RESPONSE primitive parameters */
struct dect_lce_page_param {
	struct dect_ie_collection		common;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_fixed_identity		*fixed_identity;
	struct dect_ie_nwk_assigned_identity	*nwk_assigned_identity;
	struct dect_ie_cipher_info		*cipher_info;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_mac_conn_params;
struct dect_data_link;

/**
 * Link Control Entity Ops.
 *
 * The LCE ops are used to register callback functions for the LCE primitives
 * invoked by libdect.
 */
struct dect_lce_ops {
	bool	(*lce_page_response)(struct dect_handle *dh,
				     struct dect_data_link *ddl,
				     struct dect_lce_page_param *param);
	/**< LCE_PAGE_RESPONSE-ind primitive */
	void	(*lce_group_ring_ind)(struct dect_handle *dh,
				      enum dect_alerting_patterns pattern);
	/**< LCE_GROUP_RING-ind primitive */

	void	(*dl_establish_cfm)(struct dect_handle *dh, bool success,
				    struct dect_data_link *ddl,
				    const struct dect_mac_conn_params *mcp);
	/**< DL_ESTABLISH-cfm primitive */
};

extern int dect_lce_group_ring_req(struct dect_handle *dh,
				   enum dect_alerting_patterns pattern);

extern int dect_dl_establish_req(struct dect_handle *dh, const struct dect_ipui *ipui,
				 const struct dect_mac_conn_params *mcp);


/** @} */

#ifdef __cplusplus
}
#endif
#endif /* _LIBDECT_DECT_LCE_H */
