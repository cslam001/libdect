#ifndef _LIBDECT_COMS_H
#define _LIBDECT_COMS_H

/* COMS message types */
enum dect_coms_msg_types {
	DECT_COMS_SETUP				= 0x5,
	DECT_COMS_CONNECT			= 0x7,
	DECT_COMS_NOTIFY			= 0x8,
	DECT_COMS_RELEASE			= 0x4d,
	DECT_COMS_RELEASE_COM			= 0x5a,
	DECT_COMS_INFO				= 0x7b,
	DECT_COMS_ACK				= 0x78,
};

#endif /* _LIBDECT_COMS_H */
