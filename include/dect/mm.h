/*
 * DECT Mobility Management (MM) NWK <-> IWU interface
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_DECT_MM_H
#define _LIBDECT_DECT_MM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup mm
 * @{
 **/

#include <dect/ie.h>

/**
 * @addtogroup mm_access_rights
 * @{
 */

/** MM_ACCESS_RIGHTS primitive parameters. */
struct dect_mm_access_rights_param {
	struct dect_ie_collection		common;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_list			fixed_identity;
	struct dect_ie_location_area		*location_area;
	struct dect_ie_auth_type		*auth_type;
	struct dect_ie_cipher_info		*cipher_info;
	struct dect_ie_zap_field		*zap_field;
	struct dect_ie_setup_capability		*setup_capability;
	struct dect_ie_terminal_capability	*terminal_capability;
	struct dect_ie_service_class		*service_class;
	struct dect_ie_model_identifier		*model_identifier;
	struct dect_ie_reject_reason		*reject_reason;
	struct dect_ie_duration			*duration;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
	struct dect_ie_codec_list		*codec_list;
};

/**
 * @}
 * @addtogroup mm_access_rights_terminate
 * @{
 */

/** MM_ACCESS_RIGHTS_TERMINATE primitive parameters. */
struct dect_mm_access_rights_terminate_param {
	struct dect_ie_collection		common;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_list			fixed_identity;
	struct dect_ie_reject_reason		*reject_reason;
	struct dect_ie_duration			*duration;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

/**
 * @}
 * @addtogroup mm_key_allocation
 * @{
 */

/** MM_KEY_ALLOCATE primitive parameters. */
struct dect_mm_key_allocate_param {
	struct dect_ie_collection		common;
	struct dect_ie_allocation_type		*allocation_type;
	struct dect_ie_auth_value		*rand;
	struct dect_ie_auth_value		*rs;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

/**
 * @}
 * @addtogroup mm_auth
 * @{
 */

/** MM_AUTHENTICATE primitive parameters. */
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

/**
 * @}
 * @addtogroup mm_cipher
 * @{
 */

/** MM_CIPHER primitive parameters. */
struct dect_mm_cipher_param {
	struct dect_ie_collection		common;
	struct dect_ie_cipher_info		*cipher_info;
	struct dect_ie_call_identity		*call_identity;
	struct dect_ie_connection_identity	*connection_identity;
	struct dect_ie_reject_reason		*reject_reason;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

/**
 * @}
 * @addtogroup mm_location_registration
 * @{
 */

/** MM_LOCATE primitive parameters. */
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
	struct dect_ie_codec_list		*codec_list;
};

/**
 * @}
 * @addtogroup mm_detach
 * @{
 */

/** MM_DETACH primitive parameters. */
struct dect_mm_detach_param {
	struct dect_ie_collection		common;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_nwk_assigned_identity	*nwk_assigned_identity;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

/**
 * @}
 * @addtogroup mm_identification
 * @{
 */

/** MM_IDENTITY primitive parameters */
struct dect_mm_identity_param {
	struct dect_ie_collection		common;
	struct dect_ie_list			identity_type;
	struct dect_ie_network_parameter	*network_parameter;
	struct dect_ie_list			portable_identity;
	struct dect_ie_list			fixed_identity;
	struct dect_ie_list			nwk_assigned_identity;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_model_identifier		*model_identifier;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

/**
 * @}
 * @addtogroup mm_temporary_identity_assignment
 * @{
 */

/** MM_IDENTITY_ASSIGN primitive parameters. */
struct dect_mm_identity_assign_param {
	struct dect_ie_collection		common;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_location_area		*location_area;
	struct dect_ie_nwk_assigned_identity	*nwk_assigned_identity;
	struct dect_ie_duration			*duration;
	struct dect_ie_network_parameter	*network_parameter;
	struct dect_ie_reject_reason		*reject_reason;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

/**
 * @}
 * @addtogroup mm_parameter_retrieval
 * @{
 */

/** MM_INFO primitive parameters. */
struct dect_mm_info_param {
	struct dect_ie_collection		common;
	struct dect_ie_info_type		*info_type;
	struct dect_ie_call_identity		*call_identity;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_list			fixed_identity;
	struct dect_ie_location_area		*location_area;
	struct dect_ie_nwk_assigned_identity	*nwk_assigned_identity;
	struct dect_ie_network_parameter	*network_parameter;
	struct dect_ie_reject_reason		*reject_reason;
	struct dect_ie_duration			*duration;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

/**
 * @}
 * @addtogroup mm_external_protocol_information
 * @{
 */

/** MM_IWU primitive parameters. */
struct dect_mm_iwu_param {
	struct dect_ie_collection		common;
	struct dect_ie_iwu_to_iwu		*iwu_to_iwu;
	struct dect_ie_iwu_packet		*iwu_packet;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};
/** @} */

struct dect_mm_endpoint;

extern struct dect_mm_endpoint *dect_mm_endpoint_alloc(struct dect_handle *dh,
						       const struct dect_ipui *ipui);
extern void dect_mm_endpoint_destroy(struct dect_handle *dh,
				     struct dect_mm_endpoint *mme);
extern void *dect_mm_priv(struct dect_mm_endpoint *mme);

/**
 * Mobility Management Ops.
 *
 * The MM ops are used to register callback functions for the MM primitives
 * invoked by libdect.
 */
struct dect_mm_ops {
	size_t	priv_size;
	/**< Size of the private storage area of an MM endpoint */
	void	(*mm_access_rights_ind)(struct dect_handle *dh,
					struct dect_mm_endpoint *mme,
					struct dect_mm_access_rights_param *param);
	/**< MM_ACCESS_RIGHTS-ind primitive */
	void	(*mm_access_rights_cfm)(struct dect_handle *dh,
					struct dect_mm_endpoint *mme, bool accept,
					struct dect_mm_access_rights_param *param);
	/**< MM_ACCESS_RIGHTS-cfm primitive */

