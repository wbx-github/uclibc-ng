/* The proper definitions for Linux's sigaction.
   Copyright (C) 1993-1999, 2000 Free Software Foundation, Inc.
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

#ifndef _SIGNAL_H
# error "Never include <bits/sigaction.h> directly; use <signal.h> instead."
#endif

/* These kernels have no sa_restorer in the struct that rt_sigaction expects
 * (asm-generic/signal.h without SA_RESTORER).  sigaction() hands ours to
 * the kernel unchanged, so the field must not precede sa_mask there.  */
#if defined __riscv || defined __csky__ || defined __microblaze__ \
 || defined __or1k__ || defined __nds32__ || defined __kvx__ \
 || defined __loongarch__
# define __UCLIBC_NO_KERNEL_SA_RESTORER__ 1
#endif

/* Structure describing the action to be taken when a signal arrives.
 * In uclibc, it is identical up to sa_mask to "new" struct kernel_sigaction
 * (one from the Linux 2.1.68 kernel).
 * This minimizes amount of translation in sigaction().
 */
struct sigaction {
#ifdef __USE_POSIX199309
	union {
		__sighandler_t sa_handler;
		void (*sa_sigaction)(int, siginfo_t *, void *);
	} __sigaction_handler;
# define sa_handler     __sigaction_handler.sa_handler
# define sa_sigaction   __sigaction_handler.sa_sigaction
#else
	__sighandler_t  sa_handler;
#endif
	unsigned long   sa_flags;
#ifdef __UCLIBC_NO_KERNEL_SA_RESTORER__
	/* Behind the mask: the kernel copies only its own size, so it never
	   reads or writes this field.  */
	sigset_t        sa_mask;
	void            (*sa_restorer)(void);
#else
	void            (*sa_restorer)(void);
	sigset_t        sa_mask;
#endif
};

/* Bits in `sa_flags'.  */
#define	SA_NOCLDSTOP  1		 /* Don't send SIGCHLD when children stop.  */
#define SA_NOCLDWAIT  2		 /* Don't create zombie on child death.  */
#define SA_SIGINFO    4		 /* Invoke signal-catching function with
				    three arguments instead of one.  */
#if defined __USE_UNIX98 || defined __USE_MISC
# define SA_ONSTACK   0x08000000 /* Use signal stack by using `sa_restorer'. */
# define SA_RESTART   0x10000000 /* Restart syscall on signal return.  */
# define SA_NODEFER   0x40000000 /* Don't automatically block the signal when
				    its handler is being executed.  */
# define SA_RESETHAND 0x80000000 /* Reset to SIG_DFL on entry to handler.  */
#endif
#ifdef __USE_MISC
# define SA_INTERRUPT 0x20000000 /* Historical no-op.  */

/* Some aliases for the SA_ constants.  */
# define SA_NOMASK    SA_NODEFER
# define SA_ONESHOT   SA_RESETHAND
# define SA_STACK     SA_ONSTACK
#endif

/* Values for the HOW argument to `sigprocmask'.  */
#define	SIG_BLOCK     0		 /* Block signals.  */
#define	SIG_UNBLOCK   1		 /* Unblock signals.  */
#define	SIG_SETMASK   2		 /* Set the set of blocked signals.  */
