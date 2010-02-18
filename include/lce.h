/*
 * DECT Link Control Entity (LCE)
 *
 * Copyright (c) 2009 Patrick McHardy <kaber@trash.net>
 */

#ifndef _DECT_LCE_H
#define _DECT_LCE_H

#include <assert.h>
#include <linux/dect.h>
#include <list.h>
#include <s_fmt.h>
#include <utils.h>

static inline void dect_mbuf_dump(const struct dect_msg_buf *mb,
				  const char *prefix)
{
	dect_hexdump(prefix, mb->data, mb->len);
}

/**
 * enum dect_transaction_role - the LCE's role in a transaction
 *
 * @DECT_TRANSACTION_INITIATOR:	Transaction was initiated by higher layer protocol entity
 * @DECT_TRANSACTION_RESPONDER:	Transaction was initiated by network
 */
enum dect_transaction_role {
	DECT_TRANSACTION_INITIATOR,
	DECT_TRANSACTION_RESPONDER,
	__DECT_TRANSACTION_MAX
};
#define DECT_TRANSACTION_MAX		(__DECT_TRANSACTION_MAX - 1)

/* Connectionless NWK layer transaction value */
#define DECT_TV_CONNECTIONLESS		6

enum dect_release_modes {
	DECT_DDL_RELEASE_NORMAL,
	DECT_DDL_RELEASE_PARTIAL,
};

/**
 * struct dect_transaction - DECT protocol transaction
 *
 * @list:	Datalink transaction list node
 * @link:	Associated data link
 * @pd:		Protocol discriminator
 * @role:	Role (initiator/responder)
 * @tv:		Transaction value
 */
enum dect_s_pd_values;
struct dect_transaction {
	struct list_head		list;
	struct dect_data_link		*link;
	enum dect_pds			pd;
	enum dect_transaction_role	role;
	uint16_t			tv;
};

extern int dect_ddl_open_transaction(struct dect_handle *dh,
				     struct dect_transaction *ta,
				     struct dect_data_link *ddl,
				     enum dect_pds pd);
extern struct dect_data_link *dect_ddl_connect(struct dect_handle *dh,
					       const struct dect_ipui *ipui);
extern int dect_open_transaction(struct dect_handle *dh,
				 struct dect_transaction *ta,
				 const struct dect_ipui *ipui,
				 enum dect_pds pd);
extern void dect_confirm_transaction(struct dect_handle *dh,
				     struct dect_transaction *ta,
				     const struct dect_transaction *req);
extern void dect_close_transaction(struct dect_handle *dh,
				   struct dect_transaction *ta,
				   enum dect_release_modes mode);
extern void dect_transaction_get_ulei(struct sockaddr_dect_lu *addr,
				      const struct dect_transaction *ta);

extern int dect_lce_send(const struct dect_handle *dh,
			 const struct dect_transaction *ta,
			 const struct dect_sfmt_msg_desc *desc,
			 const struct dect_msg_common *msg, uint8_t type);

/**
 * struct dect_nwk_protocol - NWK layer protocol
 *
 * @name:	Protocol name
 * @pd:		Protocol discriminator
 * @open:	Open a new transaction initiated by the network
 * @shutdown:	Perform an active shutdown of the transaction
 * @rcv:	Receive a message related to an active transaction
 */
enum dect_cipher_states;
struct dect_nwk_protocol {
	const char		*name;
	enum dect_pds		pd;
	uint16_t		max_transactions;
	void			(*open)(struct dect_handle *dh,
				        const struct dect_transaction *req,
				        struct dect_msg_buf *mb);
	void			(*shutdown)(struct dect_handle *dh,
					    struct dect_transaction *req);
	void			(*rcv)(struct dect_handle *dh,
				       struct dect_transaction *ta,
				       struct dect_msg_buf *mb);
	void			(*encrypt_ind)(struct dect_handle *dh,
					       struct dect_transaction *ta,
					       enum dect_cipher_states state);
};

extern void dect_lce_register_protocol(const struct dect_nwk_protocol *protocol);

/**
 * struct dect_lte - Location Table Entry
 *
 * @list:	Location table list node
 * @ipui:	International Portable User ID
 * @tpui:	Temporary Portable User ID
 */
