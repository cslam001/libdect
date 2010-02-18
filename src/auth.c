/*
 * DECT Security processes and related functions
 *
 * Copyright (c) 2009 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <asm/byteorder.h>

#include <dect/auth.h>
#include <libdect.h>
#include <utils.h>

/**
 * dect_pin_to_ac - convert PIN to authentication code
 *
 * @pin:	PIN code
 * @ac:		buffer to store authentication code
 * @ac_len:	length of destination buffer
 *
 * Convert a PIN to an authentication code, which is the BCD encoded PIN,
 * left-padded with 0xf. The number of PIN digits must not exceed 2 * ac_len.
 */
void dect_pin_to_ac(const char *pin, uint8_t *ac, unsigned int ac_len)
{
	unsigned int i, shift, len;

	memset(ac, 0xff, ac_len);

	len = strlen(pin);
	for (i = 0; i < len; i++) {
		shift = i & 0x1 ? 4 : 0;
		ac[ac_len - i / 2 - 1] &= ~(0xf << shift);
		ac[ac_len - i / 2 - 1] |= (pin[len - i - 1] - '0') << shift;
	}
}

/**
 * dect_auth_b1 - derive authentication key K from UAK/AC
 *
 * @val:	user authentication key (UAK) or authentication code (AC)
 * @len:	length of UAK/AC
 * @k:		buffer to store authentication key of size DECT_AUTH_KEY_LEN
 *
 * Derive an authentication key from a user authentication key or an
 * authentication code.
 */
void dect_auth_b1(const uint8_t *val, unsigned int len, uint8_t *k)
{
	unsigned int i;

	for (i = 0; i < DECT_AUTH_KEY_LEN; i++)
		k[i] = val[i % len];
}

/**
 * dect_auth_b2 - derive authentication key K from UAK and UPI
 *
 * @uak:	user authentication key (UAK)
 * @uak_len:	length of UAK
 * @upi:	user personal identity (UPI)
 * @upi_len:	length of UPI
 * @k:		buffer to store authentication key of size DECT_AUTH_KEY_LEN
 *
 * Derive an authentication key from a user authentication key and an user
 * personal identity.
 */
void dect_auth_b2(const uint8_t *uak, unsigned int uak_len,
		  const uint8_t *upi, unsigned int upi_len, uint8_t *k)
{
	unsigned int i;

	for (i = 0; i < DECT_AUTH_KEY_LEN; i++)
		k[i] = uak[i % uak_len] ^ upi[i % upi_len];
}

static void dect_auth_calc(const uint8_t *key, uint64_t val, uint8_t *e)
{
	const struct dect_aalg *aalg = &dect_dsaa_aalg;
	uint8_t d1[aalg->d1_len];
	uint8_t d2[aalg->d2_len];
	unsigned int i;

	for (i = 0; i < aalg->d1_len; i++)
		d1[i] = key[i % DECT_AUTH_KEY_LEN];
	for (i = 0; i < aalg->d2_len; i++)
		d2[i] = val >> (i % sizeof(val) * 8);

	aalg->calc(d1, d2, e);
}

/**
 * dect_auth_a11 - derive authentication session key
 *
 * @k:		authentication key K
 * @rs:		random seed
 * @ks:		buffer to store session authentication key of size DECT_AUTH_KEY_LEN
 *
 * Derive the session authentication keys KS from the authentication key K
 * and random seed RS.
 */
void dect_auth_a11(const uint8_t *k, uint64_t rs, uint8_t *ks)
{
	dect_auth_calc(k, rs, ks);
}

/**
 * dect_auth_a12 - derive cipher key and authentication response
 *
 * @ks:		session authentication key KS
 * @rand_f:	FP random value
 * @dck:	buffer to store derived cipher key (DCK) of size DECT_CIPHER_KEY_LEN
 * @res1:	buffer to store authentication response
 *
 * Derive the derived cipher key DCK and authentication response RES1 from the
 * session authentication key KS and the random value rand_f.
 */
void dect_auth_a12(const uint8_t *ks, uint64_t rand_f, uint8_t *dck, uint32_t *res1)
{
	uint8_t e[DECT_AUTH_KEY_LEN];

	dect_auth_calc(ks, rand_f, e);
	memcpy(dck, e + 4, DECT_CIPHER_KEY_LEN);
	memcpy(res1, e + 12, DECT_AUTH_RES_LEN);
}

