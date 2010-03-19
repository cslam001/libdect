#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dect/libdect.h>
#include "common.h"
#include <lce.h>

static struct dect_ops ops;

int main(int argc, char **argv)
{
	dect_common_init(&ops);

	dect_lce_group_ring(dh, 0);
	dect_event_loop();

	dect_common_cleanup(dh);
	return 0;
}
