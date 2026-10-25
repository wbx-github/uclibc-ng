/* Copyright (C) 1999, 2000, 2003, 2004, 2007 Free Software Foundation, Inc.

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

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <features.h>
#include <asm/unistd.h>
#include <sys/syscall.h>
#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg,
	     pid_t *ptid, void *tls, pid_t *ctid); */

	.text
ENTRY(__clone)
	/* sanity check arguments.  */
	tst	r4, r4
	bt/s	0f
	 tst	r5, r5
	bf	1f
0:
	bra	.Lsyscall_error
	 mov	#-EINVAL,r0
1:
	/* insert the args onto the new stack */
	mov.l	r7, @-r5
	/* save the function pointer as the 0th element */
	mov.l	r4, @-r5

	/* do the system call */
	mov	r6, r4
	mov.l	@r15, r6
	mov.l	@(8,r15), r7
	mov.l	@(4,r15), r0
	mov	#+SYS_ify(clone), r3
	trapa	#(__SH_SYSCALL_TRAP_BASE + 5)
	mov     r0, r1
#if defined(__sh2__) || defined(__CONFIG_J2__)
/* 12 arithmetic shifts for the sh2, because shad doesn't exist! */
        shar    r1
        shar    r1
        shar    r1
        shar    r1
        shar    r1
        shar    r1
        shar    r1
        shar    r1
        shar    r1
        shar    r1
        shar    r1
        shar    r1
#else
	mov	#-12, r2
	shad	r2, r1
#endif
	not	r1, r1			// r1=0 means r0 = -1 to -4095
	tst	r1, r1			// i.e. error in linux
	bf	.Lclone_end
.Lsyscall_error:
#ifdef __SH_FDPIC__
	/* SYSCALL_ERROR_HANDLER has no FDPIC variant; this one takes
	   __errno_location's descriptor from the GOT in r12 */
	bra	__syscall_error
	 mov	r0, r4
#else
	SYSCALL_ERROR_HANDLER
#endif
.Lclone_end:
	tst	r0, r0
	bt	2f
.Lpseudo_end:
	rts
	 nop
2:
	/* terminate the stack frame */
	mov	#0, r14

	/* thread starts */
	mov.l	@r15, r1
#ifdef __SH_FDPIC__
	/* fn is a function descriptor: entry point and GOT */
	mov.l	@(4,r1), r12
	mov.l	@r1, r1
#endif
	jsr	@r1
	 mov.l	@(4,r15), r4

	/* we are done, passing the return value through r0  */
	mov	r0, r4
	mov	#+SYS_ify(exit), r3
	trapa	#(__SH_SYSCALL_TRAP_BASE + 5)

PSEUDO_END (__clone)
weak_alias (__clone, clone)

#ifdef __SH_FDPIC__
#include "syscall_error.S"
#endif
