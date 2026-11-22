/* Prototype declarations for math functions; helper file for <math.h>.
   Copyright (C) 1996-2002, 2003, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* NOTE: Because of the special way this file is used by <math.h>, this
   file must NOT be protected from multiple inclusion as header files
   usually are.

   This file provides prototype declarations for the math functions.
   Most functions are declared using the macro:

   __MATHCALL (NAME,[_r], (ARGS...))

   This means there is a function `NAME' returning `double' and a function
   `NAMEf' returning `float'.  Each place `_Mdouble_' appears in the
   prototype, that is actually `double' in the prototype for `NAME' and
   `float' in the prototype for `NAMEf'.  Reentrant variant functions are
   called `NAME_r' and `NAMEf_r'.

   Functions returning other types like `int' are declared using the macro:

   __MATHDECL (TYPE, NAME,[_r], (ARGS...))

   This is just like __MATHCALL but for a function returning `TYPE'
   instead of `_Mdouble_'.  In all of these cases, there is still
   both a `NAME' and a `NAMEf' that takes `float' arguments.

   Note that there must be no whitespace before the argument passed for
   NAME, to make token pasting work with -traditional.  */

#ifndef _MATH_H
# error "Never include <bits/mathcalls.h> directly; include <math.h> instead."
#endif


/* __MATHCALLX(type,function,[suffix],args,attrib) and
 * __MATHCALLI(type,function,[suffix],args) include libm_hidden_proto
 * (for "double" versions only, xxxf and xxxl do not get this treatment).
 *
 * __MATHDECL(type,function,[suffix],args) does not.
 * __MATHCALL(function,[suffix],args) also does not
 * (it is just a shortcut to __MATHDECL(_Mdouble_,function,[suffix],args)).
 *
 * __MATHDECL_PRIV(type,function,[suffix],args,attrib)
 * includes libm_hidden_proto (always) and declares __foo, not foo.
 */


/* Trigonometric functions.  */

_Mdouble_BEGIN_NAMESPACE
/* Arc cosine of X.  */
__MATHCALLI (acos,, (_Mdouble_ __x))
/* Arc sine of X.  */
__MATHCALLI (asin,, (_Mdouble_ __x))
/* Arc tangent of X.  */
__MATHCALLI (atan,, (_Mdouble_ __x))
/* Arc tangent of Y/X.  */
__MATHCALLI (atan2,, (_Mdouble_ __y, _Mdouble_ __x))

/* Cosine of X.  */
__MATHCALLI (cos,, (_Mdouble_ __x))
# if defined _LIBC && defined _Mlong_double_
libm_hidden_proto(cosl)
# endif
# if defined _LIBC && defined _Mfloat_
libm_hidden_proto(cosf)
# endif

/* Sine of X.  */
__MATHCALLI (sin,, (_Mdouble_ __x))
# if defined _LIBC && defined _Mlong_double_
libm_hidden_proto(sinl)
# endif
# if defined _LIBC && defined _Mfloat_
libm_hidden_proto(sinf)
# endif

/* Tangent of X.  */
__MATHCALLI (tan,, (_Mdouble_ __x))

/* Hyperbolic functions.  */

/* Hyperbolic cosine of X.  */
__MATHCALLI (cosh,, (_Mdouble_ __x))
/* Hyperbolic sine of X.  */
__MATHCALLI (sinh,, (_Mdouble_ __x))
/* Hyperbolic tangent of X.  */
__MATHCALLI (tanh,, (_Mdouble_ __x))
_Mdouble_END_NAMESPACE

#if defined __USE_GNU
/* Cosine and sine of X.  */
__MATHDECL (void,sincos,,
	    (_Mdouble_ __x, _Mdouble_ *__sinx, _Mdouble_ *__cosx))
#endif

#if defined __USE_MISC || defined __USE_XOPEN_EXTENDED || defined __USE_ISOC99
__BEGIN_NAMESPACE_C99
/* Hyperbolic arc cosine of X.  */
__MATHCALLI (acosh,, (_Mdouble_ __x))
/* Hyperbolic arc sine of X.  */
__MATHCALLI (asinh,, (_Mdouble_ __x))
/* Hyperbolic arc tangent of X.  */
__MATHCALLI (atanh,, (_Mdouble_ __x))
__END_NAMESPACE_C99
#endif

