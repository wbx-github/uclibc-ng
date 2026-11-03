/* erf from Arm's optimized-routines (math/erf.c, MIT), erfc from fdlibm.
   Selected by UCLIBC_LIBM_OPTIMIZED.

   Both functions live in one file because libm/s_erf.c does, and Arm has no
   scalar erfc -- only an AArch64 vector one -- so erfc stays the generic
   fdlibm routine here, unchanged.

   What Arm's erf buys, and what it costs, measured over 1195 points against
   correctly rounded results: it is 0.21 times the run time of fdlibm on a
   target with an fma instruction, and it misses the correctly rounded result
   111 times where fdlibm misses 35 -- never by more than one step; the file's
   own comment claims a highest error of 1.01 ulp.  So this variant is faster
   and slightly less accurate than the small one.  Without an fma instruction it
   is 9.5 times slower than fdlibm, because the 86 fma steps then become calls
   into libm/s_fma.c; a target that cares should stay with the small variant, or
   take the accurate one, which is exact and costs about the same there.

   Changes against Arm's original: the test-harness macros at the end are gone,
   and what it needs from math_config.h is provided here -- asuint64 from
   musl-support.h, the data declarations from erf_data.h, and unlikely and
   check_uflow below.  */

#include <math.h>
#include <stdint.h>
#include <errno.h>
#include "math_private.h"
#include "musl-support.h"
#include "erf_data.h"

#ifndef unlikely
# define unlikely(x) __builtin_expect (!!(x), 0)
#endif

/* Arm's check_uflow: an underflow to zero is reported through errno. */
static __inline double check_uflow (double x)
{
	if (x == 0.0)
		__set_errno (ERANGE);
	return x;
}

#define TwoOverSqrtPiMinusOne 0x1.06eba8214db69p-3
#define C 0x1.b0ac16p-1
#define PA __erf_data.erf_poly_A
#define NA __erf_data.erf_ratio_N_A
#define DA __erf_data.erf_ratio_D_A
#define NB __erf_data.erf_ratio_N_B
#define DB __erf_data.erf_ratio_D_B
#define PC __erf_data.erfc_poly_C
#define PD __erf_data.erfc_poly_D
#define PE __erf_data.erfc_poly_E
#define PF __erf_data.erfc_poly_F

/* Top 32 bits of a double.  */
static inline uint32_t
top32 (double x)
{
  return asuint64 (x) >> 32;
}

/* Fast erf implementation using a mix of
   rational and polynomial approximations.
   Highest measured error is 1.01 ULPs at 0x1.39956ac43382fp+0.  */