	void	(*mm_access_rights_terminate_ind)(struct dect_handle *dh,
						  struct dect_mm_endpoint *mme,
						  struct dect_mm_access_rights_terminate_param *param);
	/**< MM_ACCESS_RIGHTS_TERMINATE-ind primitive */
	void	(*mm_access_rights_terminate_cfm)(struct dect_handle *dh,
						  struct dect_mm_endpoint *mme, bool accept,
						  struct dect_mm_access_rights_terminate_param *param);
	/**< MM_ACCESS_RIGHTS_TERMINATE-cfm primitive */

	void	(*mm_key_allocate_ind)(struct dect_handle *dh,
				       struct dect_mm_endpoint *mme,
				       struct dect_mm_key_allocate_param *param);
	/**< MM_KEY_ALLOCATE-ind primitive */

	void	(*mm_authenticate_ind)(struct dect_handle *dh,
				       struct dect_mm_endpoint *mme,
				       struct dect_mm_authenticate_param *param);
	/**< MM_AUTHENTICATE-ind primitive */
	void	(*mm_authenticate_cfm)(struct dect_handle *dh,
				       struct dect_mm_endpoint *mme, bool accept,
				       struct dect_mm_authenticate_param *param);
	/**< MM_AUTHENTICATE-cfm primitive */

	void	(*mm_cipher_ind)(struct dect_handle *dh,
				 struct dect_mm_endpoint *mme,
				 struct dect_mm_cipher_param *param);
	/**< MM_CIPHER-ind primitive */
	void	(*mm_cipher_cfm)(struct dect_handle *dh,
				 struct dect_mm_endpoint *mme, bool accept,
				 struct dect_mm_cipher_param *param);
	/**< MM_CIPHER-cfm primitive */

	void	(*mm_locate_ind)(struct dect_handle *dh,
				 struct dect_mm_endpoint *mme,
				 struct dect_mm_locate_param *param);
	/**< MM_LOCATE-ind primitive */
	void	(*mm_locate_cfm)(struct dect_handle *dh,
				 struct dect_mm_endpoint *mme, bool accept,
				 struct dect_mm_locate_param *param);
	/**< MM_LOCATE-cfm primitive */

	void	(*mm_detach_ind)(struct dect_handle *dh,
				 struct dect_mm_endpoint *mme,
				 struct dect_mm_detach_param *param);
	/**< MM_DETACH-ind primitive */

	void	(*mm_identity_ind)(struct dect_handle *dh,
				   struct dect_mm_endpoint *mme,
				   struct dect_mm_identity_param *param);
	/**< MM_IDENTITY-ind primitive */
	void	(*mm_identity_cfm)(struct dect_handle *dh,
				   struct dect_mm_endpoint *mme,
				   struct dect_mm_identity_param *param);
	/**< MM_IDENTITY-cfm primitive */

	void	(*mm_identity_assign_ind)(struct dect_handle *dh,
					  struct dect_mm_endpoint *mme,
					  struct dect_mm_identity_assign_param *param);
	/**< MM_IDENTITY_ASSIGN-ind primitive */
	void	(*mm_identity_assign_cfm)(struct dect_handle *dh,
					  struct dect_mm_endpoint *mme, bool accept,
					  struct dect_mm_identity_assign_param *param);
	/**< MM_IDENTITY_ASSIGN-cfm primitive */

	void	(*mm_info_ind)(struct dect_handle *dh,
			       struct dect_mm_endpoint *mme,
			       struct dect_mm_info_param *param);
	/**< MM_INFO-ind primitive */
	void	(*mm_info_cfm)(struct dect_handle *dh,
			       struct dect_mm_endpoint *mme, bool accept,
			       struct dect_mm_info_param *param);
	/**< MM_INFO-cfm primitive */

	void	(*mm_iwu_ind)(struct dect_handle *dh,
			      struct dect_mm_endpoint *mme,
			      struct dect_mm_iwu_param *param);
	/**< MM_IWU-ind primitive */
};

extern int dect_mm_access_rights_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
				     const struct dect_mm_access_rights_param *param);
extern int dect_mm_access_rights_res(struct dect_handle *dh, struct dect_mm_endpoint *mme,
				     bool accept, const struct dect_mm_access_rights_param *param);

extern int dect_mm_access_rights_terminate_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
					       const struct dect_mm_access_rights_terminate_param *param);
extern int dect_mm_access_rights_terminate_res(struct dect_handle *dh, struct dect_mm_endpoint *mme,
					       bool accept, const struct dect_mm_access_rights_terminate_param *param);

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

extern int dect_mm_iwu_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
			   const struct dect_mm_iwu_param *param);

/** @} */

#ifdef __cplusplus
}
#endif
#endif /* _LIBDECT_DECT_MM_H */
