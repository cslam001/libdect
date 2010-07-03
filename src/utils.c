/*
 * libdect utility functions
 *
 * Copyright (c) 2009 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include <libdect.h>
#include <utils.h>

#define BLOCKSIZE	16

#ifdef DEBUG
void __dect_hexdump(const char *prefix, const uint8_t *buf, size_t size)
{
	unsigned int i, off;
	char hbuf[3 * BLOCKSIZE + 1], abuf[BLOCKSIZE + 1];

	for (i = 0; i < size; i++) {
		off = i % BLOCKSIZE;

		sprintf(hbuf + 3 * off, "%.2x ", buf[i]);
		abuf[off] = isascii(buf[i]) && isprint(buf[i]) ? buf[i] : '.';

		if (off == BLOCKSIZE - 1 || i == size - 1) {
			abuf[off + 1] = '\0';
			dect_debug("%s: %-48s    |%s|\n", prefix, hbuf, abuf);
		}
	}
}

const char *__dect_flags2str(const struct dect_trans_tbl *tbl, unsigned int nelem,
			     char *buf, size_t size, uint64_t val)
{
	char tmp[sizeof("unknown (0xffffffffffffffff)")];
	const char *delim = NULL;
	uint64_t flags = val;
	unsigned int i;

	buf[0] = '\0';
	for (i = 0; i < nelem && flags ; i++) {
		if (tbl[i].val & flags) {
			flags &= ~tbl[i].val;
			if (delim)
				strncat(buf, delim, size - strlen(buf) - 1);
			strncat(buf, tbl[i].str, size - strlen(buf) - 1);
			delim = ",";
		}
	}

	if (flags) {
		snprintf(tmp, size - strlen(buf), "unknown (%" PRIx64 ")", flags);
		if (delim)
			strncat(tmp, delim, size - strlen(buf) - 1);
		strncat(buf, tmp, size - strlen(buf) - 1);
	}

	snprintf(tmp, size - strlen(buf), " (%" PRIx64 ")", val);
	strncat(buf, tmp, size - strlen(buf) - 1);

	return buf;
}

const char *__dect_val2str(const struct dect_trans_tbl *tbl, unsigned int nelem,
			   char *buf, size_t size, uint64_t val)
{
	unsigned int i;

	for (i = 0; i < nelem; i++) {
		if (tbl[i].val == val) {
			snprintf(buf, size, "%s (%" PRIx64 ")", tbl[i].str, val);
			return buf;
		}
	}

	snprintf(buf, size, "unknown (%" PRIx64 ")", val);
	return buf;
}
#endif

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
