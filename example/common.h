#ifndef _DECT_TEST_COMMON_H
#define _DECT_TEST_COMMON_H

#include <libdect.h>
#include <utils.h>
#include <list.h>

extern struct dect_handle *dh;
extern int dect_event_ops_init(struct dect_ops *ops);
extern void dect_event_loop(void);
extern void dect_event_ops_cleanup(void);

extern void dummy_ops_init(struct dect_ops *ops);

#include "../src/ccitt-adpcm/g72x.h"

struct dect_audio_handle {
	struct g72x_state	codec;
	struct list_head	queue;
};

extern struct dect_audio_handle *dect_audio_open(void);
extern void dect_audio_queue(struct dect_audio_handle *ah, struct dect_msg_buf *mb);

#endif /* _DECT_TEST_COMMON_H */
