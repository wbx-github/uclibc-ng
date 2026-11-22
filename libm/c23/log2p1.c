/* log2p1, log10p1 and logp1 (ISO C23): logarithms of 1 + x.  log1p
   keeps the small x exact, its result is scaled by 1/ln2 (1/ln10) as a
   head and tail; where 1 + x is exact and away from 1, log2 (log10)
   of it is used, so that log2p1(3) is 2.

   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#include "c23_private.h"

static const double LOG2E_HI = 0x1.71547652b82fep+0;
static const double LOG2E_LO = 0x1.777d0ffda0d24p-56;
static const double LOG10E_HI = 0x1.bcb7b1526e50ep-2;
static const double LOG10E_LO = 0x1.95355baaafad3p-57;

static double scaled_log1p(double x, double c_hi, double c_lo)
{
	double l = log1p(x);
	double hi = l * c_hi;

	if (!isfinite(hi) || hi == 0)
		return hi;
	return hi + (fma(l, c_hi, -hi) + l * c_lo);
}

double log2p1(double x)
{
	double t = 1.0 + x;

	if (fabs(x) > 0.5 && t - 1.0 == x)
		return log2(t);
	return scaled_log1p(x, LOG2E_HI, LOG2E_LO);
}

double log10p1(double x)
{
	double t = 1.0 + x;

	if (fabs(x) > 0.5 && t - 1.0 == x)
		return log10(t);
	return scaled_log1p(x, LOG10E_HI, LOG10E_LO);
}

double logp1(double x)
{
	return log1p(x);
}

float log2p1f(float x)
{
	return log2p1(x);
}

float log10p1f(float x)
{
	return log10p1(x);
}

float logp1f(float x)
{
	return log1p(x);
}

#if C23_LDBL
long double log2p1l(long double x)
{
	return log2p1((double) x);
}

long double log10p1l(long double x)
{
	return log10p1((double) x);
}

long double logp1l(long double x)
{
	return log1p((double) x);
}
#endif
