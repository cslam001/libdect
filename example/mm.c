#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dect/libdect.h>
#include "common.h"

enum phones { PHONE1, PHONE2, PHONE3, };
static const struct dect_ipui ipuis[] = {
	[PHONE1] = {
		.put		= DECT_IPUI_N,
		.pun.n.ipei = {
			.emc	= 0x08ae,
			.psn	= 0x83d1e,
		},
	},
	[PHONE2] = {
		.put		= DECT_IPUI_N,
		.pun.n.ipei = {
			.emc	= 0x08ae,
			.psn	= 0x8969f,
		},
	},
	[PHONE3] = {
		.put		= DECT_IPUI_N,
		.pun.n.ipei = {
			.emc	= 0x08ae,
			.psn	= 0x5b9a0,
		},
	},
};

static const struct dect_mm_ops mm_ops = {
	.mm_access_rights_ind	= 0,
	.mm_access_rights_cfm	= 0,
};

static int mm_access_rights_request(struct dect_handle *dh)
{
	struct dect_ie_portable_identity portable_identity;
	struct dect_mm_access_rights_param param = {
		.portable_identity	= &portable_identity,
	};

	dect_ie_init(&portable_identity);
	portable_identity.type = ID_TYPE_IPUI;
	portable_identity.ipui = ipuis[PHONE1];

	return dect_mm_access_rights_req(dh, &param);
}

static struct dect_ops ops = {
	.mm_ops			= &mm_ops,
};

int main(int argc, char **argv)
{
	if (dect_event_ops_init(&ops) < 0)
		exit(1);

	dh = dect_alloc_handle(&ops);
	if (dh == NULL)
		exit(1);

	if (dect_init(dh) < 0)
		exit(1);

	if (mm_access_rights_request(dh) < 0)
		exit(1);

	dect_event_loop();
	dect_close_handle(dh);
	dect_event_ops_cleanup();
	return 0;
}
