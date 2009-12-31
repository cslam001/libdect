#include <libdect.h>
#include "common.h"

/*
 * CC Ops
 */

static void mncc_setup_ind(struct dect_handle *dh, struct dect_call *call,
			   struct dect_mncc_setup_param *param)
{
}

static void mncc_setup_ack_ind(struct dect_handle *dh, struct dect_call *call,
			       struct dect_mncc_setup_ack_param *param)
{
}

static void mncc_reject_ind(struct dect_handle *dh, struct dect_call *call,
			    struct dect_mncc_release_param *param)
{
}

static void mncc_call_proc_ind(struct dect_handle *dh, struct dect_call *call,
			       struct dect_mncc_call_proc_param *param)
{
}

static void mncc_alert_ind(struct dect_handle *dh, struct dect_call *call,
			   struct dect_mncc_alert_param *param)
{
}

static void mncc_connect_ind(struct dect_handle *dh, struct dect_call *call,
			     struct dect_mncc_connect_param *param)
{
}

static void mncc_connect_cfm(struct dect_handle *dh, struct dect_call *call,
			     struct dect_mncc_connect_param *param)
{
}

static void mncc_release_ind(struct dect_handle *dh, struct dect_call *call,
			     struct dect_mncc_release_param *param)
{
}

static void mncc_release_cfm(struct dect_handle *dh, struct dect_call *call,
			     struct dect_mncc_release_param *param)
{
}

static void mncc_facility_ind(struct dect_handle *dh, struct dect_call *call,
			      struct dect_mncc_facility_param *param)
{
}

static void mncc_info_ind(struct dect_handle *dh, struct dect_call *call,
			  struct dect_mncc_info_param *param)
{
}

static void mncc_modify_ind(struct dect_handle *dh, struct dect_call *call,
			    struct dect_mncc_modify_param *param)
{
}

static void mncc_modify_cfm(struct dect_handle *dh, struct dect_call *call,
			    struct dect_mncc_modify_param *param)
{
}

static void mncc_hold_ind(struct dect_handle *dh, struct dect_call *call,
			  struct dect_mncc_hold_param *param)
{
}

static void mncc_hold_cfm(struct dect_handle *dh, struct dect_call *call,
			  struct dect_mncc_hold_param *param)
{
}

static void mncc_retrieve_ind(struct dect_handle *dh, struct dect_call *call,
			      struct dect_mncc_hold_param *param)
{
}

static void mncc_retrieve_cfm(struct dect_handle *dh, struct dect_call *call,
			      struct dect_mncc_hold_param *param)
{
}

static void mncc_iwu_info_ind(struct dect_handle *dh, struct dect_call *call,
			      struct dect_mncc_iwu_info_param *param)
{
}

static void dl_u_data_ind(struct dect_handle *dh, struct dect_call *call,
			  struct dect_msg_buf *mb)
{
}

static struct dect_cc_ops dummy_cc_ops;

/*
 * MM Ops
 */

static void mm_access_rights_ind(struct dect_handle *dh,
				 struct dect_mm_endpoint *mme,
				 struct dect_mm_access_rights_param *param)
{
}

static void mm_access_rights_cfm(struct dect_handle *dh,
				 struct dect_mm_endpoint *mme, bool accept,
				 struct dect_mm_access_rights_param *param)
{
}

static void mm_access_rights_terminate_ind(struct dect_handle *dh,
					   struct dect_mm_endpoint *mme,
					   struct dect_mm_access_rights_terminate_param *param)
{
}

static void mm_access_rights_terminate_cfm(struct dect_handle *dh,
					   struct dect_mm_endpoint *mme, bool accept,
					   struct dect_mm_access_rights_terminate_param *param)
{
}

static void mm_key_allocate_ind(struct dect_handle *dh,
				struct dect_mm_endpoint *mme,
				struct dect_mm_key_allocate_param *param)
{
}

static void mm_authenticate_ind(struct dect_handle *dh,
				struct dect_mm_endpoint *mme,
				struct dect_mm_authenticate_param *param)
{
}

static void mm_authenticate_cfm(struct dect_handle *dh,
				struct dect_mm_endpoint *mme, bool accept,
				struct dect_mm_authenticate_param *param)
{
}

