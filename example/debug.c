#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>

#include <libdect.h>
#include "common.h"

#define NORMAL		"\033[0;m"
#define RED		"\033[0;31m"
#define GREEN		"\033[0;32m"
#define BROWN		"\033[0;33m"
#define BLUE		"\033[0;34m"
#define PURPLE		"\033[0;35m"
#define CYAN		"\033[0;36m"
#define LIGHT_GRAY	"\033[0;37m"
#define DARK_GRAY	"\033[1;30m"
#define LIGHT_RED	"\033[1;31m"
#define LIGHT_GREEN	"\033[1;32m"
#define YELLOW		"\033[1;33m"
#define LIGHT_BLUE	"\033[1;34m"
#define LIGHT_PURPLE	"\033[1;35m"
#define LIGHT_CYAN	"\033[1;36m"
#define WHITE		"\033[1;37m"

static const char * const debug_colors[] = {
	[DECT_DEBUG_UNKNOWN]	= NORMAL,
	[DECT_DEBUG_LCE]	= LIGHT_BLUE,
	[DECT_DEBUG_CC]		= YELLOW,
	[DECT_DEBUG_SS]		= LIGHT_CYAN,
	[DECT_DEBUG_MM]		= YELLOW,
	[DECT_DEBUG_SFMT]	= LIGHT_GREEN,
	[DECT_DEBUG_NL]		= LIGHT_PURPLE,
};

static bool tty;

static void __fmtstring(2, 0) dect_debug_fn(enum dect_debug_subsys subsys,
					    const char *fmt, va_list ap)
{
	char buf[1024];

	vsnprintf(buf, sizeof(buf), fmt, ap);
	printf("%s%s%s",
	       tty ? debug_colors[subsys] : "",
	       buf,
	       tty ? NORMAL : "");
}

void dect_debug_init(void)
{
	tty = isatty(fileno(stdout));
	dect_set_debug_hook(dect_debug_fn);
}
