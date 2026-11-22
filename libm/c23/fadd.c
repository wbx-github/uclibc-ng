/* fadd, fsub, fmul, fdiv, ffma and fsqrt (ISO C23): double operations
   rounded once to float.  See c23_narrow.h.

   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#include "c23_narrow.h"
#include "x87-precision.h"

/* HI made odd if the rest LO of the exact value is not zero.  Only the
   sign of LO counts; HI is not zero.  */
static __inline double round_odd(double hi, double lo)
{
	uint64_t i = c23_asuint64(hi);

	if (lo != 0 && !(i & 1))
		i += ((lo < 0) == (hi < 0)) ? 1 : -1;
	return c23_asdouble(i);
}

/* Below this a double result is far under the float range, and only
   its sign and that it is not zero matter.  */
#define TINY	0x1p-900
/* The smallest double with the sign of the exact product of X and Y,
   standing in for a product that underflowed to zero.  */
#define STICKY(x, y)	(signbit(x) != signbit(y) ? -0x1p-1074 : 0x1p-1074)

/* The workers return the odd-rounded result, or 0 or a non-finite
   value when the caller has to work it out in its own rounding mode
   (exact zeros, infinities, NaNs, overflow).  */

X87_ROUND53_WORKER double add_odd(double x, double y)
{
	double s, e;

	TWO_SUM(double, x, y, s, e);
	if (!isfinite(s) || s == 0)
		return s;
	return round_odd(s, e);
}

X87_ROUND53_WORKER double mul_odd(double x, double y)
{
	double p = x * y;

	if (!isfinite(p))
		return p;
	if (p == 0)
		return (x == 0 || y == 0) ? 0 : STICKY(x, y);
	if (fabs(p) < TINY)
		return p;
	return round_odd(p, fma(x, y, -p));
}

X87_ROUND53_WORKER double div_odd(double x, double y)
{
	double q = x / y, r;

	if (!isfinite(q) || x == 0 || isinf(y))
		return isfinite(q) ? 0 : q;
	if (q == 0)
		return STICKY(x, y);
	if (fabs(q) < TINY)
		return q;
	/* The remainder is exact unless x is tiny; then |y| < 2^60.  */
	if (fabs(x) < TINY) {
		x *= 0x1p200;
		y *= 0x1p200;
	}
	r = fma(-q, y, x);
	return round_odd(q, r == 0 ? 0 : ((r < 0) == (y < 0) ? 1 : -1));
}

X87_ROUND53_WORKER double sqrt_odd(double x)
{
	double s = sqrt(x);

	if (!(x > 0) || isinf(x))
		return x == 0 ? 0 : s;
	if (x < TINY)
		return s;
	return round_odd(s, fma(-s, s, x));
}

/* x y + z, all finite, x and y not zero.  The exact sum of z, x y
   = ph + pl is reduced to h + l (+ te) with TwoSum steps.  */
X87_ROUND53_WORKER double fma_odd(double x, double y, double z)
{
	double ph, pl, s1, e1, s2, e2, t, te, h, l, r;
	int scaled = 0;

	ph = x * y;
	if (!(fabs(ph) <= 0x1p1000) || fabs(z) > 0x1p1000) {
		/* Scale so that nothing overflows; a z that becomes inexact
		   only matters when the result overflows float anyway.  */
		if (fabs(x) > fabs(y))
			x *= 0x1p-128;
		else
			y *= 0x1p-128;
		z *= 0x1p-128;
		ph = x * y;
		scaled = 1;
	}
	if (fabs(ph) < TINY) {
		if (fabs(z) < 0x1p-800)
			return 0;		/* all far below float */
		r = round_odd(z, ph != 0 ? ph : STICKY(x, y));
	} else {
		pl = fma(x, y, -ph);
		TWO_SUM(double, z, ph, s1, e1);
		TWO_SUM(double, s1, pl, s2, e2);
		TWO_SUM(double, e1, e2, t, te);
		TWO_SUM(double, s2, t, h, l);
		if (h == 0)			/* s2 + t == 0, te is exact */
			r = te;
		else
			r = round_odd(h, l != 0 ? l : te);
	}
	return scaled ? r * 0x1p128 : r;
}

#define NARROW(call)							\
	volatile double r;						\
	X87_ROUND53_DECL;						\
	{								\
		RN_BEGIN();						\
		X87_ROUND53_BEGIN();					\
		r = call;						\
		X87_ROUND53_END();					\
		RN_END();						\
	}

float fadd(double x, double y)
{
	NARROW(add_odd(x, y));
	if (r == 0 || !isfinite(r))
		return LATE(x) + LATE(y);
	return r;
}

float fsub(double x, double y)
{
	return fadd(x, -y);
}

float fmul(double x, double y)
{
	NARROW(mul_odd(x, y));
	if (r == 0 || !isfinite(r))
		return LATE(x) * LATE(y);
	return r;
}

float fdiv(double x, double y)
{
	NARROW(div_odd(x, y));
	if (r == 0 || !isfinite(r))
		return LATE(x) / LATE(y);
	return r;
}

float fsqrt(double x)
{
	NARROW(sqrt_odd(x));
	if (r == 0 || !isfinite(r))
		return sqrt(LATE(x));
	return r;
}

float ffma(double x, double y, double z)
{
	if (!isfinite(x) || !isfinite(y) || !isfinite(z) || x == 0 || y == 0)
		return fma(x, y, z);
	{
		NARROW(fma_odd(x, y, z));
		if (r == 0 || !isfinite(r))
			return fma(LATE(x), LATE(y), LATE(z));
		return r;
	}
}
