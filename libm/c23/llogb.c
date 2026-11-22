/* llogb (ISO C23): ilogb with a long result.

   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#include "c23_private.h"
#include <limits.h>

long int llogb(double x)
{
	/* Zero, infinity and NaN are domain errors (C23 F.10.3.9).  */
	if (isnan(x) || x == 0 || isinf(x)) {
		c23_raise_invalid();
		return isnan(x) ? FP_LLOGBNAN : (x == 0 ? FP_LLOGB0 : LONG_MAX);
	}
	return ilogb(x);
}

long int llogbf(float x)
{
	return llogb(x);
}

#if C23_LDBL
# ifndef C23_LDBL_BITS		/* long double is double */
long int llogbl(long double x)
{
	return llogb((double) x);
}
# else
long int llogbl(long double x)
{
	struct c23_ldbits b;
	unsigned int e;

	if (c23_isnan(x)) {
		c23_raise_invalid();
		return FP_LLOGBNAN;
	}
	c23_ldget(&x, &b);
	if (c23_ldiszero(&b) || c23_ldisinf(&b)) {
		c23_raise_invalid();
		return c23_ldiszero(&b) ? FP_LLOGB0 : LONG_MAX;
	}
	e = c23_ldexpf(&b);
	if (e != 0)
		return (long int) e - 16383;
	/* Subnormal: the exponent of the leading significand bit.  */
#  if C23_LDBL_MANT_DIG == 64
	return 63 - __builtin_clzll(b.lo) - 16445;
#  else
	return (b.hi != 0
		? 111 - __builtin_clzll(b.hi) + 64
		: 63 - __builtin_clzll(b.lo)) - 16494;
#  endif
}
# endif
#endif
