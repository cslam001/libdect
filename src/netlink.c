/*
 * DECT Netlink Interface
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/**
 * @defgroup llme Lower Layer Management Entity (LLME)
 * @{
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <errno.h>

#include <netlink/netlink.h>
#include <netlink/object.h>
#include <netlink/msg.h>
#include <netlink/dect/cluster.h>
#include <netlink/dect/llme.h>
#include <netlink/dect/ari.h>

#include <libdect.h>
#include <netlink.h>
#include <utils.h>
#include <io.h>

#define nl_debug_entry(fmt, args...) \
	dect_debug(DECT_DEBUG_NL, "\nnetlink: " fmt, ## args)
#define nl_debug(fmt, args...) \
	dect_debug(DECT_DEBUG_NL, "netlink: " fmt, ## args)

struct dect_netlink_handler {
	struct dect_handle	*dh;
	void			(*rcv)(struct dect_handle *, bool,
				       struct nl_object *);
	bool			request;
};

static void __maybe_unused dect_netlink_obj_dump(struct nl_object *obj)
{
#ifdef DEBUG
	char buf[2048];
	struct nl_dump_params dp = {
		.dp_type	= NL_DUMP_LINE,
		.dp_buf		= buf,
		.dp_buflen	= sizeof(buf),
	};

	buf[0] = '\0';
	nl_object_dump(obj, &dp);
	dect_debug(DECT_DEBUG_NL, "%s", buf);
#endif
}

static void dect_netlink_obj_rcv(struct nl_object *obj, void *arg)
{
	struct dect_netlink_handler *handler = arg;

	handler->rcv(handler->dh, handler->request, obj);
}

static int dect_netlink_msg_rcv(struct nl_msg *msg, void *arg)
{
	if (nl_msg_parse(msg, dect_netlink_obj_rcv, arg) < 0)
		nl_debug("message parsing failed type %u\n",
			 nlmsg_hdr(msg)->nlmsg_type);

	return NL_OK;
}

static void dect_netlink_event(struct dect_handle *dh, struct dect_fd *fd,
			       uint32_t event)
{
	if (nl_recvmsgs_default(dh->nlsock) < 0)
		nl_debug("nl_recvmsgs: %s\n", strerror(errno));
}

static void dect_netlink_set_callback(struct dect_handle *dh,
				      nl_recvmsg_msg_cb_t func,
				      void *arg)
{
	nl_socket_modify_cb(dh->nlsock, NL_CB_VALID, NL_CB_CUSTOM, func, arg);
}

/*
 * Cluster
 */

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

static void dect_netlink_cluster_rcv(struct dect_handle *dh, bool request,
				     struct nl_object *obj)
{
	struct nl_dect_cluster *cl = nl_object_priv(obj);

	dh->index = nl_dect_cluster_get_index(cl);
	dh->mode  = nl_dect_cluster_get_mode(cl);
	dect_netlink_parse_ari(&dh->pari, nl_dect_cluster_get_pari(cl));

	nl_debug("%s: mode %s ARI: class A: EMC: %.4x FPN: %.5x\n",
		 nl_dect_cluster_get_name(cl),
		 dh->mode == DECT_MODE_FP ? "FP" : "PP",
		 dh->pari.emc, dh->pari.fpn);
}

static int dect_netlink_get_cluster(struct dect_handle *dh, const char *name)
{
	struct dect_netlink_handler handler = {
		.dh	= dh,
		.rcv	= dect_netlink_cluster_rcv,
	};
	struct nl_dect_cluster *cl;
	int err;

	cl = nl_dect_cluster_alloc();
	if (cl == NULL)
		return -1;
	nl_dect_cluster_set_name(cl, name);

	dect_netlink_set_callback(dh, dect_netlink_msg_rcv, &handler);
	err = nl_dect_cluster_query(dh->nlsock, cl, 0);
	dect_netlink_set_callback(dh, NULL, NULL);
	nl_dect_cluster_put(cl);
	return err;
}

/*
 * LLME
 */

static struct nl_dect_llme_msg *dect_llme_msg_init(const struct dect_handle *dh,
						   enum dect_llme_msg_types type)
{
	struct nl_dect_llme_msg *lmsg;

	lmsg = nl_dect_llme_msg_alloc();
	if (lmsg == NULL)
		return NULL;
	nl_dect_llme_msg_set_type(lmsg, type);
	nl_dect_llme_msg_set_op(lmsg, DECT_LLME_REQUEST);
	nl_dect_llme_msg_set_index(lmsg, dh->index);
	return lmsg;
}

