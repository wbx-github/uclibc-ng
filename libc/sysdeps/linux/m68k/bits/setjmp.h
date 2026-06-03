/* Copyright (C) 2002, David McCullough <davidm@snapgear.com> */
/* Copyright (C) 1997,1998,2005,2006 Free Software Foundation, Inc.
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

/* Define the machine-dependent type `jmp_buf'.  m68k version.  */
#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H	1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

#include <features.h>

/* uClibc's m68k setjmp.S stores "%d2-%d7/%a2-%a7" as one moveml block at
   JB_REGS, followed by the return PC at JB_PC.  Only the call-saved registers
   are kept (d0/d1/a0/a1 and fp0/fp1 are caller-saved), so these members mirror
   that on-stack layout -- in particular the stack pointer is %a7, saved as the
   last address register, hence its own __sp member here.  */
typedef struct
  {
    long int __dregs[6];	/* %d2-%d7 */
    int *__aregs[5];		/* %a2-%a6 */
    int *__sp;			/* %a7, the stack pointer */
    int *__pc;			/* return address */

#if defined __HAVE_68881__ || defined __UCLIBC_HAS_FPU__
    /* %fp2-%fp7, IEEE 96-bit extended format (8 bytes each on ColdFire).  */
    char __fpregs[6 * (96 / 8)];
#endif

  } __jmp_buf[1];

#endif	/* bits/setjmp.h */