double
erf (double x)
{
  /* Get top word and sign.  */
  uint32_t ix = top32 (x);
  uint32_t ia = ix & 0x7fffffff;
  uint32_t sign = ix >> 31;

  /* Normalized and subnormal cases */
  if (ia < 0x3feb0000)
    { /* a = |x| < 0.84375.  */

      if (ia < 0x3e300000)
	{ /* a < 2^(-28).  */
	  if (ia < 0x00800000)
	    { /* a < 2^(-1015).  */
	      double y =  fma (TwoOverSqrtPiMinusOne, x, x);
	      return check_uflow (y);
	    }
	  return x + TwoOverSqrtPiMinusOne * x;
	}

      double x2 = x * x;

      if (ia < 0x3fe00000)
	{ /* a < 0.5  - Use polynomial approximation.  */
	  double r1 = fma (x2, PA[1], PA[0]);
	  double r2 = fma (x2, PA[3], PA[2]);
	  double r3 = fma (x2, PA[5], PA[4]);
	  double r4 = fma (x2, PA[7], PA[6]);
	  double r5 = fma (x2, PA[9], PA[8]);
	  double x4 = x2 * x2;
	  double r = r5;
	  r = fma (x4, r, r4);
	  r = fma (x4, r, r3);
	  r = fma (x4, r, r2);
	  r = fma (x4, r, r1);
	  return fma (r, x, x); /* This fma is crucial for accuracy.  */
	}
      else
	{ /* 0.5 <= a < 0.84375 - Use rational approximation.  */
	  double x4, x8, r1n, r2n, r1d, r2d, r3d;

	  r1n = fma (x2, NA[1], NA[0]);
	  x4 = x2 * x2;
	  r2n = fma (x2, NA[3], NA[2]);
	  x8 = x4 * x4;
	  r1d = fma (x2, DA[0], 1.0);
	  r2d = fma (x2, DA[2], DA[1]);
	  r3d = fma (x2, DA[4], DA[3]);
	  double P = r1n + x4 * r2n + x8 * NA[4];
	  double Q = r1d + x4 * r2d + x8 * r3d;
	  return fma (P / Q, x, x);
	}
    }
  else if (ia < 0x3ff40000)
    { /* 0.84375 <= |x| < 1.25.  */
      double a2, a4, a6, r1n, r2n, r3n, r4n, r1d, r2d, r3d, r4d;
      double a = fabs (x) - 1.0;
      r1n = fma (a, NB[1], NB[0]);
      a2 = a * a;
      r1d = fma (a, DB[0], 1.0);
      a4 = a2 * a2;
      r2n = fma (a, NB[3], NB[2]);
      a6 = a4 * a2;
      r2d = fma (a, DB[2], DB[1]);
      r3n = fma (a, NB[5], NB[4]);
      r3d = fma (a, DB[4], DB[3]);
      r4n = NB[6];
      r4d = DB[5];
      double P = r1n + a2 * r2n + a4 * r3n + a6 * r4n;
      double Q = r1d + a2 * r2d + a4 * r3d + a6 * r4d;
      if (sign)
	return -C - P / Q;
      else
	return C + P / Q;
    }
  else if (ia < 0x40000000)
    { /* 1.25 <= |x| < 2.0.  */
      double a = fabs (x);
      a = a - 1.25;

      double r1 = fma (a, PC[1], PC[0]);
      double r2 = fma (a, PC[3], PC[2]);
      double r3 = fma (a, PC[5], PC[4]);
      double r4 = fma (a, PC[7], PC[6]);
      double r5 = fma (a, PC[9], PC[8]);
      double r6 = fma (a, PC[11], PC[10]);
      double r7 = fma (a, PC[13], PC[12]);
      double r8 = fma (a, PC[15], PC[14]);

      double a2 = a * a;

      double r = r8;
      r = fma (a2, r, r7);
      r = fma (a2, r, r6);
      r = fma (a2, r, r5);
      r = fma (a2, r, r4);
      r = fma (a2, r, r3);
      r = fma (a2, r, r2);
      r = fma (a2, r, r1);

      if (sign)
	return -1.0 + r;
      else
	return 1.0 - r;
    }
  else if (ia < 0x400a0000)
    { /* 2 <= |x| < 3.25.  */
      double a = fabs (x);
      a = fma (0.5, a, -1.0);

      double r1 = fma (a, PD[1], PD[0]);
      double r2 = fma (a, PD[3], PD[2]);
      double r3 = fma (a, PD[5], PD[4]);
      double r4 = fma (a, PD[7], PD[6]);
      double r5 = fma (a, PD[9], PD[8]);
      double r6 = fma (a, PD[11], PD[10]);
      double r7 = fma (a, PD[13], PD[12]);
      double r8 = fma (a, PD[15], PD[14]);
      double r9 = fma (a, PD[17], PD[16]);

      double a2 = a * a;

      double r = r9;
      r = fma (a2, r, r8);
      r = fma (a2, r, r7);
      r = fma (a2, r, r6);
      r = fma (a2, r, r5);
      r = fma (a2, r, r4);
      r = fma (a2, r, r3);
      r = fma (a2, r, r2);
      r = fma (a2, r, r1);

      if (sign)
	return -1.0 + r;
      else
	return 1.0 - r;
    }
  else if (ia < 0x40100000)
    { /* 3.25 <= |x| < 4.0.  */
      double a = fabs (x);
      a = a - 3.25;

      double r1 = fma (a, PE[1], PE[0]);
      double r2 = fma (a, PE[3], PE[2]);
      double r3 = fma (a, PE[5], PE[4]);
      double r4 = fma (a, PE[7], PE[6]);
      double r5 = fma (a, PE[9], PE[8]);
      double r6 = fma (a, PE[11], PE[10]);
      double r7 = fma (a, PE[13], PE[12]);

      double a2 = a * a;

      double r = r7;
      r = fma (a2, r, r6);
      r = fma (a2, r, r5);
      r = fma (a2, r, r4);
      r = fma (a2, r, r3);
      r = fma (a2, r, r2);
      r = fma (a2, r, r1);

      if (sign)
	return -1.0 + r;
      else
	return 1.0 - r;
    }
  else if (ia < 0x4017a000)
    { /* 4 <= |x| < 5.90625.  */
      double a = fabs (x);
      a = fma (0.5, a, -2.0);

      double r1 = fma (a, PF[1], PF[0]);
      double r2 = fma (a, PF[3], PF[2]);
      double r3 = fma (a, PF[5], PF[4]);
      double r4 = fma (a, PF[7], PF[6]);
      double r5 = fma (a, PF[9], PF[8]);
      double r6 = fma (a, PF[11], PF[10]);
      double r7 = fma (a, PF[13], PF[12]);
      double r8 = fma (a, PF[15], PF[14]);
      double r9 = PF[16];

      double a2 = a * a;

      double r = r9;
      r = fma (a2, r, r8);
      r = fma (a2, r, r7);
      r = fma (a2, r, r6);
      r = fma (a2, r, r5);
      r = fma (a2, r, r4);
      r = fma (a2, r, r3);
      r = fma (a2, r, r2);
      r = fma (a2, r, r1);

      if (sign)
	return -1.0 + r;
      else
	return 1.0 - r;
    }
  else
    {
      /* Special cases : erf(nan)=nan, erf(+inf)=+1 and erf(-inf)=-1.  */
      if (unlikely (ia >= 0x7ff00000))
	return (double) (1.0 - (sign << 1)) + 1.0 / x;

      if (sign)
	return -1.0;
      else
	return 1.0;
    }
}
libm_hidden_def(erf)

