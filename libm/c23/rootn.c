/* rsqrt, rootn, pown, powr and compoundn (ISO C23).

   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#include "c23_private.h"
#include "x87-precision.h"

double rsqrt(double x)
{
	return 1.0 / sqrt(x);
}

/* The n-th root.  pow with the rounded 1/n is off by up to a few
   hundred ulp for large |log x|; one Newton step on y^n = |x| fixes it.
   For a huge |n| the root is so close to 1 that y^n jumps between
   neighbours of y, and the step is left out.  */
double rootn(double x, long long n)
{
	double ax, y, z;
	int odd = n & 1, m;

	if (n == 0)
		return c23_invalid();
	if (isnan(x))
		return x + x;
	if (x < 0 && !odd)
		return c23_invalid();
	if (x == 0) {
		if (n > 0)
			return odd ? x : 0.0;
		return c23_pole(odd ? copysign(1.0, x) : 1.0);
	}
	if (isinf(x))
		return n > 0 ? x : (odd ? copysign(0.0, x) : 0.0);
	switch (n) {
	case 1:
		return x;
	case 2:
		return sqrt(x);
	case 3:
		return cbrt(x);
	case -1:
		return 1.0 / x;
	case -2:
		return rsqrt(x);
	}
	ax = fabs(x);
	/* Near the ends of the range y^n overflows or is too short for the
	   Newton step; scale by 2^m with m a multiple of n first.  */
	m = 0;
	if ((ax < 0x1p-1000 || ax > 0x1p1000) && n >= -1024 && n <= 1024) {
		int an = n < 0 ? -n : (int) n;

		m = an * ((128 + an - 1) / an);
		if (ax > 1)
			m = -m;
		ax = ldexp(ax, m);
	}
	y = pow(ax, 1.0 / (double) n);
	/* The step needs y^n, which is ax again.  Where ax is outside the
	   range and no scaling was possible, that overflows for a rounded
	   1/n; leave the step out rather than raise a spurious overflow.  */
	if (m != 0 || (ax >= 0x1p-1000 && ax <= 0x1p1000)) {
		z = pow(y, (double) n);
		if (isfinite(z) && z != 0 && fabs(z - ax) < 0.5 * z)
			y -= y * ((z - ax) / z) / (double) n;
	}
	if (m)
		y = ldexp(y, -m / (int) n);
	return x < 0 ? -y : y;
}

/* pow with an integer exponent.  It always goes through |x| and sets
   the sign from the parity afterwards: fdlibm's pow leaves the sign
   out of its shortcut for |y| > 2^31 (e_pow.c), so pow(-2, 2^32+1)
   comes back +inf.  Beyond 2^53 the exponent is not exact in double
   and is split into a part that is and a small rest.  */
double pown(double x, long long n)
{
	double r, ax;
	long long n1;

	if (n == 0)		/* 1 even for a quiet NaN, as pow */
		return issignaling(x) ? x + x : 1.0;
	ax = fabs(x);
	if (n >= -(1LL << 53) && n <= (1LL << 53)) {
		r = pow(ax, (double) n);
	} else {
		/* Clear the bits a double cannot hold.  Going through
		   (double) n / 2 * 2 instead would round 2^62 - 1 up and
		   overflow long long at the doubling.  */
		unsigned long long m = n < 0 ? -(unsigned long long) n
					     : (unsigned long long) n;
		int k = 63 - __builtin_clzll(m);

		n1 = n & ~((1LL << (k - 52)) - 1);
		r = pow(ax, (double) n1);
		if (n != n1 && isfinite(r) && r != 0)
			r *= pow(ax, (double) (n - n1));
	}
	return (signbit(x) && (n & 1)) ? -r : r;
}

/* pow defined as exp(y log x): no negative x, and the cases pow
   settles by convention (0^0, inf^0, 1^inf) are domain errors.  */
double powr(double x, double y)
{
	if (isnan(x) || isnan(y))
		return x + y;
	if (x < 0)
		return c23_invalid();
	if (x == 0) {				/* -0 counts as +0 */
		if (y == 0)
			return c23_invalid();
		if (y == -INFINITY)
			return INFINITY;
		return y < 0 ? c23_pole(1.0) : 0.0;
	}
	if (isinf(x)) {
		if (y == 0)
			return c23_invalid();
		return y < 0 ? 0.0 : x;
	}
	if (x == 1) {
		if (isinf(y))
			return c23_invalid();
		return 1.0;
	}
	return pow(x, y);
}

/* (1 + x)^n.  1 + x is split exactly into t + tl, and the result is
   t^n (1 + tl/t)^n: pow does the first more accurately than
   exp(n log1p(x)) would, and the second is exp(n tl/t) to well under an
   ulp since |tl/t| <= 2^-53.  */
/* 1 + x as a head and a tail, then (1 + x)^n = t^n * exp(n * tl / t).
   The tail is only right if each step rounds to double, so the worker
   runs with the x87 precision control set.  */
X87_ROUND53_WORKER double compoundn_worker(double x, long long n)
{
	double t, tl, r;

	t = 1.0 + x;
	/* The tail of the sum, exactly: subtract back the operand that is
	   not the larger one.  */
	tl = x > 1 ? 1.0 - (t - x) : x - (t - 1.0);
	r = pown(t, n);
	if (!isfinite(r) || r == 0)
		/* t^n is out of range, but with the second factor the
		   result may not be: go through the logarithm.  */
		return exp((double) n * log1p(x));
	if (tl == 0)
		return r;
	return r * exp((double) n * (tl / t));
}

double compoundn(double x, long long n)
{
	X87_ROUND53_DECL;
	double r;

	if (isnan(x))
		return (n == 0 && !issignaling(x)) ? 1.0 : x + x;
	if (x < -1)
		return c23_invalid();
	if (n == 0)
		return 1.0;
	if (x == -1)
		return n < 0 ? c23_pole(1.0) : 0.0;
	if (isinf(x))
		return n < 0 ? 0.0 : x;
	X87_ROUND53_BEGIN();
	r = compoundn_worker(x, n);
	X87_ROUND53_END();
	return r;
}

float rsqrtf(float x)
{
	return rsqrt(x);
}

float rootnf(float x, long long n)
{
	return rootn(x, n);
}

float pownf(float x, long long n)
{
	return pown(x, n);
}

float powrf(float x, float y)
{
	return powr(x, y);
}

float compoundnf(float x, long long n)
{
	return compoundn(x, n);
}

#if C23_LDBL
long double rsqrtl(long double x)
{
	return rsqrt((double) x);
}

long double rootnl(long double x, long long n)
{
	return rootn((double) x, n);
}

long double pownl(long double x, long long n)
{
	return pown((double) x, n);
}

long double powrl(long double x, long double y)
{
	return powr((double) x, (double) y);
}

long double compoundnl(long double x, long long n)
{
	return compoundn((double) x, n);
}
#endif
