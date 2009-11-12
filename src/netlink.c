/*
 * DECT Netlink Interface
 *
 * Copyright (c) 2009 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include <netlink/netlink.h>
#include <netlink/object.h>
#include <netlink/msg.h>
#include <netlink/dect/cluster.h>
#include <netlink/dect/ari.h>

#include <libdect.h>
#include <netlink.h>
#include <utils.h>

static void dect_netlink_event(struct dect_handle *dh, struct dect_fd *fd,
			       uint32_t event)
{
	nl_recvmsgs_default(dh->nlsock);
}

static void dect_netlink_set_callback(struct dect_handle *dh,
				      nl_recvmsg_msg_cb_t func,
				      void *arg)
{
	nl_socket_modify_cb(dh->nlsock, NL_CB_VALID, NL_CB_CUSTOM, func, arg);
}

static void dect_netlink_parse_ari(struct dect_ari *ari, const struct nl_dect_ari *nlari)
{
	ari->arc = nl_dect_ari_get_class(nlari);
	switch (ari->arc) {
	case DECT_ARC_A:
		ari->emc = nl_dect_ari_get_emc(nlari);
		ari->fpn = nl_dect_ari_get_fpn(nlari);
		break;
	case DECT_ARC_B:
		ari->eic = nl_dect_ari_get_eic(nlari);
		ari->fpn = nl_dect_ari_get_fpn(nlari);
		ari->fps = nl_dect_ari_get_fps(nlari);
		break;
	case DECT_ARC_C:
		ari->poc = nl_dect_ari_get_poc(nlari);
		ari->fpn = nl_dect_ari_get_fpn(nlari);
		ari->fps = nl_dect_ari_get_fps(nlari);
		break;
	case DECT_ARC_D:
		ari->gop = nl_dect_ari_get_gop(nlari);
		ari->fpn = nl_dect_ari_get_fpn(nlari);
		break;
	case DECT_ARC_E:
		ari->fil = nl_dect_ari_get_fil(nlari);
		ari->fpn = nl_dect_ari_get_fpn(nlari);
		break;
	}
}

static void get_cluster_cb(struct nl_object *obj, void *arg)
{
	struct dect_handle *dh = arg;
	struct nl_dect_cluster *cl = (struct nl_dect_cluster *)obj;

	dh->index = nl_dect_cluster_get_index(cl);
	dh->mode  = nl_dect_cluster_get_mode(cl);
	dect_netlink_parse_ari(&dh->pari, nl_dect_cluster_get_pari(cl));
	dect_debug("netlink: %s: mode %s ARI: class A: EMC: %.4x FPN: %.5x\n",
		   nl_dect_cluster_get_name(cl),
		   dh->mode == DECT_MODE_FP ? "FP" : "PP",
		   dh->pari.emc, dh->pari.fpn);
}

static int dect_netlink_get_cluster_cb(struct nl_msg *msg, void *arg)
{
	return nl_msg_parse(msg, get_cluster_cb, arg);
}

int dect_netlink_init(struct dect_handle *dh)
{
	struct nl_dect_cluster *cl;
	int err;

	dh->nlsock = nl_socket_alloc();
	if (dh->nlsock == NULL)
		goto err1;

	err = nl_connect(dh->nlsock, NETLINK_DECT);
	if (err < 0)
		goto err2;

	err = nl_socket_set_nonblocking(dh->nlsock);
	if (err < 0)
		goto err2;

	dh->nlfd = dect_alloc_fd(dh);
	if (dh->nlfd == NULL)
		goto err2;
	dh->nlfd->fd = nl_socket_get_fd(dh->nlsock);

	dect_setup_fd(dh->nlfd, dect_netlink_event, NULL);
	if (dect_register_fd(dh, dh->nlfd, DECT_FD_READ))
		goto err3;

	cl = nl_dect_cluster_alloc();
	if (cl == NULL)
		goto err4;
	nl_dect_cluster_set_name(cl, "cluster0");

	dect_netlink_set_callback(dh, dect_netlink_get_cluster_cb, dh);
	err = nl_dect_cluster_query(dh->nlsock, cl, 0);
	dect_netlink_set_callback(dh, NULL, NULL);
	if (err < 0)
		goto err5;

	return 0;
err5:
	nl_dect_cluster_put(cl);
err4:
	dect_unregister_fd(dh, dh->nlfd);
err3:
	dect_free(dh, dh->nlfd);
err2:
	nl_close(dh->nlsock);
err1:
	return -1;
}

void dect_netlink_exit(struct dect_handle *dh)
{
	dect_unregister_fd(dh, dh->nlfd);
	nl_close(dh->nlsock);
	dect_free(dh, dh->nlfd);
}
