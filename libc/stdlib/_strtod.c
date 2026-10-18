/*
 * Copyright (C) 2000-2005     Manuel Novoa III
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 * dec_to_fpmax() is derived from decfloat() in musl's
 * src/internal/floatscan.c, which carries this notice:
 *
 * Copyright © 2005-2020 Rich Felker, et al.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* Notes:
 *
 * All the strto{floating pt}() functions, and scanf, go through the internal
 * __strtofpmax(), which recognizes "nan", "inf", "infinity", hexadecimal
 * floating point notation, the locale's decimal point and wide characters.
 *
 * The conversion is exact: the digits are kept as integers, scaled with
 * integer arithmetic and rounded once, to nearest-even, at the precision of
 * the type that was asked for (prec: 0 float, 1 double, 2 __fpmax_t).  Only
 * then is the value built in __fpmax_t, by multiplying with powers of two,
 * which cannot round.  The decimal scaling follows musl's decfloat, see
 * the notice above.
 */

/**********************************************************************/
/*                      OPTIONS                                       */
/**********************************************************************/

/* Defined if we want to recognize "nan", "inf", and "infinity". (C99) */
#define _STRTOD_NAN_INF_STRINGS  1

/* Defined if we want support hexadecimal floating point notation. (C99) */
/* Note!  Now controlled by uClibc configuration.  See below. */
#define _STRTOD_HEXADECIMAL_FLOATS 1

/**********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <bits/uClibc_fpmax.h>

#include <locale.h>

#ifdef __UCLIBC_HAS_WCHAR__
# include <wchar.h>
# include <wctype.h>
# include <bits/uClibc_uwchar.h>
#endif

/* Handle _STRTOD_HEXADECIMAL_FLOATS via uClibc config now. */
#undef _STRTOD_HEXADECIMAL_FLOATS
#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
# define _STRTOD_HEXADECIMAL_FLOATS 1
#endif

/**********************************************************************/

#undef _STRTOD_FPMAX

#if FPMAX_TYPE == 3

#define NEED_STRTOLD_WRAPPER
#define NEED_STRTOD_WRAPPER
#define NEED_STRTOF_WRAPPER

#elif FPMAX_TYPE == 2

#define NEED_STRTOD_WRAPPER
#define NEED_STRTOF_WRAPPER

#elif FPMAX_TYPE == 1

#define NEED_STRTOF_WRAPPER

#else

#error unknown FPMAX_TYPE!

#endif

extern void __fp_range_check(__fpmax_t y, __fpmax_t x) attribute_hidden;

/**********************************************************************/
#if defined(L___strtofpmax) || defined(L___strtofpmax_l) || defined(L___wcstofpmax) || defined(L___wcstofpmax_l)

#if defined(L___wcstofpmax) || defined(L___wcstofpmax_l)

#define __strtofpmax    __wcstofpmax
#define __strtofpmax_l  __wcstofpmax_l

#define Wchar wchar_t
#ifdef __UCLIBC_DO_XLOCALE
#define ISSPACE(C) iswspace_l((C), locale_arg)
#else
#define ISSPACE(C) iswspace((C))
#endif

#else  /* defined(L___wcstofpmax) || defined(L___wcstofpmax_l) */

#define Wchar char
#ifdef __UCLIBC_DO_XLOCALE
#define ISSPACE(C) isspace_l((C), locale_arg)
#else
#define ISSPACE(C) isspace((C))
#endif

#endif /* defined(L___wcstofpmax) || defined(L___wcstofpmax_l) */


#if defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE)

__fpmax_t attribute_hidden __strtofpmax(const Wchar *str, Wchar **endptr,
										int exponent_power, int prec)
{
	return __strtofpmax_l(str, endptr, exponent_power, prec, __UCLIBC_CURLOCALE);
}

#else  /* defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE) */

#if FPMAX_MAX_EXP < -FPMAX_MIN_EXP
#error scaling code can not handle FPMAX_MAX_EXP < -FPMAX_MIN_EXP
#endif

