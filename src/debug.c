/*
 * libdect debugging helpers
 *
 * Copyright (c) 2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <ctype.h>

#include <libdect.h>
#include <utils.h>

/**
 * @defgroup debug Debugging
 * @{
 */

static void __fmtstring(2, 0) (*debug_hook)(enum dect_debug_subsys subsys,
					    const char *fmt, va_list ap);

/**
 * Set callback hook for debugging messages
 *
 * @param fn	callback function
 */
void dect_set_debug_hook(void (*fn)(enum dect_debug_subsys subsys,
				    const char *fmt, va_list ap))
{
	debug_hook = fn;
}
EXPORT_SYMBOL(dect_set_debug_hook);

#ifdef DEBUG
void __fmtstring(2, 3) __dect_debug(enum dect_debug_subsys subsys,
				    const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (debug_hook != NULL)
		debug_hook(subsys, fmt, ap);
	else
		vprintf(fmt, ap);
	va_end(ap);
}

#define BLOCKSIZE	16

void __dect_hexdump(enum dect_debug_subsys subsys, const char *prefix,
		    const uint8_t *buf, size_t size)
{
	unsigned int i, off;
	char hbuf[3 * BLOCKSIZE + 1], abuf[BLOCKSIZE + 1];

	for (i = 0; i < size; i++) {
		off = i % BLOCKSIZE;

		sprintf(hbuf + 3 * off, "%.2x ", buf[i]);
		abuf[off] = isascii(buf[i]) && isprint(buf[i]) ? buf[i] : '.';

		if (off == BLOCKSIZE - 1 || i == size - 1) {
			abuf[off + 1] = '\0';
			dect_debug(subsys, "%s: %-48s    |%s|\n",
				   prefix, hbuf, abuf);
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

/** @} */
