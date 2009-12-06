#ifndef _LIBDECT_DECT_RAW_H
#define _LIBDECT_DECT_RAW_H

extern struct dect_fd *dect_raw_socket(struct dect_handle *dh);
extern ssize_t dect_raw_transmit(const struct dect_fd *dfd, uint8_t slot,
				 const struct dect_msg_buf *mb);

#endif /* _LIBDECT_DECT_RAW_H */
