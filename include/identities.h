#ifndef _DECT_IDENTITIES_H
#define _DECT_IDENTITIES_H

/*
 * Acess Rights Identity (ARI)
 */

#define DECT_ARI_ARC_MASK	0xe000000000000000ULL
#define DECT_ARI_ARC_SHIFT	61

/* Class A */
#define DECT_ARI_A_EMC_MASK	0x1fffe00000000000ULL
#define DECT_ARI_A_EMC_SHIFT	45

#define DECT_ARI_A_FPN_MASK	0x00001ffff0000000ULL
#define DECT_ARI_A_FPN_SHIFT	28

/* Class B */
#define DECT_ARI_B_EIC_MASK	0x1fffe00000000000ULL
#define DECT_ARI_B_EIC_SHIFT	45

#define DECT_ARI_B_FPN_MASK	0x00001fe000000000ULL
#define DECT_ARI_B_FPN_SHIFT	37

#define DECT_ARI_B_FPS_MASK	0x0000001e00000000ULL
#define DECT_ARI_B_FPS_SHIFT	33

/* Class C */
#define DECT_ARI_C_POC_MASK	0x1fffe00000000000ULL
#define DECT_ARI_C_POC_SHIFT	45

#define DECT_ARI_C_FPN_MASK	0x00001fe000000000ULL
#define DECT_ARI_C_FPN_SHIFT	37

#define DECT_ARI_C_FPS_MASK	0x0000001e00000000ULL
#define DECT_ARI_C_FPS_SHIFT	33

/* Class D */
#define DECT_ARI_D_GOP_MASK	0x1ffffe0000000000ULL
#define DECT_ARI_D_GOP_SHIFT	41

#define DECT_ARI_D_FPN_MASK	0x000001fe00000000ULL
#define DECT_ARI_D_FPN_SHIFT	33

/* Class E */
#define DECT_ARI_E_FIL_MASK	0x1fffe00000000000ULL
#define DECT_ARI_E_FIL_SHIFT	45

#define DECT_ARI_E_FPN_MASK	0x00001ffe00000000ULL
#define DECT_ARI_E_FPN_SHIFT	33


/*
 * IPEI
 */

#define DECT_IPEI_EMC_MASK	0x0000000ffff00000ULL
#define DECT_IPEI_EMC_SHIFT	20

#define DECT_IPEI_PSN_MASK	0x00000000000fffffULL

/*
 * IPUI
 */

#define DECT_IPUI_PUT_MASK		0xf0

extern bool dect_parse_ipui(struct dect_ipui *ipui,
			    const uint8_t *ptr, uint8_t len);
extern uint8_t dect_build_ipui(uint8_t *ptr, const struct dect_ipui *ipui);
extern void dect_dump_ipui(const struct dect_ipui *ipui);

/*
 * TPUI
 */

#define DECT_TPUI_CONNECTIONLESS_GROUP_ID	0xcc000

#define DECT_TPUI_CALL_GROUP_ID			0xdd000

#define DECT_TPUI_DEFAULT_INDIVIDUAL_ID		0xe0000
#define DECT_TPUI_DEFAULT_INDIVIDUAL_IPUI_MASK	0x0ffff

#define DECT_TPUI_EMERGENCY_ID			0xf1000

extern uint32_t dect_build_tpui(const struct dect_tpui *tpui);

/*
 * PMID (Portable Part Identifier)
 */

#define DECT_PMID_MASK			0x000fffff
#define DECT_PMID_SIZE			20

#define DECT_PMID_DEFAULT_ID_MASK	0x000f0000
#define DECT_PMID_DEFAULT_ID		0x000e0000
#define DECT_PMID_DEFAULT_NUM_MASK	0x0000ffff

#define DECT_PMID_EMERGENCY_ID_MASK	0x000ff000
#define DECT_PMID_EMERGENCY_ID		0x000f1000
#define DECT_PMID_EMERGENCY_TPUI_MASK	0x00000fff

#define DECT_PMID_ASSIGNED_TPUI_MASK	0x000fffff

/**
 * @DECT_PMID_DEFAULT:		1110 + arbitrary number (16 bits)
 * @DECT_PMID_ASSIGNED:		Assigned individual TPUI
 * @DECT_PMID_EMERGENCY:	1111 0001 + 12 bits of emergency TPUI
 */
enum dect_pmid_types {
	DECT_PMID_DEFAULT,
	DECT_PMID_ASSIGNED,
	DECT_PMID_EMERGENCY,
};

struct dect_pmid {
	enum dect_pmid_types	type;
	union {
		uint32_t	tpui;
		uint32_t	num;
	};
};

#endif /* _DECT_IDENTITIES_H */
