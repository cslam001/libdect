/*
 * DECT authentication functions
 *
 * Copyright (c) 2009-2010 Patrick McHardy <kaber@trash.net>
 */

#ifndef _LIBDECT_DECT_AUTH_H
#define _LIBDECT_DECT_AUTH_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup security
 * @{
 */

#define DECT_AUTH_CODE_LEN	4	/**< Length of authentication code */
#define DECT_UPI_LEN		4	/**< Length of User Personal Identification */
#define DECT_AUTH_KEY_LEN	16	/**< Length of a DECT authentication key */
#define DECT_CIPHER_KEY_LEN	8	/**< Length of a DECT cipher key */

/** DECT authentication algorithm */
struct dect_aalg {
	uint8_t		type;						/**< algorithm ID */
	unsigned int	d1_len;						/**< length of input D1 */
	unsigned int	d2_len;						/**< length of input D2 */
	void		(*calc)(const uint8_t *d1, const uint8_t *d2,
				uint8_t *e);				/**< function to calculate output E */
};

extern const struct dect_aalg dect_dsaa_aalg;

extern void dect_pin_to_ac(const char *pin, uint8_t *ac, unsigned int ac_len);

extern void dect_auth_b1(const uint8_t *val, unsigned int len, uint8_t *k);
extern void dect_auth_b2(const uint8_t *uak, unsigned int uak_len,
			 const uint8_t *upi, unsigned int upi_len, uint8_t *k);

extern void dect_auth_a11(const uint8_t *k, uint64_t rs, uint8_t *ks);
extern void dect_auth_a12(const uint8_t *ks, uint64_t rand_f, uint8_t *dck,
			  uint32_t *res1);
extern void dect_auth_a21(const uint8_t *k, uint64_t rs, uint8_t *ks);
extern void dect_auth_a22(const uint8_t *ks, uint64_t rand_p, uint32_t *res2);

/** @} */

#ifdef __cplusplus
}
#endif
#endif /* _LIBDECT_DECT_AUTH_H */
