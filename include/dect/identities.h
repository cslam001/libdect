/*
 * DECT Identities
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_DECT_IDENTITIES_H
#define _LIBDECT_DECT_IDENTITIES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @addtogroup identity
 * @{
 * @defgroup identity_ari Access Rights Identity (ARI)
 *
 * @sa ETSI EN 300 175-6 (DECT Common Interface - Identities and addressing),
 *     section 5
 * @{
 */

/**
 * DECT ARI classes
 */
enum dect_ari_classes {
	DECT_ARC_A		= 0x0, /**< Residential and private (PBX) single- and small multiple cell systems */
	DECT_ARC_B		= 0x1, /**< Private (PABXs) multiple cell */
	DECT_ARC_C		= 0x2, /**< Public single and multiple cell systems */
	DECT_ARC_D		= 0x3, /**< Public DECT access to a GSM network */
	DECT_ARC_E		= 0x4, /**< PP to PP direct communication (private) */
};

/**
 * DECT Access Rights Identifier
 */
struct dect_ari {
	enum dect_ari_classes	arc;
	uint32_t		fpn;
	uint32_t		fps;
	union {
		uint16_t	emc;
		uint16_t	eic;
		uint16_t	poc;
		uint32_t	gop;
		uint16_t	fil;
	};
};

enum dect_ari_lengths {
	DECT_ARC_A_LEN		= 36,
	DECT_ARC_B_LEN		= 31,
	DECT_ARC_C_LEN		= 31,
	DECT_ARC_D_LEN		= 31,
	DECT_ARC_E_LEN		= 31,
};

extern bool dect_ari_cmp(const struct dect_ari *a1, const struct dect_ari *a2);

/**
 * @}
 * @defgroup identity_park Portable Access Rights Key (PARK)
 *
 * @sa ETSI EN 300 175-6 (DECT Common Interface - Identities and addressing),
 *     section 6.1
 *
 * @{
 */

/**
 * DECT Portable access rights key
 *
 * @arg park	FP ARI
 * @arg pli	FP ARI prefix length
 */
struct dect_park {
	struct dect_ari		park;
	uint8_t			pli;
};

/**
 * @}
 * @defgroup identity_ipui International Portable User ID (IPUI)
 *
 * @sa ETSI EN 300 175-6 (DECT Common Interface - Identities and addressing),
 *     section 6.2
 *
 * @{
 */

/**
 * DECT International portable equipment ID
 *
 * @arg emc	Equipment Manufacturer Code
 * @arg psn	Portable Equipment Serial Number
 */
struct dect_ipei {
	uint16_t	emc;
	uint32_t	psn;
};

/* IPUI */

#define DECT_IPUI_PUT_MASK		0xf0
#define DECT_IPUI_PUT_SHIFT		4

/**
 * DECT International portable User ID types
 */
enum dect_ipui_types {
	DECT_IPUI_N	= 0x0 << DECT_IPUI_PUT_SHIFT, /**< Portable user identity type N (residential/default) */
	DECT_IPUI_O	= 0x1 << DECT_IPUI_PUT_SHIFT, /**< Portable user identity type O (private) */
	DECT_IPUI_P	= 0x2 << DECT_IPUI_PUT_SHIFT, /**< Portable user identity type P (public/public access service) */
	DECT_IPUI_Q	= 0x3 << DECT_IPUI_PUT_SHIFT, /**< Portable user identity type Q (public/general) */
	DECT_IPUI_R	= 0x4 << DECT_IPUI_PUT_SHIFT, /**< Portable user identity type R (public/IMSI) */
	DECT_IPUI_S	= 0x5 << DECT_IPUI_PUT_SHIFT, /**< Portable user identity type S (PSTN/ISDN) */
	DECT_IPUI_T	= 0x6 << DECT_IPUI_PUT_SHIFT, /**< Portable user identity type T (private extended) */
	DECT_IPUI_U	= 0x7 << DECT_IPUI_PUT_SHIFT, /**< Portable user identity type U (public/general) */
};

/**
 * DECT International portable User ID
 *
 * @arg put	Portable User Identity Type
 * @arg pun	Type specific data
 */
struct dect_ipui {
	enum dect_ipui_types	put;
	unsigned int		len;
	union {
		struct {
			struct dect_ipei	ipei;
		} n;
		struct {
			uint64_t		number;
		} o;
		struct {
			uint16_t		poc;
			uint8_t			acc[10];
		} p;
		struct {
			uint8_t			bacn[10];
		} q;
		struct {
			uint64_t		imsi;
		} r;
		struct {
			uint64_t		number;
		} s;
		struct {
			uint16_t		eic;
			uint64_t		number;
		} t;
		struct {
			uint8_t			cacn[10];
		} u;
	} pun;
};

extern bool dect_ipui_cmp(const struct dect_ipui *u1,
			  const struct dect_ipui *u2);

/**
 * @}
 * @defgroup identity_tpui Temporary Portable User ID (TPUI)
 *
 * @sa ETSI EN 300 175-6 (DECT Common Interface - Identities and addressing),
 *     section 6.3
 *
 * @{
 */

/**
 * DECT Temporary User ID types
 */
enum dect_tpui_types {
	DECT_TPUI_INDIVIDUAL_ASSIGNED,	/**< Assigned individual TPUI */
	DECT_TPUI_CONNECTIONLESS_GROUP, /**< Connectionless group TPUI */
	DECT_TPUI_CALL_GROUP,		/**< Call group TPUI */
	DECT_TPUI_INDIVIDUAL_DEFAULT,	/**< Default individual TPUI */
	DECT_TPUI_EMERGENCY,		/**< Emergency TPUI */
};

/**
 * DECT Temporary Portable User ID
 *
 * @arg type	TPUI type
 * @arg tpui	type specific value (12/16/20 bits)
 */
struct dect_tpui {
	enum dect_tpui_types	type;
	union {
		struct {
			uint8_t	digits[5];
		} ia;
		struct {
			uint16_t group;
		} cg;
		struct {
			uint16_t ipui;
		} id;
	};
};

extern struct dect_tpui *dect_ipui_to_tpui(struct dect_tpui *tpui,
					   const struct dect_ipui *ipui);
extern void dect_dump_tpui(const struct dect_tpui *tpui);

/* Collective broadcast identifier */
#define DECT_TPUI_CBI		0xcfff

/** @} */
/** @} */

#ifdef __cplusplus
}
#endif
#endif /* _LIBDECT_DECT_IDENTITIES_H */
