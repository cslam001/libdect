#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <inttypes.h>

#include <dect/libdect.h>
#include <dect/auth.h>
#include "common.h"

static int rand_fd;

#define debug(fmt, args...)	printf("IWU: PP-MM: " fmt, ## args)

struct dect_ipui ipui = {
	.put		= DECT_IPUI_N,
	.pun.n.ipei = {
		.emc    = 0x0ba8,
		.psn    = 0xa782a,
	}
};

struct mm_priv {
	uint64_t			rand;
};

static void mm_cipher_cfm(struct dect_handle *dh,
			  struct dect_mm_endpoint *mme, bool accept,
			  struct dect_mm_cipher_param *param)
{
}

static void mm_cipher_req(struct dect_handle *dh, struct dect_mm_endpoint *mme,
			  uint8_t ck[DECT_CIPHER_KEY_LEN])
{
	struct dect_ie_cipher_info cipher_info;
	struct dect_mm_cipher_param param = {
		.cipher_info	= &cipher_info,
	};

	cipher_info.enable		= true;
	cipher_info.cipher_alg_id	= DECT_CIPHER_STANDARD_1;
	cipher_info.cipher_key_type	= DECT_CIPHER_DERIVED_KEY;
	cipher_info.cipher_key_num	= 0;

	dect_mm_cipher_req(dh, mme, &param, ck);
}

static void mm_authenticate_cfm(struct dect_handle *dh,
				struct dect_mm_endpoint *mme, bool accept,
				struct dect_mm_authenticate_param *param)
{
	struct mm_priv *priv = dect_mm_priv(mme);
	uint8_t k[DECT_AUTH_KEY_LEN], ks[DECT_AUTH_KEY_LEN];
	uint8_t dck[DECT_CIPHER_KEY_LEN];
	uint8_t ac[4];
	uint32_t res2;

	if (!accept)
		goto out;

	dect_pin_to_ac("00000000", ac, sizeof(ac));
	dect_auth_b1(ac, sizeof(ac), k);

	dect_auth_a21(k, param->rs->value, ks);
	dect_auth_a22(ks, priv->rand, &res2);

	if (res2 == param->res->value) {
		debug("authentication success\n");
		if (0)
			mm_cipher_req(dh, mme, dck);
	} else
		debug("authentication failure: rand: %" PRIx64 " %.8x | %.8x\n",
		       priv->rand, res2, param->res->value);
out:
	dect_event_loop_stop();
}

static int mm_authenticate_req(struct dect_handle *dh,
			       struct dect_mm_endpoint *mme)
{
	struct mm_priv *priv = dect_mm_priv(mme);
	struct dect_ie_auth_type auth_type;
	struct dect_ie_auth_value rand;
	struct dect_mm_authenticate_param param = {
		.auth_type	= &auth_type,
		.rand		= &rand,
	};

	auth_type.auth_id	 = DECT_AUTH_DSAA;
	auth_type.auth_key_type	 = DECT_KEY_USER_AUTHENTICATION_KEY;
	auth_type.auth_key_num   = 0 | DECT_AUTH_KEY_IPUI_PARK;
	auth_type.cipher_key_num = 0;
	auth_type.flags		 = 0; //DECT_AUTH_FLAG_UPC;
	read(rand_fd, &rand.value, sizeof(rand.value));
	priv->rand = rand.value;

	return dect_mm_authenticate_req(dh, mme, &param);
}

static void mm_authenticate_ind(struct dect_handle *dh,
				struct dect_mm_endpoint *mme,
				struct dect_mm_authenticate_param *param)
{
	struct dect_ie_auth_res res1;
	struct dect_mm_authenticate_param reply = {
		.res		= &res1,
	};
	uint8_t k[DECT_AUTH_KEY_LEN], ks[DECT_AUTH_KEY_LEN];
	uint8_t dck[DECT_CIPHER_KEY_LEN];
	uint8_t ac[4];

	dect_pin_to_ac("0000", ac, sizeof(ac));
	dect_auth_b1(ac, sizeof(ac), k);

	dect_auth_a11(k, param->rs->value, ks);
	dect_auth_a12(ks, param->rand->value, dck, &res1.value);

	dect_mm_authenticate_res(dh, mme, true, &reply);
}

static void mm_key_allocate_ind(struct dect_handle *dh,
				struct dect_mm_endpoint *mme,
				struct dect_mm_key_allocate_param *param)
{
	struct mm_priv *priv = dect_mm_priv(mme);
	struct dect_ie_auth_type auth_type;
	struct dect_ie_auth_value rand;
	struct dect_ie_auth_res res1;
	struct dect_mm_authenticate_param reply = {
		.auth_type	= &auth_type,
		.rand		= &rand,
		.res		= &res1,
	};
	uint8_t k[DECT_AUTH_KEY_LEN], ks[DECT_AUTH_KEY_LEN];
	uint8_t dck[DECT_CIPHER_KEY_LEN];
	uint8_t ac[4];

