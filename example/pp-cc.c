/*
 * DECT PP Call Control example
 *
 * Copyright (c) 2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dect/libdect.h>
#include <timer.h>
#include "common.h"

struct call {
	struct dect_audio_handle	*audio;
};

static void dect_mncc_reject_ind(struct dect_handle *dh, struct dect_call *call,
				 enum dect_causes cause,
				 struct dect_mncc_release_param *param)
{
	dect_event_loop_stop();
}

static void dect_mncc_release_ind(struct dect_handle *dh, struct dect_call *call,
				  struct dect_mncc_release_param *param)
{
	dect_mncc_release_res(dh, call, param);
	dect_event_loop_stop();
}

static void dect_open_call(struct dect_handle *dh, const struct dect_ipui *ipui)
{
	struct dect_ie_basic_service basic_service;
	struct dect_mncc_setup_param param = {
		.basic_service = &basic_service,
	};
	struct dect_call *call;
	struct call *priv;

	call = dect_call_alloc(dh);
	if (call == NULL)
		return;
	priv = dect_call_priv(call);

	priv->audio  = dect_audio_open();
	if (priv->audio == NULL)
		return;

	basic_service.class   = DECT_CALL_CLASS_NORMAL;
	basic_service.service = DECT_SERVICE_BASIC_SPEECH_DEFAULT;

	dect_mncc_setup_req(dh, call, ipui, &param);
}

static void dect_dl_u_data_ind(struct dect_handle *dh, struct dect_call *call,
			       struct dect_msg_buf *mb)
{
	struct call *priv = dect_call_priv(call);

	dect_dl_u_data_req(dh, call, mb);
	dect_audio_queue(priv->audio, mb);
}

static struct dect_cc_ops cc_ops = {
	.priv_size		= sizeof(struct call),
	.mncc_reject_ind	= dect_mncc_reject_ind,
	.mncc_release_ind	= dect_mncc_release_ind,
	.dl_u_data_ind		= dect_dl_u_data_ind,
};

static struct dect_ops ops = {
	.cc_ops			= &cc_ops,
};

int main(int argc, char **argv)
{
	dect_pp_common_options(argc, argv);
	dect_pp_common_init(&ops, cluster, &ipui);

	dect_open_call(dh, &ipui);

	dect_event_loop();
	dect_common_cleanup(dh);
	return 0;
}
