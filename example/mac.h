#ifndef _MAC_H

/*
 * A-Field
 */

#define DECT_A_FIELD_SIZE	8

#define DECT_RA_FIELD_SIZE	2
#define DECT_RA_FIELD_OFF	6

/*
 * Header field
 */

#define DECT_HDR_FIELD_SIZE	1
#define DECT_HDR_FIELD_OFF	0

#define DECT_HDR_TA_OFF		0
#define DECT_HDR_TA_MASK	0xe0
#define DECT_HDR_TA_SHIFT	5

#define DECT_HDR_Q1_OFF		0
#define DECT_HDR_Q1_FLAG	0x10

#define DECT_HDR_BA_OFF		0
#define DECT_HDR_BA_MASK	0x0e
#define DECT_HDR_BA_SHIFT	1

#define DECT_HDR_Q2_OFF		0
#define DECT_HDR_Q2_FLAG	0x01


/*
 * T-Field
 */

#define DECT_T_FIELD_OFF	1
#define DECT_T_FIELD_SIZE	5

/**
 * dect_tail_identification - MAC layer T-Field identification
 *
 * @DECT_TI_CT_PKT_0:		C_T data packet number 0
 * @DECT_TI_CT_PKT_1:		C_T data packet number 1
 * @DECT_TI_NT_CL:		Identities information on connectionless bearer
 * @DECT_TI_NT:			Identities information
 * @DECT_TI_QT:			Multiframe synchronisation und system information
 * @DECT_TI_RESERVED:		Reserved
 * @DECT_TI_MT:			MAC layer control
 * @DECT_TI_PT:			Paging tail (RFP only)
 * @DECT_TI_MT_PKT_0:		MAC layer control (first PP transmission, PP only)
 */
enum dect_tail_identifications {
	DECT_TI_CT_PKT_0	= 0x0 << DECT_HDR_TA_SHIFT,
	DECT_TI_CT_PKT_1	= 0x1 << DECT_HDR_TA_SHIFT,
	DECT_TI_NT_CL		= 0x2 << DECT_HDR_TA_SHIFT,
	DECT_TI_NT		= 0x3 << DECT_HDR_TA_SHIFT,
	DECT_TI_QT		= 0x4 << DECT_HDR_TA_SHIFT,
	DECT_TI_RESERVED	= 0x5 << DECT_HDR_TA_SHIFT,
	DECT_TI_MT		= 0x6 << DECT_HDR_TA_SHIFT,
	DECT_TI_PT		= 0x7 << DECT_HDR_TA_SHIFT,
	DECT_TI_MT_PKT_0	= 0x7 << DECT_HDR_TA_SHIFT,
};

/*
 * Paging Tail (P-channel)
 */

#define DECT_PT_HDR_EXTEND_FLAG		0x8000000000000000ULL

#define DECT_PT_HDR_LENGTH_MASK		0x7000000000000000ULL
#define DECT_PT_HDR_LENGTH_SHIFT	60

/**
 * @DECT_PT_ZERO_PAGE:		zero length page
 * @DECT_PT_SHORT_PAGE:		short page
 * @DECT_PT_FULL_PAGE:		full page
 * @DECT_PT_MAX_RESUME_PAGE:	MAC resume and control page
 * @DECT_PT_LONG_PAGE:		not the last 36 bits of a long page
 * @DECT_PT_LONG_PAGE_FIRST:	the first 36 bits of a long page
 * @DECT_PT_LONG_PAGE_LAST:	the last 36 bits of a long page
 * @DECT_PT_LONG_PAGE_ALL:	all of a long page (first and last)
 *
 */