	auth_type.auth_id	 = DECT_AUTH_DSAA;
	auth_type.auth_key_type	 = DECT_KEY_AUTHENTICATION_CODE;
	auth_type.auth_key_num   = 0 | DECT_AUTH_KEY_IPUI_PARK;
	auth_type.cipher_key_num = 0;
	auth_type.flags		 = DECT_AUTH_FLAG_UPC;

	read(rand_fd, &rand.value, sizeof(rand.value));
	priv->rand = rand.value;

	dect_pin_to_ac("0000", ac, sizeof(ac));
	dect_auth_b1(ac, sizeof(ac), k);

	dect_auth_a11(k, param->rs->value, ks);
	dect_auth_a12(ks, param->rand->value, dck, &res1.value);

	dect_mm_authenticate_req(dh, mme, &reply);
}

static void mm_access_rights_terminate_ind(struct dect_handle *dh,
					   struct dect_mm_endpoint *mme,
					   struct dect_mm_access_rights_terminate_param *param)
{
	dect_mm_access_rights_terminate_res(dh, mme, false, param);
}

static void init_terminal_capability(struct dect_ie_terminal_capability *terminal_capability)
{
	terminal_capability->tone = DECT_TONE_CAPABILITY_DIAL_TONE_ONLY;
	terminal_capability->echo = DECT_ECHO_PARAMETER_FULL_TCLW;
	terminal_capability->noise_rejection = DECT_NOISE_REJECTION_NONE;
	terminal_capability->volume_ctrl = DECT_ADAPTIVE_VOLUME_PP_CONTROL_NONE;
	terminal_capability->slot = DECT_SLOT_CAPABILITY_FULL_SLOT;

	terminal_capability->display = DECT_DISPLAY_CAPABILITY_FULL_DISPLAY;
	terminal_capability->display_memory = 48;
	terminal_capability->display_lines = 3;
	terminal_capability->display_columns = 16;
	terminal_capability->display_control = DECT_DISPLAY_CONTROL_CODE_CODING_1;
	terminal_capability->display_charsets = 0;
	terminal_capability->scrolling = DECT_SCROLLING_NOT_SPECIFIED;
	terminal_capability->profile_indicator = DECT_PROFILE_GAP_SUPPORTED;
}

static void mm_access_rights_cfm(struct dect_handle *dh,
				 struct dect_mm_endpoint *mme, bool accept,
				 struct dect_mm_access_rights_param *param)
{
	dect_event_loop_stop();
}

static int mm_access_rights_req(struct dect_handle *dh, struct dect_mm_endpoint *mme)
{
	struct dect_ie_portable_identity portable_identity;
	struct dect_ie_auth_type auth_type;
	struct dect_ie_terminal_capability terminal_capability;
	struct dect_mm_access_rights_param param = {
		.portable_identity	= &portable_identity,
		.auth_type		= &auth_type,
		.terminal_capability	= &terminal_capability,
	};

	portable_identity.type	 = DECT_PORTABLE_ID_TYPE_IPUI;
	portable_identity.ipui	 = ipui;

	auth_type.auth_id	 = DECT_AUTH_DSAA;
	auth_type.auth_key_type	 = DECT_KEY_AUTHENTICATION_CODE;
	auth_type.auth_key_num   = 0 | DECT_AUTH_KEY_IPUI_PARK;
	auth_type.cipher_key_num = 0;
	auth_type.flags		 = 0;

	init_terminal_capability(&terminal_capability);

	return dect_mm_access_rights_req(dh, mme, &param);
}

static void mm_locate_cfm(struct dect_handle *dh, struct dect_mm_endpoint *mme,
			  bool accept, struct dect_mm_locate_param *param)
{
	dect_event_loop_stop();
}

static int mm_locate_req(struct dect_handle *dh, struct dect_mm_endpoint *mme)
{
	struct dect_ie_portable_identity portable_identity;
	struct dect_ie_location_area location_area;
	struct dect_ie_terminal_capability terminal_capability;
	struct dect_mm_locate_param param = {
		.portable_identity	= &portable_identity,
		.location_area		= &location_area,
		.terminal_capability	= &terminal_capability,
	};

	portable_identity.type	 = DECT_PORTABLE_ID_TYPE_IPUI;
	portable_identity.ipui	 = ipui;

	location_area.type	= DECT_LOCATION_AREA_LEVEL;
	location_area.level	= 36;

	init_terminal_capability(&terminal_capability);

	return dect_mm_locate_req(dh, mme, &param);
}

