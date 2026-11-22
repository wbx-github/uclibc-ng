/* roundeven (ISO C23): round to the nearest integer, ties to even,
   whatever the current rounding mode.

   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#include "c23_private.h"

/* round() takes ties away from zero; for a tie, r - x is exactly 0.5
   (both are multiples of ulp(x)), and halving gives the even one.  */
double roundeven(double x)
{
	double r;
	C23_KEEP_INEXACT;

	if (!isfinite(x))
		return x + x;
	r = round(x);
	if (fabs(r - x) == 0.5)
		r = 2.0 * round(x * 0.5);
	C23_PUT_INEXACT_BACK;
	return r;
}

float roundevenf(float x)
{
	float r;
	C23_KEEP_INEXACT;

	if (!isfinite(x))
		return x + x;
	r = roundf(x);
	if (fabsf(r - x) == 0.5f)
		r = 2.0f * roundf(x * 0.5f);
	C23_PUT_INEXACT_BACK;
	return r;
}

#if C23_LDBL
# ifndef C23_LDBL_BITS		/* long double is double */
long double roundevenl(long double x)
{
	return roundeven((double) x);
}
# else
/* Exactly, on the bits: clearing the fractional bits is trunc, the
   remainder x - trunc(x) is exact, and adding 1 to an integer below
   2^LDBL_MANT_DIG is exact too.  A tie goes to the even neighbour
   through trunc(x / 2) * 2, as above.  */
long double roundevenl(long double x)
{
	long double t, r;

	if (!c23_isfinite(x))
		return x + x;
	t = c23_ldtrunc(x);
	r = x - t;
	if (r > 0.5L)
		t += 1.0L;
	else if (r < -0.5L)
		t -= 1.0L;
	else if (r == 0.5L || r == -0.5L)
		t = 2.0L * c23_ldtrunc(x * 0.5L);
	return t;
}
# endif
#endif