struct dect_lte {
	struct list_head		list;
	struct dect_ipui		ipui;
	struct dect_tpui		tpui;
};

struct dect_location_table {
	struct list_head		entries;
};

enum dect_data_link_states {
	DECT_DATA_LINK_RELEASED,
	DECT_DATA_LINK_ESTABLISHED,
	DECT_DATA_LINK_ESTABLISH_PENDING,
	DECT_DATA_LINK_RELEASE_PENDING,
	DECT_DATA_LINK_SUSPENDED,
	DECT_DATA_LINK_SUSPEND_PENDING,
	DECT_DATA_LINK_RESUME_PENDING,
	__DECT_DATA_LINK_STATE_MAX
};
#define DECT_DATA_LINK_STATE_MAX	(__DECT_DATA_LINK_STATE_MAX - 1)

/**
 * struct dect_data_link
 *
 * @list:		DECT handle link list node
 * @dfd:		Associated socket file descriptor
 * @dlei:		Data Link Endpoint identifier
 * @ipui:		International Portable User ID
 * @state:		Data link state
 * @sdu_timer:		Establish without SDU timer (LCE.05)
 * @release_timer:	Normal link release timer (LCE.01)
 * @page_timer:		Indirect establish timer (LCE.03)
 * @page_count:		Number of page messages sent
 * @msg_queue:		Message queue used during ESTABLISH_PENDING state
 */
struct dect_data_link {
	struct list_head		list;
	struct sockaddr_dect_ssap	dlei;
	struct dect_ipui		ipui;
	struct dect_fd			*dfd;
	enum dect_data_link_states	state;
	enum dect_cipher_states		cipher;
	struct dect_timer		*sdu_timer;
	struct dect_timer		*release_timer;
	struct dect_timer		*page_timer;
	uint8_t				page_count;
	struct list_head		msg_queue;
	struct list_head		transactions;
};

#define DECT_DDL_RELEASE_TIMEOUT	5	/* seconds */
#define DECT_DDL_ESTABLISH_SDU_TIMEOUT	5	/* seconds */
#define DECT_DDL_PAGE_TIMEOUT		5	/* seconds */
#define DECT_DDL_PAGE_RETRANS_MAX	3	/* N.300 */

extern int dect_ddl_set_cipher_key(const struct dect_data_link *ddl,
				   const uint8_t ck[]);
extern int dect_ddl_encrypt_req(const struct dect_data_link *ddl,
				enum dect_cipher_states status);

/* LCE message types */
enum dect_lce_msg_types {
	DECT_LCE_PAGE_RESPONSE			= 0x71,
	DECT_LCE_PAGE_REJECT			= 0x72,
};

/**
 * struct dect_lce_page_response - LCE Page response S-Format message
 *
 * @common:			Message header
 * @portable_identity:		Portable Identity IE
 * @fixed_identity:		Fixed Identity IE
 * @nwk_assigned_identity:	NWK-assigned Identity IE (optional)
 * @cipher_info:		Cipher Info IE (optional)
 * @escape_to_proprietary:	Escape to proprietary IE (optional)
 */
struct dect_lce_page_response {
	struct dect_msg_common			common;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_fixed_identity		*fixed_identity;
	struct dect_ie_nwk_assigned_identity	*nwk_assigned_identity;
	struct dect_ie_cipher_info		*cipher_info;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

/**
 * struct dect_lce_page_reject - LCE Page reject S-Format message
 *
 * @common:			Message header
 * @portable_identity:		Portable Identity IE
 * @fixed_identity:		Fixed Identity IE
 * @reject_reason:		Reject reason IE
 * @escape_to_proprietary:	Escape to proprietary IE (optional)
 */
struct dect_lce_page_reject {
	struct dect_msg_common			common;
	struct dect_ie_portable_identity	*portable_identity;
	struct dect_ie_fixed_identity		*fixed_identity;
	struct dect_ie_reject_reason		*reject_reason;
	struct dect_ie_escape_to_proprietary	*escape_to_proprietary;
};

extern int dect_lce_init(struct dect_handle *dh);
extern void dect_lce_exit(struct dect_handle *dh);

#endif /* _DECT_LCE_H */