/* Exponential and logarithmic functions.  */

_Mdouble_BEGIN_NAMESPACE
/* Exponential function of X.  */
__MATHCALLI (exp,, (_Mdouble_ __x))
# if defined _LIBC && defined _Mlong_double_
libm_hidden_proto(expl)
# endif

/* Break VALUE into a normalized fraction and an integral power of 2.  */
__MATHCALLI (frexp,, (_Mdouble_ __x, int *__exponent))

/* X times (two to the EXP power).  */
__MATHCALLI (ldexp,, (_Mdouble_ __x, int __exponent))

/* Natural logarithm of X.  */
__MATHCALLI (log,, (_Mdouble_ __x))

/* Base-ten logarithm of X.  */
__MATHCALLI (log10,, (_Mdouble_ __x))

/* Break VALUE into integral and fractional parts.  */
__MATHCALLI (modf,, (_Mdouble_ __x, _Mdouble_ *__iptr))
_Mdouble_END_NAMESPACE

#if defined __USE_GNU || defined __USE_ISOC23
/* Compute exponent to base ten (ISO C23, a GNU extension before).  */
__MATHCALLI (exp10,, (_Mdouble_ __x))
#endif
#if defined __USE_GNU
/* Another name occasionally used.  */
__MATHCALLI (pow10,, (_Mdouble_ __x))
#endif

#if defined __USE_MISC || defined __USE_XOPEN_EXTENDED || defined __USE_ISOC99
__BEGIN_NAMESPACE_C99
/* Return exp(X) - 1.  */
__MATHCALLI (expm1,, (_Mdouble_ __x))

/* Return log(1 + X).  */
__MATHCALLI (log1p,, (_Mdouble_ __x))

/* Return the base 2 signed integral exponent of X.  */
__MATHCALLI (logb,, (_Mdouble_ __x))
__END_NAMESPACE_C99
#endif

#ifdef __USE_ISOC99
__BEGIN_NAMESPACE_C99
/* Compute base-2 exponential of X.  */
__MATHCALLI (exp2,, (_Mdouble_ __x))

/* Compute base-2 logarithm of X.  */
__MATHCALLI (log2,, (_Mdouble_ __x))
__END_NAMESPACE_C99
#endif


/* Power functions.  */

_Mdouble_BEGIN_NAMESPACE
/* Return X to the Y power.  */
__MATHCALLI (pow,, (_Mdouble_ __x, _Mdouble_ __y))

/* Return the square root of X.  */
__MATHCALLI (sqrt,, (_Mdouble_ __x))
_Mdouble_END_NAMESPACE

#if defined __USE_MISC || defined __USE_XOPEN || defined __USE_ISOC99
__BEGIN_NAMESPACE_C99
/* Return `sqrt(X*X + Y*Y)'.  */
__MATHCALLI (hypot,, (_Mdouble_ __x, _Mdouble_ __y))
# if defined _LIBC && defined _Mlong_double_
libm_hidden_proto(hypotl)
# endif
__END_NAMESPACE_C99
#endif

#if defined __USE_MISC || defined __USE_XOPEN_EXTENDED || defined __USE_ISOC99
__BEGIN_NAMESPACE_C99
/* Return the cube root of X.  */
__MATHCALLI (cbrt,, (_Mdouble_ __x))
__END_NAMESPACE_C99
#endif


/* Nearest integer, absolute value, and remainder functions.  */

_Mdouble_BEGIN_NAMESPACE
/* Smallest integral value not less than X.  */
__MATHCALLX (ceil,, (_Mdouble_ __x), (__const__))

/* Absolute value of X.  */
__MATHCALLX (fabs,, (_Mdouble_ __x), (__const__))

/* Largest integer not greater than X.  */
__MATHCALLX (floor,, (_Mdouble_ __x), (__const__))

/* Floating-point modulo remainder of X/Y.  */
__MATHCALLI (fmod,, (_Mdouble_ __x, _Mdouble_ __y))


/* Return 0 if VALUE is finite or NaN, +1 if it
   is +Infinity, -1 if it is -Infinity.  */
