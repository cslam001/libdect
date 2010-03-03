#ifndef _LIBDECT_NETLINK_H
#define _LIBDECT_NETLINK_H

extern int dect_netlink_init(struct dect_handle *dh, const char *cluster);
extern void dect_netlink_exit(struct dect_handle *dh);

#endif /* _LIBDECT_NETLINK_H */
