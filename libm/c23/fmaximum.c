/* fmaximum, fminimum and their _num, _mag and _mag_num forms (ISO C23).
   The plain forms return NaN if either argument is one and order -0
   below +0; the _num forms prefer a number to a NaN; the _mag forms
   compare magnitudes first.

   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#include "c23_private.h"

/* The magnitudes come from a negation, not fabsl, and the tests from
   the builtins, not isnan and signbit: both would go through double
   for long double and raise overflow on the way.  */
#undef isnan
#undef signbit
#define isnan(x)	c23_isnan (x)
#define signbit(x)	c23_signbit (x)

#define DEFINE(T, S)							\
T fmaximum##S(T x, T y)							\
{									\
	if (isnan(x) || isnan(y))					\
		return x + y;						\
	if (x != y)							\
		return x > y ? x : y;					\
	return signbit(x) ? y : x;					\
}									\
									\
T fminimum##S(T x, T y)							\
{									\
	if (isnan(x) || isnan(y))					\
		return x + y;						\
	if (x != y)							\
		return x < y ? x : y;					\
	return signbit(x) ? x : y;					\
}									\
									\
T fmaximum_mag##S(T x, T y)						\
{									\
	T ax = x < 0 ? -x : x, ay = y < 0 ? -y : y;			\
									\
	if (isnan(x) || isnan(y))					\
		return x + y;						\
	if (ax != ay)							\
		return ax > ay ? x : y;					\
	return fmaximum##S(x, y);					\
}									\
									\
T fminimum_mag##S(T x, T y)						\
{									\
	T ax = x < 0 ? -x : x, ay = y < 0 ? -y : y;			\
									\
	if (isnan(x) || isnan(y))					\
		return x + y;						\
	if (ax != ay)							\
		return ax < ay ? x : y;					\
	return fminimum##S(x, y);					\
}									\
									\
/* A signaling NaN still raises invalid, but the number wins.  */	\
T fmaximum_num##S(T x, T y)						\
{									\
	if (isnan(x) || isnan(y)) {					\
		volatile T t = x + y;					\
									\
		(void) t;						\
		return isnan(x) ? (isnan(y) ? x + y : y) : x;		\
	}								\
	return fmaximum##S(x, y);					\
}									\
									\
T fminimum_num##S(T x, T y)						\
{									\
	if (isnan(x) || isnan(y)) {					\
		volatile T t = x + y;					\
									\
		(void) t;						\
		return isnan(x) ? (isnan(y) ? x + y : y) : x;		\
	}								\
	return fminimum##S(x, y);					\
}									\
									\
T fmaximum_mag_num##S(T x, T y)						\
{									\
	if (isnan(x) || isnan(y))					\
		return fmaximum_num##S(x, y);				\
	return fmaximum_mag##S(x, y);					\
}									\
									\
T fminimum_mag_num##S(T x, T y)						\
{									\
	if (isnan(x) || isnan(y))					\
		return fminimum_num##S(x, y);				\
	return fminimum_mag##S(x, y);					\
}

DEFINE(double, )
DEFINE(float, f)
#if C23_LDBL
DEFINE(long double, l)
#endif
