/*
 * DECT Mobility Management (MM) NWK <-> IWU interface
 *
 * Copyright (c) 2009 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_DECT_MM_H
#define _LIBDECT_DECT_MM_H

#include <dect/ie.h>

struct dect_mm_access_rights_param {
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_fixed_identity		*fixed_identity;
	struct dect_ie_location_area		*location_area;
	struct dect_ie_auth_type		*auth_type;
	struct dect_ie_cipher_info		*cipher_info;
	struct dect_ie_zap_field		*zap_field;
	struct dect_ie_service_class		*service_class;
	struct dect_ie_model_identifier		*model_identifier;
	struct dect_ie_reject_reason		*reject_reason;
	struct dect_ie_duration			*duration;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
};

struct dect_mm_ops {
	void	(*mm_access_rights_ind)(struct dect_handle *dh,
					const struct dect_mm_access_rights_param *param);
	void	(*mm_access_rights_cfm)(struct dect_handle *dh,
					const struct dect_mm_access_rights_param *param);
};

extern int dect_mm_access_rights_req(struct dect_handle *dh,
				     const struct dect_mm_access_rights_param *param);
extern int dect_mm_access_rights_res(struct dect_handle *dh,
				     const struct dect_mm_access_rights_param *param);

#endif /* _LIBDECT_DECT_MM_H */
