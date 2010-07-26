/*
 * DECT Identities
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/**
 * @defgroup identity Identities
 *
 * This module implements the NWK-Layer identities specified in ETSI EN 300 175-6.
 *
 * @{
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <asm/byteorder.h>

#include <libdect.h>
#include <identities.h>
#include <utils.h>

#define sfmt_debug(fmt, args...) \
	dect_debug(DECT_DEBUG_SFMT, fmt, ## args)

static const char * const ari_classes[] = {
	[DECT_ARC_A]	= "A",
	[DECT_ARC_B]	= "B",
	[DECT_ARC_C]	= "C",
	[DECT_ARC_D]	= "D",
	[DECT_ARC_E]	= "E",
};

void dect_dump_ari(const struct dect_ari *ari)
{
	sfmt_debug("\tclass: %s\n", ari_classes[ari->arc]);

	switch (ari->arc) {
	case DECT_ARC_A:
		sfmt_debug("\tEMC: %.4x\n", ari->emc);
		sfmt_debug("\tFPN: %.5x\n", ari->fpn);
		break;
	case DECT_ARC_B:
		sfmt_debug("\tEIC: %x\n", ari->eic);
		sfmt_debug("\tFPN: %x\n", ari->fpn);
		sfmt_debug("\tFPS: %x\n", ari->fps);
		break;
	case DECT_ARC_C:
	case DECT_ARC_D:
	case DECT_ARC_E:
		break;
	}
}

uint8_t dect_parse_ari(struct dect_ari *ari, uint64_t a)
{
	ari->arc = (a & DECT_ARI_ARC_MASK) >> DECT_ARI_ARC_SHIFT;
	switch (ari->arc) {
	case DECT_ARC_A:
		ari->emc = (a & DECT_ARI_A_EMC_MASK) >> DECT_ARI_A_EMC_SHIFT;
		ari->fpn = (a & DECT_ARI_A_FPN_MASK) >> DECT_ARI_A_FPN_SHIFT;
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

static void dect_dump_ipei(const struct dect_ipei *ipei)
{
	sfmt_debug("\tEMC: %.4x\n", ipei->emc);
	sfmt_debug("\tPSN: %.5x\n", ipei->psn);
}

static bool dect_parse_ipei(struct dect_ipei *ipei, uint64_t i)
{
	ipei->emc = (i & DECT_IPEI_EMC_MASK) >> DECT_IPEI_EMC_SHIFT;
	ipei->psn = (i & DECT_IPEI_PSN_MASK);
	return true;
}

static uint64_t dect_build_ipei(const struct dect_ipei *ipei)
{
	uint64_t i = 0;

	i |= (uint64_t)ipei->emc << DECT_IPEI_EMC_SHIFT;
	i |= (uint64_t)ipei->psn;
	return i;
}

void dect_dump_ipui(const struct dect_ipui *ipui)
{
	switch (ipui->put) {
	case DECT_IPUI_N:
		sfmt_debug("\tPUT: N (IPEI)\n");
		return dect_dump_ipei(&ipui->pun.n.ipei);
	case DECT_IPUI_O:
		sfmt_debug("\tPUT: O (private)\n");
		sfmt_debug("\tNumber: %" PRIx64 "\n", ipui->pun.o.number);
		return;
	case DECT_IPUI_P:
	case DECT_IPUI_Q:
	case DECT_IPUI_R:
	case DECT_IPUI_S:
	case DECT_IPUI_T:
	case DECT_IPUI_U:
	default:
		sfmt_debug("\tIPUI: unhandled type %u\n", ipui->put);
	}
}

bool dect_parse_ipui(struct dect_ipui *ipui, const uint8_t *ptr, uint8_t len)
{
	uint64_t tmp;

	if (len < 4)
		return false;

	tmp = __be64_to_cpu(*(__be64 *)&ptr[0]);

	ipui->put = ptr[0] & DECT_IPUI_PUT_MASK;
	switch (ipui->put) {
	case DECT_IPUI_N:
		if (len != 40)
			return false;
		return dect_parse_ipei(&ipui->pun.n.ipei, tmp >> 24);
	case DECT_IPUI_O:
		/* Shift away trailing bits */
		tmp >>= 64 - len;
		/* Clear PUT */
		tmp &= ~(0xfULL << (len - 4));

		ipui->pun.o.number = tmp;
		return true;
	case DECT_IPUI_P:
	case DECT_IPUI_Q:
	case DECT_IPUI_R:
	case DECT_IPUI_S:
	case DECT_IPUI_T:
	case DECT_IPUI_U:
	default:
		sfmt_debug("\tIPUI: unhandled type %u\n", ipui->put);
		return false;
	}
}

