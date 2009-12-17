/*
 * DECT Mobility Management (MM) NWK <-> IWU interface
 *
 * Copyright (c) 2009 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_DECT_MM_H
#define _LIBDECT_DECT_MM_H

#include <dect/ie.h>

struct dect_mm_access_rights_param {
	struct dect_ie_collection		common;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_list			fixed_identity;
	struct dect_ie_location_area		*location_area;
	struct dect_ie_auth_type		*auth_type;
	struct dect_ie_cipher_info		*cipher_info;
	struct dect_ie_zap_field		*zap_field;
	struct dect_ie_terminal_capability	*terminal_capability;
	struct dect_ie_service_class		*service_class;
	struct dect_ie_model_identifier		*model_identifier;
	struct dect_ie_reject_reason		*reject_reason;
	struct dect_ie_duration			*duration;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_mm_key_allocate_param {
	struct dect_ie_collection		common;
	struct dect_ie_allocation_type		*allocation_type;
	struct dect_ie_auth_value		*rand;
	struct dect_ie_auth_value		*rs;
};

struct dect_mm_authenticate_param {
	struct dect_ie_collection		common;
	struct dect_ie_auth_type		*auth_type;
	struct dect_ie_auth_value		*rand;
	struct dect_ie_auth_res			*res;
	struct dect_ie_auth_value		*rs;
	struct dect_ie_cipher_info		*cipher_info;
	struct dect_ie_zap_field		*zap_field;
	struct dect_ie_service_class		*service_class;
	struct dect_ie_key			*key;
	struct dect_ie_reject_reason		*reject_reason;
	struct dect_ie_list			iwu_to_iwu;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_mm_cipher_param {
	struct dect_ie_collection		common;
	struct dect_ie_cipher_info		*cipher_info;
	struct dect_ie_call_identity		*call_identity;
	struct dect_ie_connection_identity	*connection_identity;
	struct dect_ie_reject_reason		*reject_reason;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_mm_locate_param {
	struct dect_ie_collection		common;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_fixed_identity		*fixed_identity;
	struct dect_ie_location_area		*location_area;
	struct dect_ie_nwk_assigned_identity	*nwk_assigned_identity;
	struct dect_ie_cipher_info		*cipher_info;
	struct dect_ie_reject_reason		*reject_reason;
	struct dect_ie_setup_capability		*setup_capability;
	struct dect_ie_terminal_capability	*terminal_capability;
	struct dect_ie_duration			*duration;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_model_identifier		*model_identifier;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_mm_detach_param {
	struct dect_ie_collection		common;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_nwk_assigned_identity	*nwk_assigned_identity;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
};

struct dect_mm_identity_param {
	struct dect_ie_collection		common;
	struct dect_ie_identity_type		*identity_type;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_fixed_identity		*fixed_identity;
	struct dect_ie_nwk_assigned_identity	*nwk_assigned_identity;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_model_identifier		*model_identifier;
};

struct dect_mm_identity_assign_param {
	struct dect_ie_collection		common;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_nwk_assigned_identity	*nwk_assigned_identity;
	struct dect_ie_duration			*duration;
	struct dect_ie_reject_reason		*reject_reason;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_mm_info_param {
	struct dect_ie_collection		common;
	struct dect_ie_info_type		*info_type;
	struct dect_ie_call_identity		*call_identity;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_list			*fixed_identity;
	struct dect_ie_location_area		*location_area;
	struct dect_ie_nwk_assigned_identity	*nwk_assigned_identity;
	struct dect_ie_network_parameter	*network_parameter;
	struct dect_ie_reject_reason		*reject_reason;
	struct dect_ie_duration			*duration;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

struct dect_mm_endpoint;

extern struct dect_mm_endpoint *dect_mm_endpoint_alloc(struct dect_handle *dh);
extern void *dect_mm_priv(struct dect_mm_endpoint *mme);

struct dect_mm_ops {
	size_t	priv_size;
	void	(*mm_access_rights_ind)(struct dect_handle *dh,
					struct dect_mm_endpoint *mme,
					struct dect_mm_access_rights_param *param);
	void	(*mm_access_rights_cfm)(struct dect_handle *dh,
					struct dect_mm_endpoint *mme, bool accept,
					struct dect_mm_access_rights_param *param);

	void	(*mm_key_allocate_ind)(struct dect_handle *dh,
				       struct dect_mm_endpoint *mme,
				       struct dect_mm_key_allocate_param *param);

	void	(*mm_authenticate_ind)(struct dect_handle *dh,
				       struct dect_mm_endpoint *mme,
				       struct dect_mm_authenticate_param *param);
	void	(*mm_authenticate_cfm)(struct dect_handle *dh,
				       struct dect_mm_endpoint *mme, bool accept,
				       struct dect_mm_authenticate_param *param);

	void	(*mm_cipher_ind)(struct dect_handle *dh,
				 struct dect_mm_endpoint *mme,
				 struct dect_mm_cipher_param *param);
	void	(*mm_cipher_cfm)(struct dect_handle *dh,
				 struct dect_mm_endpoint *mme, bool accept,
				 struct dect_mm_cipher_param *param);

	void	(*mm_locate_ind)(struct dect_handle *dh,
				 struct dect_mm_endpoint *mme,
				 struct dect_mm_locate_param *param);
	void	(*mm_locate_res)(struct dect_handle *dh,
				 struct dect_mm_endpoint *mme,
				 struct dect_mm_locate_param *param);

	void	(*mm_detach_ind)(struct dect_handle *dh,
				 struct dect_mm_endpoint *mme,
				 struct dect_mm_detach_param *param);

	void	(*mm_identity_ind)(struct dect_handle *dh,
				   struct dect_mm_endpoint *mme,
				   struct dect_mm_identity_param *param);
	void	(*mm_identity_cfm)(struct dect_handle *dh,
				   struct dect_mm_endpoint *mme, bool accept,
				   struct dect_mm_identity_param *param);

	void	(*mm_identity_assign_ind)(struct dect_handle *dh,
					  struct dect_mm_endpoint *mme,
					  struct dect_mm_identity_assign_param *param);
	void	(*mm_identity_assign_cfm)(struct dect_handle *dh,
					  struct dect_mm_endpoint *mme, bool accept,
					  struct dect_mm_identity_assign_param *param);

	void	(*mm_info_ind)(struct dect_handle *dh,
			       struct dect_mm_endpoint *mme,
			       struct dect_mm_info_param *param);
	void	(*mm_info_cfm)(struct dect_handle *dh,
			       struct dect_mm_endpoint *mme, bool accept,
			       struct dect_mm_info_param *param);
};

extern int dect_mm_access_rights_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
				     const struct dect_mm_access_rights_param *param);
extern int dect_mm_access_rights_res(struct dect_handle *dh, struct dect_mm_endpoint *mme,
				     bool accept, const struct dect_mm_access_rights_param *param);

extern int dect_mm_key_allocate_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
				    const struct dect_mm_key_allocate_param *param);

extern int dect_mm_authenticate_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
				    const struct dect_mm_authenticate_param *param);
extern int dect_mm_authenticate_res(struct dect_handle *dh, struct dect_mm_endpoint *mme,
				    bool accept, const struct dect_mm_authenticate_param *param);

extern int dect_mm_cipher_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
			      const struct dect_mm_cipher_param *param,
			      const uint8_t ck[]);
extern int dect_mm_cipher_res(struct dect_handle *dh, struct dect_mm_endpoint *mme,
			      bool accept, const struct dect_mm_cipher_param *param,
			      const uint8_t ck[]);

extern int dect_mm_locate_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
			      const struct dect_mm_locate_param *param);
extern int dect_mm_locate_res(struct dect_handle *dh, struct dect_mm_endpoint *mme,
			      bool accept, const struct dect_mm_locate_param *param);

extern int dect_mm_detach_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
			      struct dect_mm_detach_param *param);

extern int dect_mm_identity_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
				const struct dect_mm_identity_param *param);
extern int dect_mm_identity_res(struct dect_handle *dh, struct dect_mm_endpoint *mme,
				const struct dect_mm_identity_param *param);

extern int dect_mm_identity_assign_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
				       const struct dect_mm_identity_assign_param *param);
extern int dect_mm_identity_assign_res(struct dect_handle *dh, struct dect_mm_endpoint *mme,
				       bool accept, const struct dect_mm_identity_assign_param *param);

extern int dect_mm_info_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
			    struct dect_mm_info_param *param);
extern int dect_mm_info_res(struct dect_handle *dh, struct dect_mm_endpoint *mme,
			    bool accept, struct dect_mm_info_param *param);

#endif /* _LIBDECT_DECT_MM_H */
