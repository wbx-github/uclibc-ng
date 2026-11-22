/* fromfp, ufromfp, fromfpx, ufromfpx (ISO C23): round to an integer in
   the given direction and check that it fits in WIDTH bits, signed or
   unsigned.  The x forms also raise inexact when the value changed.

   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#include "c23_private.h"

static double fromfp_common(double x, int rnd, unsigned int width,
			    int is_unsigned, int raise_inexact)
{
	double r, lim;
	C23_KEEP_INEXACT;

	if (width == 0 || !isfinite(x))
		return c23_invalid();
	switch (rnd) {
	case FP_INT_UPWARD:
		r = ceil(x);
		break;
	case FP_INT_DOWNWARD:
		r = floor(x);
		break;
	case FP_INT_TOWARDZERO:
		r = trunc(x);
		break;
	case FP_INT_TONEARESTFROMZERO:
		r = round(x);
		break;
	case FP_INT_TONEAREST:
		r = roundeven(x);
		break;
	default:			/* unspecified; glibc truncates */
		r = trunc(x);
		break;
	}
	C23_PUT_INEXACT_BACK;
	/* 2^width (or 2^(width-1)); past the double range every finite
	   value fits.  */
	if (is_unsigned) {
		lim = width < 1100 ? ldexp(1.0, width) : HUGE_VAL;
		if (r < 0 || r >= lim)
			return c23_invalid();
	} else {
		lim = width < 1100 ? ldexp(1.0, width - 1) : HUGE_VAL;
		if (r < -lim || r >= lim)
			return c23_invalid();
	}
	if (raise_inexact && r != x) {
		volatile double one = 1.0;

		one += 0x1p-60;
	}
	return r;
}

double fromfp(double x, int rnd, unsigned int width)
{
	return fromfp_common(x, rnd, width, 0, 0);
}

double ufromfp(double x, int rnd, unsigned int width)
{
	return fromfp_common(x, rnd, width, 1, 0);
}

double fromfpx(double x, int rnd, unsigned int width)
{
	return fromfp_common(x, rnd, width, 0, 1);
}

double ufromfpx(double x, int rnd, unsigned int width)
{
	return fromfp_common(x, rnd, width, 1, 1);
}

float fromfpf(float x, int rnd, unsigned int width)
{
	return fromfp_common(x, rnd, width, 0, 0);
}

float ufromfpf(float x, int rnd, unsigned int width)
{
	return fromfp_common(x, rnd, width, 1, 0);
}

float fromfpxf(float x, int rnd, unsigned int width)
{
	return fromfp_common(x, rnd, width, 0, 1);
}

float ufromfpxf(float x, int rnd, unsigned int width)
{
	return fromfp_common(x, rnd, width, 1, 1);
}

#if C23_LDBL
# ifndef C23_LDBL_BITS		/* long double is double */
#  define fromfpl_common(x, r, w, u, i)	fromfp_common(x, r, w, u, i)
# else
/* Exactly, on the bits: clearing the fractional bits is trunc, the
   remainder x - trunc(x) is exact, and adding 1 to an integer below
   2^LDBL_MANT_DIG is exact too.  Nothing here raises inexact, so the
   x forms raise it themselves.  */
static long double fromfpl_common(long double x, int rnd, unsigned int width,
				  int is_unsigned, int raise_inexact)
{
	long double r, t, f, lim;

	if (width == 0 || !c23_isfinite(x))
		return c23_invalid();
	t = c23_ldtrunc(x);
	f = x - t;				/* |f| < 1, exact */
	switch (rnd) {
	case FP_INT_UPWARD:
		r = f > 0 ? t + 1.0L : t;
		break;
	case FP_INT_DOWNWARD:
		r = f < 0 ? t - 1.0L : t;
		break;
	case FP_INT_TOWARDZERO:
		r = t;
		break;
	case FP_INT_TONEARESTFROMZERO:
		r = f >= 0.5L ? t + 1.0L : (f <= -0.5L ? t - 1.0L : t);
		break;
	case FP_INT_TONEAREST:
		if (f > 0.5L)
			r = t + 1.0L;
		else if (f < -0.5L)
			r = t - 1.0L;
		else if (f == 0.5L || f == -0.5L)
			r = 2.0L * c23_ldtrunc(x * 0.5L);
		else
			r = t;
		break;
	default:			/* unspecified; glibc truncates */
		r = t;
		break;
	}
	/* 2^width (or 2^(width-1)); past the long double range every
	   finite value fits.  */
	if (is_unsigned) {
		lim = width < 16384 ? c23_ldexp2((int) width) : HUGE_VALL;
		if (r < 0 || r >= lim)
			return c23_invalid();
	} else {
		lim = width < 16384 ? c23_ldexp2((int) width - 1) : HUGE_VALL;
		if (r < -lim || r >= lim)
			return c23_invalid();
	}
	if (raise_inexact && r != x) {
		volatile double one = 1.0;

		one += 0x1p-60;
	}
	return r;
}
# endif

long double fromfpl(long double x, int rnd, unsigned int width)
{
	return fromfpl_common(x, rnd, width, 0, 0);
}

long double ufromfpl(long double x, int rnd, unsigned int width)
{
	return fromfpl_common(x, rnd, width, 1, 0);
}

long double fromfpxl(long double x, int rnd, unsigned int width)
{
	return fromfpl_common(x, rnd, width, 0, 1);
}

long double ufromfpxl(long double x, int rnd, unsigned int width)
{
	return fromfpl_common(x, rnd, width, 1, 1);
}
#endif
