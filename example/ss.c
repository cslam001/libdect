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
	struct dect_ie_events_notification events_notification;
	struct dect_mnss_param param = {
		.events_notification	= &events_notification,
	};

	sse = dect_ss_endpoint_alloc(dh, ipui);
	if (sse == NULL)
		return;

	events_notification.num = 2;
	events_notification.events[0].type    = DECT_EVENT_MISSED_CALL;
	events_notification.events[0].subtype = DECT_EVENT_MISSED_CALL_VOICE;
	events_notification.events[0].multiplicity = 10;

	events_notification.events[1].type    = DECT_EVENT_MESSAGE_WAITING;
	events_notification.events[1].subtype = DECT_EVENT_MESSAGE_WAITING_VOICE;
	events_notification.events[1].multiplicity = 10;

	dect_mnss_facility_req(dh, sse, &param);
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
