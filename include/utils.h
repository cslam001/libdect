#ifndef _LIBDECT_UTILS_H
#define _LIBDECT_UTILS_H

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#include <timer.h>

#ifndef AF_DECT
#define AF_DECT 37
#endif
#ifndef SOL_DECT
#define SOL_DECT 278
#endif

#define __init			__attribute__((constructor))
#define __exit			__attribute__((destructor))
#define __must_check		__attribute__((warn_unused_result))
#define __maybe_unused		__attribute__((unused))
#define __noreturn		__attribute__((__noreturn__))
#define __fmtstring(x, y)	__attribute__((format(printf, x, y)))
#define __aligned(x)		__attribute__((aligned(x)))
#define __packed		__attribute__((packed))
#define __visible		__attribute__((visibility("default")))

extern void __dect_debug(const char *fmt, ...) __fmtstring(1, 2);
extern void __dect_hexdump(const char *prefix, const uint8_t *buf, size_t size);

#ifdef DEBUG
#define dect_debug(fmt, ...)		__dect_debug(fmt, ## __VA_ARGS__)
#define dect_hexdump(pfx, buf, size)	__dect_hexdump(pfx, buf, size)
#else
#define dect_debug(fmt, ...)		({ if (0) __dect_debug(fmt, ## __VA_ARGS__); })
#define dect_hexdump(pfx, buf, size)	({ if (0) __dect_hexdump(pfx, buf, size); })
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

struct dect_handle;
extern void *dect_malloc(const struct dect_handle *dh, size_t size);
extern void *dect_zalloc(const struct dect_handle *dh, size_t size);
extern void dect_free(const struct dect_handle *dh, void *ptr);

extern struct dect_fd *dect_alloc_fd(const struct dect_handle *dh);
extern void dect_setup_fd(struct dect_fd *fd,
			  void (*cb)(struct dect_handle *, struct dect_fd *, uint32_t),
			  void *data);
extern void dect_close(const struct dect_handle *dh, struct dect_fd *dfd);

#include <sys/socket.h> // FIXME: socklen_t
extern struct dect_fd *dect_socket(const struct dect_handle *dh,
				   int type, int protocol);
extern struct dect_fd *dect_accept(const struct dect_handle *dh,
				   const struct dect_fd *dfd,
				   struct sockaddr *addr, socklen_t len);

extern int dect_register_fd(const struct dect_handle *dh, struct dect_fd *dfd,
			    uint32_t events);
extern void dect_unregister_fd(const struct dect_handle *dh, struct dect_fd *dfd);

#define EXPORT_SYMBOL(x)	typeof(x) (x) __visible
#define BUG()			assert(0)

/* Force a compilation error if condition is true */
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))
#define BUILD_BUG_ON_ZERO(e) (sizeof(char[1 - 2 * !!(e)]) - 1)

#define __must_be_array(a) \
	BUILD_BUG_ON_ZERO(__builtin_types_compatible_p(typeof(a), typeof(&a[0])))

#define array_size(arr)		(sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))
#define field_sizeof(t, f)	(sizeof(((t *)NULL)->f))

#define div_round_up(n, d)	(((n) + (d) - 1) / (d))

#define min(x, y) ({				\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })

#define max(x, y) ({				\
	typeof(x) _max1 = (x);			\
	typeof(y) _max2 = (y);			\
	(void) (&_max1 == &_max2);		\
	_max1 > _max2 ? _max1 : _max2; })

static inline unsigned int fls(uint64_t v)
{
	unsigned int len = 0;

	while (v) {
		v >>= 1;
		len++;
	}
	return len;
}

#endif /* _LIBDECT_UTILS_H */