/* The rest is libm/s_erf.c verbatim: the constants and erfc.  */
static const double
tiny	    = 1e-300,
half=  5.00000000000000000000e-01, /* 0x3FE00000, 0x00000000 */
one =  1.00000000000000000000e+00, /* 0x3FF00000, 0x00000000 */
two =  2.00000000000000000000e+00, /* 0x40000000, 0x00000000 */
	/* c = (float)0.84506291151 */
erx =  8.45062911510467529297e-01, /* 0x3FEB0AC1, 0x60000000 */
/*
 * Coefficients for approximation to  erf on [0,0.84375]
 */
efx =  1.28379167095512586316e-01, /* 0x3FC06EBA, 0x8214DB69 */
efx8=  1.02703333676410069053e+00, /* 0x3FF06EBA, 0x8214DB69 */
pp0  =  1.28379167095512558561e-01, /* 0x3FC06EBA, 0x8214DB68 */
pp1  = -3.25042107247001499370e-01, /* 0xBFD4CD7D, 0x691CB913 */
pp2  = -2.84817495755985104766e-02, /* 0xBF9D2A51, 0xDBD7194F */
pp3  = -5.77027029648944159157e-03, /* 0xBF77A291, 0x236668E4 */
pp4  = -2.37630166566501626084e-05, /* 0xBEF8EAD6, 0x120016AC */
qq1  =  3.97917223959155352819e-01, /* 0x3FD97779, 0xCDDADC09 */
qq2  =  6.50222499887672944485e-02, /* 0x3FB0A54C, 0x5536CEBA */
qq3  =  5.08130628187576562776e-03, /* 0x3F74D022, 0xC4D36B0F */
qq4  =  1.32494738004321644526e-04, /* 0x3F215DC9, 0x221C1A10 */
qq5  = -3.96022827877536812320e-06, /* 0xBED09C43, 0x42A26120 */
/*
 * Coefficients for approximation to  erf  in [0.84375,1.25]
 */
