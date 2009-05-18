/*
 * DECT Identities
 *
 * Copyright (c) 2009 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <asm/byteorder.h>

#include <libdect.h>
#include <identities.h>
#include <utils.h>


uint8_t dect_parse_ari(struct dect_ari *ari, uint64_t a)
{
	ari->arc = (a & DECT_ARI_ARC_MASK) >> DECT_ARI_ARC_SHIFT;
	switch (ari->arc) {
	case DECT_ARC_A:
		ari->emc = (a & DECT_ARI_A_EMC_MASK) >> DECT_ARI_A_EMC_SHIFT;
		ari->fpn = (a & DECT_ARI_A_FPN_MASK) >> DECT_ARI_A_FPN_SHIFT;
		dect_debug("ARI class A: EMC: %.4x FPN: %.5x\n",
			   ari->emc, ari->fpn);
		return DECT_ARC_A_LEN;
	case DECT_ARC_B:
		ari->eic = (a & DECT_ARI_B_EIC_MASK) >> DECT_ARI_B_EIC_SHIFT;
		ari->fpn = (a & DECT_ARI_B_FPN_MASK) >> DECT_ARI_B_FPN_SHIFT;
		ari->fps = (a & DECT_ARI_B_FPS_MASK) >> DECT_ARI_B_FPS_SHIFT;
		return DECT_ARC_B_LEN;
	case DECT_ARC_C:
		ari->poc = (a & DECT_ARI_C_POC_MASK) >> DECT_ARI_C_POC_SHIFT;
		ari->fpn = (a & DECT_ARI_C_FPN_MASK) >> DECT_ARI_C_FPN_SHIFT;
		ari->fps = (a & DECT_ARI_C_FPS_MASK) >> DECT_ARI_C_FPS_SHIFT;
		return DECT_ARC_C_LEN;
	case DECT_ARC_D:
		ari->gop = (a & DECT_ARI_D_GOP_MASK) >> DECT_ARI_D_GOP_SHIFT;
		ari->fpn = (a & DECT_ARI_D_FPN_MASK) >> DECT_ARI_D_FPN_SHIFT;
		return DECT_ARC_D_LEN;
	case DECT_ARC_E:
		ari->fil = (a & DECT_ARI_E_FIL_MASK) >> DECT_ARI_E_FIL_SHIFT;
		ari->fpn = (a & DECT_ARI_E_FPN_MASK) >> DECT_ARI_E_FPN_SHIFT;
		return DECT_ARC_E_LEN;
	default:
		return 0;
	}
}

uint64_t dect_build_ari(const struct dect_ari *ari)
{
	uint64_t a = 0;

	a |= (uint64_t)ari->arc << DECT_ARI_ARC_SHIFT;
	switch (ari->arc) {
	case DECT_ARC_A:
		a |= (uint64_t)ari->emc << DECT_ARI_A_EMC_SHIFT;
		a |= (uint64_t)ari->fpn << DECT_ARI_A_FPN_SHIFT;
		break;
	case DECT_ARC_B:
		a |= (uint64_t)ari->eic << DECT_ARI_B_EIC_SHIFT;
		a |= (uint64_t)ari->fpn << DECT_ARI_B_FPN_SHIFT;
		a |= (uint64_t)ari->fps << DECT_ARI_B_FPS_SHIFT;
		break;
	case DECT_ARC_C:
		a |= (uint64_t)ari->poc << DECT_ARI_C_POC_SHIFT;
		a |= (uint64_t)ari->fpn << DECT_ARI_C_FPN_SHIFT;
		a |= (uint64_t)ari->fps << DECT_ARI_C_FPS_SHIFT;
		break;
	case DECT_ARC_D:
		a |= (uint64_t)ari->gop << DECT_ARI_D_GOP_SHIFT;
		a |= (uint64_t)ari->fpn << DECT_ARI_D_FPN_SHIFT;
		break;
	case DECT_ARC_E:
		a |= (uint64_t)ari->fil << DECT_ARI_E_FIL_SHIFT;
		a |= (uint64_t)ari->fpn << DECT_ARI_E_FPN_SHIFT;
		break;
	}
	return a;
}

static bool dect_parse_ipei(struct dect_ipei *ipei, uint64_t i)
{
	ipei->emc = (i & DECT_IPEI_EMC_MASK) >> DECT_IPEI_EMC_SHIFT;
	ipei->psn = (i & DECT_IPEI_PSN_MASK);
	dect_debug("IPEI: EMC: %.4x PSN: %.5x\n", ipei->emc, ipei->psn);
	return true;
}

static uint64_t dect_build_ipei(const struct dect_ipei *ipei)
{
	uint64_t i = 0;

	i |= (uint64_t)ipei->emc << DECT_IPEI_EMC_SHIFT;
	i |= (uint64_t)ipei->psn;
	return i;
}

bool dect_parse_ipui(struct dect_ipui *ipui, const uint8_t *ptr, uint8_t len)
{
	uint64_t tmp;

	tmp = __be64_to_cpu(*(__be64 *)&ptr[0]) >> 24;

	ipui->put = ptr[0] & DECT_IPUI_PUT_MASK;
	switch (ipui->put) {
	case DECT_IPUI_N:
		if (len != 40)
			return false;
		return dect_parse_ipei(&ipui->pun.n.ipei, tmp);
	case DECT_IPUI_O:
	case DECT_IPUI_P:
	case DECT_IPUI_Q:
	case DECT_IPUI_R:
	case DECT_IPUI_S:
	case DECT_IPUI_T:
	case DECT_IPUI_U:
	default:
		dect_debug("IPUI: unhandled type %u\n", ipui->put);
		return false;
	}
}

uint8_t dect_build_ipui(uint8_t *ptr, const struct dect_ipui *ipui)
{
	uint64_t tmp;

	switch (ipui->put) {
	case DECT_IPUI_N:
		tmp = dect_build_ipei(&ipui->pun.n.ipei);
		break;
	case DECT_IPUI_O:
	case DECT_IPUI_P:
	case DECT_IPUI_Q:
	case DECT_IPUI_R:
	case DECT_IPUI_S:
	case DECT_IPUI_T:
	case DECT_IPUI_U:
		return 0;
	default:
		return 0;
	}

	ptr[0]  = ipui->put;
	ptr[0] |= (tmp >> 32) & ~DECT_IPUI_PUT_MASK;
	ptr[1]  = tmp >> 24;
	ptr[2]  = tmp >> 16;
	ptr[3]  = tmp >> 8;
	ptr[4]  = tmp >> 0;
	return 40;
}

bool dect_ipui_cmp(const struct dect_ipui *i1, const struct dect_ipui *i2)
{
	return memcmp(i1, i2, sizeof(*i1));
}

static uint32_t dect_build_default_individual_tpui(const struct dect_tpui *tpui)
{
	const struct dect_ipui *ipui = tpui->id.ipui;
	uint32_t t;

	t = DECT_TPUI_DEFAULT_INDIVIDUAL_ID;
	switch (ipui->put) {
	case DECT_IPUI_N:
		t |= ipui->pun.n.ipei.psn & DECT_TPUI_DEFAULT_INDIVIDUAL_IPUI_MASK;
		break;
	case DECT_IPUI_O:
	case DECT_IPUI_P:
	case DECT_IPUI_Q:
	case DECT_IPUI_R:
	case DECT_IPUI_S:
	case DECT_IPUI_T:
	case DECT_IPUI_U:
		break;
	}
	return t;
}

uint32_t dect_build_tpui(const struct dect_tpui *tpui)
{
	uint32_t t = 0;

	switch (tpui->type) {
	case DECT_TPUI_INDIVIDUAL_ASSIGNED:
		t  = tpui->ia.digits[0] << 16;
		t |= tpui->ia.digits[1] << 12;
		t |= tpui->ia.digits[2] << 8;
		t |= tpui->ia.digits[3] << 4;
		t |= tpui->ia.digits[4] << 0;
		break;
	case DECT_TPUI_CONNECTIONLESS_GROUP:
		t  = DECT_TPUI_CONNECTIONLESS_GROUP_ID;
		break;
	case DECT_TPUI_CALL_GROUP:
		t  = DECT_TPUI_CALL_GROUP_ID;
		break;
	case DECT_TPUI_INDIVIDUAL_DEFAULT:
		t  = dect_build_default_individual_tpui(tpui);
		break;
	case DECT_TPUI_EMERGENCY:
		t  = DECT_TPUI_EMERGENCY_ID;
		break;
	}

	return t;
}
