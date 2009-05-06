#ifndef _UITLS_H
#define _UTILS_H

#include <assert.h>

#ifndef AF_DECT
#define AF_DECT 36
#endif

#define __init			__attribute__((constructor))
#define __exit			__attribute__((destructor))
#define __must_check		__attribute__((warn_unused_result))
#define __noreturn		__attribute__((__noreturn__))
#define __fmtstring(x, y)	__attribute__((format(printf, x, y)))
#define __aligned(x)		__attribute__((aligned(x)))
#define __packed		__attribute__((packed))

extern void *dect_malloc(const struct dect_handle *dh, size_t size);
extern void *dect_zalloc(const struct dect_handle *dh, size_t size);
extern void dect_free(const struct dect_handle *dh, void *ptr);

extern struct dect_timer *dect_alloc_timer(const struct dect_handle *dh);
extern void dect_start_timer(const struct dect_handle *dh,
			     struct dect_timer *timer, unsigned int timeout);
extern void dect_stop_timer(const struct dect_handle *dh, struct dect_timer *timer);

struct dect_fd *dect_alloc_fd(const struct dect_handle *dh);
extern void dect_close(const struct dect_handle *dh, struct dect_fd *dfd);

extern void dect_debug(const char *fmt, ...) __fmtstring(1, 2);
extern void dect_hexdump(const char *prefix, const uint8_t *buf, size_t size);

#include <sys/socket.h> // FIXME: socklen_t
extern struct dect_fd *dect_socket(const struct dect_handle *dh,
				   int type, int protocol);
extern struct dect_fd *dect_accept(const struct dect_handle *dh,
				   const struct dect_fd *dfd,
				   struct sockaddr *addr, socklen_t len);

extern int dect_register_fd(const struct dect_handle *dh, struct dect_fd *dfd,
			    uint32_t events);
extern void dect_unregister_fd(const struct dect_handle *dh, struct dect_fd *dfd);

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

#endif /* _UTILS_H */
