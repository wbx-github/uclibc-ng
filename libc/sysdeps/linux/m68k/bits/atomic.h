/* Atomic operations.  m68k version.
   Copyright (C) 2026 Ramin Moussavi <ramin.moussavi@yacoub.de>

   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.

   m68k (68020 and up, which is everything the MMU Linux port runs on) has
   the CAS instruction.  We implement the compare-and-exchange primitive
   with cas.b/cas.w/cas.l; include/atomic.h derives every higher-level
   operation from it.

   Without this file m68k fell back to the generic non-atomic
   bits/atomic.h, whose "atomic" compare-and-exchange is a plain
   read-compare-write.  Under sustained contention a preemption between
   the read and the write corrupts NPTL's lock and condition-variable
   state, which showed up as lost wakeups (tst-cond16 hung until the test
   timeout).  */

#ifndef _M68K_BITS_ATOMIC_H
#define _M68K_BITS_ATOMIC_H	1

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

/* UP m68k; CAS is itself the read-modify-write atomic, so a plain
   compiler barrier around it is enough.  */
#define atomic_full_barrier()	__asm__ __volatile__ ("" ::: "memory")
#define atomic_read_barrier()	atomic_full_barrier ()
#define atomic_write_barrier()	atomic_full_barrier ()

/* cas Dc,Du,<ea>: compare <ea> with Dc; if equal store Du, else load <ea>
   into Dc.  Either way Dc ends up holding the original *MEM, which is the
   value compare-and-exchange must return.  */
#define __arch_compare_and_exchange_val_8_acq(mem, newval, oldval) \
  ({ __typeof (*(mem)) __ret = (oldval);				\
     __asm__ __volatile__ ("cas%.b %0,%2,%1"				\
		       : "=d" (__ret), "+m" (*(mem))			\
		       : "d" (newval), "0" (__ret)			\
		       : "memory");					\
     __ret; })

#define __arch_compare_and_exchange_val_16_acq(mem, newval, oldval) \
  ({ __typeof (*(mem)) __ret = (oldval);				\
     __asm__ __volatile__ ("cas%.w %0,%2,%1"				\
		       : "=d" (__ret), "+m" (*(mem))			\
		       : "d" (newval), "0" (__ret)			\
		       : "memory");					\
     __ret; })

#define __arch_compare_and_exchange_val_32_acq(mem, newval, oldval) \
  ({ __typeof (*(mem)) __ret = (oldval);				\
     __asm__ __volatile__ ("cas%.l %0,%2,%1"				\
		       : "=d" (__ret), "+m" (*(mem))			\
		       : "d" (newval), "0" (__ret)			\
		       : "memory");					\
     __ret; })

/* m68k has no 64-bit CAS; NPTL and include/atomic.h only use int- and
   pointer-sized objects, so the bysize dispatch never reaches this.  */
#define __arch_compare_and_exchange_val_64_acq(mem, newval, oldval) \
  ({ __typeof (*(mem)) __ret = *(mem); abort (); (void) (newval);	\
     (void) (oldval); __ret; })

#endif /* _M68K_BITS_ATOMIC_H */
