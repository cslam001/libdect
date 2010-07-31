#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <inttypes.h>

#include <dect/libdect.h>
#include <dect/auth.h>
#include "common.h"

#define debug(fmt, args...)	printf("IWU: PP-MM: " fmt, ## args)

static int rand_fd;

static struct dect_ipui ipui = {
	.put		= DECT_IPUI_N,
	.pun.n.ipei = {
		.emc	= 0x0ba8,
		.psn	= 0xa782a,
	}
};

struct mm_priv {
	uint64_t	rand;
	uint64_t	rs;
	uint8_t		uak[DECT_AUTH_KEY_LEN];
};

static void mm_authenticate_ind(struct dect_handle *dh,
				struct dect_mm_endpoint *mme,
				struct dect_mm_authenticate_param *param)
{
	struct mm_priv *priv = dect_mm_priv(mme);
	struct dect_ie_auth_res res1;
	struct dect_mm_authenticate_param reply = {
		.res	= &res1,
	};
	uint8_t k[DECT_AUTH_KEY_LEN], ks[DECT_AUTH_KEY_LEN];
	uint8_t dck[DECT_CIPHER_KEY_LEN];

	dect_auth_b1(priv->uak, sizeof(priv->uak), k);

	dect_auth_a11(k, param->rs->value, ks);
	dect_auth_a12(ks, param->rand->value, dck, &res1.value);

	dect_mm_authenticate_res(dh, mme, true, &reply);
}

static void mm_authenticate_cfm(struct dect_handle *dh,
				struct dect_mm_endpoint *mme, bool accept,
				struct dect_mm_authenticate_param *param)
{
	struct mm_priv *priv = dect_mm_priv(mme);
	uint8_t k[DECT_AUTH_KEY_LEN], ks[DECT_AUTH_KEY_LEN];
	//uint8_t dck[DECT_CIPHER_KEY_LEN];
	uint8_t ac[4];
	uint32_t res2;

	if (!accept)
		goto out;

	dect_pin_to_ac("0000", ac, sizeof(ac));
	dect_auth_b1(ac, sizeof(ac), k);

	dect_auth_a21(k, priv->rs, ks);
	dect_auth_a22(ks, priv->rand, &res2);

	if (res2 == param->res->value) {
		debug("authentication success\n");
		memcpy(priv->uak, ks, sizeof(priv->uak));
		dect_write_uak(&ipui, priv->uak);
		return;
	} else
		debug("authentication failure: rand: %" PRIx64 " %.8x | %.8x\n",
		      priv->rand, res2, param->res->value);
out:
	dect_event_loop_stop();
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

	auth_type.auth_id		= DECT_AUTH_DSAA;
	auth_type.auth_key_type		= DECT_KEY_AUTHENTICATION_CODE;
	auth_type.auth_key_num		= 0 | DECT_AUTH_KEY_IPUI_PARK;
	auth_type.cipher_key_num	= 0;
	auth_type.flags			= DECT_AUTH_FLAG_UPC;

	read(rand_fd, &rand.value, sizeof(rand.value));
	priv->rand = rand.value;

	dect_pin_to_ac("0000", ac, sizeof(ac));
	dect_auth_b1(ac, sizeof(ac), k);

	dect_auth_a11(k, param->rs->value, ks);
	dect_auth_a12(ks, param->rand->value, dck, &res1.value);

	priv->rs = param->rs->value;

	dect_mm_authenticate_req(dh, mme, &reply);
}

static void init_terminal_capability(struct dect_ie_terminal_capability *terminal_capability)
{
	terminal_capability->tone		= DECT_TONE_CAPABILITY_DIAL_TONE_ONLY;
	terminal_capability->echo		= DECT_ECHO_PARAMETER_FULL_TCLW;
	terminal_capability->noise_rejection	= DECT_NOISE_REJECTION_NONE;
	terminal_capability->volume_ctrl	= DECT_ADAPTIVE_VOLUME_PP_CONTROL_NONE;
	terminal_capability->slot		= DECT_SLOT_CAPABILITY_FULL_SLOT;

	terminal_capability->display		= DECT_DISPLAY_CAPABILITY_FULL_DISPLAY;
	terminal_capability->display_memory	= 48;
	terminal_capability->display_lines	= 3;
	terminal_capability->display_columns	= 16;
	terminal_capability->display_control	= DECT_DISPLAY_CONTROL_CODE_CODING_1;
	terminal_capability->display_charsets	= 0;
	terminal_capability->scrolling		= DECT_SCROLLING_NOT_SPECIFIED;
	terminal_capability->profile_indicator	= DECT_PROFILE_GAP_SUPPORTED |
						  DECT_PROFILE_NG_DECT_PART_1 |
						  DECT_PROFILE_NG_DECT_PART_3;
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

	portable_identity.type		= DECT_PORTABLE_ID_TYPE_IPUI;
	portable_identity.ipui		= ipui;

	auth_type.auth_id		= DECT_AUTH_DSAA;
	auth_type.auth_key_type		= DECT_KEY_AUTHENTICATION_CODE;
	auth_type.auth_key_num		= 0 | DECT_AUTH_KEY_IPUI_PARK;
	auth_type.cipher_key_num	= 0;
	auth_type.flags			= 0;

	init_terminal_capability(&terminal_capability);

	return dect_mm_access_rights_req(dh, mme, &param);
}

static struct dect_mm_ops mm_ops = {
	.priv_size		= sizeof(struct mm_priv),
	.mm_key_allocate_ind	= mm_key_allocate_ind,
	.mm_authenticate_cfm	= mm_authenticate_cfm,
	.mm_authenticate_ind	= mm_authenticate_ind,
	.mm_access_rights_cfm	= mm_access_rights_cfm,
};

static struct dect_ops ops = {
	.mm_ops		 = &mm_ops,
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

	mm_access_rights_req(dh, mme);
	dect_event_loop();

	dect_common_cleanup(dh);
	close(rand_fd);
	return 0;
}
