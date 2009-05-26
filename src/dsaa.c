/*
 * DECT Standard Authentication Algorithm
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Copyright (c) 2009 Matthias Wenzel <dect at mazzoo dot de>
 *
 */

#include <stdlib.h>
#include <stdarg.h>
#include <libdect.h>

static const uint8_t sbox[256] = {
	176, 104, 111, 246, 125, 232,  22, 133,
	57,  124, 127, 222,  67, 240,  89, 169,
	251, 128,  50, 174,  95,  37, 140, 245,
	148, 107, 216, 234, 136, 152, 194,  41,
	207,  58,  80, 150,  28,   8, 149, 244,
	130,  55,  10,  86,  44, 255,  79, 196,
	96,  165, 131,  33,  48, 248, 243,  40,
	250, 147,  73,  52,  66, 120, 191, 252,
	97,  198, 241, 167,  26,  83,   3,  77,
	134, 211,   4, 135, 126, 143, 160, 183,
	49,  179, 231,  14,  47, 204, 105, 195,
	192, 217, 200,  19, 220, 139,   1,  82,
	193,  72, 239, 175, 115, 221,  92,  46,
	25,  145, 223,  34, 213,  61,  13, 163,
	88,  129,  62, 253,  98,  68,  36,  45,
	182, 141,  90,   5,  23, 190,  39,  84,
	93,  157, 214, 173, 108, 237, 100, 206,
	242, 114,  63, 212,  70, 164,  16, 162,
	59,  137, 151,  76, 110, 116, 153, 228,
	227, 187, 238, 112,   0, 189, 101,  32,
	15,  122, 233, 158, 155, 199, 181,  99,
	230, 170, 225, 138, 197,   7,   6,  30,
	94,   29,  53,  56, 119,  20,  17, 226,
	185, 132,  24, 159,  42, 203, 218, 247,
	166, 178, 102, 123, 177, 156, 109, 106,
	249, 254, 202, 201, 168,  65, 188, 121,
	219, 184, 103, 186, 172,  54, 171, 146,
	75,  215, 229, 154, 118, 205,  21,  31,
	78,   74,  87, 113,  27,  85,   9,  81,
	51,   12, 180, 142,  43, 224, 208,  91,
	71,  117,  69,  64,   2, 209,  60, 236,
	35,  235,  11, 210, 161, 144,  38,  18,
};

static void bitperm(uint8_t start, uint8_t step, uint8_t * key)
{
	static uint8_t copy[8];
	unsigned int i;

	memcpy(copy, key, 8);
	memset(key, 0, 8);

	for (i = 0; i < 64; i++) {
		key[start/8] |= ((copy[i / 8] & (1 << (i % 8))) >>
				(i % 8)) << (start % 8);
		start += step;
		start %= 64;
	}
}

#if 0
static void bitperm1(uint8_t * key)
{
	bitperm(46, 35, key);
}

static void bitperm2(uint8_t * key)
{
	bitperm(25, 47, key);
}

static void bitperm3(uint8_t * key)
{
	bitperm(60, 27, key);
}

static void bitperm4(uint8_t * key)
{
	bitperm(55, 39, key);
}
#endif

static const uint8_t mix_factor[3][8] = {
	{2, 2, 2, 2, 3, 3, 3, 3},
	{2, 2, 3, 3, 2, 2, 3, 3},
	{2, 3, 2, 3, 2, 3, 2, 3},
};

static const uint8_t mix_index[3][8] = {
	{4, 5, 6, 7, 0, 1, 2, 3},
	{2, 3, 0, 1, 6, 7, 4, 5},
	{1, 0, 3, 2, 5, 4, 7, 6},
};

static void mix(uint8_t start, uint8_t alg, uint8_t * key)
{
	unsigned int i;
	uint8_t copy[8];

	memcpy(copy, key, 8);
	for (i=0; i<8; i++)
		key[i] = copy[mix_index[alg][i]] + mix_factor[alg][i] * copy[i];
}

static void mix1(uint8_t * key)
{
	mix(4, 0, key);
}

static void mix2(uint8_t * key)
{
	mix(2, 1, key);
}

static void mix3(uint8_t * key)
{
	mix(1, 2, key);
}

static void sub(uint8_t * s, uint8_t * t)
{
	unsigned int i;

	for (i = 0; i < 8; i++)
		s[i] = sbox[s[i] ^ t[i]];
}

/* return in s */
static void cassable(uint8_t start, uint8_t step, uint8_t * t, uint8_t * s)
{
	unsigned int i;

	for(i = 0; i < 2; i++) {
		bitperm(start, step, t);
		sub(s, t);
		mix1(s);

		bitperm(start, step, t);
		sub(s, t);
		mix2(s);

		bitperm(start, step, t);
		sub(s, t);
		mix3(s);
	}
}

/* return in rand, modifies key */
static void step1(uint8_t * rand, uint8_t * key)
{

	uint8_t tmp[8];

	memcpy(tmp, rand, 8);

	cassable(46, 35, tmp, key);
	cassable(25, 47, key, rand);

	memcpy(key, rand, 8);
}

static void step2(uint8_t * rand, uint8_t * key)
{

	uint8_t tmp[8];

	memcpy(tmp, rand, 8);

	cassable(60, 27, tmp, key);
	cassable(55, 39, key, rand);

	memcpy(key, rand, 8);
}

static void rev(uint8_t * v, uint8_t n)
{
	unsigned int i;
	uint8_t tmp;

	for (i = 0; i < n / 2; i++) {
		tmp	     = v[i];
		v[i]	     = v[n - i - 1];
		v[n - i - 1] = tmp;
	}
}

void dsaa_main(uint8_t * rand, uint8_t * key, uint8_t * out);
void dsaa_main(uint8_t * rand, uint8_t * key, uint8_t * out)
{
	uint8_t a[8];
	uint8_t b[8];

	rev(rand, 8);
	rev(key, 16);

	step1(rand, key + 4);

	memcpy(a, key + 4, 8);

	memcpy(key + 4, key + 12, 4);
	memcpy(b, a, 8);
	step2(b, key);

	rev(a, 8);
	rev(key, 4);
	rev(key + 4, 4);

	memcpy(out, key + 4, 4);
	memcpy(out + 4, a, 8);
	memcpy(out + 12, key, 4);
}
