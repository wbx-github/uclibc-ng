/*
 * Copyright (C) 2000,2001,2003,2004	Manuel Novoa III <mjn3@codepoet.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"
#include <printf.h>
#include <float.h>
#include <locale.h>
#include <math.h>
#include "_fpmaxtostr.h"

/*
 * Function:
 *
 *     ssize_t _fpmaxtostr(FILE * fp, __fpmax_t x, struct printf_info *info,
 *                         __fp_outfunc_t fp_outfunc);
 *
 * This is derived from the old _dtostr, whic I wrote for uClibc to provide
 * floating point support for the printf functions.  It handles +/- infinity,
 * nan, and signed 0 assuming you have ieee arithmetic.  It also now handles
 * digit grouping (for the uClibc supported locales) and hexadecimal float
 * notation.  Finally, via the fp_outfunc parameter, it now supports wide
 * output.
 *
 * Notes:
 *
 * At most DECIMAL_DIG significant digits are kept, and printed as 0
 * beyond that.  The digits that are kept are exact: they come out of the
 * binary value by integer arithmetic, see fpmax_dec_digits().
 *
 * It should also be fairly portable, as no assumptions are made about the
 * bit-layout of doubles.  Of course, that does make it less efficient than
 * it could be.
 */

/*****************************************************************************/
/* Don't change anything that follows unless you know what you're doing.     */
/*****************************************************************************/
/* Fairly portable nan check.  Bitwise for i386 generated larger code.
 * If you have a better version, comment this out.
 */
#define isnan(x)             ((x) != (x))

/*****************************************************************************/
/* Don't change anything that follows peroid!!!  ;-)                         */
/*****************************************************************************/
#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
#if FLT_RADIX != 2
#error FLT_RADIX != 2 is not currently supported
#endif
#endif /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */

#define NUM_HEX_DIGITS      ((FPMAX_MANT_DIG + 3)/ 4)

#define HEX_DIGITS_PER_BLOCK 8

/* Maximum number of subcases to output double is...
 *  0 - sign
 *  1 - padding and initial digit
 *  2 - digits left of the radix
 *  3 - 0s left of the radix        or   radix
 *  4 - radix                       or   digits right of the radix
 *  5 - 0s right of the radix
 *  6 - exponent
 *  7 - trailing space padding
 * although not all cases may occur.
 */
#define MAX_CALLS 8

/*****************************************************************************/

#define NUM_HEX_DIGIT_BLOCKS \
   ((NUM_HEX_DIGITS+HEX_DIGITS_PER_BLOCK-1)/HEX_DIGITS_PER_BLOCK)

/*****************************************************************************/

static const char fmt[] = "inf\0INF\0nan\0NAN\0.\0,";

#define INF_OFFSET        0		/* must be 1st */
#define NAN_OFFSET        8		/* must be 2nd.. see hex sign handling */
#define DECPT_OFFSET     16
#define THOUSEP_OFFSET   18

#define EMPTY_STRING_OFFSET 3

/*****************************************************************************/

/*****************************************************************************/
#if FLT_RADIX != 2
#error FLT_RADIX != 2 is not currently supported
#endif

#if FPMAX_MAX_EXP < -FPMAX_MIN_EXP
#error scaling code can not handle FPMAX_MAX_EXP < -FPMAX_MIN_EXP
#endif

/* 2^FPMAX_MAX_EXP itself is out of range, so stop one entry short of it;
 * values too small for one pass over the table get a second one below. */
static const __fpmax_t exp16_table[] = {
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
	0x1.0p16384L
#endif
#if FPMAX_MAX_EXP >= 32768
#error unsupported FPMAX_MAX_EXP.  please increase table
#endif
};

#define EXP16_TABLE_SIZE     (sizeof(exp16_table)/sizeof(exp16_table[0]))
#define EXP16_TABLE_MAX      (1U<<(EXP16_TABLE_SIZE-1))

/*****************************************************************************/

struct dec_out {
	char *o;
	int left;
	int sticky;
};