__MATHDECL_PRIV (int,isinf,, (_Mdouble_ __value), (__const__))

/* Return nonzero if VALUE is finite and not NaN.  */
__MATHDECL_PRIV (int,finite,, (_Mdouble_ __value), (__const__))
_Mdouble_END_NAMESPACE

#ifdef __USE_MISC
#if 0
/* Return 0 if VALUE is finite or NaN, +1 if it
   is +Infinity, -1 if it is -Infinity.  */
__MATHDECL_PRIV (int,isinf,, (_Mdouble_ __value), (__const__))

/* Return nonzero if VALUE is finite and not NaN.  */
__MATHDECL_PRIV (int,finite,, (_Mdouble_ __value), (__const__))
#endif
/* Return the remainder of X/Y.  */
__MATHCALL (drem,, (_Mdouble_ __x, _Mdouble_ __y))


/* Return the fractional part of X after dividing out `ilogb (X)'.  */
__MATHCALLI (significand,, (_Mdouble_ __x))
#endif /* Use misc.  */

#if defined __USE_MISC || defined __USE_ISOC99
__BEGIN_NAMESPACE_C99
/* Return X with its signed changed to Y's.  */
__MATHCALLX (copysign,, (_Mdouble_ __x, _Mdouble_ __y), (__const__))
__END_NAMESPACE_C99
#endif

#ifdef __USE_ISOC99
__BEGIN_NAMESPACE_C99
/* Return representation of NaN for double type.  */
__MATHCALLX (nan,, (const char *__tagb), (__const__))
__END_NAMESPACE_C99
#endif

#if defined __USE_MISC || defined __USE_XOPEN || defined __USE_ISOC99
/* Return nonzero if VALUE is not a number.  */
__BEGIN_NAMESPACE_C99
__MATHDECL_PRIV (int,isnan,, (_Mdouble_ __value), (__const__))
__END_NAMESPACE_C99
#endif

#if defined __USE_MISC || defined __USE_XOPEN
# ifdef __DO_XSI_MATH__
/* Bessel functions.  */
__MATHCALL (j0,, (_Mdouble_))
__MATHCALL (j1,, (_Mdouble_))
__MATHCALL (jn,, (int, _Mdouble_))
__MATHCALL (y0,, (_Mdouble_))
__MATHCALL (y1,, (_Mdouble_))
__MATHCALL (yn,, (int, _Mdouble_))
# endif
#endif


#if defined __USE_MISC || defined __USE_XOPEN || defined __USE_ISOC99
__BEGIN_NAMESPACE_C99
/* Error and gamma functions.  */
__MATHCALLI (erf,, (_Mdouble_))
__MATHCALLI (erfc,, (_Mdouble_))
__MATHCALLI (lgamma,, (_Mdouble_))
__END_NAMESPACE_C99
#endif

#ifdef __USE_ISOC99
__BEGIN_NAMESPACE_C99
/* True gamma function.  */
__MATHCALLI (tgamma,, (_Mdouble_))
__END_NAMESPACE_C99
#endif

#if defined __USE_MISC || defined __USE_XOPEN
/* Obsolete alias for `lgamma'.  */
__MATHCALLI (gamma,, (_Mdouble_))
#endif

//#ifdef __USE_MISC
/* To provide compatibility with finite-math-only in C99 and C11 
 * standerts function lgamma_r should be declared, when defined __USE_MISC.
 */
/* Reentrant version of lgamma.  This function uses the global variable
   `signgam'.  The reentrant version instead takes a pointer and stores
   the value through it.  */
__MATHCALL (lgamma,_r, (_Mdouble_, int *__signgamp))
/* __MATHCALLI does not work here, probably due to ,_r, */
libm_hidden_proto(lgamma_r)
//#endif


#if defined __USE_MISC || defined __USE_XOPEN_EXTENDED || defined __USE_ISOC99
__BEGIN_NAMESPACE_C99
/* Return the integer nearest X in the direction of the
   prevailing rounding mode.  */
__MATHCALLI (rint,, (_Mdouble_ __x))