/**
 * dect_auth_a21 - derive authentication session key
 *
 * @k:		authentication key K
 * @rs:		random seed
 * @ks:		buffer to store session authentication key of size DECT_AUTH_KEY_LEN
 *
 * Derive the session authentication keys KS' from the authentication key K
 * and random seed RS.
 */
void dect_auth_a21(const uint8_t *k, uint64_t rs, uint8_t *ks)
{
	unsigned int i;

	dect_auth_a11(k, rs, ks);
	for (i = 0; i < DECT_AUTH_KEY_LEN; i++)
		ks[i] ^= 0xaa;
}

/**
 * dect_auth_a22 - derive authentication response
 *
 * @ks:		session authentication key KS'
 * @rand_p:	PP random value
 * @res2:	buffer to store authentication response
 *
 * Derive the authentication response RES2 from the session authentication
 * key KS' and the random value rand_p.
 */
void dect_auth_a22(const uint8_t *ks, uint64_t rand_p, uint32_t *res2)
{
	uint8_t e[DECT_AUTH_KEY_LEN];

	dect_auth_calc(ks, rand_p, e);
	memcpy(res2, e + 12, DECT_AUTH_RES_LEN);
}

/*
 * DSAA/DSC key allocation test from ETS EN 300 175-7 Annex K
 */
#define DECT_AUTH_TEST_RS1		__cpu_to_be64(0)
#define DECT_AUTH_TEST_RAND1		__cpu_to_be64(0xd01ff9e211680421ULL)
#define DECT_AUTH_TEST_RES1		0xf28cf911

#define DECT_AUTH_TEST_RAND2		__cpu_to_be64(0xa03ff2c523d00842ULL)
#define DECT_AUTH_TEST_RES2		0xeeba4db9

#define DECT_AUTH_TEST_RS2		__cpu_to_be64(0)
#define DECT_AUTH_TEST_RAND3		__cpu_to_be64(0x9b114f2554518859ULL)
#define DECT_AUTH_TEST_RES3		0x961b3a9f
#define DECT_AUTH_TEST_DCK		0x8560dc324ea49f37ULL

static void __init dect_auth_test(void)
{
	uint8_t k[DECT_AUTH_KEY_LEN];
	uint8_t ks[DECT_AUTH_KEY_LEN], ks_[DECT_AUTH_KEY_LEN];
	union {
		uint8_t key[DECT_CIPHER_KEY_LEN];
		uint64_t val;
	} dck;
	uint32_t res1, res2;
	uint8_t ac[4];

	/* Authentication code "9124" */
	dect_pin_to_ac("9124", ac, sizeof(ac));
	dect_auth_b1(ac, sizeof(ac), k);

	/* FT auth request */
	dect_auth_a11(k, DECT_AUTH_TEST_RS1, ks);
	dect_auth_a12(ks, DECT_AUTH_TEST_RAND1, dck.key, &res1);

	if (__be32_to_cpu(res1) != DECT_AUTH_TEST_RES1)
		printf("dect_auth_test: fail1 res1=%.8x\n", res1);

	/* PT auth request + UAK allocation (UAK = KS') */
	dect_auth_a21(k, DECT_AUTH_TEST_RS1, ks_);
	dect_auth_a22(ks_, DECT_AUTH_TEST_RAND2, &res2);

	if (__cpu_to_be32(res2) != DECT_AUTH_TEST_RES2)
		printf("dect_auth_test: fail2 res2=%.8x\n", res2);

	/* DCK allocation using UAK */
	dect_auth_a11(ks_, DECT_AUTH_TEST_RS2, ks_);
	dect_auth_a12(ks_, DECT_AUTH_TEST_RAND3, dck.key, &res1);

	if (__cpu_to_be32(res1) != DECT_AUTH_TEST_RES3)
		printf("dect_auth_test: fail3 res1=%.8x\n", res1);
	if (__cpu_to_be64(dck.val) != DECT_AUTH_TEST_DCK)
		printf("dect_auth_test: fail4 dck=%.16" PRIx64 "\n",
		       (uint64_t)__cpu_to_be64(dck.val));
}
