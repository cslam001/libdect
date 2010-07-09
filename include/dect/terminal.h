#ifndef _LIBDECT_DECT_TERMINAL
#define _LIBDECT_DECT_TERMINAL

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup terminal Terminal
 * @{
 */

/*
 * DECT Standard 8-bit characters
 *
 * DECT Control codes from 0x00 - 0x1f. Characters between 0x20 and 0x7f
 * are IA5 coded.
 */

/**
 * DECT Control Characters specified in ETSI EN 300 175-5 D.2.2.
 *
 * Flash on/off is a toggle action that applies to all following characters.
 */
enum dect_control_characters {
	DECT_C_CANCEL_DTMF		= 0x00, /**< Null/Cancel DTMF tone */
	DECT_C_RETURN_HOME		= 0x02, /**< Return home */
	DECT_C_RETURN_END		= 0x03, /**< Return end */
	DECT_C_DIALING_PAUSE		= 0x05, /**< Dialing Pause */
	DECT_C_MOVE_NEXT_TAB		= 0x06, /**< Move forward to next column tab position */
	DECT_C_MOVE_PREV_TAB		= 0x07, /**< Move backward to next column tab position */
	DECT_C_MOVE_PREV_COL		= 0x08, /**< Move backward one column */
	DECT_C_MOVE_NEXT_COL		= 0x09, /**< Move forward one column */
	DECT_C_MOVE_NEXT_ROW		= 0x0a, /**< Move down one row */
	DECT_C_MOVE_PREV_ROW		= 0x0b, /**< Move up one row */
	DECT_C_CLEAR_DISPLAY		= 0x0c, /**< Clear display (and return home) */
	DECT_C_RETURN			= 0x0d, /**< Return (to start of current row) */
	DECT_C_FLASH_OFF		= 0x0e, /**< Flash off */
	DECT_C_FLASH_ON			= 0x0f, /**< Flash on */
	DECT_C_XON			= 0x11, /**< XON (resume transmission) */
	DECT_C_PULSE_DIALING		= 0x12, /**< Go to pulse dialing */
	DECT_C_XOFF			= 0x13, /**< XOFF (stop transmission) */
	DECT_C_DTMF_DIALING_DEFINED	= 0x14, /**< Go to DTMF dialing mode; defined tone length */
	DECT_C_REGISTER_RECALL		= 0x15, /**< Register recall */
	DECT_C_DTMF_DIALING_INFINITE	= 0x16, /**< Go to DTMF dialing mode; infinite tone length */
	DECT_C_INTERNAL_CALL		= 0x17, /**< Internal call */
	DECT_C_SERVICE			= 0x18, /**< Service */
	DECT_C_CLEAR_TO_END_OF_DISPLAY	= 0x19, /**< Clear to end of display (maintain cursor position) */
	DECT_C_CLEAR_TO_END_OF_LINE	= 0x1a, /**< Clear to end of line (maintain cursor position) */
	DECT_C_ESC			= 0x1b, /**< ESCape in the IA5 sense */
	DECT_C_SUPPLEMENTARY_SERVICE	= 0x1c, /**< Supplementary servce */
};

#define DECT_TABSIZE	10

/** @} */

#ifdef __cplusplus
}
#endif
#endif /* _LIBDECT_DECT_TERMINAL */
