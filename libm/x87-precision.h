/* Round every operation to double where the hardware computes wider.
 *
 * Code that recovers rounding errors exactly -- the argument reduction of sin,
 * cos and tan carries pi/2 in two words -- needs each double operation to
 * round to double.  On x87 that is the FPU's precision control field, not
 * the compiler's business, so it takes an fldcw -- see glibc's
 * SET_RESTORE_ROUND_53BIT.  __FLT_EVAL_METHOD__ is 2 exactly where arithmetic
 * is carried out wider than the type; elsewhere this compiles to nothing.
 *
 * Wrap a whole function, not single expressions: fldcw flushes the FPU
 * pipeline, and the algorithms return from many places.
 */

#ifndef _X87_PRECISION_H
#define _X87_PRECISION_H

#if defined __FLT_EVAL_METHOD__ && __FLT_EVAL_METHOD__ == 2 && defined __i386__

#include <fpu_control.h>

# define X87_ROUND53_DECL	fpu_control_t __x87_cw, __x87_saved_cw
# define X87_ROUND53_BEGIN()						\
	do {								\
		_FPU_GETCW (__x87_saved_cw);				\
		__x87_cw = (__x87_saved_cw & ~_FPU_EXTENDED)		\
			   | _FPU_DOUBLE;				\
		_FPU_SETCW (__x87_cw);					\
	} while (0)
# define X87_ROUND53_END()	_FPU_SETCW (__x87_saved_cw)

/* The worker has to stay out of line: were it inlined into the wrapper, the
   compiler could hoist an operation out of the window the two fldcw's open.  */
# define X87_ROUND53_WORKER	static __attribute__ ((noinline))

#else

# define X87_ROUND53_DECL	struct { int __unused; } __x87_none __attribute__ ((unused))
# define X87_ROUND53_BEGIN()	do { } while (0)
# define X87_ROUND53_END()	do { } while (0)

/* Nothing to keep out of line here, so let the wrapper collapse into the
   worker and leave the object exactly as it was without the guard.  */
# define X87_ROUND53_WORKER	static inline __attribute__ ((always_inline))

#endif

#endif /* _X87_PRECISION_H */