/* Append the nd decimal digits of v, most significant first; what does not
 * fit any more only matters for rounding. */
static void put_limb(struct dec_out *w, uint32_t v, int nd)
{
	char t[9];
	int i;

	for (i = nd; i--; ) {
		t[i] = '0' + v % 10;
		v /= 10;
	}
	for (i = 0; i < nd; i++) {
		if (w->left) {
			*w->o++ = t[i];
			w->left--;
		} else if (t[i] != '0') {
			w->sticky = 1;
		}
	}
}

static int limb_digits(uint32_t v)
{
	int n = 1;

	while (v >= 10) {
		v /= 10;
		n++;
	}
	return n;
}

/* Base 1e9 limbs of the integer in n[0..w-1] (destroys it), least
 * significant first; returns their number. */
static int to_limbs(uint32_t *n, int w, uint32_t *d)
{
	int nl = 0, k;

	while (w && !n[w - 1])
		w--;
	while (w) {
		uint32_t rem = 0;

		for (k = w; k--; ) {
			uint64_t cur = (uint64_t) rem << 32 | n[k];
			n[k] = cur / 1000000000;
			rem = cur % 1000000000;
		}
		d[nl++] = rem;
		while (w && !n[w - 1])
			w--;
	}
	return nl;
}

static void put_limbs(struct dec_out *w, const uint32_t *d, int nl)
{
	put_limb(w, d[nl - 1], limb_digits(d[nl - 1]));
	while (--nl)
		put_limb(w, d[nl - 1], 9);
}

#define MANT_WORDS ((FPMAX_MANT_DIG + 31) / 32)

/* Write the first nd decimal digits of x > 0 to out, exactly, and return
 * the decimal exponent of the first one.  *sticky tells whether anything
 * nonzero follows them.
 *
 * x is M * 2^e2 with an FPMAX_MANT_DIG-bit integer M.  Taking it apart
 * only multiplies and divides by powers of two, which is exact, and the
 * rest is integer arithmetic, so it needs no precision beyond x's own. */