/* 2^(2^i), up to the largest that is still finite. */
static const __fpmax_t pow2_table[] = {
	0x1.0p1L, 0x1.0p2L, 0x1.0p4L, 0x1.0p8L, 0x1.0p16L, 0x1.0p32L, 0x1.0p64L,
#if FPMAX_MAX_EXP > 128
	0x1.0p128L,
#endif
#if FPMAX_MAX_EXP > 256
	0x1.0p256L,
#endif
#if FPMAX_MAX_EXP > 512
	0x1.0p512L,
#endif
#if FPMAX_MAX_EXP > 1024
	0x1.0p1024L,
#endif
#if FPMAX_MAX_EXP > 2048
	0x1.0p2048L,
#endif
#if FPMAX_MAX_EXP > 4096
	0x1.0p4096L,
#endif
#if FPMAX_MAX_EXP > 8192
	0x1.0p8192L,
#endif
#if FPMAX_MAX_EXP > 16384
#error unsupported FPMAX_MAX_EXP.  please increase table
#endif
};
#define POW2_TABLE_SIZE (sizeof(pow2_table) / sizeof(pow2_table[0]))

/* The integer that gets rounded: up to 160 bits, more than the 113 of the
 * widest __fpmax_t plus what rounding needs. */
#define MW 5

static int mw_bitlen(const uint32_t *m)
{
	int k = MW, n;
	uint32_t v;

	while (k && !m[k - 1])
		k--;
	if (!k)
		return 0;
	n = 32 * (k - 1);
	for (v = m[k - 1]; v; v >>= 1)
		n++;
	return n;
}

static int mw_bit(const uint32_t *m, int b)
{
	return b >= 0 && b < 32 * MW && ((m[b / 32] >> (b % 32)) & 1);
}

/* Is any bit below bit b set? */
static int mw_any_below(const uint32_t *m, int b)
{
	int k;

	if (b > 32 * MW)
		b = 32 * MW;
	for (k = 0; k < b / 32; k++)
		if (m[k])
			return 1;
	return (b % 32) && (m[b / 32] & ((1u << (b % 32)) - 1));
}

static void mw_shr(uint32_t *m, int s)
{
	int k, ws = s / 32, bs = s % 32;

	for (k = 0; k < MW; k++) {
		uint32_t lo = k + ws < MW ? m[k + ws] : 0;
		uint32_t hi = k + ws + 1 < MW ? m[k + ws + 1] : 0;

		m[k] = bs ? lo >> bs | hi << (32 - bs) : lo;
	}
}

/* m = m * mul + add */
static void mw_mul_add(uint32_t *m, uint32_t mul, uint32_t add)
{
	uint64_t c = add;
	int k;

	for (k = 0; k < MW; k++) {
		c += (uint64_t) m[k] * mul;
		m[k] = (uint32_t) c;
		c >>= 32;
	}
}

/* The value m * 2^e2, plus a tail below the last bit of m (0 none, 1 less
 * than half of it, 2 exactly half, 3 more), rounded to nearest-even with
 * bits significant bits; emin is the exponent of the smallest subnormal.
 * Sets ERANGE on overflow and on an inexact subnormal or zero result. */
static __fpmax_t round_to_fpmax(uint32_t *m, int tail, long e2,
								int bits, int emin, int neg)
{
	__fpmax_t v = 0;
	int nb = mw_bitlen(m), inexact = tail != 0, up;
	long top, b, drop;
	int i, k;

	if (!nb)
		goto SIGN;

	top = e2 + nb - 1;			/* exponent of the leading bit */
	b = bits;
	if (top - (bits - 1) < emin) {	/* subnormal: fewer bits */
		b = top - emin + 1;
		if (b < -1)
			b = -1;
	}
	drop = nb - b;
	if (drop > 0) {
		int r = mw_bit(m, drop - 1);
		int lower = tail || mw_any_below(m, drop - 1);

		inexact = r || lower;
		mw_shr(m, drop);
		e2 += drop;
		up = r && (lower || (m[0] & 1));
	} else {
		up = tail == 3 || (tail == 2 && (m[0] & 1));
	}
	if (up)
		mw_mul_add(m, 1, 1);

	nb = mw_bitlen(m);
	if (!nb) {
		if (inexact)
			__set_errno(ERANGE);
		goto SIGN;
	}
	top = e2 + nb - 1;
	if (top > 2 - emin - bits) {	/* above MAX_EXP - 1 */
		__set_errno(ERANGE);
		v = (__fpmax_t) __builtin_huge_vall();
		goto SIGN;
	}
	if (inexact && top < emin + bits - 1)	/* below MIN_EXP - 1 */
		__set_errno(ERANGE);

	for (k = MW; k--; )			/* exact: at most bits + 1 bits */
		v = v * 4294967296.0 + m[k];
	for (i = POW2_TABLE_SIZE; i--; ) {	/* exact: powers of two */
		long p = 1L << i;

		while (e2 >= p) {
			v *= pow2_table[i];
			e2 -= p;
		}
		while (e2 <= -p) {
			v /= pow2_table[i];
			e2 += p;
		}
	}
 SIGN:
	return neg ? -v : v;
}

