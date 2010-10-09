/*
 * DECT Suplementary Services (SS) NWK <-> IWU interface
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_DECT_SS_H
#define _LIBDECT_DECT_SS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup ss
 * @{
 */

#include <dect/ie.h>

/** MNSS primitive parameters. */
struct dect_mnss_param {
	struct dect_ie_collection		common;
	struct dect_ie_list			facility;
	struct dect_ie_display			*display;
	struct dect_ie_keypad			*keypad;
	struct dect_ie_feature_activate		*feature_activate;
	struct dect_ie_feature_indicate		*feature_indicate;
	struct dect_ie_list			iwu_to_iwu;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
	struct dect_ie_time_date		*time_date;
	struct dect_ie_events_notification	*events_notification;
	struct dect_ie_call_information		*call_information;
	struct dect_ie_release_reason		*release_reason;
};

struct dect_handle;
struct dect_ss_endpoint;

extern void *dect_ss_priv(struct dect_ss_endpoint *sse);
extern struct dect_ss_endpoint *dect_ss_endpoint_alloc(struct dect_handle *dh);
extern void dect_ss_endpoint_destroy(struct dect_handle *dh, struct dect_ss_endpoint *sse);

/**
 * Supplementary Services Ops.
 *
 * The SS ops are used to register callback functions for the SS primitives
 * invoked by libdect.
 */
struct dect_ss_ops {
	size_t	priv_size;
	/**< Size of the private storage area of an SS endpoint */
	void	(*mnss_setup_ind)(struct dect_handle *dh, struct dect_ss_endpoint *sse,
				  struct dect_mnss_param *param);
	/**< MNSS_SETUP-ind primitive */
	void	(*mnss_facility_ind)(struct dect_handle *dh, struct dect_ss_endpoint *sse,
				     struct dect_mnss_param *param);
	/**< MNSS_FACILITY-ind primitive */
	void	(*mnss_release_ind)(struct dect_handle *dh, struct dect_ss_endpoint *sse,
				    struct dect_mnss_param *param);
	/**< MNSS_RELEASE-ind primitve */
};

extern int dect_mnss_setup_req(struct dect_handle *dh, struct dect_ss_endpoint *sse,
			       const struct dect_ipui *ipui,
			       const struct dect_mnss_param *param);
extern int dect_mnss_facility_req(struct dect_handle *dh, struct dect_ss_endpoint *sse,
				  const struct dect_mnss_param *param);
extern int dect_mnss_release_req(struct dect_handle *dh, struct dect_ss_endpoint *sse,
				 const struct dect_mnss_param *param);

/** @} */

#ifdef __cplusplus
}
#endif
#endif /* _LIBDECT_DECT_SS_H */