static int mm_detach_req(struct dect_handle *dh, struct dect_mm_endpoint *mme)
{
	struct dect_ie_portable_identity portable_identity;
	struct dect_mm_detach_param param = {
		.portable_identity	= &portable_identity,
	};

	portable_identity.type	 = DECT_PORTABLE_ID_TYPE_IPUI;
	portable_identity.ipui	 = ipui;

	return dect_mm_detach_req(dh, mme, &param);
}

static void mm_info_cfm(struct dect_handle *dh,
			struct dect_mm_endpoint *mme, bool accept,
			struct dect_mm_info_param *param)
{
	dect_event_loop_stop();
}

static int mm_info_req(struct dect_handle *dh, struct dect_mm_endpoint *mme)
{
	struct dect_ie_portable_identity portable_identity;
	struct dect_ie_info_type info_type;
	struct dect_mm_info_param param = {
		.portable_identity	= &portable_identity,
		.info_type		= &info_type,
	};

	portable_identity.type	 = DECT_PORTABLE_ID_TYPE_IPUI;
	portable_identity.ipui	 = ipui;

	info_type.num = 1;
	info_type.type[0] = DECT_INFO_HANDOVER_REFERENCE;

	return dect_mm_info_req(dh, mme, &param);
}

static void mm_access_rights_terminate_cfm(struct dect_handle *dh,
					   struct dect_mm_endpoint *mme, bool accept,
					   struct dect_mm_access_rights_terminate_param *param)
{
	dect_event_loop_stop();
}

static int mm_access_rights_terminate_req(struct dect_handle *dh,
					  struct dect_mm_endpoint *mme)
{
	struct dect_ie_portable_identity portable_identity;
	struct dect_mm_access_rights_terminate_param param = {
		.portable_identity	= &portable_identity,
	};

	portable_identity.type = DECT_PORTABLE_ID_TYPE_IPUI;
	portable_identity.ipui = ipui;

	return dect_mm_access_rights_terminate_req(dh, mme, &param);
}

static void mm_identity_ind(struct dect_handle *dh,
			    struct dect_mm_endpoint *mme,
			    struct dect_mm_identity_param *param)
{
	struct dect_mm_identity_param reply = {};

	dect_mm_identity_res(dh, mme, &reply);
}

static struct dect_mm_ops mm_ops = {
	.priv_size			= sizeof(struct mm_priv),
	.mm_authenticate_ind		= mm_authenticate_ind,
	.mm_authenticate_cfm		= mm_authenticate_cfm,
	.mm_access_rights_cfm		= mm_access_rights_cfm,
	.mm_locate_cfm			= mm_locate_cfm,
	.mm_key_allocate_ind		= mm_key_allocate_ind,
	.mm_access_rights_terminate_ind	= mm_access_rights_terminate_ind,
	.mm_access_rights_terminate_cfm	= mm_access_rights_terminate_cfm,
	.mm_cipher_cfm			= mm_cipher_cfm,
	.mm_identity_ind		= mm_identity_ind,
	.mm_info_cfm			= mm_info_cfm,
};

static void dect_mncc_setup_ind(struct dect_handle *dh, struct dect_call *call,
				struct dect_mncc_setup_param *setup)
{
	struct dect_mncc_connect_param connect = {};

	dect_mncc_connect_req(dh, call, &connect);
}

static struct dect_cc_ops cc_ops = {
	.mncc_setup_ind		= dect_mncc_setup_ind,
};

static struct dect_ops ops = {
	.mm_ops			= &mm_ops,
	.cc_ops			= &cc_ops,
};

int main(int argc, char **argv)
{
	struct dect_mm_endpoint *mme;

	rand_fd = open("/dev/urandom", O_RDONLY);
	if (rand_fd < 0)
		exit(1);

	dect_common_init(&ops, argv[1]);

	mme = dect_mm_endpoint_alloc(dh, &ipui);
	if (mme == NULL)
		exit(1);

	mm_locate_req(dh, mme);
	dect_event_loop();

	mm_authenticate_req(dh, mme);
	dect_event_loop();

	mm_access_rights_req(dh, mme);
	dect_event_loop();

	mm_access_rights_terminate_req(dh, mme);
	dect_event_loop();

	mm_info_req(dh, mme);
	dect_event_loop();

	if (0) {
		mm_detach_req(dh, mme);
	}

	dect_common_cleanup(dh);
	close(rand_fd);
	return 0;
}