pa0  = -2.36211856075265944077e-03, /* 0xBF6359B8, 0xBEF77538 */
pa1  =  4.14856118683748331666e-01, /* 0x3FDA8D00, 0xAD92B34D */
pa2  = -3.72207876035701323847e-01, /* 0xBFD7D240, 0xFBB8C3F1 */
pa3  =  3.18346619901161753674e-01, /* 0x3FD45FCA, 0x805120E4 */
pa4  = -1.10894694282396677476e-01, /* 0xBFBC6398, 0x3D3E28EC */
pa5  =  3.54783043256182359371e-02, /* 0x3FA22A36, 0x599795EB */
pa6  = -2.16637559486879084300e-03, /* 0xBF61BF38, 0x0A96073F */
qa1  =  1.06420880400844228286e-01, /* 0x3FBB3E66, 0x18EEE323 */
qa2  =  5.40397917702171048937e-01, /* 0x3FE14AF0, 0x92EB6F33 */
qa3  =  7.18286544141962662868e-02, /* 0x3FB2635C, 0xD99FE9A7 */
qa4  =  1.26171219808761642112e-01, /* 0x3FC02660, 0xE763351F */
qa5  =  1.36370839120290507362e-02, /* 0x3F8BEDC2, 0x6B51DD1C */
qa6  =  1.19844998467991074170e-02, /* 0x3F888B54, 0x5735151D */
/*
 * Coefficients for approximation to  erfc in [1.25,1/0.35]
 */
ra0  = -9.86494403484714822705e-03, /* 0xBF843412, 0x600D6435 */
ra1  = -6.93858572707181764372e-01, /* 0xBFE63416, 0xE4BA7360 */
ra2  = -1.05586262253232909814e+01, /* 0xC0251E04, 0x41B0E726 */
ra3  = -6.23753324503260060396e+01, /* 0xC04F300A, 0xE4CBA38D */
ra4  = -1.62396669462573470355e+02, /* 0xC0644CB1, 0x84282266 */
ra5  = -1.84605092906711035994e+02, /* 0xC067135C, 0xEBCCABB2 */
ra6  = -8.12874355063065934246e+01, /* 0xC0545265, 0x57E4D2F2 */
ra7  = -9.81432934416914548592e+00, /* 0xC023A0EF, 0xC69AC25C */
sa1  =  1.96512716674392571292e+01, /* 0x4033A6B9, 0xBD707687 */
sa2  =  1.37657754143519042600e+02, /* 0x4061350C, 0x526AE721 */
sa3  =  4.34565877475229228821e+02, /* 0x407B290D, 0xD58A1A71 */
sa4  =  6.45387271733267880336e+02, /* 0x40842B19, 0x21EC2868 */
sa5  =  4.29008140027567833386e+02, /* 0x407AD021, 0x57700314 */
sa6  =  1.08635005541779435134e+02, /* 0x405B28A3, 0xEE48AE2C */
sa7  =  6.57024977031928170135e+00, /* 0x401A47EF, 0x8E484A93 */
sa8  = -6.04244152148580987438e-02, /* 0xBFAEEFF2, 0xEE749A62 */
/*
 * Coefficients for approximation to  erfc in [1/.35,28]
 */
