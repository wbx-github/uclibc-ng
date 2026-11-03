/* Euclidean distance function, taken from glibc (sysdeps/ieee754/dbl-64/e_hypot.c,
   LGPL-2.1-or-later like this library).  Selected by UCLIBC_LIBM_OPTIMIZED: more
   accurate than the fdlibm version in libm/e_hypot.c and about as fast -- measured
   over 42000 pairs, fdlibm misses the correctly rounded result 974 times, this one
   6 times, neither by more than one representable step.

   Changes against the original: the glibc-internal headers are replaced by the two
   macros actually used, the alias boilerplate is gone, the signaling-NaN branch is
   dropped (uClibc-ng does not distinguish sNaN anywhere else), and fmax/fmin are
   compared rather than called, since they are real functions here.  */

#include <math.h>
#include <errno.h>
#include "math_private.h"

/* On x87 an intermediate double may be kept at higher precision; force it out. */
#if defined __FLT_EVAL_METHOD__ && __FLT_EVAL_METHOD__ != 0
# define math_narrow_eval(x) ({ volatile double __v = (x); __v; })
#else
# define math_narrow_eval(x) (x)
#endif
#define __glibc_unlikely(x) __builtin_expect((x), 0)
#define USE_FMAX_BUILTIN 0
#define USE_FMIN_BUILTIN 0
/* glibc forces the underflow flag for a subnormal result here.  Without fenv
   nothing can read it, and w_hypot.c does its own SVID error handling, so the
   value is what matters -- leave the flag alone. */
#define math_check_force_underflow_nonneg(x) do { } while (0)

#define SCALE     0x1p-600
#define LARGE_VAL 0x1p+511
#define TINY_VAL  0x1p-459
#define EPS       0x1p-54

static inline double
handle_errno (double r)
{
  if (isinf (r))
    __set_errno (ERANGE);
  return r;
}

/* Hypot kernel. The inputs must be adjusted so that ax >= ay >= 0
   and squaring ax, ay and (ax - ay) does not overflow or underflow.  */
static inline double
kernel (double ax, double ay)
{
  double t1, t2;
#ifdef __FP_FAST_FMA
  t1 = ay + ay;
  t2 = ax - ay;

  if (t1 >= ax)
    return sqrt (fma (t1, ax, t2 * t2));
  else
    return sqrt (fma (ax, ax, ay * ay));

#else
  double h = sqrt (ax * ax + ay * ay);
  if (h <= 2.0 * ay)
    {
      double delta = h - ay;
      t1 = ax * (2.0 * delta - ax);
      t2 = (delta - 2.0 * (ax - ay)) * delta;
    }
  else
    {
      double delta = h - ax;
      t1 = 2.0 * delta * (ax - 2.0 * ay);
      t2 = (4.0 * delta - ay) * ay + delta * delta;
    }

  h -= (t1 + t2) / (2.0 * h);
  return h;
#endif
}

double
__ieee754_hypot (double x, double y)
{
  if (!isfinite(x) || !isfinite(y))
    {
      if (isinf (x) || isinf (y))
	return INFINITY;
      return x + y;
    }

  x = fabs (x);
  y = fabs (y);

  double ax = USE_FMAX_BUILTIN ? fmax (x, y) : (x < y ? y : x);
  double ay = USE_FMIN_BUILTIN ? fmin (x, y) : (x < y ? x : y);

  /* If ax is huge, scale both inputs down.  */
  if (__glibc_unlikely (ax > LARGE_VAL))
    {
      if (__glibc_unlikely (ay <= ax * EPS))
	return handle_errno (math_narrow_eval (ax + ay));

      return handle_errno (math_narrow_eval (kernel (ax * SCALE, ay * SCALE)
					     / SCALE));
    }

  /* If ay is tiny, scale both inputs up.  */
  if (__glibc_unlikely (ay < TINY_VAL))
    {
      if (__glibc_unlikely (ax >= ay / EPS))
	return math_narrow_eval (ax + ay);

      ax = math_narrow_eval (kernel (ax / SCALE, ay / SCALE) * SCALE);
      math_check_force_underflow_nonneg (ax);
      return ax;
    }

  /* Common case: ax is not huge and ay is not tiny.  */
  if (__glibc_unlikely (ay <= ax * EPS))
    return ax + ay;

  return kernel (ax, ay);
}
