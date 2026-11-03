/* expm1, taken from glibc (sysdeps/ieee754/dbl-64/s_expm1.c, LGPL-2.1-or-later
   like this library).  Selected by UCLIBC_LIBM_OPTIMIZED.

   This is the same fdlibm routine libm/s_expm1.c has, with what glibc changed
   since 1993 -- the constants moved into an array "for performance improvement on
   pipelined processors", and the exception handling reworked.

   Measured, and worth knowing before anyone repeats the exercise elsewhere: the
   accuracy is the same.  Over the 1000 points of the test's reference table both
   miss the correctly rounded result 61 times, and 60 of those are the same points.
   Over 400000 points it is 14301 against 14294 -- seven fewer, which is noise.
   The code is the same size to the byte on riscv32 (1716) and about 13 percent
   faster.  Roughly a hundred of our files share the fdlibm heritage, and this says
   the corrections in glibc's copies are not the cheap way to better accuracy;
   replacing the algorithm is (see libm/accurate/s_expm1.c, which misses none).

   Changes against the original: the glibc-internal headers give way to the three
   macros actually used, and the alias boilerplate is gone.  */

#include <math.h>
#include <errno.h>
#include "math_private.h"

#if defined __FLT_EVAL_METHOD__ && __FLT_EVAL_METHOD__ != 0
# define math_narrow_eval(x) ({ volatile double __v = (x); __v; })
#else
# define math_narrow_eval(x) (x)
#endif
#define math_force_eval(x) do { volatile double __v = (x); (void) __v; } while (0)
#define math_check_force_underflow(x) do { } while (0)
#define __glibc_unlikely(x) __builtin_expect((x), 0)

#define one Q[0]
static const double
  huge = 1.0e+300,
  tiny = 1.0e-300,
  o_threshold = 7.09782712893383973096e+02,  /* 0x40862E42, 0xFEFA39EF */
  ln2_hi = 6.93147180369123816490e-01,       /* 0x3fe62e42, 0xfee00000 */
  ln2_lo = 1.90821492927058770002e-10,       /* 0x3dea39ef, 0x35793c76 */
  invln2 = 1.44269504088896338700e+00,       /* 0x3ff71547, 0x652b82fe */
/* scaled coefficients related to expm1 */
  Q[] = { 1.0, -3.33333333333331316428e-02, /* BFA11111 111110F4 */
	  1.58730158725481460165e-03, /* 3F5A01A0 19FE5585 */
	  -7.93650757867487942473e-05, /* BF14CE19 9EAADBB7 */
	  4.00821782732936239552e-06, /* 3ED0CFCA 86E65239 */
	  -2.01099218183624371326e-07 }; /* BE8AFDB7 6E09C32D */

#ifndef SECTION
# define SECTION
#endif

SECTION
double
expm1 (double x)
{
  double y, hi, lo, c, t, e, hxs, hfx, r1, h2, h4, R1, R2, R3;
  int32_t k, xsb;
  uint32_t hx;

  GET_HIGH_WORD (hx, x);
  xsb = hx & 0x80000000;                /* sign bit of x */
  if (xsb == 0)
    y = x;
  else
    y = -x;                             /* y = |x| */
  hx &= 0x7fffffff;                     /* high word of |x| */

  /* filter out huge and non-finite argument */
  if (hx >= 0x4043687A)                         /* if |x|>=56*ln2 */
    {
      if (hx >= 0x40862E42)                     /* if |x|>=709.78... */
	{
	  if (hx >= 0x7ff00000)
	    {
	      uint32_t low;
	      GET_LOW_WORD (low, x);
	      if (((hx & 0xfffff) | low) != 0)
		return x + x;            /* NaN */
	      else
		return (xsb == 0) ? x : -1.0;    /* exp(+-inf)={inf,-1} */
	    }
	  if (x > o_threshold)
	    {
	      __set_errno (ERANGE);
	      return huge * huge;   /* overflow */
	    }
	}
      if (xsb != 0)      /* x < -56*ln2, return -1.0 with inexact */
	{
	  math_force_eval (x + tiny);           /* raise inexact */
	  return tiny - one;            /* return -1 */
	}
    }

  /* argument reduction */
  if (hx > 0x3fd62e42)                  /* if  |x| > 0.5 ln2 */
    {
      if (hx < 0x3FF0A2B2)              /* and |x| < 1.5 ln2 */
	{
	  if (xsb == 0)
	    {
	      hi = x - ln2_hi; lo = ln2_lo;  k = 1;
	    }
	  else
	    {
	      hi = x + ln2_hi; lo = -ln2_lo;  k = -1;
	    }
	}
      else
	{
	  k = invln2 * x + ((xsb == 0) ? 0.5 : -0.5);
	  t = k;
	  hi = x - t * ln2_hi;          /* t*ln2_hi is exact here */
	  lo = t * ln2_lo;
	}
      x = hi - lo;
      c = (hi - x) - lo;
    }
  else if (hx < 0x3c900000)             /* when |x|<2**-54, return x */
    {
      math_check_force_underflow (x);
      t = huge + x;     /* return x with inexact flags when x!=0 */
      return x - (t - (huge + x));
    }
  else
    k = 0;

  /* x is now in primary range */
  hfx = 0.5 * x;
  hxs = x * hfx;
  R1 = one + hxs * Q[1]; h2 = hxs * hxs;
  R2 = Q[2] + hxs * Q[3]; h4 = h2 * h2;
  R3 = Q[4] + hxs * Q[5];
  r1 = R1 + h2 * R2 + h4 * R3;
  t = 3.0 - r1 * hfx;
  e = hxs * ((r1 - t) / (6.0 - x * t));
  if (k == 0)
    return x - (x * e - hxs);                   /* c is 0 */
  else
    {
      e = (x * (e - c) - c);
      e -= hxs;
      if (k == -1)
	return 0.5 * (x - e) - 0.5;
      if (k == 1)
	{
	  if (x < -0.25)
	    return -2.0 * (e - (x + 0.5));
	  else
	    return one + 2.0 * (x - e);
	}
      if (k <= -2 || k > 56)         /* suffice to return exp(x)-1 */
	{
	  uint32_t high;
	  y = one - (e - x);
	  GET_HIGH_WORD (high, y);
	  SET_HIGH_WORD (y, high + (k << 20));  /* add k to y's exponent */
	  return y - one;
	}
      t = one;
      if (k < 20)
	{
	  uint32_t high;
	  SET_HIGH_WORD (t, 0x3ff00000 - (0x200000 >> k));    /* t=1-2^-k */
	  y = t - (e - x);
	  GET_HIGH_WORD (high, y);
	  SET_HIGH_WORD (y, high + (k << 20));  /* add k to y's exponent */
	}
      else
	{
	  uint32_t high;
	  SET_HIGH_WORD (t, ((0x3ff - k) << 20));       /* 2^-k */
	  y = x - (e + t);
	  y += one;
	  GET_HIGH_WORD (high, y);
	  SET_HIGH_WORD (y, high + (k << 20));  /* add k to y's exponent */
	}
    }
  return y;
}
libm_hidden_def(expm1)