static void mm_cipher_ind(struct dect_handle *dh,
			  struct dect_mm_endpoint *mme,
			  struct dect_mm_cipher_param *param)
{
}

static void mm_cipher_cfm(struct dect_handle *dh,
			  struct dect_mm_endpoint *mme, bool accept,
			  struct dect_mm_cipher_param *param)
{
}

static void mm_locate_ind(struct dect_handle *dh,
			  struct dect_mm_endpoint *mme,
			  struct dect_mm_locate_param *param)
{
}

static void mm_locate_cfm(struct dect_handle *dh,
			  struct dect_mm_endpoint *mme, bool accept,
			  struct dect_mm_locate_param *param)
{
}

static void mm_detach_ind(struct dect_handle *dh,
			  struct dect_mm_endpoint *mme,
			  struct dect_mm_detach_param *param)
{
}

static void mm_identity_ind(struct dect_handle *dh,
			    struct dect_mm_endpoint *mme,
			    struct dect_mm_identity_param *param)
{
}

static void mm_identity_cfm(struct dect_handle *dh,
			    struct dect_mm_endpoint *mme, bool accept,
			    struct dect_mm_identity_param *param)
{
}

static void mm_identity_assign_ind(struct dect_handle *dh,
				   struct dect_mm_endpoint *mme,
				   struct dect_mm_identity_assign_param *param)
{
}

static void mm_identity_assign_cfm(struct dect_handle *dh,
				   struct dect_mm_endpoint *mme, bool accept,
				   struct dect_mm_identity_assign_param *param)
{
}

static void mm_info_ind(struct dect_handle *dh,
			struct dect_mm_endpoint *mme,
			struct dect_mm_info_param *param)
{
}

static void mm_info_cfm(struct dect_handle *dh,
			struct dect_mm_endpoint *mme, bool accept,
			struct dect_mm_info_param *param)
{
}

static void mm_iwu_ind(struct dect_handle *dh,
		       struct dect_mm_endpoint *mme,
		       struct dect_mm_iwu_param *param)
{
}

static struct dect_mm_ops dummy_mm_ops;

