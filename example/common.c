#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <dect/libdect.h>
#include "common.h"

const struct dect_ipui ipui = {
	.put		= DECT_IPUI_N,
	.pun.n.ipei	= {
		.emc	= 0x0ba8,
		.psn	= 0xa782a,
	},
};

void pexit(const char *str)
{
	perror(str);
	exit(1);
}

void dect_common_init(struct dect_ops *ops, const char *cluster)
{
	dect_debug_init();
	dect_dummy_ops_init(ops);

	if (dect_event_ops_init(ops))
		pexit("dect_event_ops_init");

	dh = dect_open_handle(ops, cluster);
	if (dh == NULL)
		pexit("dect_init");
}

void dect_common_cleanup(struct dect_handle *dh)
{
	dect_close_handle(dh);
	dect_event_ops_cleanup();
}
