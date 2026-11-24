/*
 * INLINE_SETXID_SYSCALL: a set*id system call that changes the IDs of
 * every thread, not only the caller's, as POSIX requires and Linux
 * does not do by itself.  Without NPTL there is the one thread.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _INTERNAL_SETXID_H
#define _INTERNAL_SETXID_H

#include <sys/syscall.h>

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
# include <setxid.h>
#else
# define INLINE_SETXID_SYSCALL(name, nr, args...) \
	INLINE_SYSCALL (name, nr, args)
#endif

#endif
