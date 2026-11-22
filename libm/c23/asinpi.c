/* asinpi, acospi, atanpi and atan2pi (ISO C23): the inverse functions
   in half-revolutions.  Dividing by the double pi keeps the exact
   results exact: asin(1) is that pi over 2, so asinpi(1) is 0.5.

   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#include "c23_private.h"

static const double PI_HI = 0x1.921fb54442d18p+1;

double asinpi(double x)
{
	return asin(x) / PI_HI;
}

double acospi(double x)
{
	return acos(x) / PI_HI;
}

double atanpi(double x)
{
	return atan(x) / PI_HI;
}

double atan2pi(double y, double x)
{
	/* 3 pi / 4 is not exact in double; the specials are spelled out.  */
	if (isinf(x) && isinf(y))
		return copysign(x > 0 ? 0.25 : 0.75, y);
	return atan2(y, x) / PI_HI;
}

float asinpif(float x)
{
	return asinpi(x);
}

float acospif(float x)
{
	return acospi(x);
}

float atanpif(float x)
{
	return atanpi(x);
}

float atan2pif(float y, float x)
{
	return atan2pi(y, x);
}

#if C23_LDBL
long double asinpil(long double x)
{
	return asinpi((double) x);
}

long double acospil(long double x)
{
	return acospi((double) x);
}

long double atanpil(long double x)
{
	return atanpi((double) x);
}

long double atan2pil(long double y, long double x)
{
	return atan2pi((double) y, (double) x);
}
#endif
