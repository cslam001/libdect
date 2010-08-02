#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>

#include <dect/libdect.h>
#include <dect/terminal.h>
#include <dect/keypad.h>
#include <timer.h>
#include "common.h"

struct call {
	struct dect_keypad_buffer	*keybuf;
	struct dect_audio_handle	*audio;
	struct dect_timer		*timer;
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

static void dect_mncc_timer(struct dect_handle *dh, struct dect_timer *timer);
static void dect_mncc_timer_schedule(struct dect_handle *dh, struct dect_call *call)
{
	struct call *priv = dect_call_priv(call);

	dect_timer_setup(priv->timer, dect_mncc_timer, call);
	dect_timer_start(dh, priv->timer, 1);
}

static void dect_mncc_timer(struct dect_handle *dh, struct dect_timer *timer)
{
	struct dect_call *call = timer->data;
	struct dect_ie_display display;
	struct dect_ie_signal signal;
	struct dect_mncc_info_param info = {
		//.signal		= &signal,
		.display	= &display,
	};
	static int code;

	dect_ie_init(&signal);
	signal.code = DECT_SIGNAL_ALERTING_BASE | (code % 10);

	dect_display_init(&display);
	dect_display_append_char(&display, code++);

	dect_mncc_info_req(dh, call, &info);
	dect_mncc_timer_schedule(dh, call);
}

static void dect_keypad_complete(struct dect_handle *dh, void *call,
				 struct dect_ie_keypad *keypad)
{
	printf("keypad complete: '%.*s'\n", keypad->len, keypad->info);
}

static void dect_mncc_connect_ind(struct dect_handle *dh, struct dect_call *call,
				  struct dect_mncc_connect_param *param)
{
}

static void dect_mncc_setup_ind(struct dect_handle *dh, struct dect_call *call,
				struct dect_mncc_setup_param *setup)
{
	struct call *priv = dect_call_priv(call);
	struct dect_ie_signal signal;
	struct dect_mncc_connect_param connect = {
		//.signal		= &signal,
	};

	dect_ie_init(&signal);
	signal.code = DECT_SIGNAL_DIAL_TONE_ON;

	priv->timer  = dect_timer_alloc(dh);
	priv->keybuf = dect_keypad_buffer_init(dh, 3, dect_keypad_complete, call);
	priv->audio  = dect_audio_open();

	dect_mncc_connect_req(dh, call, &connect);
}

static void dect_mncc_setup_ack_ind(struct dect_handle *dh, struct dect_call *call,
				    struct dect_mncc_setup_ack_param *param)
{
}

static void dect_mncc_info_ind(struct dect_handle *dh, struct dect_call *call,
			       struct dect_mncc_info_param *param)
{
	struct call *priv = dect_call_priv(call);
	struct dect_ie_progress_indicator progress_indicator;
	struct dect_ie_signal signal;
	struct dect_mncc_info_param info = {
		.signal			= &signal,
	};

	return;
	dect_keypad_append(dh, priv->keybuf, param->keypad,
			   param->sending_complete);

	dect_ie_init(&signal);
	signal.code = DECT_SIGNAL_DIAL_TONE_ON;

	dect_ie_init(&progress_indicator);
	progress_indicator.location = DECT_LOCATION_PRIVATE_NETWORK_SERVING_LOCAL_USER;
	progress_indicator.progress = DECT_PROGRESS_INBAND_INFORMATION_NOW_AVAILABLE;
	info.progress_indicator.list = &progress_indicator.common;

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

static void dect_mncc_info_timer(struct dect_handle *dh, struct dect_timer *timer);
static void dect_mncc_info_timer_schedule(struct dect_handle *dh, struct dect_call *call)
{
	struct call *priv = dect_call_priv(call);

	dect_timer_setup(priv->timer, dect_mncc_info_timer, call);
	dect_timer_start(dh, priv->timer, 1);
}

static void dect_mncc_info_timer(struct dect_handle *dh, struct dect_timer *timer)
{
	struct dect_call *call = timer->data;

	dect_mncc_send_call_info(call);
	dect_mncc_info_timer_schedule(dh, call);
}

static void dect_mncc_alert_ind(struct dect_handle *dh, struct dect_call *call,
				struct dect_mncc_alert_param *param)
{
	dect_mncc_send_call_info(call);
	dect_mncc_info_timer_schedule(dh, call);
}

static void dect_mncc_reject_ind(struct dect_handle *dh, struct dect_call *call,
				 struct dect_mncc_release_param *param)
{
	struct call *priv = dect_call_priv(call);

	dect_timer_stop(dh, priv->timer);
}

static void dect_mncc_release_ind(struct dect_handle *dh, struct dect_call *call,
				  struct dect_mncc_release_param *param)
{
	dect_mncc_release_res(dh, call, param);
	dect_event_loop_stop();
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

static struct dect_cc_ops cc_ops = {
	.priv_size		= sizeof(struct call),
	.mncc_connect_ind	= dect_mncc_connect_ind,
	.mncc_setup_ind		= dect_mncc_setup_ind,
	.mncc_setup_ack_ind	= dect_mncc_setup_ack_ind,
	.mncc_info_ind		= dect_mncc_info_ind,
	.mncc_alert_ind		= dect_mncc_alert_ind,
	.mncc_reject_ind	= dect_mncc_reject_ind,
	.mncc_release_ind	= dect_mncc_release_ind,
	.dl_u_data_ind		= dect_dl_u_data_ind,
};

static struct dect_ops ops = {
	.cc_ops			= &cc_ops,
};

int main(int argc, char **argv)
{
	dect_common_init(&ops, argv[1]);
#if 0
	//dect_lce_group_ring(dh, 0xf);
	dect_open_call(dh, &ipuis[PHONE1]);
	dect_open_call(dh, &ipuis[PHONE3]);
#else
	dect_open_call(dh, &ipuis[PHONE2]);
#endif
	dect_event_loop();
	dect_common_cleanup(dh);
	return 0;
}