/* 2^P - 1 in base 1e9 for the scaling precision P: 53, 64 and 113 bits. */
static const uint32_t th53[] = { 9007199, 254740991 };
static const uint32_t th64[] = { 18, 446744073, 709551615 };
static const uint32_t th113[] = { 10384593, 717069655, 257060992, 658440191 };

/* 0.d1d2d3... * 10^lrp, with the significant digits in x[0..k] (nine per
 * limb, j of them in x[k]) and dc of them in all.  This is musl's decfloat
 * from where the digits have been read, with the floating point assembly
 * replaced by round_to_fpmax(). */
static __fpmax_t dec_to_fpmax(uint32_t *x, int kmax, int k, int j,
							  long long lrp, long long dc,
							  int bits, int emin, int neg)
{
	static const int p10s[] = { 10, 100, 1000, 10000,
		100000, 1000000, 10000000, 100000000 };
	const uint32_t *th;
	int b1b, mask = kmax - 1;
	int i, a, z, rp;
	long e2;
	uint32_t m[MW];
	int tail = 0;

	if (bits <= 53) {
		th = th53;
		b1b = 2;
	} else if (bits <= 64) {
		th = th64;
		b1b = 3;
	} else {
		th = th113;
		b1b = 4;
	}
	memset(m, 0, sizeof(m));

	if (!x[0])
		return neg ? -(__fpmax_t) 0 : 0;

	/* small integers are exact */
	if (lrp == dc && dc < 10 && (bits > 30 || x[0] >> bits == 0)) {
		m[0] = x[0];
		return round_to_fpmax(m, 0, 0, bits, emin, neg);
	}
	if (lrp > -emin / 2) {
		__set_errno(ERANGE);
		return neg ? -(__fpmax_t) __builtin_huge_vall()
			: (__fpmax_t) __builtin_huge_vall();
	}
	if (lrp < emin - 2 * 113) {
		__set_errno(ERANGE);
		return neg ? -(__fpmax_t) 0 : 0;
	}

	/* Align incomplete final B1B digit */
	if (j) {
		for (; j < 9; j++)
			x[k] *= 10;
		k++;
		j = 0;
	}

	a = 0;
	z = k;
	e2 = 0;
	rp = lrp;

	/* Drop trailing zeros */
	for (; !x[z - 1]; z--)
		;

	/* Align radix point to B1B digit boundary */
	if (rp % 9) {
		int rpm9 = rp >= 0 ? rp % 9 : rp % 9 + 9;
		int p10 = p10s[8 - rpm9];
		uint32_t carry = 0;
		int kk;

		for (kk = a; kk != z; kk++) {
			uint32_t tmp = x[kk] % p10;

			x[kk] = x[kk] / p10 + carry;
			carry = 1000000000 / p10 * tmp;
			if (kk == a && !x[kk]) {
				a = (a + 1) & mask;
				rp -= 9;
			}
		}
		if (carry)
			x[z++] = carry;
		rp += 9 - rpm9;
	}

	/* Upscale until desired number of bits are left of radix point */
	while (rp < 9 * b1b || (rp == 9 * b1b && x[a] < th[0])) {
		uint32_t carry = 0;
		int kk;

		e2 -= 29;
		for (kk = (z - 1) & mask; ; kk = (kk - 1) & mask) {
			uint64_t tmp = ((uint64_t) x[kk] << 29) + carry;

			if (tmp >= 1000000000) {
				carry = tmp / 1000000000;
				x[kk] = tmp % 1000000000;
			} else {
				carry = 0;
				x[kk] = tmp;
			}
			if (kk == ((z - 1) & mask) && kk != a && !x[kk])
				z = kk;
			if (kk == a)
				break;
		}
		if (carry) {
			rp += 9;
			a = (a - 1) & mask;
			if (a == z) {
				z = (z - 1) & mask;
				x[(z - 1) & mask] |= x[z];
			}
			x[a] = carry;
		}
	}

	/* Downscale until exactly number of bits are left of radix point */
	for (;;) {
		uint32_t carry = 0;
		int sh = 1, kk;

		for (i = 0; i < b1b; i++) {
			kk = (a + i) & mask;
			if (kk == z || x[kk] < th[i]) {
				i = b1b;
				break;
			}
			if (x[(a + i) & mask] > th[i])
				break;
		}
		if (i == b1b && rp == 9 * b1b)
			break;
		if (rp > 9 + 9 * b1b)
			sh = 9;
		e2 += sh;
		for (kk = a; kk != z; kk = (kk + 1) & mask) {
			uint32_t tmp = x[kk] & ((1 << sh) - 1);

			x[kk] = (x[kk] >> sh) + carry;
			carry = (1000000000 >> sh) * tmp;
			if (kk == a && !x[kk]) {
				a = (a + 1) & mask;
				i--;
				rp -= 9;
			}
		}
		if (carry) {
			if (((z + 1) & mask) != a) {
				x[z] = carry;
				z = (z + 1) & mask;
			} else {
				x[(z - 1) & mask] |= 1;
			}
		}
	}

	/* The integer part, b1b limbs, and what is left as the tail */
	for (i = 0; i < b1b; i++) {
		if (((a + i) & mask) == z) {
			x[z] = 0;
			z = (z + 1) & mask;
		}
		mw_mul_add(m, 1000000000, x[(a + i) & mask]);
	}
	if (((a + i) & mask) != z) {
		uint32_t t = x[(a + i) & mask];
		int more = ((a + i + 1) & mask) != z;

		if (t < 500000000)
			tail = (t || more) ? 1 : 0;
		else if (t > 500000000)
			tail = 3;
		else
			tail = more ? 3 : 2;
	}

	return round_to_fpmax(m, tail, e2, bits, emin, neg);
}

