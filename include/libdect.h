#ifndef _LIBDECT_H
#define _LIBDECT_H

#include <linux/dect_netlink.h>
#include <dect/libdect.h>
#include <list.h>

struct dect_fp_capabilities {
	uint32_t		fpc;
	uint16_t		hlc;
	uint16_t		efpc;
	uint32_t		ehlc;
	uint16_t		efpc2;
	uint32_t		ehlc2;
};

/**
 * struct dect_handle - libdect handle
 *
 * @ops:	user ops
 * @nlsock:	netlink socket
 * @nlfd:	netlink file descriptor
 * @index:	cluster index
 * @mode:	cluster mode
 * @pari:	FP's PARI
 * @fpc:	FP capabilities
 * @b_sap:	B-SAP socket
 * @s_sap:	S-SAP listener socket
 * @links:	list of data links
 * @mme_list:	MM endpoint list
 */
struct dect_handle {
	const struct dect_ops		*ops;

	struct nl_sock			*nlsock;
	struct dect_fd			*nlfd;

	int				index;
	enum dect_cluster_modes		mode;
	struct dect_ari			pari;
	struct dect_fp_capabilities	fpc;

	struct dect_fd			*b_sap;
	struct dect_fd			*s_sap;
	struct list_head		links;

	struct list_head		mme_list;
};

#endif /* _LIBDECT_H */
