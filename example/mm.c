#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dect/libdect.h>
#include <dect/auth.h>
#include "common.h"

static uint8_t prefix[3] = { 1, 0, 0};
static uint16_t num;

struct mm_priv {
	struct dect_mm_locate_param	*locate;
};

static void mm_authenticate_ind(struct dect_handle *dh,
				struct dect_mm_endpoint *mme,
				struct dect_mm_authenticate_param *param)
{
	printf("MM_AUTHENTICATE-ind\n");
}

static void mm_authenticate_req(struct dect_handle *dh,
				struct dect_mm_endpoint *mme)
{
	struct dect_ie_auth_type auth_type;
	struct dect_ie_auth_value rand, rs;
	struct dect_mm_authenticate_param param = {
		.auth_type	= &auth_type,
		.rand		= &rand,
		.rs		= &rs,
	};

	auth_type.auth_id	 = DECT_AUTH_DSAA;
	auth_type.auth_key_type	 = DECT_KEY_AUTHENTICATION_CODE;
	auth_type.auth_key_num   = 0 | DECT_AUTH_KEY_IPUI_PARK;
	auth_type.cipher_key_num = 0;
	auth_type.flags		 = 0;
	rand.value = 1;
	rs.value = 0;

	dect_mm_authenticate_req(dh, mme, &param);
}

static void mm_locate_res(struct dect_handle *dh,
			  struct dect_mm_endpoint *mme)
{
	struct mm_priv *priv = dect_mm_priv(mme);
	struct dect_mm_locate_param *param = priv->locate;
	struct dect_ie_portable_identity portable_identity;
	struct dect_ie_duration duration;
	struct dect_mm_locate_param reply = {
		.location_area		= param->location_area,
		.portable_identity	= &portable_identity,
		.duration		= &duration,
	};

	dect_ie_init(&portable_identity);
	portable_identity.type = DECT_PORTABLE_ID_TYPE_TPUI;
	portable_identity.tpui.type = DECT_TPUI_INDIVIDUAL_ASSIGNED;
	memcpy(&portable_identity.tpui.ia.digits, prefix, sizeof(prefix));
	portable_identity.tpui.ia.digits[3] = num / 10;
	portable_identity.tpui.ia.digits[4] = num % 10;
	num++;

	dect_ie_init(&duration);
	duration.lock = DECT_LOCK_TEMPORARY_USER_LIMIT_1;
	duration.time = DECT_TIME_LIMIT_DEFINED_TIME_LIMIT_1;
	duration.duration = 1;

	dect_mm_locate_res(dh, mme, &reply);
}

static void mm_authenticate_cfm(struct dect_handle *dh,
				struct dect_mm_endpoint *mme, bool accept,
				struct dect_mm_authenticate_param *param)
{
	struct mm_priv *priv = dect_mm_priv(mme);
	uint8_t k[DECT_AUTH_KEY_LEN], ks[DECT_AUTH_KEY_LEN];
	uint8_t dck[DECT_CIPHER_KEY_LEN];
	uint8_t ac[4];
	uint32_t res1;

	printf("MM_AUTHENTICATE-cfm accept: %u\n", accept);
	if (!accept)
		return;

	dect_pin_to_ac("1234", ac, sizeof(ac));
	dect_auth_b1(ac, sizeof(ac), k);

	dect_auth_a11(k, 0, ks);
	dect_auth_a12(ks, 1, dck, &res1);

	if (res1 == param->res->value) {
		printf("authentication success\n");
		mm_locate_res(dh, mme);
	} else
		printf("authentication failure\n");

	dect_ie_collection_put(dh, priv->locate);
}

static void mm_access_rights_ind(struct dect_handle *dh,
				 struct dect_mm_endpoint *mme,
				 struct dect_mm_access_rights_param *param)
{
	printf("MM_ACCESS_RIGHTS-ind\n");
	dect_mm_access_rights_res(dh, mme, true, param);
}

static void mm_locate_ind(struct dect_handle *dh,
			  struct dect_mm_endpoint *mme,
			  struct dect_mm_locate_param *param)
{
	struct mm_priv *priv = dect_mm_priv(mme);

	printf("MM_LOCATE-ind\n");

	priv->locate = (void *)dect_ie_collection_hold(param);
	mm_authenticate_req(dh, mme);
}

static void mm_identity_assign_cfm(struct dect_handle *dh,
				   struct dect_mm_endpoint *mme, bool accept,
				   struct dect_mm_identity_assign_param *param)
{
	printf("MM_IDENTITY_ASSIGN-cfm\n");
}

static const struct dect_mm_ops mm_ops = {
	.priv_size		= sizeof(struct mm_priv),
	.mm_authenticate_ind	= mm_authenticate_ind,
	.mm_authenticate_cfm	= mm_authenticate_cfm,
	.mm_access_rights_ind	= mm_access_rights_ind,
	.mm_locate_ind		= mm_locate_ind,
	.mm_identity_assign_cfm	= mm_identity_assign_cfm,
};

static struct dect_ops ops = {
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

	dect_event_loop();
	dect_close_handle(dh);
	dect_event_ops_cleanup();
	return 0;
}