uint8_t dect_build_ipui(uint8_t *ptr, const struct dect_ipui *ipui)
{
	unsigned int i, len;
	uint64_t tmp;

	switch (ipui->put) {
	case DECT_IPUI_N:
		tmp = dect_build_ipei(&ipui->pun.n.ipei);
		len = 36;
		break;
	case DECT_IPUI_O:
		tmp = ipui->pun.o.number;
		len = fls(tmp);
		break;
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

	if (len < 4)
		tmp <<= 4 - len;

	memset(ptr, 0, div_round_up(4 + len, 8));
	ptr[0] = ipui->put;

	for (i = 0; i < div_round_up(len, 8U); i++)
		ptr[i] |= tmp >> (max(len, 4U) - 4 - 8 * i);

	return 4 + len;
}

bool dect_ipui_cmp(const struct dect_ipui *i1, const struct dect_ipui *i2)
{
	return memcmp(i1, i2, sizeof(*i1));
}
EXPORT_SYMBOL(dect_ipui_cmp);

void dect_ipui_to_tpui(struct dect_tpui *tpui, const struct dect_ipui *ipui)
{
	tpui->type = DECT_TPUI_INDIVIDUAL_DEFAULT;

	switch (ipui->put) {
	case DECT_IPUI_N:
		tpui->id.ipui = ipui->pun.n.ipei.psn & DECT_TPUI_DEFAULT_INDIVIDUAL_IPUI_MASK;
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
}

void dect_dump_tpui(const struct dect_tpui *tpui)
{
	unsigned int i;

	switch (tpui->type) {
	case DECT_TPUI_INDIVIDUAL_ASSIGNED:
		sfmt_debug("\ttype: individual assigned\n");
		sfmt_debug("\tdigits: ");
		for (i = 0; i < 5; i++) {
			if (tpui->ia.digits[i] != 0xb)
				sfmt_debug("%u", tpui->ia.digits[i]);
		}
		sfmt_debug("\n");
		return;
	case DECT_TPUI_CONNECTIONLESS_GROUP:
		sfmt_debug("\ttype: connectionless group\n");
		return;
	case DECT_TPUI_CALL_GROUP:
		sfmt_debug("\ttype: call group\n");
		return;
	case DECT_TPUI_INDIVIDUAL_DEFAULT:
		sfmt_debug("\ttype: individual default\n");
		sfmt_debug("\tIPUI: %.4x\n", tpui->id.ipui);
		return;
	case DECT_TPUI_EMERGENCY:
		sfmt_debug("\ttype: emergency\n");
		return;
	}
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
		t  = DECT_TPUI_DEFAULT_INDIVIDUAL_ID;
		t |= tpui->id.ipui;
		break;
	case DECT_TPUI_EMERGENCY:
		t  = DECT_TPUI_EMERGENCY_ID;
		break;
	}

	return t;
}

void dect_tpui_to_pmid(struct dect_pmid *pmid, const struct dect_tpui *tpui)
{
	uint32_t t;

	t = dect_build_tpui(tpui);

	switch (tpui->type) {
	case DECT_TPUI_INDIVIDUAL_ASSIGNED:
		pmid->type = DECT_PMID_ASSIGNED;
		pmid->tpui = t & DECT_PMID_ASSIGNED_TPUI_MASK;
		break;
	case DECT_TPUI_EMERGENCY:
		pmid->type = DECT_PMID_EMERGENCY;
		pmid->tpui = t & DECT_PMID_EMERGENCY_TPUI_MASK;
		break;
	default:
		BUG();
	}
}

void dect_parse_pmid(struct dect_pmid *pmid, uint32_t p)
{
	if ((p & DECT_PMID_DEFAULT_ID_MASK) == DECT_PMID_DEFAULT_ID) {
		pmid->type = DECT_PMID_DEFAULT;
		pmid->num  = p & DECT_PMID_DEFAULT_NUM_MASK;
	} else if ((p & DECT_PMID_EMERGENCY_ID_MASK) == DECT_PMID_EMERGENCY_ID) {
		pmid->type = DECT_PMID_EMERGENCY;
		pmid->tpui = p & DECT_PMID_EMERGENCY_TPUI_MASK;
	} else {
		pmid->type = DECT_PMID_ASSIGNED;
		pmid->tpui = p & DECT_PMID_ASSIGNED_TPUI_MASK;
	}
}

uint32_t dect_build_pmid(const struct dect_pmid *pmid)
{
	uint32_t p = 0;

	switch (pmid->type) {
	case DECT_PMID_DEFAULT:
		p |= DECT_PMID_DEFAULT_ID;
		p |= pmid->num;
		break;
	case DECT_PMID_EMERGENCY:
		p |= DECT_PMID_EMERGENCY_ID;
		p |= pmid->tpui;
		break;
	case DECT_PMID_ASSIGNED:
		p |= pmid->tpui;
		break;
	}
	return p;
}

/** @} */
