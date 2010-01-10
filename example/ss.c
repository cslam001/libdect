#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <event.h>

#include <dect/libdect.h>
#include "common.h"

static struct dect_ipui ipui = {
	.put		= DECT_IPUI_N,
	.pun.n.ipei = {
		.emc	= 0x0ba8,
		.psn	= 0xa782a,
	},
};

static void mnss_setup_ind(struct dect_handle *dh, struct dect_ss_endpoint *sse,
			   struct dect_mnss_param *param)
{

}

static void mnss_facility_ind(struct dect_handle *dh, struct dect_ss_endpoint *sse,
			      struct dect_mnss_param *param)
{

}

static void mnss_release_ind(struct dect_handle *dh, struct dect_ss_endpoint *sse,
			     struct dect_mnss_param *param)

{

}

static void dect_invoke_ss(struct dect_handle *dh, const struct dect_ipui *ipui)
{
	struct dect_ss_endpoint *sse;
	struct dect_ie_feature_activate feature_activate;
	struct dect_mnss_param param = {
		.feature_activate	= &feature_activate,
	};

	sse = dect_ss_endpoint_alloc(dh);
	if (sse == NULL)
		return;

	feature_activate.feature = DECT_FEATURE_INDICATION_OF_SUBSCRIBER_NUMBER;

	dect_mnss_setup_req(dh, sse, ipui, &param);
}

static struct dect_ss_ops ss_ops = {
	.mnss_setup_ind		= mnss_setup_ind,
	.mnss_facility_ind	= mnss_facility_ind,
	.mnss_release_ind	= mnss_release_ind,
};

static struct dect_ops ops = {
	.ss_ops			= &ss_ops,
};

int main(int argc, char **argv)
{
	dummy_ops_init(&ops);

	if (dect_event_ops_init(&ops) < 0)
		exit(1);

	dh = dect_alloc_handle(&ops);
	if (dh == NULL)
		exit(1);

	if (dect_init(dh) < 0)
		exit(1);

	dect_invoke_ss(dh, &ipui);

	dect_event_loop();
	dect_close_handle(dh);
	dect_event_ops_cleanup();
	return 0;
}
