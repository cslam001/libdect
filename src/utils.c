/*
 * libdect utility functions
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include <libdect.h>
#include <utils.h>

void *dect_malloc(const struct dect_handle *dh, size_t size)
{
	return dh->ops->malloc(size);
}

void *dect_zalloc(const struct dect_handle *dh, size_t size)
{
	void *ptr;

	ptr = dect_malloc(dh, size);
	if (ptr != NULL)
		memset(ptr, 0, size);
	return ptr;
}

void dect_free(const struct dect_handle *dh, void *ptr)
{
	dh->ops->free(ptr);
}
