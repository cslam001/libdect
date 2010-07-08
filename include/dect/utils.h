#ifndef _LIBDECT_DECT_UTILS_H
#define _LIBDECT_DECT_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#define container_of(ptr, type, member) ({				\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);		\
	(type *)( (char *)__mptr - offsetof(type,member) );})

#ifdef __cplusplus
}
#endif
#endif /* _LIBDECT_DECT_UTILS_H */
