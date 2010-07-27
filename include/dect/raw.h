#ifndef _LIBDECT_DECT_RAW_H
#define _LIBDECT_DECT_RAW_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup raw
 * @{
 */

struct dect_raw_ops {
	void	(*raw_rcv)(struct dect_handle *dh, struct dect_fd *dfd,
			   uint8_t slot, struct dect_msg_buf *mb);
};

extern struct dect_fd *dect_raw_socket(struct dect_handle *dh);
extern ssize_t dect_raw_transmit(struct dect_handle *dh, struct dect_fd *dfd,
				 uint8_t slot, struct dect_msg_buf *mb);

/** @} */

#ifdef __cplusplus
}
#endif
#endif /* _LIBDECT_DECT_RAW_H */
