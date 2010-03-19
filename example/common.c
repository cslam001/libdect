#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <dect/libdect.h>
#include "common.h"

void pexit(const char *str)
{
	perror(str);
	exit(1);
}

void dect_common_init(struct dect_ops *ops)
{
	dummy_ops_init(ops);

	if (dect_event_ops_init(ops))
		pexit("dect_event_ops_init");

	dh = dect_alloc_handle(ops);
	if (dh == NULL)
		pexit("dect_alloc_handle");

	if (dect_init(dh) < 0)
		pexit("dect_init");
}

void dect_common_cleanup(struct dect_handle *dh)
{
	dect_close_handle(dh);
	dect_event_ops_cleanup();
}
