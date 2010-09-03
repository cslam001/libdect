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

static const char *name = "- FP 1 -";

#define debug(fmt, args...)	printf("IWU: " fmt, ## args);

static void dect_invoke_clms(struct dect_handle *dh)
{
	DECT_DEFINE_MSG_BUF_ONSTACK(_mb), *mb = &_mb;
	struct dect_ie_network_parameter network_parameter;

	network_parameter.discriminator = DECT_NETWORK_PARAMETER_DEVICE_NAME;
	network_parameter.len		= strlen(name);
	memcpy(network_parameter.data, name, network_parameter.len);

	debug("build CLMS message\n");
	if (dect_build_sfmt_ie(dh, DECT_IE_NETWORK_PARAMETER, mb,
			       &network_parameter.common) < 0)
		return;

	dect_mncl_unitdata_req(dh, DECT_CLMS_FIXED, NULL, mb);
}

static struct dect_ops ops;

int main(int argc, char **argv)
{
	dect_common_init(&ops, argv[1]);

	dect_invoke_clms(dh);

	dect_common_cleanup(dh);
	return 0;
}
