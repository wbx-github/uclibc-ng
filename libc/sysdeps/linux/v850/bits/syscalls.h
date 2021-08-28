#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

#ifndef __ASSEMBLER__

#include <errno.h>

/*
 * _syscall<N> macros initially from linux-2.6.19/include/asm-v850/unistd.h:
 * Copyright (C) 2001,02,03,04  NEC Electronics Corporation
 * Copyright (C) 2001,02,03,04  Miles Bader <miles@gnu.org>
 *
 * Since then completely rewritten as INLINE_SYSCALL/INTERNAL_SYSCALL/etc.
 */

/* Syscall protocol:
   Syscall number in r12, args in r6-r9, r13-r14
   Return value in r10
   Trap 0 for `short' syscalls, where all the args can fit in function
   call argument registers, and trap 1 when there are additional args in
   r13-r14.  */

#define SYSCALL_NUM	"r12"
#define SYSCALL_ARG0	"r6"
#define SYSCALL_ARG1	"r7"
#define SYSCALL_ARG2	"r8"
#define SYSCALL_ARG3	"r9"
#define SYSCALL_ARG4	"r13"
#define SYSCALL_ARG5	"r14"
#define SYSCALL_RET	"r10"

#define SYSCALL_SHORT_TRAP	"0"
#define SYSCALL_LONG_TRAP	"1"

/* Registers clobbered by any syscall.  This _doesn't_ include the syscall
   number (r12) or the `extended arg' registers (r13, r14), even though
   they are actually clobbered too (this is because gcc's `asm' statement
   doesn't allow a clobber to be used as an input or output).  */
#define SYSCALL_CLOBBERS	"r1", "r5", "r11", "r15", "r16", \
				"r17", "r18", "r19"

/* Registers clobbered by a `short' syscall.  This includes all clobbers
   except the syscall number (r12).  */
#define SYSCALL_SHORT_CLOBBERS	SYSCALL_CLOBBERS, "r13", "r14"

#define LOAD_ARGS_0()
#define LOAD_REGS_0
#define ASM_OUTS_0
#define ASM_SHORT_ARGS_0
#define ASM_LONG_ARGS_0

#define LOAD_ARGS_1(a1)							\
  unsigned long __arg1 = (unsigned long) (a1);				\
  LOAD_ARGS_0 ()
#define LOAD_REGS_1							\
  register unsigned long __a1 __asm__ (SYSCALL_ARG0) = __arg1;		\
  LOAD_REGS_0
#define ASM_OUTS_1
#define ASM_SHORT_ARGS_1	ASM_SHORT_ARGS_0, "r" (__a1)
#define ASM_LONG_ARGS_1		ASM_SHORT_ARGS_1

#define LOAD_ARGS_2(a1, a2)						\
  unsigned long __arg2 = (unsigned long) (a2);				\
  LOAD_ARGS_1 (a1)
#define LOAD_REGS_2							\
  register unsigned long __a2 __asm__ (SYSCALL_ARG1) = __arg2;		\
  LOAD_REGS_1
#define ASM_OUTS_2
#define ASM_SHORT_ARGS_2	ASM_SHORT_ARGS_1, "r" (__a2)
#define ASM_LONG_ARGS_2		ASM_SHORT_ARGS_2

#define LOAD_ARGS_3(a1, a2, a3)						\
  unsigned long __arg3 = (unsigned long) (a3);				\
  LOAD_ARGS_2 (a1, a2)
#define LOAD_REGS_3							\
  register unsigned long __a3 __asm__ (SYSCALL_ARG2) = __arg3;		\
  LOAD_REGS_2
#define ASM_OUTS_3
#define ASM_SHORT_ARGS_3	ASM_SHORT_ARGS_2, "r" (__a3)
#define ASM_LONG_ARGS_3		ASM_SHORT_ARGS_3

#define LOAD_ARGS_4(a1, a2, a3, a4)					\
  unsigned long __arg4 = (unsigned long) (a4);				\
  LOAD_ARGS_3 (a1, a2, a3)
#define LOAD_REGS_4							\
  register unsigned long __a4 __asm__ (SYSCALL_ARG3) = __arg4;		\
  LOAD_REGS_3
#define ASM_OUTS_4
#define ASM_SHORT_ARGS_4	ASM_SHORT_ARGS_3, "r" (__a4)
#define ASM_LONG_ARGS_4		ASM_SHORT_ARGS_4

#define LOAD_ARGS_5(a1, a2, a3, a4, a5)					\
  unsigned long __arg5 = (unsigned long) (a5);				\
  LOAD_ARGS_4 (a1, a2, a3, a4)
#define LOAD_REGS_5							\
  register unsigned long __a5 __asm__ (SYSCALL_ARG4) = __arg5;		\
  LOAD_REGS_4
#define ASM_OUTS_5		ASM_OUTS_4, "=r" (__a5)
#define ASM_SHORT_ARGS_5	ASM_SHORT_ARGS_4
#define ASM_LONG_ARGS_5		ASM_LONG_ARGS_4, "2" (__a5)

#define LOAD_ARGS_6(a1, a2, a3, a4, a5, a6)				\
  unsigned long __arg6 = (unsigned long) (a6);				\
  LOAD_ARGS_5 (a1, a2, a3, a4, a5)
#define LOAD_REGS_6							\
  register unsigned long __a6 __asm__ (SYSCALL_ARG5) = __arg6;		\
  LOAD_REGS_5
#define ASM_OUTS_6		ASM_OUTS_5, "=r" (__a6)
#define ASM_SHORT_ARGS_6	ASM_SHORT_ARGS_4
#define ASM_LONG_ARGS_6		ASM_LONG_ARGS_5, "3" (__a6)

#define INTERNAL_SYSCALL_NCS(name, err, nr, args...)			\
(__extension__								\
  ({									\
    LOAD_ARGS_##nr (args)						\
    LOAD_REGS_##nr							\
    register unsigned long __syscall __asm__ (SYSCALL_NUM) = name;	\
    register unsigned long __ret __asm__ (SYSCALL_RET);			\
    if ((unsigned int) (nr) < 5)					\
      __asm__ __volatile__ (						\
	"trap " SYSCALL_SHORT_TRAP					\
	: "=r" (__ret), "=r" (__syscall)				\
	: "1" (__syscall) ASM_SHORT_ARGS_##nr				\
	: SYSCALL_SHORT_CLOBBERS);					\
    else								\
      __asm__ __volatile__ (						\
	"trap " SYSCALL_LONG_TRAP					\
	: "=r" (__ret), "=r" (__syscall) ASM_OUTS_##nr			\
	: "1" (__syscall) ASM_LONG_ARGS_##nr				\
	: SYSCALL_CLOBBERS);						\
    (long) __ret; })							\
)

/* __syscall_return is legacy but needed by the v850 versions of syscall.c and clone.c */
#define __syscall_return(type, res)					      \
  do {									      \
	  /* user-visible error numbers are in the range -1 - -4095:	      \
	     see <asm-v850/errno.h> */					      \
	  if (__builtin_expect ((unsigned long)(res) >= (unsigned long)(-4095), 0)) { \
		  /* Avoid using res directly which is declared to be in register r10; \
		     __set_errno might expand to a function call and clobber it.  */ \
		  int __err = -(res);					      \
		  __set_errno (__err);					      \
		  res = -1;						      \
	  }								      \
	  return (type) (res);						      \
  } while (0)

#endif /* !__ASSEMBLER__ */

#endif /* _BITS_SYSCALLS_H */