rb0  = -9.86494292470009928597e-03, /* 0xBF843412, 0x39E86F4A */
rb1  = -7.99283237680523006574e-01, /* 0xBFE993BA, 0x70C285DE */
rb2  = -1.77579549177547519889e+01, /* 0xC031C209, 0x555F995A */
rb3  = -1.60636384855821916062e+02, /* 0xC064145D, 0x43C5ED98 */
rb4  = -6.37566443368389627722e+02, /* 0xC083EC88, 0x1375F228 */
rb5  = -1.02509513161107724954e+03, /* 0xC0900461, 0x6A2E5992 */
rb6  = -4.83519191608651397019e+02, /* 0xC07E384E, 0x9BDC383F */
sb1  =  3.03380607434824582924e+01, /* 0x403E568B, 0x261D5190 */
sb2  =  3.25792512996573918826e+02, /* 0x40745CAE, 0x221B9F0A */
sb3  =  1.53672958608443695994e+03, /* 0x409802EB, 0x189D5118 */
sb4  =  3.19985821950859553908e+03, /* 0x40A8FFB7, 0x688C246A */
sb5  =  2.55305040643316442583e+03, /* 0x40A3F219, 0xCEDF3BE6 */
sb6  =  4.74528541206955367215e+02, /* 0x407DA874, 0xE79FE763 */
sb7  = -2.24409524465858183362e+01; /* 0xC03670E2, 0x42712D62 */

double erfc(double x)
{
	int32_t hx,ix;
	double R,S,P,Q,s,y,z,r;
	GET_HIGH_WORD(hx,x);
	ix = hx&0x7fffffff;
	if(ix>=0x7ff00000) {			/* erfc(nan)=nan */
						/* erfc(+-inf)=0,2 */
	    return (double)(((u_int32_t)hx>>31)<<1)+one/x;
	}

	if(ix < 0x3feb0000) {		/* |x|<0.84375 */
	    if(ix < 0x3c700000)  	/* |x|<2**-56 */
		return one-x;
	    z = x*x;
	    r = pp0+z*(pp1+z*(pp2+z*(pp3+z*pp4)));
	    s = one+z*(qq1+z*(qq2+z*(qq3+z*(qq4+z*qq5))));
	    y = r/s;
	    if(hx < 0x3fd00000) {  	/* x<1/4 */
		return one-(x+x*y);
	    } else {
		r = x*y;
		r += (x-half);
	        return half - r ;
	    }
	}
	if(ix < 0x3ff40000) {		/* 0.84375 <= |x| < 1.25 */
	    s = fabs(x)-one;
	    P = pa0+s*(pa1+s*(pa2+s*(pa3+s*(pa4+s*(pa5+s*pa6)))));
	    Q = one+s*(qa1+s*(qa2+s*(qa3+s*(qa4+s*(qa5+s*qa6)))));
	    if(hx>=0) {
	        z  = one-erx; return z - P/Q;
	    } else {
		z = erx+P/Q; return one+z;
	    }
	}
	if (ix < 0x403c0000) {		/* |x|<28 */
	    x = fabs(x);
 	    s = one/(x*x);
	    if(ix< 0x4006DB6D) {	/* |x| < 1/.35 ~ 2.857143*/
	        R=ra0+s*(ra1+s*(ra2+s*(ra3+s*(ra4+s*(
				ra5+s*(ra6+s*ra7))))));
	        S=one+s*(sa1+s*(sa2+s*(sa3+s*(sa4+s*(
				sa5+s*(sa6+s*(sa7+s*sa8)))))));
	    } else {			/* |x| >= 1/.35 ~ 2.857143 */
		if(hx<0&&ix>=0x40180000) return two-tiny;/* x < -6 */
	        R=rb0+s*(rb1+s*(rb2+s*(rb3+s*(rb4+s*(
				rb5+s*rb6)))));
	        S=one+s*(sb1+s*(sb2+s*(sb3+s*(sb4+s*(
				sb5+s*(sb6+s*sb7))))));
	    }
	    z  = x;
	    SET_LOW_WORD(z,0);
	    r  =  __ieee754_exp(-z*z-0.5625)*
			__ieee754_exp((z-x)*(z+x)+R/S);
	    if(hx>0) return r/x; else return two-r/x;
	} else {
	    if(hx>0) return tiny*tiny; else return two-tiny;
	}
}
libm_hidden_def(erfc)
