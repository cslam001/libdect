#ifndef _LIBDECT_UTILS_H
#define _LIBDECT_UTILS_H

#include <assert.h>

#ifndef AF_DECT
#define AF_DECT 38
#endif
#ifndef SOL_DECT
#define SOL_DECT 279
#endif
#ifndef NETLINK_DECT
#define NETLINK_DECT 20
#endif

#define __init			__attribute__((constructor))
#define __exit			__attribute__((destructor))
#define __must_check		__attribute__((warn_unused_result))
#define __maybe_unused		__attribute__((unused))
#define __noreturn		__attribute__((__noreturn__))
#define __aligned(x)		__attribute__((aligned(x)))
#define __packed		__attribute__((packed))
#define __visible		__attribute__((visibility("default")))

struct dect_handle;
extern void *dect_malloc(const struct dect_handle *dh, size_t size);
extern void *dect_zalloc(const struct dect_handle *dh, size_t size);
extern void dect_free(const struct dect_handle *dh, void *ptr);
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