enum dect_page_lengths {
	DECT_PT_ZERO_PAGE		= 0x0ULL << DECT_PT_HDR_LENGTH_SHIFT,
	DECT_PT_SHORT_PAGE		= 0x1ULL << DECT_PT_HDR_LENGTH_SHIFT,
	DECT_PT_FULL_PAGE		= 0x2ULL << DECT_PT_HDR_LENGTH_SHIFT,
	DECT_PT_RESUME_PAGE		= 0x3ULL << DECT_PT_HDR_LENGTH_SHIFT,
	DECT_PT_LONG_PAGE		= 0x4ULL << DECT_PT_HDR_LENGTH_SHIFT,
	DECT_PT_LONG_PAGE_FIRST		= 0x5ULL << DECT_PT_HDR_LENGTH_SHIFT,
	DECT_PT_LONG_PAGE_LAST		= 0x6ULL << DECT_PT_HDR_LENGTH_SHIFT,
	DECT_PT_LONG_PAGE_ALL		= 0x7ULL << DECT_PT_HDR_LENGTH_SHIFT,
};

/* zero and short page B_S channel data */
#define DECT_PT_SZP_BS_DATA_MASK	0x0fffff0000000000ULL
#define DECT_PT_SZP_BS_DATA_SHIFT	40
#define DECT_PT_SZP_BS_DATA_SIZE	3

/* long and full page B_S channel data */
#define DECT_PT_LFP_BS_DATA_MASK	0x0fffffffff000000ULL
#define DECT_PT_LFP_BS_DATA_SHIFT	24
#define DECT_PT_LFP_BS_DATA_SIZE	5

/* MAC layer information */
#define DECT_PT_INFO_TYPE_MASK		0x000000f000000000ULL
#define DECT_PT_INFO_TYPE_SHIFT		36
#define DECT_PT_INFO_TYPE_SIZE		2

/**
 * @DECT_PT_IT_FILL_BITS_OR_BLIND_LONG_SLOTS:	fill bits/blind long slots if bit 47 set
 * @DECT_PT_IT_BLIND_FULL_SLOT:			blind full slot information
 * @DECT_PT_IT_OTHER_BEARER:
 * @DECT_PT_IT_RECOMMENDED_OTHER_BEARER:
 * @DECT_PT_IT_GOOD_RFP_BEARER:
 * @DECT_PT_IT_DUMMY_OR_CL_BEARER_POSITION:
 * @DECT_PT_IT_RFP_IDENTITY:
 * @DECT_PT_IT_ESCAPE:
 * @DECT_PT_IT_DUMMY_OR_CL_BEARER_MARKER:
 * @DECT_PT_IT_BEARER_HANDOVER_INFO:
 * @DECT_PT_IT_RFP_STATUS:
 * @DECT_PT_IT_ACTIVE_CARRIERS:
 * @DECT_PT_IT_CL_BEARER_POSITION:
 * @DECT_PT_IT_RECOMMENDED_POWER_LEVEL:
 * @DECT_PT_IT_BLIND_DOUBLE_SLOT:
 * @DECT_PT_IT_BLIND_FULL_SLOT_PACKET_MODE:
 *
 */
