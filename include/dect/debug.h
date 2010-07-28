#ifndef _LIBDECT_DECT_DEBUG_H
#define _LIBDECT_DECT_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <dect/utils.h>

/**
 * @addtogroup debug
 * @{
 */

/**
 * Debugging subsystems
 */
enum dect_debug_subsys {
	DECT_DEBUG_UNKNOWN,	/**< Unknown */
	DECT_DEBUG_LCE,		/**< Link Control Entity */
	DECT_DEBUG_CC,		/**< Call Control */
	DECT_DEBUG_SS,		/**< Supplementary Services */
	DECT_DEBUG_MM,		/**< Mobility Management */
	DECT_DEBUG_SFMT,	/**< S-Format message parsing/construction */
	DECT_DEBUG_NL,		/**< Netlink communication */
};

extern void dect_set_debug_hook(void (*fn)(enum dect_debug_subsys subsys,
					   const char *fmt, va_list ap)
				__fmtstring(2, 0));

/** @} */

#ifdef __cplusplus
}
#endif
#endif /* _LIBDECT_DECT_DEBUG_H */
