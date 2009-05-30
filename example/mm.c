#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dect/libdect.h>
#include "common.h"

static uint8_t prefix[3] = { 1, 0, 0};
static uint16_t num;

static void mm_access_rights_ind(struct dect_handle *dh,
				 struct dect_mm_transaction *mmta,
				 struct dect_mm_access_rights_param *param)
{
	printf("MM_ACCESS_RIGHTS-ind\n");
	dect_mm_access_rights_res(dh, mmta, true, param);
}

static void mm_locate_ind(struct dect_handle *dh,
			  struct dect_mm_transaction *mmta,
			  struct dect_mm_locate_param *param)
{
	struct dect_ie_portable_identity portable_identity;
	struct dect_ie_duration duration;
	struct dect_mm_locate_param reply = {
		.location_area		= param->location_area,
		.portable_identity	= &portable_identity,
		.duration		= &duration,
	};

	printf("MM_LOCATE-ind\n");

	dect_ie_init(&portable_identity);
	portable_identity.type = DECT_PORTABLE_ID_TYPE_TPUI;
	portable_identity.tpui.type = DECT_TPUI_INDIVIDUAL_ASSIGNED;
	memcpy(&portable_identity.tpui.ia.digits, prefix, sizeof(prefix));
	portable_identity.tpui.ia.digits[3] = num / 10;
	portable_identity.tpui.ia.digits[4] = num % 10;
	num++;

	dect_ie_init(&duration);
	duration.lock = DECT_LOCK_TEMPORARY_USER_LIMIT_1;
	duration.time = DECT_TIME_LIMIT_DEFINED_TIME_LIMIT_1;
	duration.duration = 1;

	dect_mm_locate_res(dh, mmta, &reply);
}

static void mm_identity_assign_cfm(struct dect_handle *dh,
				   struct dect_mm_transaction *mmta, bool accept,
				   struct dect_mm_identity_assign_param *param)
{
	printf("MM_IDENTITY_ASSIGN-cfm\n");
}

static const struct dect_mm_ops mm_ops = {
	.mm_access_rights_ind	= mm_access_rights_ind,
	.mm_locate_ind		= mm_locate_ind,
	.mm_identity_assign_cfm	= mm_identity_assign_cfm,
};

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

	dect_event_loop();
	dect_close_handle(dh);
	dect_event_ops_cleanup();
	return 0;
}
