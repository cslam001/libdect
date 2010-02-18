/*
 * libdect public API functions
 *
 * Copyright (c) 2009 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

#include <libdect.h>
#include <netlink.h>
#include <utils.h>
#include <lce.h>

static int __fmtstring(1, 0) (*debug_hook)(const char *fmt, va_list ap);

void dect_set_debug_hook(int (*fn)(const char *fmt, va_list ap))
{
	debug_hook = fn;
}
EXPORT_SYMBOL(dect_set_debug_hook);

#ifdef DEBUG
void __fmtstring(1, 2) __dect_debug(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (debug_hook != NULL)
		debug_hook(fmt, ap);
	else
		vprintf(fmt, ap);
	va_end(ap);
}
#endif

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
	dh->ops = ops;
	init_list_head(&dh->links);
	init_list_head(&dh->mme_list);
	return dh;
}
EXPORT_SYMBOL(dect_alloc_handle);

int dect_init(struct dect_handle *dh)
{
	int err;

	err = dect_netlink_init(dh);
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

void dect_close_handle(struct dect_handle *dh)
{
	dect_lce_exit(dh);
	dect_netlink_exit(dh);
	dect_free(dh, dh);
}
EXPORT_SYMBOL(dect_close_handle);
