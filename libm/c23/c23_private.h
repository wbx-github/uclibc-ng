/* Helpers for the ISO C23 functions in libm/c23.

   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#ifndef _C23_PRIVATE_H
#define _C23_PRIVATE_H 1

#include <math.h>
#include <float.h>
#include <stdint.h>
#include <string.h>
#include "math_private.h"

/* The long double functions compute in double, like the rest of libm
   (see ldouble_wrappers.c); only the ones that look at the encoding
   know the long double formats.  */
#if defined __UCLIBC_HAS_LONG_DOUBLE_MATH__ && !defined __NO_LONG_DOUBLE_MATH
# define C23_LDBL 1
#else
# define C23_LDBL 0
#endif

/* Ask the compiler for the format, not <float.h>: m68k and h8300 carry
   a float.h of their own in libc/sysdeps/linux/<arch>, which the library
   build finds first (Rules.mak) and which says 53 while the ABI has an
   80-bit long double.  Programs see gcc's 64.  */
#ifdef __LDBL_MANT_DIG__
# define C23_LDBL_MANT_DIG __LDBL_MANT_DIG__
#else
# define C23_LDBL_MANT_DIG LDBL_MANT_DIG
#endif

/* isnan and isinf go through __isnanl and __isinfl for long double, and
   those narrow to double (ldouble_wrappers.c): a value outside the
   double range then raises overflow, and isinfl even answers wrongly.
   The builtins look at the value itself.  */
#define c23_isnan(x)	__builtin_isnan (x)
#define c23_isinf(x)	__builtin_isinf (x)
#define c23_signbit(x)	__builtin_signbit (x)
/* __builtin_isfinite wants gcc 4.4 and metag is built with 4.2.  */
#define c23_isfinite(x)	(!c23_isnan (x) && !c23_isinf (x))

/* round, ceil, floor and trunc force inexact with "huge + x".  C23
   F.10.6.8 and F.10.6.10 say roundeven, fromfp and ufromfp must not
   raise it, so note the flag and put it back.  */
#ifdef __UCLIBC_HAS_FENV__
# include <fenv.h>
#endif
/* A target without the hardware flags has fenv but no FE_INEXACT: its
   bits/fenv.h sets FE_ALL_EXCEPT to 0 and names nothing.  */
#ifdef FE_INEXACT
# define C23_KEEP_INEXACT	int __c23_inex = fetestexcept (FE_INEXACT)
# define C23_PUT_INEXACT_BACK						\
	do {								\
		if (!__c23_inex)					\
			feclearexcept (FE_INEXACT);			\
	} while (0)
#else
# define C23_KEEP_INEXACT	((void) 0)
# define C23_PUT_INEXACT_BACK	((void) 0)
#endif

static __inline uint64_t c23_asuint64(double x)
{
	uint64_t i;

	memcpy(&i, &x, sizeof i);
	return i;
}

static __inline double c23_asdouble(uint64_t i)
{
	double x;

	memcpy(&x, &i, sizeof x);
	return x;
}

static __inline uint32_t c23_asuint32(float x)
{
	uint32_t i;

	memcpy(&i, &x, sizeof i);
	return i;
}

static __inline float c23_asfloat(uint32_t i)
{
	float x;

	memcpy(&x, &i, sizeof x);
	return x;
}

/* hppa and mips before nan2008 mark a signaling NaN by setting the high
   significand bit, the reverse of IEEE 754-2008.  */
#if defined __hppa__ || (defined __mips__ && !defined __mips_nan2008)
# define C23_SNAN_HIGH_BIT 1
#else
# define C23_SNAN_HIGH_BIT 0
#endif

/* IBM double-double (powerpc without --with-long-double-64): the value
   is the sum of two doubles, the high one first in memory on either
   byte order.  Not an IEEE format; as in glibc's ldbl-128ibm the
   encoding functions look at the high part, and the narrowing ones
   round the long double result once more.  */
#if C23_LDBL && C23_LDBL_MANT_DIG == 106
# define C23_LDBL_IBM 1
static __inline void c23_ldunpack(const long double *x, double *hi, double *lo)
{
	double d[2];

	memcpy(d, x, sizeof d);
	*hi = d[0];
	*lo = d[1];
}

