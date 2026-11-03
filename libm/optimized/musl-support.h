/* Helpers the Arm optimized-routines need, in the shapes musl gives them
   (src/internal/libm.h).  Kept local to libm/optimized so the generic
   fdlibm sources stay untouched.  */
#ifndef _LIBM_OPTIMIZED_SUPPORT_H
#define _LIBM_OPTIMIZED_SUPPORT_H 1

#include <stdint.h>
#include <math.h>
#include <errno.h>
#include <float.h>

#define WANT_ROUNDING 1
#define TOINT_INTRINSICS 0

#define predict_true(x)  __builtin_expect(!!(x), 1)
#define predict_false(x) __builtin_expect(x, 0)

static __inline uint64_t asuint64(double f)
{
	union { double f; uint64_t i; } u = { f };
	return u.i;
}

static __inline double asdouble(uint64_t i)
{
	union { uint64_t i; double f; } u = { i };
	return u.f;
}

/* Round to double, even where the target evaluates in a wider format -- on x87
   an intermediate would otherwise keep 64 mantissa bits and the algorithm's error
   bound would not hold.  Where FLT_EVAL_METHOD is 0 this costs nothing. */
#if defined __FLT_EVAL_METHOD__ && __FLT_EVAL_METHOD__ != 0
static __inline double eval_as_double(double x)
{
	volatile double y = x;
	return y;
}
#else
static __inline double eval_as_double(double x)
{
	return x;
}
#endif

static __inline double fp_barrier(double x)
{
	volatile double y = x;
	return y;
}

#define fp_force_eval(x) do { volatile double y__; y__ = (x); (void)y__; } while (0)

/* Raise the flag and set errno the way the algorithm expects; the public
   wrapper in w_*.c does its own errno work on top, which is harmless. */
static __inline double __math_xflow(uint32_t sign, double y)
{
	double z = fp_barrier(sign ? -y : y) * y;
	errno = ERANGE;
	return z;
}

static __inline double __math_oflow(uint32_t sign)
{
	return __math_xflow(sign, 0x1p769);
}

static __inline double __math_uflow(uint32_t sign)
{
	return __math_xflow(sign, 0x1p-767);
}

#endif
