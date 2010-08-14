/*
 * libdect public API functions
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/**
 * @defgroup init Initialization
 * @{
 */

#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <libdect.h>
#include <netlink.h>
#include <utils.h>
#include <lce.h>

/**
 * Allocate a new libdect DECT handle
 *
 * @param ops		DECT ops
 */
struct dect_handle *dect_alloc_handle(struct dect_ops *ops)
{
	struct dect_handle *dh;

	if (ops->malloc == NULL)
		ops->malloc = malloc;
	if (ops->free == NULL)
		ops->free = free;

	dh = ops->malloc(sizeof(*dh));
	if (dh == NULL)
		return NULL;
	memset(dh, 0, sizeof(*dh));

	dh->ops = ops;
	init_list_head(&dh->links);
	init_list_head(&dh->mme_list);
	return dh;
}
EXPORT_SYMBOL(dect_alloc_handle);

/**
 * Initialize the libdect subsystems and bind to a cluster
 *
 * @param dh		libdect DECT handle
 * @param cluster	Cluster name
 */
int dect_init(struct dect_handle *dh, const char *cluster)
{
	int err;

	if (cluster == NULL)
		cluster = "cluster0";

	srand(time(NULL));

	err = dect_netlink_init(dh, cluster);
	if (err < 0)
		goto err1;

	err = dect_lce_init(dh);
	if (err < 0)
		goto err2;
	return 0;

err2:
	dect_netlink_exit(dh);
err1:
	return err;
}
EXPORT_SYMBOL(dect_init);

/**
 * Unbind from a cluster and release the libdect DECT handle
 *
 * @param dh		libdect DECT handle
 */
void dect_close_handle(struct dect_handle *dh)
{
	dect_lce_exit(dh);
	dect_netlink_exit(dh);
	dect_free(dh, dh);
}
EXPORT_SYMBOL(dect_close_handle);

/** @} */
