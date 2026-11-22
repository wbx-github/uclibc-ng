/* exp10m1 and exp2m1 (ISO C23): 10^x - 1 and 2^x - 1.  Near zero they
   are expm1 of x ln10 (x ln2) with the product carried as head and
   tail; further out the subtraction loses little.

   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#include "c23_private.h"

static const double LN10_HI = 0x1.26bb1bbb55516p+1;
static const double LN10_LO = -0x1.f48ad494ea3e9p-53;
static const double LN2_HI = 0x1.62e42fefa39efp-1;
static const double LN2_LO = 0x1.abc9e3b39803fp-56;

/* -1 with inexact raised and the rounding mode respected; written out
   so that the compiler cannot fold it to a plain -1.0.  */
static double minus_one_inexact(void)
{
	volatile double one = 1.0;

	return -one + 0x1p-70;
}

/* expm1(x * (c_hi + c_lo)) for small x */
static double expm1_scaled(double x, double c_hi, double c_lo)
{
	double hi = x * c_hi;
	double lo = fma(x, c_hi, -hi) + x * c_lo;
	double e = expm1(hi);

	return e + (1.0 + e) * lo;
}

double exp10m1(double x)
{
	if (isnan(x))
		return x + x;
	if (isinf(x))
		return x > 0 ? x : -1.0;
	if (x == 0)				/* keeps the sign of a zero */
		return x;
	if (fabs(x) < 0.5)
		return expm1_scaled(x, LN10_HI, LN10_LO);
	if (x < -20)				/* -1, without a spurious underflow */
		return minus_one_inexact();
	return exp10(x) - 1.0;
}

double exp2m1(double x)
{
	if (isnan(x))
		return x + x;
	if (isinf(x))
		return x > 0 ? x : -1.0;
	if (x == 0)				/* keeps the sign of a zero */
		return x;
	if (fabs(x) < 1.0)
		return expm1_scaled(x, LN2_HI, LN2_LO);
	if (x < -60)
		return minus_one_inexact();
	return exp2(x) - 1.0;
}

float exp10m1f(float x)
{
	return exp10m1(x);
}

float exp2m1f(float x)
{
	return exp2m1(x);
}

#if C23_LDBL
long double exp10m1l(long double x)
{
	return exp10m1((double) x);
}

long double exp2m1l(long double x)
{
	return exp2m1((double) x);
}
#endif
