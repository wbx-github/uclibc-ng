/*
 * fork() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#if defined __ARCH_USE_MMU__
# include <unistd.h>
extern __typeof(fork) __libc_fork;
# if defined __NR_fork
#  include <cancel.h>
#  define __NR___libc_fork __NR_fork
_syscall0(pid_t, fork)

# elif defined __NR_clone  && !defined __NR_fork
#  include <sys/types.h>
#  include <signal.h>
#  include <stddef.h>
pid_t fork(void)
{
	pid_t pid = INLINE_SYSCALL(clone, 4, SIGCHLD, NULL, NULL, NULL);

	if (pid < 0)
		return -1;

	return pid;
}

# endif
# ifdef __UCLIBC_HAS_THREADS__
strong_alias(fork,__libc_fork)
libc_hidden_weak(fork)
/* With linuxthreads _Fork is in ptfork.c alone, where the child's
   thread descriptor can be put right.  Not even a weak alias here: it
   would satisfy the reference from this file, which comes first in
   libc.a, and a static link would never pull ptfork.o in.  */
# else
/* No threads: no descriptor to put right and no handler to skip.  */
strong_alias(fork,_Fork)
libc_hidden_def(fork)
# endif

#endif
