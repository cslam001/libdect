#ifndef _LIBDECT_DECT_TERMINAL
#define _LIBDECT_DECT_TERMINAL

/*
 * DECT Standard 8-bit characters
 *
 * DECT Control codes from 0x00 - 0x1f. Characters between 0x20 and 0x7f
 * are IA5 coded.
 */

/**
 * @DECT_C_CANCEL_DTMF:			Null/Cancel DTMF tone
 * @DECT_C_RETURN_HOME:			Return home
 * @DECT_C_RETURN_END:			Return end
 * @DECT_C_DIALING_PAUSE:		Dialing Pause
 * @DECT_C_MOVE_NEXT_TAB:		Move forward to next column tab position
 * @DECT_C_MOVE_PREV_TAB:		Move backward to next column tab position
 * @DECT_C_MOVE_PREV_COL:		Move backward one column
 * @DECT_C_MOVE_NEXT_COL:		Move forward one column
 * @DECT_C_MOVE_NEXT_ROW:		Move down one row
 * @DECT_C_MOVE_PREV_ROW:		Move up one row
 * @DECT_C_CLEAR_DISPLAY:		Clear display (and return home)
 * @DECT_C_RETURN:			Return (to start of current row)
 * @DECT_C_FLASH_OFF:			Flash off
 * @DECT_C_FLASH_ON:			Flash on
 * @DECT_C_XON:				XON (resume transmission)
 * @DECT_C_PULSE_DIALING:		Go to pulse dialing
 * @DECT_C_XOFF:			XOFF (stop transmission)
 * @DECT_C_DTMF_DIALING_DEFINED:	Go to DTMF dialing mode; defined tone length
 * @DECT_C_REGISTER_RECALL:		Register recall
 * @DECT_C_DTMF_DIALING_INFINITE: 	Go to DTMF dialing mode; infinite tone length
 * @DECT_C_INTERNAL_CALL:		Internal call
 * @DECT_C_SERVICE:			Service
 * @DECT_C_CLEAR_TO_END_OF_DISPLAY:	Clear to end of display (maintain cursor position)
 * @DECT_C_CLEAR_TO_END_OF_LINE:	Clear to end of line (maintain cursor position)
 * @DECT_C_ESC:				ESCape in the IA5 sense
 *
 * Flash on/off is a toggle action that applies to all following characters.
 */
enum dect_control_characters {
	DECT_C_CANCEL_DTMF		= 0x0,
	DECT_C_RETURN_HOME		= 0x2,
	DECT_C_RETURN_END		= 0x3,
	DECT_C_DIALING_PAUSE		= 0x5,
	DECT_C_MOVE_NEXT_TAB		= 0x6,
	DECT_C_MOVE_PREV_TAB		= 0x7,
	DECT_C_MOVE_PREV_COL		= 0x8,
	DECT_C_MOVE_NEXT_COL		= 0x9,
	DECT_C_MOVE_NEXT_ROW		= 0xa,
	DECT_C_MOVE_PREV_ROW		= 0xb,
	DECT_C_CLEAR_DISPLAY		= 0xc,
	DECT_C_RETURN			= 0xd,
	DECT_C_FLASH_OFF		= 0xe,
	DECT_C_FLASH_ON			= 0xf,
	DECT_C_XON			= 0x11,
	DECT_C_PULSE_DIALING		= 0x12,
	DECT_C_XOFF			= 0x13,
	DECT_C_DTMF_DIALING_DEFINED	= 0x14,
	DECT_C_REGISTER_RECALL		= 0x15,
	DECT_C_DTMF_DIALING_INFINITE	= 0x16,
	DECT_C_INTERNAL_CALL		= 0x17,
	DECT_C_SERVICE			= 0x18,
	DECT_C_CLEAR_TO_END_OF_DISPLAY	= 0x19,
	DECT_C_CLEAR_TO_END_OF_LINE	= 0x1a,
	DECT_C_ESC			= 0x1b
};

#define DECT_TABSIZE	10

#endif /* _LIBDECT_DECT_TERMINAL */
