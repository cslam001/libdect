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
 * LiA List Identifiers
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
 * LiA List Access Commands
 */
enum dect_lia_commands {
	DECT_LIA_CMD_START_SESSION		= 0x00,
	DECT_LIA_CMD_START_SESSION_CONFIRM	= 0x01,
	DECT_LIA_CMD_END_SESSION		= 0x02,
	DECT_LIA_CMD_END_SESSION_CONFIRM	= 0x03,
	DECT_LIA_CMD_QUERY_ENTRY_FIELDS		= 0x04,
	DECT_LIA_CMD_QUERY_ENTRY_FIELDS_CONFIRM	= 0x05,
	DECT_LIA_CMD_READ_ENTRIES		= 0x06,
	DECT_LIA_CMD_READ_ENTRIES_CONFIRM	= 0x07,
	DECT_LIA_CMD_EDIT_ENTRY			= 0x08,
	DECT_LIA_CMD_EDIT_ENTRY_CONFIRM		= 0x09,
	DECT_LIA_CMD_SAVE_ENTRY			= 0x0a,
	DECT_LIA_CMD_SAVE_ENTRY_CONFIRM		= 0x0b,
	DECT_LIA_CMD_DELETE_ENTRY		= 0x0c,
	DECT_LIA_CMD_DELETE_ENTRY_CONFIRM	= 0x0d,
	DECT_LIA_CMD_DELETE_LIST		= 0x0e,
	DECT_LIA_CMD_DELETE_LIST_CONFIRM	= 0x0f,
	DECT_LIA_CMD_SEARCH_ENTRIES		= 0x10,
	DECT_LIA_CMD_SEARCH_ENTRIES_CONFIRM	= 0x11,
	DECT_LIA_CMD_NEGATIVE_ACKNOWLEDGEMENT	= 0x12,
	DECT_LIA_CMD_DATA_PACKET		= 0x13,
	DECT_LIA_CMD_DATA_PACKET_LAST		= 0x14,
};

/** @} */
/** @} */

#ifdef __cplusplus
}
#endif
#endif /* _LIBDECT_DECT_LIA_H */
