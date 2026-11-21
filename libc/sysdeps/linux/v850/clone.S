/*
 * libc/sysdeps/linux/v850/clone.S -- clone syscall wrapper
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#undef C_SYMBOL_NAME
#include "clinkage.h"

#define EINVAL 22

/* int clone (int (*fn)(void *arg), void *child_stack, int flags,
	      void *arg, ...);

   FN is in r6, CHILD_STACK in r7 and FLAGS in r8.  clone is variadic and
   the v850 port of GCC does not use strict argument naming, so ARG, the
   last named argument, is passed on the stack at 0[sp] and not in r9.

   This has to be written in assembly: after the trap the child runs on
   CHILD_STACK, so it must not touch the parent's stack frame.  FN and ARG
   are therefore stored at the top of the child's stack before the trap.  */

	.text
C_ENTRY (clone):
	cmp	r0, r6			/* fn == NULL? */
	be	.Linval
	cmp	r0, r7			/* child_stack == NULL? */
	be	.Linval

	ld.w	0[sp], r10		/* arg */
	add	-8, r7			/* room for fn and arg */
	st.w	r6, 0[r7]
	st.w	r10, 4[r7]

	mov	r8, r6			/* arg 0: flags, arg 1: child_stack */
	movea	SYS_clone, r0, r12
	trap	0
	cmp	r0, r10
	bne	.Lparent

	/* Child: sp is the child stack set up above.  */
	ld.w	0[sp], r19		/* fn */
	ld.w	4[sp], r6		/* arg */
	jarl	.Lcall, lp		/* lp = .Lexit */
.Lexit:
	mov	r10, r6			/* exit status: return value of fn */
	movea	SYS_exit, r0, r12
	trap	0
.Lcall:
	jmp	[r19]

.Lparent:
	movea	-4096, r0, r11
	cmp	r11, r10
	bh	.Lerror			/* -4095 .. -1 is an error */
	jmp	[lp]

.Linval:
	movea	-EINVAL, r0, r10
.Lerror:
	/* There is no thread support on v850, so errno is a plain variable.  */
	mov	hilo(C_SYMBOL_NAME(errno)), r11
	subr	r0, r10
	st.w	r10, 0[r11]
	mov	-1, r10
	jmp	[lp]
C_END (clone)
