/*
 * DECT Link Control Entity (LCE) NWK <-> IWU interface
 *
 * Copyright (c) 2009 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_DECT_LCE_H
#define _LIBDECT_DECT_LCE_H

#include <dect/ie.h>

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
};

#endif /* _LIBDECT_DECT_LCE_H */