/**
 * Get FP capabilities
 *
 * @param dh		libdect DECT handle
 */
const struct dect_fp_capabilities *dect_llme_fp_capabilities(const struct dect_handle *dh)
{
	return &dh->fpc;
}
EXPORT_SYMBOL(dect_llme_fp_capabilities);

static void dect_fp_capabilities_dump(const struct dect_fp_capabilities *fpc)
{
	char buf1[512], buf2[512], buf3[512];

	nl_dect_llme_fpc2str(fpc->fpc, buf1, sizeof(buf1));
	nl_dect_llme_efpc2str(fpc->efpc, buf2, sizeof(buf2));
	nl_dect_llme_efpc22str(fpc->efpc2, buf3, sizeof(buf3));
	if (buf1[0] || buf2[0] || buf3[0])
		nl_debug("FPC: %s%s%s%s%s\n",
			 buf1, buf1[0] && (buf2[0] || buf3[0]) ? "," : "",
			 buf2, buf2[0] && buf3[0] ? "," : "", buf3);

	nl_dect_llme_hlc2str(fpc->hlc, buf1, sizeof(buf1));
	nl_dect_llme_ehlc2str(fpc->ehlc, buf2, sizeof(buf2));
	nl_dect_llme_ehlc22str(fpc->ehlc2, buf3, sizeof(buf3));
	if (buf1[0] || buf2[0] || buf3[0])
		nl_debug("HLC: %s%s%s%s%s\n",
			 buf1, buf1[0] && (buf2[0] || buf3[0]) ? "," : "",
			 buf2, buf2[0] && buf3[0] ? "," : "", buf3);
}

static void dect_netlink_llme_mac_info_rcv(struct dect_handle *dh, bool request,
					   struct nl_dect_llme_msg *lmsg)
{
	struct dect_fp_capabilities *fpc = &dh->fpc;

	fpc->fpc   = nl_dect_llme_mac_info_get_fpc(lmsg);
	fpc->hlc   = nl_dect_llme_mac_info_get_hlc(lmsg);
	fpc->efpc  = nl_dect_llme_mac_info_get_efpc(lmsg);
	fpc->ehlc  = nl_dect_llme_mac_info_get_ehlc(lmsg);
	fpc->efpc2 = nl_dect_llme_mac_info_get_efpc2(lmsg);
	fpc->ehlc2 = nl_dect_llme_mac_info_get_ehlc2(lmsg);

	dect_fp_capabilities_dump(fpc);
	if (!request)
		dh->ops->llme_ops->mac_me_info_ind(dh, fpc);
}

static void dect_netlink_llme_rcv(struct dect_handle *dh, bool request,
				  struct nl_object *obj)
{
	struct nl_dect_llme_msg *lmsg = nl_object_priv(obj);
	enum dect_llme_msg_types type;
	enum dect_llme_ops op;

	type = nl_dect_llme_msg_get_type(lmsg);
	op   = nl_dect_llme_msg_get_op(lmsg);

#define LLME_MSG(type, op)	(type << 16 | op)
	switch (LLME_MSG(type, op)) {
	case LLME_MSG(DECT_LLME_MAC_INFO, DECT_LLME_INDICATE):
		return dect_netlink_llme_mac_info_rcv(dh, request, lmsg);
	default:
		nl_debug("unknown LLME message: type: %u op: %u\n", type, op);
		dect_netlink_obj_dump(obj);
	}
}

/**
 * MAC_ME_RFP_PRELOAD-req primitive
 *
 * @param dh		libdect DECT handle
 * @param fpc		Fixed Part Capabilities
 *
 * Issue a MAC_ME_RFP_PRELOAD-req request to the kernel. If successful the
 * broadcasted fixed part capabilities will be changed to the supplied values.
 *
 * @sa ETSI EN 300 175-3, section 8.3.2.1.
 */
