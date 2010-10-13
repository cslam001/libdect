#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>

#include <dect/libdect.h>
#include "common.h"

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

	sse = dect_ss_endpoint_alloc(dh, ipui);
	if (sse == NULL)
		return;

	feature_activate.feature = DECT_FEATURE_INDICATION_OF_SUBSCRIBER_NUMBER;

	dect_mnss_setup_req(dh, sse, &param);
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
	dect_common_init(&ops, argv[1]);

	dect_invoke_ss(dh, &ipui);
	dect_event_loop();

	dect_common_cleanup(dh);
	return 0;
}