#ifdef __UCLIBC_HAS_LOCALE__
#if defined(L___wcstofpmax) || defined(L___wcstofpmax_l)
#define IS_DECPT(P)	(*(P) == decpt_wc)
#define DECPT_LEN	1
#else
#define IS_DECPT(P)	(!memcmp((P), decpt, decpt_len))
#define DECPT_LEN	decpt_len
#endif
#else
#define IS_DECPT(P)	(*(P) == '.')
#define DECPT_LEN	1
#endif

#define IS_DIGIT(C)	((unsigned)((C) - '0') < 10)

/* Hex digits kept: 40, i.e. all of MW words. */
#define HEX_DIGITS_KEPT (8 * MW)

__fpmax_t attribute_hidden __XL_NPP(__strtofpmax)(const Wchar *str, Wchar **endptr,
												  int exponent_power, int prec
												  __LOCALE_PARAM )
{
	__fpmax_t number = 0;
	Wchar *pos = (Wchar *) str;
	int neg = 0, hex = 0, bits, emin, kmax;
	int gotdig = 0, gotrad = 0, j = 0, k = 0, sticky = 0;
	long long lrp = 0, dc = 0, e = 0;
	uint32_t m[MW];
#ifdef _STRTOD_HEXADECIMAL_FLOATS
	Wchar *poshex = NULL;
#endif
#ifdef __UCLIBC_HAS_LOCALE__
#if defined(L___wcstofpmax) || defined(L___wcstofpmax_l)
	wchar_t decpt_wc = __LOCALE_PTR->decimal_point_wc;
#else
	const char *decpt = __LOCALE_PTR->decimal_point;
	int decpt_len = __LOCALE_PTR->decimal_point_len;
#endif
#endif

	switch (prec) {
	case 0:
		bits = FLT_MANT_DIG;
		emin = FLT_MIN_EXP - FLT_MANT_DIG;
		break;
	case 1:
		bits = DBL_MANT_DIG;
		emin = DBL_MIN_EXP - DBL_MANT_DIG;
		break;
	default:
		bits = FPMAX_MANT_DIG;
		emin = FPMAX_MIN_EXP - FPMAX_MANT_DIG;
		break;
	}
	/* Base 1e9 limbs for the decimal digits: musl's KMAX, enough to round
	 * any input correctly within the range of the type. */
	kmax = emin < -1200 ? 2048 : 128;
	{
	uint32_t x[kmax];

	memset(m, 0, sizeof(m));
	x[0] = 0;

	while (ISSPACE(*pos)) {		/* Skip leading whitespace. */
		++pos;
	}

	switch(*pos) {				/* Handle optional sign. */
		case '-': neg = 1;		/* Fall through to increment position. */
		case '+': ++pos;
	}

#ifdef _STRTOD_HEXADECIMAL_FLOATS
	if ((*pos == '0') && (((pos[1])|0x20) == 'x')) {
		poshex = ++pos;			/* Save position of 'x' in case no digits */
		++pos;					/*   and advance past it.  */
		hex = 1;
	}
#endif

	for (;; ++pos) {
		int d;

		if (IS_DECPT(pos)) {
			if (gotrad)
				break;
			gotrad = 1;
			lrp = dc;
			pos += DECPT_LEN - 1;
			continue;
		}
		if (IS_DIGIT(*pos)) {
			d = *pos - '0';
		} else if (hex && (unsigned)(((*pos)|0x20) - 'a') < 6) {
			d = ((*pos)|0x20) - 'a' + 10;
		} else {
			break;
		}
		gotdig = 1;
		if (!dc && !d) {		/* leading zero */
			if (gotrad)
				lrp--;
			continue;
		}
		dc++;
		if (hex) {
			if (dc <= HEX_DIGITS_KEPT)
				mw_mul_add(m, 16, d);
			else if (d)
				sticky = 1;
		} else if (k < kmax - 3) {
			x[k] = j ? x[k] * 10 + d : (uint32_t) d;
			if (++j == 9) {
				k++;
				j = 0;
			}
		} else if (d) {			/* beyond what is kept: a sticky 1 */
			x[kmax - 4] |= 1;
		}
	}
	if (!gotrad)
		lrp = dc;

	if (!gotdig) {				/* Must have at least one digit. */
#ifdef _STRTOD_HEXADECIMAL_FLOATS
		if (poshex) {			/* Back up to '0' in '0x' prefix. */
			pos = poshex;		/* That "0" is the number, signed. */
			number = neg ? -(__fpmax_t) 0 : 0;
			goto DONE;
		}
#endif /* _STRTOD_HEXADECIMAL_FLOATS */

#ifdef _STRTOD_NAN_INF_STRINGS
		if (!gotrad) {			/* No decimal point, so check for inf/nan. */
			/* Note: nan is the first string so 'number = i/0.;' works. */
			static const char nan_inf_str[] = "\05nan\0\012infinity\0\05inf\0";
			int i = 0;

			do {
				/* Unfortunately, we have no memcasecmp(). */
				int jj = 0;
				/* | 0x20 is a cheap lowercasing (valid for ASCII letters and numbers only) */
				while ((pos[jj] | 0x20) == nan_inf_str[i+1+jj]) {
					++jj;
					if (!nan_inf_str[i+1+jj]) {
						number = i / (__fpmax_t)0.;
						if (neg) {	/* Correct for sign. */
							number = -number;
						}
						pos += nan_inf_str[i] - 2;
						goto DONE;
					}
				}
				i += nan_inf_str[i];
			} while (nan_inf_str[i]);
		}
#endif /* _STRTOD_NAN_INF_STRINGS */
		pos = (Wchar *) str;
		goto DONE;
	}

	/* process an exponent string */
	if (((*pos)|0x20) == (hex ? 'p' : 'e')) {
		Wchar *pos1 = pos;		/* Back up to {e|E}/{p|P} without digits. */
		int eneg = 0;

		switch(*++pos) {		/* Handle optional sign. */
			case '-': eneg = 1;	/* Fall through to increment pos. */
			case '+': ++pos;
		}
		if (IS_DIGIT(*pos)) {
			for (; IS_DIGIT(*pos); ++pos) {
				if (e < 100000000)	/* far beyond any range */
					e = e * 10 + (*pos - '0');
			}
			if (eneg)
				e = -e;
		} else {
			pos = pos1;
		}
	}

	/* exponent_power counts digits, as the caller (scanf) dropped them */
	lrp += exponent_power;

	if (hex) {
		long long e2;

		if (!dc) {
			number = neg ? -(__fpmax_t) 0 : 0;
			goto DONE;
		}
		e2 = 4 * (lrp - (dc < HEX_DIGITS_KEPT ? dc : HEX_DIGITS_KEPT)) + e;
		if (e2 > 1000000)
			e2 = 1000000;
		else if (e2 < -1000000)
			e2 = -1000000;
		number = round_to_fpmax(m, sticky, (long) e2, bits, emin, neg);
	} else {
		number = dec_to_fpmax(x, kmax, k, j, lrp + e, dc,
							  bits, emin, neg);
	}
	}

 DONE:
	if (endptr) {
		*endptr = pos;
	}

	return number;
}