void dummy_ops_init(struct dect_ops *ops)
{
	struct dect_cc_ops *cc_ops;
	struct dect_mm_ops *mm_ops;

	if (!ops->cc_ops)
		ops->cc_ops = &dummy_cc_ops;
	cc_ops = (void *)ops->cc_ops;

	if (!cc_ops->mncc_setup_ind)
		cc_ops->mncc_setup_ind = mncc_setup_ind;
	if (!cc_ops->mncc_setup_ack_ind)
		cc_ops->mncc_setup_ack_ind = mncc_setup_ack_ind;
	if (!cc_ops->mncc_reject_ind)
		cc_ops->mncc_reject_ind = mncc_reject_ind;
	if (!cc_ops->mncc_call_proc_ind)
		cc_ops->mncc_call_proc_ind = mncc_call_proc_ind;
	if (!cc_ops->mncc_alert_ind)
		cc_ops->mncc_alert_ind = mncc_alert_ind;
	if (!cc_ops->mncc_connect_ind)
		cc_ops->mncc_connect_ind = mncc_connect_ind;
	if (!cc_ops->mncc_connect_cfm)
		cc_ops->mncc_connect_cfm = mncc_connect_cfm;
	if (!cc_ops->mncc_release_ind)
		cc_ops->mncc_release_ind = mncc_release_ind;
	if (!cc_ops->mncc_release_cfm)
		cc_ops->mncc_release_cfm = mncc_release_cfm;
	if (!cc_ops->mncc_facility_ind)
		cc_ops->mncc_facility_ind = mncc_facility_ind;
	if (!cc_ops->mncc_info_ind)
		cc_ops->mncc_info_ind = mncc_info_ind;
	if (!cc_ops->mncc_modify_ind)
		cc_ops->mncc_modify_ind = mncc_modify_ind;
	if (!cc_ops->mncc_modify_cfm)
		cc_ops->mncc_modify_cfm = mncc_modify_cfm;
	if (!cc_ops->mncc_hold_ind)
		cc_ops->mncc_hold_ind = mncc_hold_ind;
	if (!cc_ops->mncc_hold_cfm)
		cc_ops->mncc_hold_cfm = mncc_hold_cfm;
	if (!cc_ops->mncc_retrieve_ind)
		cc_ops->mncc_retrieve_ind = mncc_retrieve_ind;
	if (!cc_ops->mncc_retrieve_cfm)
		cc_ops->mncc_retrieve_cfm = mncc_retrieve_cfm;
	if (!cc_ops->mncc_iwu_info_ind)
		cc_ops->mncc_iwu_info_ind = mncc_iwu_info_ind;
	if (!cc_ops->dl_u_data_ind)
		cc_ops->dl_u_data_ind = dl_u_data_ind;

	if (!ops->mm_ops)
		ops->mm_ops = &dummy_mm_ops;
	mm_ops = (void *)ops->mm_ops;

	if (!mm_ops->mm_access_rights_ind)
		mm_ops->mm_access_rights_ind = mm_access_rights_ind;
	if (!mm_ops->mm_access_rights_cfm)
		mm_ops->mm_access_rights_cfm = mm_access_rights_cfm;
	if (!mm_ops->mm_access_rights_terminate_ind)
		mm_ops->mm_access_rights_terminate_ind = mm_access_rights_terminate_ind;
	if (!mm_ops->mm_access_rights_terminate_cfm)
		mm_ops->mm_access_rights_terminate_cfm = mm_access_rights_terminate_cfm;
	if (!mm_ops->mm_key_allocate_ind)
		mm_ops->mm_key_allocate_ind = mm_key_allocate_ind;
	if (!mm_ops->mm_authenticate_ind)
		mm_ops->mm_authenticate_ind = mm_authenticate_ind;
	if (!mm_ops->mm_authenticate_cfm)
		mm_ops->mm_authenticate_cfm = mm_authenticate_cfm;
	if (!mm_ops->mm_cipher_ind)
		mm_ops->mm_cipher_ind = mm_cipher_ind;
	if (!mm_ops->mm_cipher_cfm)
		mm_ops->mm_cipher_cfm = mm_cipher_cfm;
	if (!mm_ops->mm_locate_ind)
		mm_ops->mm_locate_ind = mm_locate_ind;
	if (!mm_ops->mm_locate_cfm)
		mm_ops->mm_locate_cfm = mm_locate_cfm;
	if (!mm_ops->mm_authenticate_ind)
		mm_ops->mm_authenticate_ind = mm_authenticate_ind;
	if (!mm_ops->mm_authenticate_cfm)
		mm_ops->mm_authenticate_cfm = mm_authenticate_cfm;
	if (!mm_ops->mm_cipher_ind)
		mm_ops->mm_cipher_ind = mm_cipher_ind;
	if (!mm_ops->mm_cipher_cfm)
		mm_ops->mm_cipher_cfm = mm_cipher_cfm;
	if (!mm_ops->mm_locate_ind)
		mm_ops->mm_locate_ind = mm_locate_ind;
	if (!mm_ops->mm_locate_cfm)
		mm_ops->mm_locate_cfm = mm_locate_cfm;
	if (!mm_ops->mm_detach_ind)
		mm_ops->mm_detach_ind = mm_detach_ind;
	if (!mm_ops->mm_identity_ind)
		mm_ops->mm_identity_ind = mm_identity_ind;
	if (!mm_ops->mm_identity_cfm)
		mm_ops->mm_identity_cfm = mm_identity_cfm;
	if (!mm_ops->mm_identity_assign_ind)
		mm_ops->mm_identity_assign_ind = mm_identity_assign_ind;
	if (!mm_ops->mm_identity_assign_cfm)
		mm_ops->mm_identity_assign_cfm = mm_identity_assign_cfm;
	if (!mm_ops->mm_info_ind)
		mm_ops->mm_info_ind = mm_info_ind;
	if (!mm_ops->mm_info_cfm)
		mm_ops->mm_info_cfm = mm_info_cfm;
	if (!mm_ops->mm_iwu_ind)
		mm_ops->mm_iwu_ind = mm_iwu_ind;
}
