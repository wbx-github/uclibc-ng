/* Shared by the narrowing functions (fadd, fmul, ... ISO C23).

   They have to round once, from the exact result.  The exact result is
   computed as head plus tail in round-to-nearest, the head made odd if
   the tail is not zero ("round to odd"), and that converted to the
   narrow type in the caller's rounding mode: with at least two more
   bits in the wide type, the double rounding is then exact (Boldo and
   Melquiond, "Emulation of FMA and correctly rounded sums: proved
   algorithms using rounding to odd", IEEE TC 57(4), 2008).

   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#ifndef _C23_NARROW_H
#define _C23_NARROW_H 1

#include "c23_private.h"
#ifdef __UCLIBC_HAS_FENV__
# include <fenv.h>
/* The exact part needs round-to-nearest; the final conversion runs in
   the caller's mode again.  */
# define RN_BEGIN()	int __mode = fegetround();			\
			if (__mode != FE_TONEAREST)			\
				fesetround(FE_TONEAREST)
# define RN_END()	if (__mode != FE_TONEAREST)			\
				fesetround(__mode)
#else
/* Without fenv the mode cannot be changed.  */
# define RN_BEGIN()	do { } while (0)
# define RN_END()	do { } while (0)
#endif

/* gcc knows no FENV_ACCESS and may move or reuse floating-point
   operations across fesetround.  The result leaves the round-to-
   nearest part through a volatile store, and what is computed in the
   caller's mode reads its operands through LATE, after the mode is
   back.  */
#define LATE(v)	(*(volatile __typeof__(v) *) &(v))

/* Knuth's TwoSum: s + e == a + b exactly, s = RN(a + b).  */
#define TWO_SUM(T, a, b, s, e) do {					\
	T __a = (a), __b = (b), __s = __a + __b, __t = __s - __b;	\
	(e) = (__a - __t) + (__b - (__s - __t));			\
	(s) = __s;							\
} while (0)

#endif