#endif /* defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE) */

#endif
/**********************************************************************/
#ifdef L___fp_range_check
#if defined(NEED_STRTOF_WRAPPER) || defined(NEED_STRTOD_WRAPPER)

void attribute_hidden __fp_range_check(__fpmax_t y, __fpmax_t x)
{
	if (__FPMAX_ZERO_OR_INF_CHECK(y) /* y is 0 or +/- infinity */
		&& (y != 0)	/* y is not 0 (could have x>0, y==0 if underflow) */
		&& !__FPMAX_ZERO_OR_INF_CHECK(x) /* x is not 0 or +/- infinity */
		) {
		__set_errno(ERANGE);	/* Then x is not in y's range. */
	}
}

#endif
#endif
/**********************************************************************/
#if defined(L_strtof) || defined(L_strtof_l) || defined(L_wcstof) || defined(L_wcstof_l)
#if defined(NEED_STRTOF_WRAPPER)

#if defined(L_wcstof) || defined(L_wcstof_l)
#define strtof           wcstof
#define strtof_l         wcstof_l
#define __strtofpmax     __wcstofpmax
#define __strtofpmax_l   __wcstofpmax_l
#define Wchar wchar_t
#else
#define Wchar char
#endif


float __XL_NPP(strtof)(const Wchar *str, Wchar **endptr   __LOCALE_PARAM )
{
#if FPMAX_TYPE == 1
	return __XL_NPP(__strtofpmax)(str, endptr, 0, 0   __LOCALE_ARG );
#else
	__fpmax_t x;
	float y;

	x = __XL_NPP(__strtofpmax)(str, endptr, 0, 0   __LOCALE_ARG );
	y = (float) x;

	__fp_range_check((__fpmax_t)y, x);

	return y;
#endif
}

