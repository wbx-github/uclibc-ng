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

/* __ieee754_log10(x)
 * Return the base 10 logarithm of x
 *
 * Method :
 *   1. Argument Reduction: find k and f such that
 *			x = 2^k * (1+f),
 *	   where  sqrt(2)/2 < 1+f < sqrt(2) .
 *
 *   2. Approximation of log(1+f), as in __ieee754_log: with s = f/(2+f),
 *	log(1+f) = f - hfsq + s*(hfsq+R), hfsq = f*f/2 and R the degree 14
 *	Remes polynomial in s*s whose coefficients are Lg1..Lg7 below.
 *
 *   3. Finally,  log10(x) = k*log10(2) + log(1+f)/ln10.
 *
 *	log(1+f) is kept as an unevaluated sum hi+lo, with the low word of
 *	hi cleared so that hi*ivln10hi is exact; 1/ln10 and log10(2) are
 *	each carried in two words.  Feeding an already rounded log(x) into
 *	a single multiply by 1/ln10, as this file used to, rounds twice and
 *	costs the result its last bit.
 *
 * Special cases:
 *	log10(x) is NaN with signal if x < 0;
 *	log10(+INF) is +INF with no signal; log10(0) is -INF with signal;
 *	log10(NaN) is that NaN with no signal;
 *	log10(10**N) = N  for N=0,1,...,22.
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following constants.
 * The decimal values may be used, provided that the compiler will convert
 * from decimal to binary accurately enough to produce the hexadecimal values
 * shown.
 */

#include "math.h"
#include "math_private.h"

static const double
two54      =  1.80143985094819840000e+16, /* 43500000 00000000 */
ivln10hi   =  4.34294481878168880781e-01, /* 3FDBCB7B 15200000 */
ivln10lo   =  2.50829467116452752298e-11, /* 3DBB9438 CA9AADD5 */
log10_2hi  =  3.01029995663611771306e-01, /* 3FD34413 509F6000 */
log10_2lo  =  3.69423907715893078616e-13, /* 3D59FEF3 11F12B36 */
Lg1 = 6.666666666666735130e-01,  /* 3FE55555 55555593 */
Lg2 = 3.999999999940941908e-01,  /* 3FD99999 9997FA04 */
Lg3 = 2.857142874366239149e-01,  /* 3FD24924 94229359 */
Lg4 = 2.222219843214978396e-01,  /* 3FCC71C5 1D8E78AF */
Lg5 = 1.818357216161805012e-01,  /* 3FC74664 96CB03DE */
Lg6 = 1.531383769920937332e-01,  /* 3FC39A09 D078C69F */
Lg7 = 1.479819860511658591e-01;  /* 3FC2F112 DF3E5244 */

double __ieee754_log10(double x)
{
	double hfsq,f,s,z,R,w,hi,lo,val_hi,val_lo,y,y2;
	int32_t k,hx,i;
	u_int32_t lx;

	EXTRACT_WORDS(hx,lx,x);

	k=0;
	if (hx < 0x00100000) {			/* x < 2**-1022  */
	    if (((hx&0x7fffffff)|lx)==0)
		return -two54/(x-x);		/* log(+-0)=-inf */
	    if (hx<0) return (x-x)/(x-x);	/* log(-#) = NaN */
	    k -= 54; x *= two54; /* subnormal number, scale up x */
	    GET_HIGH_WORD(hx,x);
	}
	if (hx >= 0x7ff00000) return x+x;
	k += (hx>>20)-1023;
	hx &= 0x000fffff;
	i = (hx+0x95f64)&0x100000;
	SET_HIGH_WORD(x,hx|(i^0x3ff00000));	/* normalize x or x/2 */
	k += (i>>20);
	y  = (double)k;
	f  = x-1.0;
	hfsq = 0.5*f*f;
	s  = f/(2.0+f);
	z  = s*s;
	w  = z*z;
	R  = z*(Lg1+w*(Lg3+w*(Lg5+w*Lg7))) + w*(Lg2+w*(Lg4+w*Lg6));

	/* log(1+f) = hi+lo, hi with a clear low word so hi*ivln10hi is exact */
	hi = f - hfsq;
	SET_LOW_WORD(hi,0);
	lo = (f - hi) - hfsq + s*(hfsq+R);

	val_hi = hi*ivln10hi;
	y2     = y*log10_2hi;
	val_lo = y*log10_2lo + (lo+hi)*ivln10lo + lo*ivln10hi;

	w = y2 + val_hi;
	val_lo += (y2 - w) + val_hi;
	val_hi = w;
	return val_lo + val_hi;
}