/* Return X + epsilon if X < Y, X - epsilon if X > Y.  */
__MATHCALLX (nextafter,, (_Mdouble_ __x, _Mdouble_ __y), (__const__))
# if defined _LIBC && defined _Mlong_double_
libm_hidden_proto(nextafterl)
# endif
# if defined __USE_ISOC99 && !defined __LDBL_COMPAT
__MATHCALLX (nexttoward,, (_Mdouble_ __x, long double __y), (__const__))
# endif

/* Return the remainder of integer divison X / Y with infinite precision.  */
__MATHCALLI (remainder,, (_Mdouble_ __x, _Mdouble_ __y))

# if defined __USE_MISC || defined __USE_ISOC99
/* Return X times (2 to the Nth power).  */
__MATHCALLI (scalbn,, (_Mdouble_ __x, int __n))
# endif

/* Return the binary exponent of X, which must be nonzero.  */
__MATHDECLI (int,ilogb,, (_Mdouble_ __x))
#endif

#ifdef __USE_ISOC99
/* Return X times (2 to the Nth power).  */
__MATHCALLI (scalbln,, (_Mdouble_ __x, long int __n))

/* Round X to integral value in floating-point format using current
   rounding direction, but do not raise inexact exception.  */
__MATHCALLI (nearbyint,, (_Mdouble_ __x))

/* Round X to nearest integral value, rounding halfway cases away from
   zero.  */
__MATHCALLX (round,, (_Mdouble_ __x), (__const__))

/* Round X to the integral value in floating-point format nearest but
   not larger in magnitude.  */
__MATHCALLX (trunc,, (_Mdouble_ __x), (__const__))

/* Compute remainder of X and Y and put in *QUO a value with sign of x/y
   and magnitude congruent `mod 2^n' to the magnitude of the integral
   quotient x/y, with n >= 3.  */
__MATHCALLI (remquo,, (_Mdouble_ __x, _Mdouble_ __y, int *__quo))


/* Conversion functions.  */

/* Round X to nearest integral value according to current rounding
   direction.  */
__MATHDECLI (long int,lrint,, (_Mdouble_ __x))
__MATHDECLI (long long int,llrint,, (_Mdouble_ __x))

/* Round X to nearest integral value, rounding halfway cases away from
   zero.  */
__MATHDECLI (long int,lround,, (_Mdouble_ __x))
__MATHDECLI (long long int,llround,, (_Mdouble_ __x))


/* Return positive difference between X and Y.  */
__MATHCALLI (fdim,, (_Mdouble_ __x, _Mdouble_ __y))

/* Return maximum numeric value from X and Y.  */
__MATHCALLI (fmax,, (_Mdouble_ __x, _Mdouble_ __y))

/* Return minimum numeric value from X and Y.  */
__MATHCALLI (fmin,, (_Mdouble_ __x, _Mdouble_ __y))


/* Classify given number.  */
__MATHDECL_PRIV (int, fpclassify,, (_Mdouble_ __value), (__const__))

/* Test for negative number.  */
__MATHDECL_PRIV (int, signbit,, (_Mdouble_ __value), (__const__))


/* Multiply-add function computed as a ternary operation.  */
__MATHCALLI (fma,, (_Mdouble_ __x, _Mdouble_ __y, _Mdouble_ __z))
#endif /* Use ISO C99.  */

#ifdef __USE_ISOC23
/* ISO C23.  */
/* Trigonometric functions in half-revolutions: f(pi x), f(x) / pi.  */
__MATHCALL (acospi,, (_Mdouble_ __x));
__MATHCALL (asinpi,, (_Mdouble_ __x));
__MATHCALL (atanpi,, (_Mdouble_ __x));
__MATHCALL (atan2pi,, (_Mdouble_ __y, _Mdouble_ __x));
__MATHCALL (cospi,, (_Mdouble_ __x));
__MATHCALL (sinpi,, (_Mdouble_ __x));
__MATHCALL (tanpi,, (_Mdouble_ __x));

/* 10^x - 1, 2^x - 1, and the logarithms of 1 + x.  */
__MATHCALL (exp10m1,, (_Mdouble_ __x));
__MATHCALL (exp2m1,, (_Mdouble_ __x));
__MATHCALL (log10p1,, (_Mdouble_ __x));
__MATHCALL (log2p1,, (_Mdouble_ __x));
__MATHCALL (logp1,, (_Mdouble_ __x));

