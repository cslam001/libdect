#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <event.h>

#include <dect/libdect.h>
#include <dect/terminal.h>
#include <dect/keypad.h>
#include "common.h"

struct call {
	struct dect_keypad_buffer	*keybuf;
	struct dect_audio_handle	*audio;
	struct event			event;
	enum {
		BLINK0,
		RING,
		BLINK1,
		SCROLLING,
	}				state;
	uint8_t				scroll_off;
	uint8_t				ring_pattern;
};

enum phones { PHONE1, PHONE2, PHONE3, };
static const struct dect_ipui ipuis[] = {
	[PHONE1] = {
		.put		= DECT_IPUI_N,
		.pun.n.ipei = {
			.emc	= 0x08ae,
			.psn	= 0x83d1e,
		},
	},
	[PHONE2] = {
		.put		= DECT_IPUI_N,
		.pun.n.ipei = {
			.emc	= 0x08ae,
			.psn	= 0x8969f,
		},
	},
	[PHONE3] = {
		.put		= DECT_IPUI_N,
		.pun.n.ipei = {
			.emc	= 0x08ae,
			.psn	= 0x5b9a0,
		},
	},
};

static void dect_mncc_timer(int fd, short even, void *data);
static void dect_mncc_timer_schedule(struct dect_call *call)
{
	struct call *priv = dect_call_priv(call);
	struct timeval tv = { .tv_sec = 1 };

	evtimer_set(&priv->event, dect_mncc_timer, call);
	evtimer_add(&priv->event, &tv);
}

static void dect_mncc_timer(int fd, short even, void *data)
{
	struct dect_call *call = data;
	struct dect_ie_display display;
	struct dect_ie_signal signal;
	struct dect_mncc_info_param info = {
		//.signal		= &signal,
		.display	= &display,
	};
	static int code;

	init_list_head(&info.progress_indicator.list);
	dect_ie_init(&signal);
	signal.code = DECT_SIGNAL_ALERTING_BASE | (code % 10);

	dect_display_init(&display);
	dect_display_append_char(&display, code++);

	dect_mncc_info_req(dh, call, &info);
	dect_mncc_timer_schedule(call);
}

static void dect_keypad_complete(struct dect_handle *dh, void *call,
				 struct dect_ie_keypad *keypad)
{
	printf("keypad complete: '%.*s'\n", keypad->len, keypad->info);
}

static void dect_mncc_connect_ind(struct dect_handle *dh, struct dect_call *call,
				  const struct dect_mncc_connect_param *param)
{
	printf("MNCC_CONNECT-ind\n");
}

static void dect_mncc_setup_ind(struct dect_handle *dh, struct dect_call *call,
				const struct dect_mncc_setup_param *setup)
{
	struct call *priv = dect_call_priv(call);
	struct dect_ie_signal signal;
	struct dect_mncc_connect_param connect = {
		//.signal		= &signal,
	};

	printf("MNCC_SETUP-ind\n");
	dect_ie_init(&signal);
	signal.code = DECT_SIGNAL_DIAL_TONE_ON;

	priv->keybuf = dect_keypad_buffer_init(dh, 3, dect_keypad_complete, call);
	priv->audio  = dect_audio_open();

	dect_mncc_connect_req(dh, call, &connect);
}

static void dect_mncc_setup_ack_ind(struct dect_handle *dh, struct dect_call *call,
				    const struct dect_mncc_setup_ack_param *param)
{
	printf("MNCC_SETUP_ACK-ind\n");
}

static void dect_mncc_info_ind(struct dect_handle *dh, struct dect_call *call,
			       const struct dect_mncc_info_param *param)
{
	struct call *priv = dect_call_priv(call);
	struct dect_ie_progress_indicator progress_indicator;
	struct dect_ie_signal signal;
	struct dect_mncc_info_param info = {
		.signal			= &signal,
	};

	printf("MNCC_INFO-ind\n");
	return;
	dect_keypad_append(dh, priv->keybuf, param->keypad,
			   param->sending_complete);

	dect_ie_init(&signal);
	signal.code = DECT_SIGNAL_DIAL_TONE_ON;

	dect_ie_init(&progress_indicator);
	progress_indicator.location = DECT_LOCATION_PRIVATE_NETWORK_SERVING_LOCAL_USER;
	progress_indicator.progress = DECT_PROGRESS_INBAND_INFORMATION_NOW_AVAILABLE;

	init_list_head(&info.progress_indicator.list);
	list_add_tail(&progress_indicator.common.list, &info.progress_indicator.list);

	dect_mncc_info_req(dh, call, &info);
}