static __inline void c23_ldpack(long double *x, double hi, double lo)
{
	double d[2];

	d[0] = hi;
	d[1] = lo;
	memcpy(x, d, sizeof d);
}
#endif

/* The long double encoding as sign and a magnitude that orders like
   the value: hi first, then lo.  Formats: like double (53), the 80-bit
   extended of x87 and m68k (64), binary128 (113).  */
#if C23_LDBL && C23_LDBL_MANT_DIG != 53 && C23_LDBL_MANT_DIG != 106
# include <endian.h>
# if C23_LDBL_MANT_DIG == 64 || C23_LDBL_MANT_DIG == 113
#  define C23_LDBL_BITS 1
struct c23_ldbits {
	int sign;
	uint64_t hi, lo;
};

static __inline void c23_ldget(const long double *x, struct c23_ldbits *b)
{
	unsigned char buf[sizeof(long double)];
#  if C23_LDBL_MANT_DIG == 64
	uint16_t se;
	uint64_t m;

	memcpy(buf, x, sizeof buf);
#   if __BYTE_ORDER == __LITTLE_ENDIAN		/* x87 */
	memcpy(&m, buf, 8);
	memcpy(&se, buf + 8, 2);
#   else					/* m68k: exponent, pad, mantissa */
	memcpy(&se, buf, 2);
	memcpy(&m, buf + 4, 8);
#   endif
	b->sign = se >> 15;
	b->hi = se & 0x7fff;
	b->lo = m;
#  else
	uint64_t w[2];

	memcpy(buf, x, sizeof buf);
	memcpy(w, buf, 16);
#   if __BYTE_ORDER == __LITTLE_ENDIAN
	b->hi = w[1];
	b->lo = w[0];
#   else
	b->hi = w[0];
	b->lo = w[1];
#   endif
	b->sign = b->hi >> 63;
	b->hi &= ~(1ULL << 63);
#  endif
}

static __inline void c23_ldput(long double *x, const struct c23_ldbits *b)
{
	unsigned char buf[sizeof(long double)];
#  if C23_LDBL_MANT_DIG == 64
	uint16_t se = (b->sign << 15) | (uint16_t) b->hi;

	memcpy(buf, x, sizeof buf);
#   if __BYTE_ORDER == __LITTLE_ENDIAN
	memcpy(buf, &b->lo, 8);
	memcpy(buf + 8, &se, 2);
#   else
	memcpy(buf, &se, 2);
	memcpy(buf + 4, &b->lo, 8);
#   endif
#  else
	uint64_t w[2], hi = b->hi | ((uint64_t) b->sign << 63);

#   if __BYTE_ORDER == __LITTLE_ENDIAN
	w[1] = hi;
	w[0] = b->lo;
#   else
	w[0] = hi;
	w[1] = b->lo;
#   endif
	memcpy(buf, w, 16);
#  endif
	memcpy(x, buf, sizeof buf);
}

/* The exponent field of an infinity, and the exponent of the
   significand's lowest bit when the field is E.  For the 80-bit format
   the significand is the 64-bit LO with its integer bit; for binary128
   it is the low 113 bits of HI:LO.  */
#  if C23_LDBL_MANT_DIG == 64
#   define C23_LD_INF_EXP	0x7fff
#   define C23_LD_LSB(e)	((int) (e) - 16446)
#   define C23_LD_SIG_BITS	64
#  else
#   define C23_LD_INF_EXP	0x7fff
#   define C23_LD_LSB(e)	((int) (e) - 16495)
#   define C23_LD_SIG_BITS	113
#  endif

/* The exponent field, 0 for a subnormal.  */
static __inline unsigned int c23_ldexpf(const struct c23_ldbits *b)
{
#  if C23_LDBL_MANT_DIG == 64
	return (unsigned int) b->hi;
#  else
	return (unsigned int) (b->hi >> 48);
#  endif
}

static __inline int c23_ldiszero(const struct c23_ldbits *b)
{
	return b->hi == 0 && b->lo == 0;
}