static int fpmax_dec_digits(__fpmax_t x, char *out, int nd, int *sticky)
{
	struct dec_out w = { out, nd, 0 };
	uint32_t m[MANT_WORDS];
	int e2 = 0, i, k, exp;

	for (i = EXP16_TABLE_SIZE; i--; ) {	/* into [1, 2) */
		while (x >= exp16_table[i]) {
			x /= exp16_table[i];
			e2 += 1 << i;
		}
	}
	for (i = EXP16_TABLE_SIZE; i--; ) {
		while (x * exp16_table[i] < 2) {
			x *= exp16_table[i];
			e2 -= 1 << i;
		}
	}
	x *= (__fpmax_t) (1UL << (FPMAX_MANT_DIG - 32 * (MANT_WORDS - 1) - 1));
	for (k = MANT_WORDS; k--; ) {
		m[k] = (uint32_t) x;
		x = (x - m[k]) * 4294967296.0;
	}
	e2 -= FPMAX_MANT_DIG - 1;

	if (e2 >= 0) {				/* an integer, M << e2 */
		int nw = MANT_WORDS + e2 / 32 + 1;
		uint32_t n[nw], d[nw + nw / 8 + 2];

		memset(n, 0, sizeof(n));
		for (k = 0; k < MANT_WORDS; k++) {
			uint64_t v = (uint64_t) m[k] << (e2 % 32);
			n[k + e2 / 32] |= (uint32_t) v;
			n[k + e2 / 32 + 1] |= (uint32_t) (v >> 32);
		}
		k = to_limbs(n, nw, d);
		exp = limb_digits(d[k - 1]) - 1 + 9 * (k - 1);
		put_limbs(&w, d, k);
	} else {				/* M / 2^s */
		int s = -e2, fw = (s + 31) / 32;
		int sh = 32 * fw - s;		/* g holds the fraction << sh */
		uint32_t g[fw], ip[MANT_WORDS + 1], d[MANT_WORDS + 2];

		memset(ip, 0, sizeof(ip));
		memset(g, 0, sizeof(g));
		for (k = 0; k < MANT_WORDS; k++) {
			int bit = 32 * k - s;	/* m[k] in the integer part */
			uint64_t v;

			if (bit >= 0) {
				v = (uint64_t) m[k] << (bit % 32);
				ip[bit / 32] |= (uint32_t) v;
				ip[bit / 32 + 1] |= (uint32_t) (v >> 32);
			} else if (bit > -32) {
				ip[0] |= m[k] >> -bit;
			}
			bit = 32 * k + sh;	/* m[k] in the fraction */
			if (bit < 32 * fw) {
				v = (uint64_t) m[k] << (bit % 32);
				g[bit / 32] |= (uint32_t) v;
				if (bit / 32 + 1 < fw)
					g[bit / 32 + 1] |= (uint32_t) (v >> 32);
			}
		}

		k = to_limbs(ip, MANT_WORDS + 1, d);
		exp = -1;
		if (k) {
			exp = limb_digits(d[k - 1]) - 1 + 9 * (k - 1);
			put_limbs(&w, d, k);
		}

		/* The fraction times 1e9 carries the next nine digits out of
		 * the top word. */
		for (i = 0; i < fw; ) {
			uint64_t c = 0;
			uint32_t limb;

			if (!g[i]) {
				i++;
				continue;
			}
			if (!w.left) {
				w.sticky = 1;
				break;
			}
			for (k = i; k < fw; k++) {
				c += (uint64_t) g[k] * 1000000000;
				g[k] = (uint32_t) c;
				c >>= 32;
			}
			limb = (uint32_t) c;
			if (w.left == nd) {	/* no digit yet */
				if (!limb) {
					exp -= 9;
					continue;
				}
				exp -= 9 - limb_digits(limb);
				put_limb(&w, limb, limb_digits(limb));
			} else {
				put_limb(&w, limb, 9);
			}
		}
	}
	while (w.left) {
		*w.o++ = '0';
		w.left--;
	}
	*sticky = w.sticky;
	return exp;
}
/*****************************************************************************/

#define FPO_ZERO_PAD    (0x80 | '0')
#define FPO_STR_WIDTH   (0x80 | ' ');
#define FPO_STR_PREC    'p'

#ifndef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
#define block_scale  (__fpmax_t)1e9
#define dpb          DIGITS_PER_BLOCK
#define base         10
#define ndb          NUM_DIGIT_BLOCKS
#define nd           DECIMAL_DIG
#endif

