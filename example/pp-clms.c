/*
 * DECT FP CLMS example
 *
 * Copyright (c) 2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>

#include <dect/libdect.h>
#include "common.h"

#define debug(fmt, args...)	printf("IWU: " fmt, ## args);

static void dect_mncl_unitdata_ind(struct dect_handle *dh,
				   enum dect_clms_message_types type,
				   struct dect_mncl_unitdata_param *param,
				   struct dect_msg_buf *mb)
{
	struct dect_ie_network_parameter *network_parameter;
	struct dect_ie_common *dst;
	struct dect_sfmt_ie ie;

	debug("parse CLMS message\n");
	if (dect_parse_sfmt_ie_header(&ie, mb) < 0)
		return;
	if (ie.id != DECT_IE_NETWORK_PARAMETER)
		return;
	if (dect_parse_sfmt_ie(dh, 0, &dst, &ie) < 0)
		return;

	network_parameter = dect_ie_container(network_parameter, dst);
	debug("FP-Name: '%.*s'\n",
	      network_parameter->len, network_parameter->data);

	dect_ie_put(dh, network_parameter);
	dect_event_loop_stop();
}

static struct dect_clms_ops clms_ops = {
	.mncl_unitdata_ind	= dect_mncl_unitdata_ind,
};

static struct dect_ops ops = {
	.clms_ops		= &clms_ops,
};

int main(int argc, char **argv)
{
	dect_common_init(&ops, argv[1]);

	dect_event_loop();

	dect_common_cleanup(dh);
	return 0;
}
