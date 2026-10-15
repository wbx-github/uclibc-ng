/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * __ieee754_scalb(x, fn) is provided for
 * passing various standard test suites.
 * One should use scalbn() instead.
 */

#include "math.h"
#include "math_private.h"
#include <errno.h>

double __ieee754_scalb(double x, double fn)
{
	/* A NaN argument makes the result NaN.  Adding rather than multiplying,
	   because m68k's soft-float __muldf3 returns infinity for inf * NaN --
	   gcc's libgcc/config/m68k/lb1sf68.S, where Lmuldf$a$nf signals overflow
	   for an infinite a without looking at b; the float path next to it does
	   check.  Addition hands back a quiet NaN just as well.  */
	if (isnan(x)||isnan(fn)) return x+fn;
	if (!isfinite(fn)) {
	    if(fn>0.0) return x*fn;
	    else       return x/(-fn);
	}
	if (rint(fn)!=fn) return (fn-fn)/(fn-fn);
	if ( fn > 65000.0) return scalbn(x, 65000);
	if (-fn > 65000.0) return scalbn(x,-65000);
	return scalbn(x,(int)fn);
}
