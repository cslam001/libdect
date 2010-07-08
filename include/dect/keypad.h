#ifndef _LIBDECT_DECT_KEYPAD
#define _LIBDECT_DECT_KEYPAD

#ifdef __cplusplus
extern "C" {
#endif

extern struct dect_keypad_buffer *
dect_keypad_buffer_init(const struct dect_handle *dh, uint8_t timeout,
			void (*complete)(struct dect_handle *, void *data,
					 struct dect_ie_keypad *keypad),
			void *priv);

extern void dect_keypad_append(struct dect_handle *dh,
			       struct dect_keypad_buffer *buf,
			       const struct dect_ie_keypad *keypad,
			       bool sending_complete);

#ifdef __cplusplus
}
#endif
#endif /* _LIBDECT_DECT_KEYPAD */
