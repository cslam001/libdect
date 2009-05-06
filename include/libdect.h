#ifndef _LIBDECT_H
#define _LIBDECT_H

#include <linux/dect_netlink.h>
#include <dect/libdect.h>
#include <list.h>

/**
 * struct dect_handle - libdect handle
 *
 * @ops:	user ops
 * @nlsock:	netlink socket
 * @nlfd:	netlink file descriptor
 * @index:	cluster index
 * @mode:	cluster mode
 * @pari:	FP's PARI
 * @b_sap:	B-SAP socket
 * @s_sap:	S-SAP listener socket
 * @links:	list of data links
 */
struct dect_handle {
	const struct dect_ops	*ops;
	struct nl_sock		*nlsock;
	struct dect_fd		*nlfd;

	unsigned int		index;
	enum dect_cluster_modes	mode;
	struct dect_ari		pari;

	struct dect_fd		*b_sap;
	struct dect_fd		*s_sap;
	struct list_head	links;
};

#endif /* _LIBDECT_H */