enum dect_pt_info_types {
	DECT_PT_IT_FILL_BITS_OR_BLIND_LONG_SLOTS= 0x0ULL << DECT_PT_INFO_TYPE_SHIFT,
	DECT_PT_IT_BLIND_FULL_SLOT		= 0x1ULL << DECT_PT_INFO_TYPE_SHIFT,
	DECT_PT_IT_OTHER_BEARER			= 0x2ULL << DECT_PT_INFO_TYPE_SHIFT,
	DECT_PT_IT_RECOMMENDED_OTHER_BEARER	= 0x3ULL << DECT_PT_INFO_TYPE_SHIFT,
	DECT_PT_IT_GOOD_RFP_BEARER		= 0x4ULL << DECT_PT_INFO_TYPE_SHIFT,
	DECT_PT_IT_DUMMY_OR_CL_BEARER_POSITION	= 0x5ULL << DECT_PT_INFO_TYPE_SHIFT,
	DECT_PT_IT_RFP_IDENTITY			= 0x6ULL << DECT_PT_INFO_TYPE_SHIFT,
	DECT_PT_IT_ESCAPE			= 0x7ULL << DECT_PT_INFO_TYPE_SHIFT,
	DECT_PT_IT_DUMMY_OR_CL_BEARER_MARKER	= 0x8ULL << DECT_PT_INFO_TYPE_SHIFT,
	DECT_PT_IT_BEARER_HANDOVER_INFO		= 0x9ULL << DECT_PT_INFO_TYPE_SHIFT,
	DECT_PT_IT_RFP_STATUS			= 0xaULL << DECT_PT_INFO_TYPE_SHIFT,
	DECT_PT_IT_ACTIVE_CARRIERS		= 0xbULL << DECT_PT_INFO_TYPE_SHIFT,
	DECT_PT_IT_CL_BEARER_POSITION		= 0xcULL << DECT_PT_INFO_TYPE_SHIFT,
	DECT_PT_IT_RECOMMENDED_POWER_LEVEL	= 0xdULL << DECT_PT_INFO_TYPE_SHIFT,
	DECT_PT_IT_BLIND_DOUBLE_SLOT		= 0xeULL << DECT_PT_INFO_TYPE_SHIFT,
	DECT_PT_IT_BLIND_FULL_SLOT_PACKET_MODE	= 0xfULL << DECT_PT_INFO_TYPE_SHIFT,
};

/* blind full slot information */
#define DECT_PT_BFS_MASK		0x0000000fff000000ULL
#define DECT_PT_BFS_SHIFT		24

/* Bearer description */
#define DECT_PT_BEARER_SN_MASK		0x0000000f00000000ULL
#define DECT_PT_BEARER_SN_SHIFT		32

#define DECT_PT_BEARER_SP_MASK		0x00000000c0000000ULL
#define DECT_PT_BEARER_SP_SHIFT		30

#define DECT_PT_BEARER_CN_MASK		0x000000003f000000ULL
#define DECT_PT_BEARER_CN_SHIFT		24

/* RFP identity */
#define DECT_PT_RFP_ID_MASK		0x0000000fff000000ULL
#define DECT_PT_RFP_ID_SHIFT		24

/* RFP status */
#define DECT_PT_RFPS_RFP_BUSY_FLAG	0x0000000100000000ULL
#define DECT_PT_RFPS_SYS_BUSY_FLAG	0x0000000200000000ULL

/* Active carriers */
#define DECT_PT_ACTIVE_CARRIERS_MASK	0x0000000ffc000000ULL
#define DECT_PT_ACTIVE_CARRIERS_SHIFT	26

/*
 * B-Field
 */

#define DECT_B_FIELD_SIZE	40

/**
 * dect_b_identitifications - MAC layer B-Field Identification
 *
 * @DECT_BI_UTYPE_0:		U-Type, I_N, SI_N, SI_P or I_P packet number 0
 * @DECT_BI_UTYPE_1:		U-Type, I_P error detect or I_P packet number 1
 * @DECT_BI_ETYPE_CF_0:		E-Type, all C_F or CL_F, packet number 0
 * @DECT_BI_ETYPE_CF_1:		E-Type, all C_F, packet number 1
 * @DECT_BI_ETYPE_MAC:		E-Type, all MAC control (unnumbered)
 * @DECT_BI_NONE:		no B-Field
 */
enum dect_b_identifications {
	DECT_BI_UTYPE_0		= 0x0 << DECT_HDR_BA_SHIFT,
	DECT_BI_UTYPE_1		= 0x1 << DECT_HDR_BA_SHIFT,
	DECT_BI_ETYPE_CF_0	= 0x2 << DECT_HDR_BA_SHIFT,
	DECT_BI_ETYPE_CF_1	= 0x3 << DECT_HDR_BA_SHIFT,
	DECT_BI_ETYPE_MAC	= 0x6 << DECT_HDR_BA_SHIFT,
	DECT_BI_NONE		= 0x7 << DECT_HDR_BA_SHIFT,
};

#endif /* _MAC_H */