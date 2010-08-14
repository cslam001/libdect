/*
 * DECT List Access (LiA)
 *
 * Copyright (c) 2010 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_DECT_LIA_H
#define _LIBDECT_DECT_LIA_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup cc
 * @{
 * @defgroup lia List Access (LiA)
 * @{
 */

/**
 * LiA List Identifiers specified in ETSI TS 102 527-3 section 7.4.10.3.
 */
enum dect_lia_lists {
	DECT_LIA_LIST_SUPPORTED_LISTS		= 0x0, /**< List of supported lists */
	DECT_LIA_LIST_MISSED_CALLS		= 0x1, /**< Missed calls list */
	DECT_LIA_LIST_OUTGOING_CALLS		= 0x2, /**< Outgoing calls list */
	DECT_LIA_LIST_INCOMING_ACCEPTED_CALLS	= 0x3, /**< Incoming accepted calls list */
	DECT_LIA_LIST_ALL_CALLS			= 0x4, /**< All calls list */
	DECT_LIA_LIST_CONTACTS			= 0x5, /**< Contact list */
	DECT_LIA_LIST_INTERNAL_NAMES		= 0x6, /**< Internal names list */
	DECT_LIA_LIST_DECT_SYSTEM_SETTINGS	= 0x7, /**< DECT system settings list */
	DECT_LIA_LIST_LINE_SETTINGS		= 0x8, /**< Line settings list */
};

/**
 * LiA List Access Commands specified in ETSI TS 102 527-3 section 7.4.10.4.
 */
enum dect_lia_commands {
	DECT_LIA_CMD_START_SESSION		= 0x00, /**< start session */
	DECT_LIA_CMD_START_SESSION_CONFIRM	= 0x01, /**< start session confirm */
	DECT_LIA_CMD_END_SESSION		= 0x02, /**< end session */
	DECT_LIA_CMD_END_SESSION_CONFIRM	= 0x03, /**< end session confirm */
	DECT_LIA_CMD_QUERY_ENTRY_FIELDS		= 0x04, /**< query entry fields */
	DECT_LIA_CMD_QUERY_ENTRY_FIELDS_CONFIRM	= 0x05, /**< query entry fields confirm */
	DECT_LIA_CMD_READ_ENTRIES		= 0x06, /**< read entries */
	DECT_LIA_CMD_READ_ENTRIES_CONFIRM	= 0x07, /**< read entries confirm */
	DECT_LIA_CMD_EDIT_ENTRY			= 0x08, /**< edit entry */
	DECT_LIA_CMD_EDIT_ENTRY_CONFIRM		= 0x09, /**< edit entry confirm */
	DECT_LIA_CMD_SAVE_ENTRY			= 0x0a, /**< save entry */
	DECT_LIA_CMD_SAVE_ENTRY_CONFIRM		= 0x0b, /**< save entry confirm */
	DECT_LIA_CMD_DELETE_ENTRY		= 0x0c, /**< delete entry */
	DECT_LIA_CMD_DELETE_ENTRY_CONFIRM	= 0x0d, /**< delete entry confirm */
	DECT_LIA_CMD_DELETE_LIST		= 0x0e, /**< delete list */
	DECT_LIA_CMD_DELETE_LIST_CONFIRM	= 0x0f, /**< delete list confirm */
	DECT_LIA_CMD_SEARCH_ENTRIES		= 0x10, /**< search entries */
	DECT_LIA_CMD_SEARCH_ENTRIES_CONFIRM	= 0x11, /**< search entries confirm */
	DECT_LIA_CMD_NEGATIVE_ACKNOWLEDGEMENT	= 0x12, /**< negative acknowledgement */
	DECT_LIA_CMD_DATA_PACKET		= 0x13, /**< data packet */
	DECT_LIA_CMD_DATA_PACKET_LAST		= 0x14, /**< data packet last */
};

/** @} */
/** @} */

#ifdef __cplusplus
}
#endif
#endif /* _LIBDECT_DECT_LIA_H */
