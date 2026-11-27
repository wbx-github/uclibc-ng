/* canonicalize (ISO C23), and the functions behind the issignaling,
   iscanonical and iseqsig macros.

   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#include "c23_private.h"
#include <errno.h>

static __inline int snan64(uint64_t i)
{
#if C23_SNAN_HIGH_BIT
	return (i & 0x7ff8000000000000ULL) == 0x7ff8000000000000ULL;
#else
	return (i & 0x7ff8000000000000ULL) == 0x7ff0000000000000ULL
	       && (i & 0x0007ffffffffffffULL);
#endif
}

static __inline int snan32(uint32_t i)
{
#if C23_SNAN_HIGH_BIT
	return (i & 0x7fc00000U) == 0x7fc00000U;
#else
	return (i & 0x7fc00000U) == 0x7f800000U && (i & 0x003fffffU);
#endif
}

/* Every binary encoding is canonical but a signaling NaN, which comes
   back quiet with invalid raised: the addition quiets it the way the
   hardware (or soft-fp) does, as glibc does it.  Everything else is
   copied as bits.  */
int canonicalize(double *cx, const double *x)
{
	uint64_t i;

	memcpy(&i, x, sizeof i);
	if (snan64(i)) {
		double v = *x;

		*cx = v + v;
	} else
		memcpy(cx, &i, sizeof i);
	return 0;
}

int canonicalizef(float *cx, const float *x)
{
	uint32_t i;

	memcpy(&i, x, sizeof i);
	if (snan32(i)) {
		float v = *x;

		*cx = v + v;
	} else
		memcpy(cx, &i, sizeof i);
	return 0;
}

/* The arguments arrive by value; on i386 that is on the stack, and the
   bits are read without loading them into the FPU.  */
int __issignaling(double x)
{
	return snan64(c23_asuint64(x));
}
libm_hidden_def(__issignaling)

int __issignalingf(float x)
{
	return snan32(c23_asuint32(x));
}
libm_hidden_def(__issignalingf)

/* An unordered pair raises invalid even for quiet NaNs.  */
int __iseqsig(double x, double y)
{
	if (isunordered(x, y)) {
		(void) c23_invalid();
		__set_errno(EDOM);
		return 0;
	}
	return x == y;
}
libm_hidden_def(__iseqsig)

int __iseqsigf(float x, float y)
{
	return __iseqsig(x, y);
}
libm_hidden_def(__iseqsigf)

#if C23_LDBL
# if C23_LDBL_MANT_DIG == 53
int canonicalizel(long double *cx, const long double *x)
{
	double d;
	int r;

	memcpy(&d, x, sizeof d);
	r = canonicalize(&d, &d);
	memcpy(cx, &d, sizeof d);
	return r;
}

int __issignalingl(long double x)
{
	double d;

	memcpy(&d, &x, sizeof d);
	return __issignaling(d);
}
# elif defined C23_LDBL_IBM
/* A double-double is canonical when the low part is zero, or small
   enough that the high part is the value rounded to double; a NaN
   ignores its low part, an infinity must have it zero.  glibc's
   ldbl-128ibm rule.  */
int __iscanonicall(long double x)
{
	double hi, lo;
	uint64_t ix, iy;
	int hexp, lexp, low_p2;

	c23_ldunpack(&x, &hi, &lo);
	ix = c23_asuint64(hi) & 0x7fffffffffffffffULL;
	iy = c23_asuint64(lo) & 0x7fffffffffffffffULL;
	if (iy == 0)
		return 1;
	hexp = (int) (ix >> 52);
	if (hexp == 0x7ff)
		return ix != 0x7ff0000000000000ULL;
	lexp = (int) (iy >> 52);
	if (lexp == 0) {				/* subnormal low part */
		lexp = 12 - __builtin_clzll(iy);
		low_p2 = iy == (1ULL << (51 + lexp));
	} else
		low_p2 = (iy & 0x000fffffffffffffULL) == 0;
	return hexp - lexp > 53
	       || (hexp - lexp == 53 && low_p2 && (ix & 1) == 0);
}

/* NaN-ness is the high part's.  */
int __issignalingl(long double x)
{
	double hi, lo;

	c23_ldunpack(&x, &hi, &lo);
	return snan64(c23_asuint64(hi));
}

int canonicalizel(long double *cx, const long double *x)
{
	long double v = *x;

	if (!__iscanonicall(v))
		return 1;
	if (__issignalingl(v))
		*cx = v + v;
	else
		memcpy(cx, x, sizeof *cx);
	return 0;
}
# elif defined C23_LDBL_BITS
int canonicalizel(long double *cx, const long double *x)
{
	struct c23_ldbits b;

	c23_ldget(x, &b);
#  if defined __i386__ || defined __x86_64__
	/* x87: the encodings that are no value fail, as in glibc: a set
	   exponent without the integer bit, and pseudo-denormals.  */
	if ((b.hi != 0) != !!(b.lo >> 63))
		return 1;
#  endif
	if (c23_ldsnan(&b)) {
		long double v = *x;

		*cx = v + v;
	} else
		c23_ldput(cx, &b);
	return 0;
}

int __issignalingl(long double x)
{
	struct c23_ldbits b;

	c23_ldget(&x, &b);
#  if defined __i386__ || defined __x86_64__
	/* Unnormals, pseudo-infinities and pseudo-NaNs raise invalid like
	   a signaling NaN; glibc counts them as one.  */
	if (b.hi != 0 && !(b.lo >> 63))
		return 1;
#  endif
	return c23_ldsnan(&b);
}

#  if defined __i386__ || defined __x86_64__
/* x87 has encodings that are no value: a set exponent without the
   integer bit (unnormals, pseudo-NaNs) and pseudo-denormals.  */
int __iscanonicall(long double x)
{
	struct c23_ldbits b;

	c23_ldget(&x, &b);
	return (b.hi == 0) == !(b.lo >> 63);
}
#  endif
# else
#  error "canonicalize: unknown long double format"
# endif
libm_hidden_def(__issignalingl)

int __iseqsigl(long double x, long double y)
{
	if (isunordered(x, y)) {
		(void) c23_invalid();
		__set_errno(EDOM);
		return 0;
	}
	return x == y;
}
libm_hidden_def(__iseqsigl)
#endif