/* (1 + x)^n, x^n, x^y as exp(y log x), the n-th root, 1 / sqrt(x).  */
__MATHCALL (compoundn,, (_Mdouble_ __x, long long int __n));
__MATHCALL (pown,, (_Mdouble_ __x, long long int __n));
__MATHCALL (powr,, (_Mdouble_ __x, _Mdouble_ __y));
__MATHCALL (rootn,, (_Mdouble_ __x, long long int __n));
__MATHCALL (rsqrt,, (_Mdouble_ __x));

/* Round to nearest, ties to even, in any rounding mode.  */
__MATHCALL (roundeven,, (_Mdouble_ __x));

/* The next value up and down, without overflow or underflow.  */
__MATHCALL (nextup,, (_Mdouble_ __x));
__MATHCALL (nextdown,, (_Mdouble_ __x));

/* The exponent of X as a long.  */
__MATHDECL (long int, llogb,, (_Mdouble_ __x));

/* Round X in direction ROUND to an integer that must fit in WIDTH
   bits (signed or unsigned); the x forms raise inexact if it changed.  */
__MATHCALL (fromfp,, (_Mdouble_ __x, int __round, unsigned int __width));
__MATHCALL (ufromfp,, (_Mdouble_ __x, int __round, unsigned int __width));
__MATHCALL (fromfpx,, (_Mdouble_ __x, int __round, unsigned int __width));
__MATHCALL (ufromfpx,, (_Mdouble_ __x, int __round, unsigned int __width));

/* Maximum and minimum: NaN if either is one, -0 below +0; _num prefers
   a number to a NaN; _mag compares magnitudes first.  */
__MATHCALL (fmaximum,, (_Mdouble_ __x, _Mdouble_ __y));
__MATHCALL (fminimum,, (_Mdouble_ __x, _Mdouble_ __y));
__MATHCALL (fmaximum_num,, (_Mdouble_ __x, _Mdouble_ __y));
__MATHCALL (fminimum_num,, (_Mdouble_ __x, _Mdouble_ __y));
__MATHCALL (fmaximum_mag,, (_Mdouble_ __x, _Mdouble_ __y));
__MATHCALL (fminimum_mag,, (_Mdouble_ __x, _Mdouble_ __y));
__MATHCALL (fmaximum_mag_num,, (_Mdouble_ __x, _Mdouble_ __y));
__MATHCALL (fminimum_mag_num,, (_Mdouble_ __x, _Mdouble_ __y));

/* The IEEE 754 total order, of values and of magnitudes.  */
__MATHDECL_1 (int, totalorder,, (const _Mdouble_ *__x,
				  const _Mdouble_ *__y))
     __attribute__ ((__pure__));
__MATHDECL_1 (int, totalordermag,, (const _Mdouble_ *__x,
				     const _Mdouble_ *__y))
     __attribute__ ((__pure__));

/* Store the canonical encoding of *X in *CX.  */
__MATHDECL_1 (int, canonicalize,, (_Mdouble_ *__cx, const _Mdouble_ *__x));

/* The payload of a NaN, and NaNs with a given payload.  */
__MATHCALL (getpayload,, (const _Mdouble_ *__x));
__MATHDECL_1 (int, setpayload,, (_Mdouble_ *__x, _Mdouble_ __payload));
__MATHDECL_1 (int, setpayloadsig,, (_Mdouble_ *__x, _Mdouble_ __payload));

/* Behind the issignaling and iseqsig macros.  */
__MATHDECL_PRIV (int, issignaling,, (_Mdouble_ __value), (__const__))
__MATHDECL_PRIV (int, iseqsig,, (_Mdouble_ __x, _Mdouble_ __y), ())
#endif /* Use ISO C23.  */

#if defined __USE_MISC || defined __USE_XOPEN_EXTENDED || defined __USE_ISOC99
__END_NAMESPACE_C99
#endif

#if (defined __USE_MISC || defined __USE_XOPEN_EXTENDED) \
	&& defined __UCLIBC_SUSV3_LEGACY__
/* Return X times (2 to the Nth power).  */
__MATHCALLI (scalb,, (_Mdouble_ __x, _Mdouble_ __n))
#endif