int dect_llme_rfp_preload_req(struct dect_handle *dh,
			      const struct dect_fp_capabilities *fpc)
{
	struct dect_netlink_handler handler = {
		.dh		= dh,
		.rcv		= dect_netlink_llme_rcv,
		.request	= true,
	};
	struct nl_dect_llme_msg *lmsg;
	int err;

	nl_debug_entry("MAC_ME_RFP_PRELOAD-req\n");
	dect_fp_capabilities_dump(fpc);

	lmsg = dect_llme_msg_init(dh, DECT_LLME_MAC_RFP_PRELOAD);
	if (lmsg == NULL)
		return -1;

	nl_dect_llme_mac_info_set_hlc(lmsg, fpc->hlc);
	nl_dect_llme_mac_info_set_ehlc(lmsg, fpc->ehlc);
	nl_dect_llme_mac_info_set_ehlc2(lmsg, fpc->ehlc2);

	dect_netlink_set_callback(dh, dect_netlink_msg_rcv, &handler);
	err = nl_dect_llme_request(dh->nlsock, lmsg);
	dect_netlink_set_callback(dh, NULL, NULL);
	nl_dect_llme_msg_put(lmsg);
	return err;
}
EXPORT_SYMBOL(dect_llme_rfp_preload_req);

static int dect_netlink_mac_info_req(struct dect_handle *dh)
{
	struct dect_netlink_handler handler = {
		.dh		= dh,
		.rcv		= dect_netlink_llme_rcv,
		.request	= true,
	};
	struct nl_dect_llme_msg *lmsg;
	int err;

	lmsg = dect_llme_msg_init(dh, DECT_LLME_MAC_INFO);
	if (lmsg == NULL)
		return -1;

	dect_netlink_set_callback(dh, dect_netlink_msg_rcv, &handler);
	err = nl_dect_llme_request(dh->nlsock, lmsg);
	dect_netlink_set_callback(dh, NULL, NULL);
	nl_dect_llme_msg_put(lmsg);
	return err;
}

static int dect_netlink_event_rcv(struct nl_msg *msg, void *arg)
{
	struct sockaddr_nl *addr = nlmsg_get_src(msg);
	struct dectmsg *dm = nlmsg_data(nlmsg_hdr(msg));
	struct dect_handle *dh = arg;
	struct dect_netlink_handler handler = { .dh = dh };
	unsigned int group = ffs(addr->nl_groups);

	if (dm->dm_index != dh->index)
		return NL_OK;

	nl_debug_entry("message group: %u\n", group);
	switch (group) {
	case DECTNLGRP_CLUSTER:
		handler.rcv = dect_netlink_cluster_rcv;
		break;
	case DECTNLGRP_LLME:
		handler.rcv = dect_netlink_llme_rcv;
		break;
	default:
		nl_debug("unknown message source\n");
		return NL_OK;
	}

	return dect_netlink_msg_rcv(msg, &handler);
}

int dect_netlink_init(struct dect_handle *dh, const char *cluster)
{
	int err = 0;

	dh->nlsock = nl_socket_alloc();
	if (dh->nlsock == NULL)
		goto err1;

	err = nl_connect(dh->nlsock, NETLINK_DECT);
	if (err < 0)
		goto err2;

	err = nl_socket_set_nonblocking(dh->nlsock);
	if (err < 0)
		goto err2;

	dh->nlfd = dect_fd_alloc(dh);
	if (dh->nlfd == NULL)
		goto err2;
	dh->nlfd->fd = nl_socket_get_fd(dh->nlsock);

	dect_fd_setup(dh->nlfd, dect_netlink_event, NULL);
	if (dect_fd_register(dh, dh->nlfd, DECT_FD_READ))
		goto err3;

	err = dect_netlink_get_cluster(dh, cluster);
	if (err < 0)
		goto err4;

	if (dh->mode == DECT_MODE_PP) {
		err = dect_netlink_mac_info_req(dh);
		if (err < 0)
			goto err4;
	}

	dect_netlink_set_callback(dh, dect_netlink_event_rcv, dh);
	nl_socket_disable_seq_check(dh->nlsock);
	nl_socket_add_membership(dh->nlsock, DECTNLGRP_CLUSTER);
	nl_socket_add_membership(dh->nlsock, DECTNLGRP_LLME);
	return 0;

err4:
	dect_fd_unregister(dh, dh->nlfd);
err3:
	dect_free(dh, dh->nlfd);
err2:
	nl_close(dh->nlsock);
	nl_socket_free(dh->nlsock);
err1:
	dect_debug(DECT_DEBUG_NL, "dect_netlink_init: %s\n",
		   err == 0 ? strerror(errno) : nl_geterror(err));
	return -1;
}

void dect_netlink_exit(struct dect_handle *dh)
{
	dect_fd_unregister(dh, dh->nlfd);
	nl_close(dh->nlsock);
	nl_socket_free(dh->nlsock);
	dect_free(dh, dh->nlfd);
}

/** @} */