ssize_t _fpmaxtostr(FILE * fp, __fpmax_t x, struct printf_info *info,
					__fp_outfunc_t fp_outfunc)
{
#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
	__fpmax_t lower_bnd;
	__fpmax_t upper_bnd = 1e9;
	__fpmax_t block_scale = 1e9;
#endif /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */
#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
	uint_fast32_t base = 10;
	int dpb = DIGITS_PER_BLOCK;
	int ndb = NUM_DIGIT_BLOCKS;
	int nd = DECIMAL_DIG;
	int sufficient_precision = 0;
	int non_zero_digits = 0;
#endif /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */
#ifdef __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__
	int num_groups = 0;
	int initial_group;	   /* This does not need to be initialized. */
	int tslen;		   /* This does not need to be initialized. */
	int nblk2;		   /* This does not need to be initialized. */
	const char *ts;		   /* This does not need to be initialized. */
#endif /* __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__ */
	int round, o_exp;
	int exp;
	int sticky = 0;			/* nonzero digits beyond the buffer */
	int width, preci;
	int cnt;
	char *s;
	char *e;
	intptr_t pc_fwi[3*MAX_CALLS];
	intptr_t *ppc;
	intptr_t *ppc_last;
	char exp_buf[16];
	char buf[BUF_SIZE];
	char sign_str[6];			/* Last 2 are for 1st digit + nul. */
	char o_mode;
	char mode;


	width = info->width;
	preci = info->prec;
	mode = info->spec;

	*exp_buf = 'e';
	if ((mode|0x20) == 'a') {
#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
		*exp_buf = 'p';
		if (preci < 0) {
			preci = NUM_HEX_DIGITS;
			sufficient_precision = 1;
		}
#else
		mode += ('g' - 'a');
#endif
	}

	if (preci < 0) {
		preci = 6;
	}

	*sign_str = '\0';
	if (PRINT_INFO_FLAG_VAL(info,showsign)) {
		*sign_str = '+';
	} else if (PRINT_INFO_FLAG_VAL(info,space)) {
		*sign_str = ' ';
	}

	*(sign_str+1) = 0;
	pc_fwi[5] = INF_OFFSET;
	if (isnan(x)) {				/* First, check for nan. */
		pc_fwi[5] = NAN_OFFSET;
		if (signbit(x))
			*sign_str = '-';
		goto INF_NAN;
	}

	if (x == 0) {				/* Handle 0 now to avoid false positive. */
#ifdef __UCLIBC_HAVE_SIGNED_ZERO__
		union {
			double x;
			struct {
				unsigned int l1, l2;
			} i;
		} u = {x};
		if (u.i.l1 ^ u.i.l2) { /* Handle 'signed' zero. */
			*sign_str = '-';
		}
#endif /* __UCLIBC_HAVE_SIGNED_ZERO__ */
		exp = -1;
		goto GENERATE_DIGITS;
	}

	if (x < 0) {				/* Convert negatives to positives. */
		*sign_str = '-';
		x = -x;
	}

	if (__FPMAX_ZERO_OR_INF_CHECK(x)) {	/* Inf since zero handled above. */
	INF_NAN:
		info->pad = ' ';
		ppc = pc_fwi + 6;
		pc_fwi[3] = FPO_STR_PREC;
		pc_fwi[4] = 3;
		if (mode < 'a') {
			pc_fwi[5] += 4;
		}
		pc_fwi[5] = (intptr_t)(fmt + pc_fwi[5]);
		goto EXIT_SPECIAL;
	}

	if ((mode|0x20) != 'a') {
		exp = fpmax_dec_digits(x, buf + 2, NUM_DIGIT_BLOCKS * DIGITS_PER_BLOCK,
							   &sticky);
		goto DIGITS_DONE;
	}

#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
	{
		int i = EXP16_TABLE_SIZE, j = EXP16_TABLE_MAX;
		int exp_neg = 0;

		lower_bnd = 0x1.0p28L;
		upper_bnd = 0x1.0p29L;
		block_scale = 0x1.0p32L;
		exp = 28;
		dpb = HEX_DIGITS_PER_BLOCK;
		ndb = NUM_HEX_DIGIT_BLOCKS;
		nd = NUM_HEX_DIGITS;
		base = 16;

		if (x < lower_bnd) { /* Do we need to scale up or down? */
			exp_neg = 1;
		}

		do {
			--i;
			if (exp_neg) {
				if (x * exp16_table[i] < upper_bnd) {
					x *= exp16_table[i];
					exp -= j;
				}
			} else {
				if (x / exp16_table[i] >= lower_bnd) {
					x /= exp16_table[i];
					exp += j;
				}
			}
			j >>= 1;
			/* One pass reaches less than the whole range down to the
			 * subnormals; go over the table again. */
			if (!i && exp_neg && x < lower_bnd) {
				i = EXP16_TABLE_SIZE;
				j = EXP16_TABLE_MAX;
			}
		} while (i);
	}
	if (x >= upper_bnd) {		/* Handle bad rounding case. */
		x /= exp16_table[0];
		++exp;
	}
	assert(x < upper_bnd);
#endif /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */

 GENERATE_DIGITS:
	{
		int i, j;
		s = buf + 2;			/* Leave space for '\0' and '0'. */
		i = 0;
		do {
			uint_fast32_t digit_block = (uint_fast32_t) x;
			assert(digit_block < block_scale);
			x = (x - digit_block) * block_scale;
			s += dpb;
			j = 0;
			do {
				uint_fast32_t digit = (digit_block % base);

				s[- ++j] = '0' + digit;
#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
				non_zero_digits += digit > 0;
#endif
				digit_block /= base;
			} while (j < dpb);
		} while (++i < ndb);
	}
 DIGITS_DONE:

	/*************************************************************************/

	if (mode < 'a') {
		*exp_buf -= ('a' - 'A'); /* e->E and p->P */
		mode += ('a' - 'A');
	}

	o_mode = mode;
	if ((mode == 'g') && (preci > 0)){
		--preci;
	}
	round = preci;

	if (mode == 'f') {
		round += exp;
		if (round < -1) {
			memset(buf, '0', DECIMAL_DIG); /* OK, since 'f' -> decimal case. */
		    exp = -1;
		    round = -1;
		}
	}

	s = buf;
	*s++ = 0;					/* Terminator for rounding and 0-triming. */
	*s = '0';					/* Space to round. */

	{
		int i;
		i = 0;
		e = s + nd + 1;
		if (round < nd) {
			char *dend = buf + 2 + ndb * dpb;	/* end of the digits */

			e = s + round + 2;
			if (e < dend && *e >= '0' + (base/2)) {
				i = 1;
				/* Exactly half way: round to even, as glibc does. */
				if (*e == '0' + (base/2) && !sticky) {
					char *q = e;

					while (++q < dend && *q == '0')
						;
					if (q == dend && !((e[-1] - '0') & 1)) {
						i = 0;
					}
				}
			}
		}

		do {			   /* Handle rounding and trim trailing 0s. */
			*--e += i;			/* Add the carry. */
		} while ((*e == '0') || (*e > '0' - 1 + base));
	}

#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
	if ((mode|0x20) == 'a') {
		char *q;

		for (q = e ; *q ; --q) {
			if (*q > '9') {
				*q += (*exp_buf - ('p' - 'a') - '9' - 1);
			}
		}
	}
#endif /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */

	o_exp = exp;
	if (e <= s) {				/* We carried into an extra digit. */
		++o_exp;
		e = s;					/* Needed if all 0s. */
	} else {
		++s;
	}
	*++e = 0;					/* Terminating nul char. */

	if ((mode == 'g') && ((o_exp >= -4) && (o_exp <= round))) {
		mode = 'f';
		preci = round - o_exp;
	}

	exp = o_exp;
	if (mode != 'f') {
		o_exp = 0;
	}

	if (o_exp < 0) {			/* Exponent is < 0, so */
		*--s = '0';				/* fake the first 0 digit. */
	}

	pc_fwi[3] = FPO_ZERO_PAD;
	pc_fwi[4] = 1;
	pc_fwi[5] = (intptr_t)(sign_str + 4);
	sign_str[4] = *s++;
	sign_str[5] = 0;
	ppc = pc_fwi + 6;

	{
		int i = e - s;			/* Total digits is 'i'. */
		if (o_exp >= 0) {
#ifdef __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__

			const char *p;

			if (PRINT_INFO_FLAG_VAL(info,group)
			 && *(p = __UCLIBC_CURLOCALE->grouping)
			) {
				int nblk1;

				nblk2 = nblk1 = *p;
				if (*++p) {
					nblk2 = *p;
					assert(!*++p);
				}

				if (o_exp >= nblk1) {
					num_groups = (o_exp - nblk1) / nblk2 + 1;
					initial_group = (o_exp - nblk1) % nblk2;

#ifdef __UCLIBC_HAS_WCHAR__
					if (PRINT_INFO_FLAG_VAL(info,wide)) {
						/* _fp_out_wide() will fix this up. */
						ts = fmt + THOUSEP_OFFSET;
						tslen = 1;
					} else {
#endif /* __UCLIBC_HAS_WCHAR__ */
						ts = __UCLIBC_CURLOCALE->thousands_sep;
						tslen = __UCLIBC_CURLOCALE->thousands_sep_len;
#ifdef __UCLIBC_HAS_WCHAR__
					}
#endif /* __UCLIBC_HAS_WCHAR__ */

					width -= num_groups * tslen;
				}
			}


#endif /* __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__ */
			ppc[0] = FPO_STR_PREC;
			ppc[2] = (intptr_t)(s);
			if (o_exp >= i) {		/* all digit(s) left of decimal */
				ppc[1] = i;
				ppc += 3;
				o_exp -= i;
				i = 0;
				if (o_exp>0) {		/* have 0s left of decimal */
					ppc[0] = FPO_ZERO_PAD;
					ppc[1] = o_exp;
					ppc[2] = (intptr_t)(fmt + EMPTY_STRING_OFFSET);
					ppc += 3;
				}
			} else if (o_exp > 0) {	/* decimal between digits */
				ppc[1] = o_exp;
				ppc += 3;
				s += o_exp;
				i -= o_exp;
			}
			o_exp = -1;
		}

		if (PRINT_INFO_FLAG_VAL(info,alt)
			|| (i)
			|| ((o_mode == 'e' || o_mode == 'f') && (preci > 0))
#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
			|| ((o_mode == 'a')
			    && ((info->prec < 0 && non_zero_digits > 1) || info->prec > 0))
#endif /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */
			) {
			ppc[0] = FPO_STR_PREC;
#ifdef __LOCALE_C_ONLY
			ppc[1] = 1;
			ppc[2] = (intptr_t)(fmt + DECPT_OFFSET);
#else  /* __LOCALE_C_ONLY */
#ifdef __UCLIBC_HAS_WCHAR__
			if (PRINT_INFO_FLAG_VAL(info,wide)) {
				/* _fp_out_wide() will fix this up. */
				ppc[1] = 1;
				ppc[2] = (intptr_t)(fmt + DECPT_OFFSET);
			} else {
#endif /* __UCLIBC_HAS_WCHAR__ */
				ppc[1] = __UCLIBC_CURLOCALE->decimal_point_len;
				ppc[2] = (intptr_t)(__UCLIBC_CURLOCALE->decimal_point);
#ifdef __UCLIBC_HAS_WCHAR__
			}
#endif /* __UCLIBC_HAS_WCHAR__ */
#endif /* __LOCALE_C_ONLY */
			ppc += 3;
		}

		if (++o_exp < 0) {			/* Have 0s right of decimal. */
			ppc[0] = FPO_ZERO_PAD;
			ppc[1] = -o_exp;
			ppc[2] = (intptr_t)(fmt + EMPTY_STRING_OFFSET);
			ppc += 3;
		}
		if (i) {					/* Have digit(s) right of decimal. */
			ppc[0] = FPO_STR_PREC;
			ppc[1] = i;
			ppc[2] = (intptr_t)(s);
			ppc += 3;
		}

		if (((o_mode != 'g') || PRINT_INFO_FLAG_VAL(info,alt))
#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
			&& !sufficient_precision
#endif /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */
			) {
			i -= o_exp;
			if (i < preci) {		/* Have 0s right of digits. */
				i = preci - i;
				ppc[0] = FPO_ZERO_PAD;
				ppc[1] = i;
				ppc[2] = (intptr_t)(fmt + EMPTY_STRING_OFFSET);
				ppc += 3;
			}
		}
	}

	/* Build exponent string. */
	if (mode != 'f') {
		char *p = exp_buf + sizeof(exp_buf);
		int j;
		char exp_char = *exp_buf;
		char exp_sign = '+';
#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
		int min_exp_dig_plus_2 = ((o_mode != 'a') ? (2+2) : (2+1));
#else  /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */
#define min_exp_dig_plus_2  (2+2)
#endif /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */

		if (exp < 0) {
			exp_sign = '-';
			exp = -exp;
		}

		*--p = 0;			/* nul-terminate */
		j = 2;				/* Count exp_char and exp_sign. */
		do {
			*--p = '0' + (exp % 10);
			exp /= 10;
		} while ((++j < min_exp_dig_plus_2) || exp); /* char+sign+mindigits */
		*--p = exp_sign;
		*--p = exp_char;

		ppc[0] = FPO_STR_PREC;
		ppc[1] = j;
		ppc[2] = (intptr_t)(p);
		ppc += 3;
	}

 EXIT_SPECIAL:
	{
		int i;
		ppc_last = ppc;
		ppc = pc_fwi + 4;	 /* Need width fields starting with second. */
		do {
			width -= *ppc;
			ppc += 3;
		} while (ppc < ppc_last);

		ppc = pc_fwi;
		ppc[0] = FPO_STR_WIDTH;
		ppc[1] = i = ((*sign_str) != 0);
		ppc[2] = (intptr_t) sign_str;

#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
		if (((mode|0x20) == 'a') && (pc_fwi[3] >= 16) &&
		    (pc_fwi[3] != FPO_STR_PREC)) { /* Hex sign handling. */
			/* Hex and not inf or nan, so prefix with 0x. */
			char *h = sign_str + i;
			*h = '0';
			*++h = 'x' - 'p' + *exp_buf;
			*++h = 0;
			ppc[1] = (i += 2);
		}
#endif /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */

		if ((width -= i) > 0) {
			if (PRINT_INFO_FLAG_VAL(info,left)) { /* Left-justified. */
				ppc_last[0] = FPO_STR_WIDTH;
				ppc_last[1] = width;
				ppc_last[2] = (intptr_t)(fmt + EMPTY_STRING_OFFSET);
				ppc_last += 3;
			} else if (info->pad == '0') { /* 0 padding */
				ppc[4] += width;	/* Pad second field. */
			} else {
				ppc[1] += width;	/* Pad first (sign) field. */
			}
		}

		cnt = 0;
	}

	do {
#ifdef __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__

		if ((ppc == pc_fwi + 6) && num_groups) {
			const char *gp = (const char *) ppc[2];
			int len = ppc[1];
			int blk = initial_group;

			cnt += num_groups * tslen; /* Adjust count now for sep chars. */

/* 			__printf("\n"); */
			do {
				if (!blk) {		/* Initial group could be 0 digits long! */
					blk = nblk2;
				} else if (len >= blk) { /* Enough digits for a group. */
/* 					__printf("norm:  len=%d blk=%d  \"%.*s\"\n", len, blk, blk, gp); */
					if (fp_outfunc(fp, *ppc, blk, (intptr_t) gp) != blk) {
						return -1;
					}
					assert(gp);
					if (*gp) {
						gp += blk;
					}
					len -= blk;
				} else {		/* Transition to 0s. */
/* 					__printf("trans: len=%d blk=%d  \"%.*s\"\n", len, blk, len, gp); */
					if (len) {
/* 						__printf("len\n"); */
						if (fp_outfunc(fp, *ppc, len, (intptr_t) gp) != len) {
							return -1;
						}
						gp += len;
					}

					if (ppc[3] == FPO_ZERO_PAD) { /* Need to group 0s */
/* 						__printf("zeropad\n"); */
						cnt += ppc[1];
						ppc += 3;
						gp = (const char *) ppc[2];
						blk -= len;	/* blk > len, so blk still > 0. */
						len = ppc[1];
						continue; /* Don't decrement num_groups here. */
					} else {
						assert(num_groups == 0);
						break;
					}
				}

				if (num_groups <= 0) {
					break;
				}
				--num_groups;

				if (fp_outfunc(fp, FPO_STR_PREC, tslen, (intptr_t) ts) != tslen) {
					return -1;
				}
				blk = nblk2;

/* 				__printf("num_groups=%d   blk=%d\n", num_groups, blk); */

			} while (1);
		} else

#endif /* __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__ */
		{						/* NOTE: Remember 'else' above! */
			if (fp_outfunc(fp, *ppc, ppc[1], ppc[2]) != ppc[1]) {
				return -1;
			}
		}

		cnt += ppc[1];
		ppc += 3;
	} while (ppc < ppc_last);

	return cnt;
}