static void dect_mncc_send_call_info(struct dect_call *call)
{
	struct call *priv = dect_call_priv(call);
	struct dect_ie_display display;
	struct dect_ie_signal signal;
	struct dect_mncc_info_param info = {
		.display	= &display,
		.signal		= &signal,
	};
	static const char *text = "  kaber   ";

	dect_display_init(&display);
	dect_display_append_char(&display, DECT_C_CLEAR_DISPLAY);
	dect_display_append_char(&display, '*');
	dect_display_append(&display, text + priv->scroll_off,
			    strlen(text) - priv->scroll_off);
	dect_display_append(&display, text, priv->scroll_off);
	dect_display_append_char(&display, '*');

	dect_ie_init(&signal);
	if (priv->state == RING) {
		signal.code = DECT_SIGNAL_ALERTING_BASE | priv->ring_pattern;
		priv->ring_pattern = (priv->ring_pattern + 1) % 8;
	} else
		signal.code = DECT_SIGNAL_ALERTING_BASE | DECT_RING_OFF;

	if (priv->state != SCROLLING)
		priv->state++;
	else {
		priv->scroll_off = (priv->scroll_off + 1) % (strlen(text) + 1);
		if (priv->scroll_off == 0)
			priv->state = 0;
	}

	dect_mncc_info_req(dh, call, &info);
}

static void dect_mncc_info_timer(int fd, short even, void *data);
static void dect_mncc_info_timer_schedule(struct dect_call *call)
{
	struct call *priv = dect_call_priv(call);
	struct timeval tv = { .tv_usec = 500000 };

	evtimer_set(&priv->event, dect_mncc_info_timer, call);
	evtimer_add(&priv->event, &tv);
}

static void dect_mncc_info_timer(int fd, short even, void *data)
{
	struct dect_call *call = data;

	dect_mncc_send_call_info(call);
	dect_mncc_info_timer_schedule(call);
}

static void dect_mncc_alert_ind(struct dect_handle *dh, struct dect_call *call,
				const struct dect_mncc_alert_param *param)
{
	printf("MNCC_ALERT-ind\n");
	dect_mncc_info_timer(0, 0, call);
}

static void dect_mncc_reject_ind(struct dect_handle *dh, struct dect_call *call,
				 const struct dect_mncc_release_param *param)
{
	struct call *priv = dect_call_priv(call);

	printf("MNCC_REJECT-ind\n");
	event_del(&priv->event);
}

static void dect_open_call(struct dect_handle *dh, const struct dect_ipui *ipui)
{
	struct dect_ie_basic_service basic_service;
	struct dect_mncc_setup_param param = {
		.basic_service = &basic_service,
	};
	struct dect_call *call;
	struct call *priv;

	call = dect_call_alloc(dh);
	if (call == NULL)
		return;
	priv = dect_call_priv(call);

	dect_ie_init(&basic_service);
	basic_service.class   = DECT_CALL_CLASS_NORMAL;
	basic_service.service = DECT_SERVICE_BASIC_SPEECH_DEFAULT;

	dect_mncc_setup_req(dh, call, ipui, &param);
}

static void dect_dl_u_data_ind(struct dect_handle *dh, struct dect_call *call,
			       struct dect_msg_buf *mb)
{
	struct call *priv = dect_call_priv(call);

	dect_dl_u_data_req(dh, call, mb);
	dect_audio_queue(priv->audio, mb);
}

static const struct dect_cc_ops cc_ops = {
	.priv_size		= sizeof(struct call),
	.mncc_connect_ind	= dect_mncc_connect_ind,
	.mncc_setup_ind		= dect_mncc_setup_ind,
	.mncc_setup_ack_ind	= dect_mncc_setup_ack_ind,
	.mncc_info_ind		= dect_mncc_info_ind,
	.mncc_alert_ind		= dect_mncc_alert_ind,
	.mncc_reject_ind	= dect_mncc_reject_ind,
	.dl_u_data_ind		= dect_dl_u_data_ind,
};

static const struct dect_mm_ops mm_ops = {
	.mm_access_rights_ind	= 0,
	.mm_access_rights_cfm	= 0,
};

static struct dect_ops ops = {
	.cc_ops			= &cc_ops,
	.mm_ops			= &mm_ops,
};

int main(int argc, char **argv)
{
	if (dect_event_ops_init(&ops) < 0)
		exit(1);

	dh = dect_alloc_handle(&ops);
	if (dh == NULL)
		exit(1);

	if (dect_init(dh) < 0)
		exit(1);

#if 0
	//dect_lce_group_ring(dh, 0xf);
	dect_open_call(dh, &ipuis[PHONE1]);
	dect_open_call(dh, &ipuis[PHONE3]);
#else
	dect_open_call(dh, &ipuis[PHONE2]);
#endif
	dect_event_loop();
	dect_close_handle(dh);
	dect_event_ops_cleanup();
	return 0;
}