#endif
#endif
/**********************************************************************/
#if defined(L_strtod) || defined(L_strtod_l) || defined(L_wcstod) || defined(L_wcstod_l)
#if defined(NEED_STRTOD_WRAPPER)

#if defined(L_wcstod) || defined(L_wcstod_l)
#define strtod           wcstod
#define strtod_l         wcstod_l
#define __strtofpmax     __wcstofpmax
#define __strtofpmax_l   __wcstofpmax_l
#define Wchar wchar_t
#else
#define Wchar char
#endif

double __XL_NPP(strtod)(const Wchar *__restrict str,
					Wchar **__restrict endptr   __LOCALE_PARAM )
{
#if FPMAX_TYPE == 2
	return __XL_NPP(__strtofpmax)(str, endptr, 0, 1   __LOCALE_ARG );
#else
	__fpmax_t x;
	double y;

	x = __XL_NPP(__strtofpmax)(str, endptr, 0, 1   __LOCALE_ARG );
	y = (double) x;

	__fp_range_check((__fpmax_t)y, x);

	return y;
#endif
}
#ifdef L_strtod
libc_hidden_def(strtod)
#endif

#endif
#endif
/**********************************************************************/
#if defined(L_strtold) || defined(L_strtold_l) || defined(L_wcstold) || defined(L_wcstold_l)
#if defined(NEED_STRTOLD_WRAPPER)

#if defined(L_wcstold) || defined(L_wcstold_l)
#define strtold           wcstold
#define strtold_l         wcstold_l
#define __strtofpmax     __wcstofpmax
#define __strtofpmax_l   __wcstofpmax_l
#define Wchar wchar_t
#else
#define Wchar char
#endif

long double __XL_NPP(strtold) (const Wchar *str, Wchar **endptr   __LOCALE_PARAM )
{
#if FPMAX_TYPE == 3
	return __XL_NPP(__strtofpmax)(str, endptr, 0, 2   __LOCALE_ARG );
#else
	__fpmax_t x;
	long double y;

	x = __XL_NPP(__strtofpmax)(str, endptr, 0, 2   __LOCALE_ARG );
	y = (long double) x;

	__fp_range_check(y, x);

	return y;
#endif
}

#endif
#endif
/**********************************************************************/
