/* Taken from newlib (libm/common/s_remquo.c), which took it from FreeBSD's
 * msun and notes: "Adapted for Newlib, 2009.  (Allow for int < 32 bits; return
 * *quo=0 during errors to make test scripts easier.)"
 *
 * What it replaces was newlib's own 2002 version, imported here in 2008 with
 * c7a58c2ed.  That one formed fabs(x / y) and lrint()ed it before looking at
 * the arguments at all, so a zero y raised divide-by-zero and a NaN raised
 * invalid -- neither of which remquo may raise -- and it kept only seven bits
 * of the quotient.  newlib replaced it in 2009 for the same reasons.
 */
/* @(#)e_fmod.c 1.3 95/01/18 */
/*-
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

#include <limits.h>
#include "math.h"
#include "math_private.h"

/* For quotient, return either all 31 bits that can from calculation (using
 * int32_t), or as many as can fit into an int that is smaller than 32 bits.  */
#if INT_MAX > 0x7FFFFFFFL
  #define QUO_MASK 0x7FFFFFFF
# else
  #define QUO_MASK INT_MAX
#endif

static const double Zero[] = {0.0, -0.0,};

/*
 * Return the IEEE remainder and set *quo to the last n bits of the
 * quotient, rounded to the nearest integer.  We choose n=31--if that many fit--
 * because we wind up computing all the integer bits of the quotient anyway as
 * a side-effect of computing the remainder by the shift and subtract
 * method.  In practice, this is far more bits than are needed to use
 * remquo in reduction algorithms.
 */
double
remquo(double x, double y, int *quo)
{
	__int32_t n,hx,hy,hz,ix,iy,sx,i;
	__uint32_t lx,ly,lz,q,sxy;

	EXTRACT_WORDS(hx,lx,x);
	EXTRACT_WORDS(hy,ly,y);
	sxy = (hx ^ hy) & 0x80000000;
	sx = hx&0x80000000;		/* sign of x */
	hx ^=sx;		/* |x| */
	hy &= 0x7fffffff;	/* |y| */

    /* purge off exception values */
	if((hy|ly)==0||(hx>=0x7ff00000)||	/* y=0,or x not finite */
	  ((hy|((ly|-ly)>>31))>0x7ff00000))  {	/* or y is NaN */
	    *quo = 0;	/* Not necessary, but return consistent value */
	    return (x*y)/(x*y);
	}
	if(hx<=hy) {
	    if((hx<hy)||(lx<ly)) {
		q = 0;
		goto fixup;	/* |x|<|y| return x or x-y */
	    }
	    if(lx==ly) {
		*quo = (sxy ? -1 : 1);
		return Zero[(__uint32_t)sx>>31];	/* |x|=|y| return x*0 */
	    }
	}

    /* determine ix = ilogb(x) */
	if(hx<0x00100000) {	/* subnormal x */
	    if(hx==0) {
		for (ix = -1043, i=lx; i>0; i<<=1) ix -=1;
	    } else {
		for (ix = -1022,i=(hx<<11); i>0; i<<=1) ix -=1;
	    }
	} else ix = (hx>>20)-1023;

    /* determine iy = ilogb(y) */
	if(hy<0x00100000) {	/* subnormal y */
	    if(hy==0) {
		for (iy = -1043, i=ly; i>0; i<<=1) iy -=1;
	    } else {
		for (iy = -1022,i=(hy<<11); i>0; i<<=1) iy -=1;
	    }
	} else iy = (hy>>20)-1023;

    /* set up {hx,lx}, {hy,ly} and align y to x */
	if(ix >= -1022) 
	    hx = 0x00100000|(0x000fffff&hx);
	else {		/* subnormal x, shift x to normal */
	    n = -1022-ix;
	    if(n<=31) {
	        hx = (hx<<n)|(lx>>(32-n));
	        lx <<= n;
	    } else {
		hx = lx<<(n-32);
		lx = 0;
	    }
	}
	if(iy >= -1022) 
	    hy = 0x00100000|(0x000fffff&hy);
	else {		/* subnormal y, shift y to normal */
	    n = -1022-iy;
	    if(n<=31) {
	        hy = (hy<<n)|(ly>>(32-n));
	        ly <<= n;
	    } else {
		hy = ly<<(n-32);
		ly = 0;
	    }
	}

    /* fix point fmod */
	n = ix - iy;
	q = 0;
	while(n--) {
	    hz=hx-hy;lz=lx-ly; if(lx<ly) hz -= 1;
	    if(hz<0){hx = hx+hx+(lx>>31); lx = lx+lx;}
	    else {hx = hz+hz+(lz>>31); lx = lz+lz; q++;}
	    q <<= 1;
	}
	hz=hx-hy;lz=lx-ly; if(lx<ly) hz -= 1;
	if(hz>=0) {hx=hz;lx=lz;q++;}

    /* convert back to floating value and restore the sign */
	if((hx|lx)==0) {			/* return sign(x)*0 */
	    q &= QUO_MASK;
	    *quo = (sxy ? -q : q);
	    return Zero[(__uint32_t)sx>>31];
	}
	while(hx<0x00100000) {		/* normalize x */
	    hx = hx+hx+(lx>>31); lx = lx+lx;
	    iy -= 1;
	}
	if(iy>= -1022) {	/* normalize output */
	    hx = ((hx-0x00100000)|((iy+1023)<<20));
	} else {		/* subnormal output */
	    n = -1022 - iy;
	    if(n<=20) {
		lx = (lx>>n)|((__uint32_t)hx<<(32-n));
		hx >>= n;
	    } else if (n<=31) {
		lx = (hx<<(32-n))|(lx>>n); hx = sx;
	    } else {
		lx = hx>>(n-32); hx = sx;
	    }
	}
fixup:
	INSERT_WORDS(x,hx,lx);
	y = fabs(y);
	if (y < 0x1p-1021) {
	    if (x+x>y || (x+x==y && (q & 1))) {
		q++;
		x-=y;
	    }
	} else if (x>0.5*y || (x==0.5*y && (q & 1))) {
	    q++;
	    x-=y;
	}
	GET_HIGH_WORD(hx,x);
	SET_HIGH_WORD(x,hx^sx);
	q &= QUO_MASK;
	*quo = (sxy ? -q : q);
	return x;
}
libm_hidden_def(remquo)