static __inline int c23_ldisinf(const struct c23_ldbits *b)
{
#  if C23_LDBL_MANT_DIG == 64
	return b->hi == C23_LD_INF_EXP && b->lo == (1ULL << 63);
#  else
	return b->hi == ((uint64_t) C23_LD_INF_EXP << 48) && b->lo == 0;
#  endif
}

/* The next magnitude up.  Called for a finite, non-zero value or for an
   infinity, where it gives the largest finite one.  */
static __inline void c23_ldincmag(struct c23_ldbits *b)
{
#  if C23_LDBL_MANT_DIG == 64
	/* The significand carries the integer bit, so a binade ends at
	   all ones and the next one starts at 2^63.  */
	if (b->lo == ~(uint64_t) 0) {
		b->lo = 1ULL << 63;
		++b->hi;
	} else {
		++b->lo;
	}
#  else
	if (++b->lo == 0)
		++b->hi;
#  endif
}

/* The next magnitude down.  Called for a finite, non-zero value.  */
static __inline void c23_lddecmag(struct c23_ldbits *b)
{
#  if C23_LDBL_MANT_DIG == 64
	if (b->hi >= 1 && b->lo == (1ULL << 63)) {
		--b->hi;
		/* Below the smallest normal come the subnormals, whose
		   integer bit is clear.  */
		b->lo = b->hi == 0 ? (1ULL << 63) - 1 : ~(uint64_t) 0;
	} else {
		--b->lo;
	}
#  else
	if (b->lo-- == 0)
		--b->hi;
#  endif
}

/* X with the fractional bits cleared: trunc, exactly and whatever the
   rounding mode.  X must be finite.  */
static __inline long double c23_ldtrunc(long double x)
{
	struct c23_ldbits b;
	unsigned int e;
	int f;

	c23_ldget(&x, &b);
	e = c23_ldexpf(&b);
	f = -C23_LD_LSB(e);
	if (e == 0 || f >= C23_LD_SIG_BITS) {		/* |x| < 1 */
		b.hi = 0;
		b.lo = 0;
	} else if (f > 0) {
#  if C23_LDBL_MANT_DIG == 64
		b.lo &= ~((((uint64_t) 1) << f) - 1);
#  else
		if (f < 64) {
			b.lo &= ~((((uint64_t) 1) << f) - 1);
		} else {
			b.lo = 0;
			if (f > 64)
				b.hi &= ~((((uint64_t) 1) << (f - 64)) - 1);
		}
#  endif
	} else {
		return x;				/* already an integer */
	}
	c23_ldput(&x, &b);
	return x;
}

/* 2^K as a long double, for 0 <= K < 16384.  */
static __inline long double c23_ldexp2(int k)
{
	struct c23_ldbits b;
	long double x = 0;

	b.sign = 0;
#  if C23_LDBL_MANT_DIG == 64
	b.hi = (uint64_t) (k + 16383);
	b.lo = 1ULL << 63;
#  else
	b.hi = (uint64_t) (k + 16383) << 48;
	b.lo = 0;
#  endif
	c23_ldput(&x, &b);
	return x;
}

/* Is the magnitude a signaling NaN?  */
static __inline int c23_ldsnan(const struct c23_ldbits *b)
{
#  if C23_LDBL_MANT_DIG == 64
	return b->hi == 0x7fff && !(b->lo & (1ULL << 62))
	       && (b->lo & ((1ULL << 62) - 1));
#  elif C23_SNAN_HIGH_BIT
	return (b->hi >> 47) == 0xffff;
#  else
	return (b->hi >> 48) == 0x7fff && !(b->hi & (1ULL << 47))
	       && ((b->hi & ((1ULL << 47) - 1)) | b->lo);
#  endif
}
# endif
#endif

/* Raise invalid and return a quiet NaN, for a domain error.  */
static __inline double c23_invalid(void)
{
	volatile double z = 0.0;

	return z / z;
}

/* Raise invalid where there is no result to carry it.  The quotient
   goes into a volatile, or the compiler drops the division.  */
static __inline void c23_raise_invalid(void)
{
	volatile double z = 0.0, r;

	r = z / z;
	(void) r;
}

/* Raise divide-by-zero and return an infinity with the sign of S.  */
static __inline double c23_pole(double s)
{
	volatile double z = 0.0;

	return s / z;
}

#endif
