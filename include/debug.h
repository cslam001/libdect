#ifndef _LIBDECT_DEBUG_H
#define _LIBDECT_DEBUG_H

#include <utils.h>

extern void __dect_debug(enum dect_debug_subsys subsys, const char *fmt, ...) __fmtstring(2, 3);
extern void __dect_hexdump(enum dect_debug_subsys subsys, const char *prefix,
			   const uint8_t *buf, size_t size);

#ifdef DEBUG
#define dect_debug(subsys, fmt, ...) \
	__dect_debug(subsys, fmt, ## __VA_ARGS__)
#define dect_hexdump(subsys, pfx, buf, size) \
	__dect_hexdump(subsys, pfx, buf, size)
#else
#define dect_debug(subsys, fmt, ...) \
	({ if (0) __dect_debug(subsys, fmt, ## __VA_ARGS__); })
#define dect_hexdump(subsys, pfx, buf, size) \
	({ if (0) __dect_hexdump(subsys, pfx, buf, size); })
#endif

struct dect_trans_tbl {
	uint64_t	val;
	const char	*str;
};

#define TRANS_TBL(_val, _str)	{ .val = (_val), .str = (_str) }

extern const char *__dect_flags2str(const struct dect_trans_tbl *tbl, unsigned int nelem,
				    char *buf, size_t size, uint64_t val);

#define dect_val2str(trans, buf, val) \
	__dect_val2str(trans, array_size(trans), buf, sizeof(buf), val)

extern const char *__dect_val2str(const struct dect_trans_tbl *tbl, unsigned int nelem,
				  char *buf, size_t len, uint64_t val);

#define dect_flags2str(trans, buf, val) \
	__dect_flags2str(trans, array_size(trans), buf, sizeof(buf), val)

#endif /* _LIBDECT_DEBUG_H */
