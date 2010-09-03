#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <dect/libdect.h>
#include "common.h"

const char *cluster;
struct dect_ipui ipui = {
	.put		= DECT_IPUI_N,
	.pun.n.ipei	= {
		.emc	= 0x0ba8,
		.psn	= 0xa782a,
	},
};

int dect_parse_ipui(struct dect_ipui *ipui, const char *optarg)
{
	ipui->put = DECT_IPUI_N;
	if (sscanf(optarg, "%04hx%05x",
		   &ipui->pun.n.ipei.emc,
		   &ipui->pun.n.ipei.psn) != 2) {
		fprintf(stderr, "could not parse IPUI '%s'\n", optarg);
		return -1;
	}
	return 0;
}

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
