#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dect/libdect.h>
#include "common.h"
#include <lce.h>

static struct dect_ops ops;

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

	dect_lce_group_ring(dh, 0);
	dect_event_loop();

	dect_close_handle(dh);
	dect_event_ops_cleanup();
	return 0;
}
