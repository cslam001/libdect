/*
 * DECT PP List Access (LiA) example
 *
 * Copyright (c) 2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>

#include <dect/libdect.h>
#include <dect/auth.h>
#include <dect/lia.h>
#include "common.h"

static void dect_iwu_info_req(struct dect_handle *dh, struct dect_call *call)
{
	struct dect_ie_iwu_to_iwu iwu_to_iwu;
	struct dect_mncc_iwu_info_param req = {
		.iwu_to_iwu	= &iwu_to_iwu,
	};

	iwu_to_iwu.sr		= true;
	iwu_to_iwu.pd		= DECT_IWU_TO_IWU_PD_LIST_ACCESS;
	iwu_to_iwu.len		= 3;
	iwu_to_iwu.data[0]	= DECT_LIA_CMD_START_SESSION;
	iwu_to_iwu.data[1]	= DECT_LIA_LIST_DECT_SYSTEM_SETTINGS;
	iwu_to_iwu.data[2]	= 0x0;

	dect_mncc_iwu_info_req(dh, call, &req);
}

static void dect_mncc_call_proc_ind(struct dect_handle *dh, struct dect_call *call,
				    struct dect_mncc_call_proc_param *param)
{
	dect_iwu_info_req(dh, call);
}

static void dect_mncc_reject_ind(struct dect_handle *dh, struct dect_call *call,
				 enum dect_causes cause,
				 struct dect_mncc_release_param *param)
{
	dect_event_loop_stop();
}

static void dect_mncc_release_ind(struct dect_handle *dh, struct dect_call *call,
				  struct dect_mncc_release_param *param)
{
	struct dect_mncc_release_param res = {};

	dect_mncc_release_res(dh, call, &res);
	dect_event_loop_stop();
}

static void dect_open_call(struct dect_handle *dh, const struct dect_ipui *ipui)
{
	struct dect_ie_basic_service basic_service;
	struct dect_mncc_setup_param req = {
		.basic_service	= &basic_service,
	};
	struct dect_call *call;

	call = dect_call_alloc(dh);
	if (call == NULL)
		return;

	basic_service.class   = DECT_CALL_CLASS_LIA_SERVICE_SETUP;
	basic_service.service = DECT_SERVICE_WIDEBAND_SPEECH;

	dect_mncc_setup_req(dh, call, ipui, &req);
}

static void dect_dl_establish_cfm(struct dect_handle *dh, bool success,
				  struct dect_data_link *ddl,
				  const struct dect_mac_conn_params *mcp)
{
	dect_open_call(dh, &ipui);
}

static struct dect_cc_ops cc_ops = {
	.mncc_call_proc_ind	= dect_mncc_call_proc_ind,
	.mncc_release_ind	= dect_mncc_release_ind,
	.mncc_reject_ind	= dect_mncc_reject_ind,
};

static struct dect_lce_ops lce_ops = {
	.dl_establish_cfm	= dect_dl_establish_cfm,
};

static struct dect_ops ops = {
	.lce_ops		= &lce_ops,
	.cc_ops			= &cc_ops,
};

int main(int argc, char **argv)
{
	const struct dect_fp_capabilities *fpc;
	struct dect_mac_conn_params mcp = {
		.service	= DECT_SERVICE_IN_MIN_DELAY,
                .slot       = DECT_FULL_SLOT,
	};

	dect_pp_common_options(argc, argv);
	dect_pp_common_init(&ops, cluster, &ipui);

	fpc = dect_llme_fp_capabilities(dh);
	if (!(fpc->ehlc2 & DECT_EHLC2_LIST_ACCESS_FEATURES)) {
		fprintf(stderr, "FP does not support List Access (LiA)\n");
		goto out;
	}

	dect_dl_establish_req(dh, &ipui, &mcp);
	dect_event_loop();
out:
	dect_common_cleanup(dh);
	return 0;
}
