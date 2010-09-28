#ifndef _DECT_TEST_COMMON_H
#define _DECT_TEST_COMMON_H

#include <libdect.h>
#include <utils.h>
#include <list.h>

extern struct dect_handle *dh;
extern struct dect_ipui ipui;
extern const char *cluster;

extern int dect_event_ops_init(struct dect_ops *ops);
extern void dect_event_loop(void);
extern void dect_event_loop_stop(void);
extern void dect_event_ops_cleanup(void);
extern void dect_dummy_ops_init(struct dect_ops *ops);
extern void dect_debug_init(void);

extern void dect_common_init(struct dect_ops *ops, const char *cluster);
extern void dect_common_cleanup(struct dect_handle *dh);

extern int dect_parse_ipui(struct dect_ipui *ipui, const char *optarg);

extern int dect_write_uak(const struct dect_ipui *ipui,
			  const uint8_t uak[DECT_AUTH_KEY_LEN]);
extern int dect_read_uak(const struct dect_ipui *ipui,
			 uint8_t uak[DECT_AUTH_KEY_LEN]);

extern void dect_fp_common_options(int argc, char **argv);

extern void dect_pp_auth_init(struct dect_ops *ops,
			      const struct dect_ipui *ipui);
extern void dect_pp_common_init(struct dect_ops *ops, const char *cluster,
				const struct dect_ipui *ipui);
extern void dect_pp_common_options(int argc, char **argv);
extern void dect_pp_init_terminal_capability(struct dect_ie_terminal_capability *tcap);

struct mm_auth_priv {
	uint8_t         dck[DECT_CIPHER_KEY_LEN];
};

extern void pexit(const char *str);

#include "../src/ccitt-adpcm/g72x.h"

struct dect_audio_handle {
	struct g72x_state	codec;
	struct list_head	queue;
};

extern struct dect_audio_handle *dect_audio_open(void);
extern void dect_audio_queue(struct dect_audio_handle *ah, struct dect_msg_buf *mb);

#endif /* _DECT_TEST_COMMON_H */
