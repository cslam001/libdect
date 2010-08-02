/*
 * DECT Link Control Entity (LCE) NWK <-> IWU interface
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_DECT_LCE_H
#define _LIBDECT_DECT_LCE_H

#include <dect/ie.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup lce
 * @{
 */

struct dect_lce_page_param {
	struct dect_ie_collection		common;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_fixed_identity		*fixed_identity;
	struct dect_ie_nwk_assigned_identity	*nwk_assigned_identity;
	struct dect_ie_cipher_info		*cipher_info;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_lce_ops {
	bool	(*lce_page_response)(struct dect_handle *dh,
				     struct dect_lce_page_param *param);
	void	(*lce_group_ring_ind)(struct dect_handle *dh,
				      enum dect_ring_patterns pattern);
};

extern int dect_lce_group_ring_req(struct dect_handle *dh,
				   enum dect_ring_patterns pattern);

/** @} */

#ifdef __cplusplus
}
#endif
#endif /* _LIBDECT_DECT_LCE_H */
