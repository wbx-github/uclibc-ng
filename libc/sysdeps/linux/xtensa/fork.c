/*
 * fork() for Xtensa uClibc
 *
 * Copyright (C) 2007 Tensilica Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */


/* Xtensa doesn't provide a 'fork' system call, so we use 'clone'.  */
#include <sys/syscall.h>

#if defined __NR_clone && defined __ARCH_USE_MMU__
# include <unistd.h>
# include <signal.h>
# include <cancel.h>

pid_t fork(void)
{
	return (pid_t) INLINE_SYSCALL(clone, 2, SIGCHLD, 0);
}
#ifndef __UCLIBC_HAS_THREADS__
/* No threads: no descriptor to put right and no handler to skip.  */
strong_alias(fork,_Fork)
#endif
/* With linuxthreads _Fork is in ptfork.c alone, where the child's
   thread descriptor can be put right.  Not even a weak alias here: it
   would satisfy the reference from this file, which comes first in
   libc.a, and a static link would never pull ptfork.o in.  */
# ifdef __UCLIBC_HAS_THREADS__
strong_alias(fork,__libc_fork)
libc_hidden_weak(fork)
# else
libc_hidden_def(fork)
# endif
#endif
