/* sinpi, cospi and tanpi (ISO C23): sin, cos and tan of pi times x.
   The argument is reduced exactly by halves (or wholes for tanpi), so
   the fdlibm kernels see |pi r| <= pi/4 with pi r as a head and tail.

   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#include "c23_private.h"

static const double PI_HI = 0x1.921fb54442d18p+1;
static const double PI_LO = 0x1.1a62633145c07p-53;

/* pi r as hi + lo, |r| <= 1/4.  */
static __inline double pi_times(double r, double *lo)
{
	double hi = r * PI_HI;

	*lo = fma(r, PI_HI, -hi) + r * PI_LO;
	return hi;
}

static double sin_pi(double r)
{
	double lo, hi = pi_times(r, &lo);

	return __kernel_sin(hi, lo, 1);
}

static double cos_pi(double r)
{
	double lo, hi = pi_times(r, &lo);

	return __kernel_cos(hi, lo);
}

/* ax = n/2 + r with |r| <= 1/4 and q = n mod 4, for 0 <= ax < 2^52.  */
static __inline double reduce_half(double ax, unsigned int *q)
{
	double n = round(ax * 2.0);

	*q = (unsigned int) ((long long) n & 3);
	return ax - n * 0.5;
}

double sinpi(double x)
{
	double ax = fabs(x), r, s;
	unsigned int q;

	if (!isfinite(x))
		return isnan(x) ? x + x : c23_invalid();
	if (ax >= 0x1p52)			/* an integer */
		return copysign(0.0, x);
	r = reduce_half(ax, &q);
	switch (q) {
	case 0: s = sin_pi(r); break;
	case 1: s = cos_pi(r); break;
	case 2: s = -sin_pi(r); break;
	default: s = -cos_pi(r); break;
	}
	if (s == 0)				/* sinpi(n) is +0 for n > 0 */
		s = 0.0;
	return signbit(x) ? -s : s;		/* and sinpi(-0) is -0 */
}

double cospi(double x)
{
	double ax = fabs(x), r, s;
	unsigned int q;

	if (!isfinite(x))
		return isnan(x) ? x + x : c23_invalid();
	if (ax >= 0x1p52)
		return (ax < 0x1p53 && ((long long) ax & 1)) ? -1.0 : 1.0;
	r = reduce_half(ax, &q);
	switch (q) {
	case 0: s = cos_pi(r); break;
	case 1: s = -sin_pi(r); break;
	case 2: s = -cos_pi(r); break;
	default: s = sin_pi(r); break;
	}
	if (s == 0)				/* cospi(n + 1/2) is +0 */
		s = 0.0;
	return s;
}

double tanpi(double x)
{
	double ax = fabs(x), n, r, t, lo, hi;

	if (!isfinite(x))
		return isnan(x) ? x + x : c23_invalid();
	if (ax >= 0x1p52) {
		n = ax;
		r = 0;
	} else {
		n = round(ax);
		r = ax - n;			/* |r| <= 1/2, exact */
	}
	if (r == 0) {
		/* +0 for positive even and negative odd n, else -0.  */
		t = (n < 0x1p53 && ((long long) n & 1)) ? -0.0 : 0.0;
	} else if (fabs(r) == 0.5) {
		/* A pole: +inf at n + 1/2 for even n, -inf for odd n.  */
		n = ax - 0.5;
		t = c23_pole(((long long) n & 1) ? -1.0 : 1.0);
	} else if (fabs(r) <= 0.25) {
		hi = pi_times(r, &lo);
		t = __kernel_tan(hi, lo, 1);
	} else {
		/* tan(pi r) = 1 / tan(pi (+-1/2 - r)) */
		hi = pi_times(copysign(0.5, r) - r, &lo);
		t = -__kernel_tan(hi, lo, -1);
	}
	return signbit(x) ? -t : t;
}

float sinpif(float x)
{
	return sinpi(x);
}

float cospif(float x)
{
	return cospi(x);
}

float tanpif(float x)
{
	return tanpi(x);
}

#if C23_LDBL
long double sinpil(long double x)
{
	return sinpi((double) x);
}

long double cospil(long double x)
{
	return cospi((double) x);
}

long double tanpil(long double x)
{
	return tanpi((double) x);
}
#endif
