/* Atomic operations.  MicroBlaze version.
   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.

   MicroBlaze (ISA v8+) has 32-bit load/store-exclusive instructions
   (lwx/swx).  We implement the compare-and-exchange primitive directly
   with them rather than via the __sync_* builtins: gcc's MicroBlaze
   __sync code generation is broken (it emits the lwx/swx pair without
   the compare-branch and without retrying on a failed swx), so the
   builtin compare-and-exchange neither swaps nor returns the right
   value.  A hand-written lwx/swx loop works correctly.

   Without this file MicroBlaze fell back to the generic non-atomic
   bits/atomic.h, whose "atomic" compare-and-exchange is a plain
   read-compare-write.  Under sustained contention a preemption between
   the read and the write corrupts NPTL's lock and condition-variable
   state, which showed up as lost wakeups (tst-cond16/tst-cond17 hung
   until the test timeout).

   Only 32-bit objects are supported (lwx/swx are word operations); the
   NPTL and include/atomic.h users only ever operate on int- and
   pointer-sized objects, and the higher-level operations are derived
   from the compare-and-exchange primitive below.  */

#ifndef _MICROBLAZE_BITS_ATOMIC_H
#define _MICROBLAZE_BITS_ATOMIC_H	1

#include <stdint.h>

typedef int8_t atomic8_t;
typedef uint8_t uatomic8_t;
typedef int_fast8_t atomic_fast8_t;
typedef uint_fast8_t uatomic_fast8_t;

typedef int16_t atomic16_t;
typedef uint16_t uatomic16_t;
typedef int_fast16_t atomic_fast16_t;
typedef uint_fast16_t uatomic_fast16_t;

typedef int32_t atomic32_t;
typedef uint32_t uatomic32_t;
typedef int_fast32_t atomic_fast32_t;
typedef uint_fast32_t uatomic_fast32_t;

typedef int64_t atomic64_t;
typedef uint64_t uatomic64_t;
typedef int_fast64_t atomic_fast64_t;
typedef uint_fast64_t uatomic_fast64_t;

typedef intptr_t atomicptr_t;
typedef uintptr_t uatomicptr_t;
typedef intmax_t atomic_max_t;
typedef uintmax_t uatomic_max_t;

/* UP MicroBlaze; lwx/swx provide the ordering for the atomic itself, a
   plain compiler barrier is enough around it.  */
#define atomic_full_barrier()	__asm__ __volatile__ ("" ::: "memory")
#define atomic_read_barrier()	atomic_full_barrier ()
#define atomic_write_barrier()	atomic_full_barrier ()

/* Store NEWVAL in *MEM if *MEM equals OLDVAL; return the old *MEM value.
   lwx loads exclusive (arming the reservation), swx stores exclusive and
   sets MSR[C]=1 if the reservation was lost, in which case we retry.  */
#define atomic_compare_and_exchange_val_acq(mem, newval, oldval)	\
  ({ __typeof (*(mem)) __acev_ret;					\
     __typeof (*(mem)) __acev_nv = (newval);				\
     __typeof (*(mem)) __acev_ov = (oldval);				\
     int __acev_fail;							\
     __asm__ __volatile__ (						\
	"1:	lwx	%0, %2, r0\n\t"					\
	"	cmpu	%1, %0, %3\n\t"					\
	"	bnei	%1, 2f\n\t"					\
	"	swx	%4, %2, r0\n\t"					\
	"	addic	%1, r0, 0\n\t"					\
	"	bnei	%1, 1b\n\t"					\
	"2:"								\
	: "=&r" (__acev_ret), "=&r" (__acev_fail)			\
	: "r" (mem), "r" (__acev_ov), "r" (__acev_nv)			\
	: "cc", "memory");						\
     __acev_ret; })

#define atomic_compare_and_exchange_bool_acq(mem, newval, oldval)	\
  ({ __typeof (*(mem)) __aceb_ov = (oldval);				\
     atomic_compare_and_exchange_val_acq ((mem), (newval), __aceb_ov)	\
       != __aceb_ov; })

/* atomic_exchange_acq and the higher-level operations are derived from
   the compare-and-exchange primitive by include/atomic.h.  */

#endif /* _MICROBLAZE_BITS_ATOMIC_H */
