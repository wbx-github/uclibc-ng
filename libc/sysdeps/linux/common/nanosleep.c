/*
 * nanosleep() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <time.h>
#include <cancel.h>

#if !defined(__NR_clock_nanosleep) && !defined(__NR_clock_nanosleep_time64) && defined(__NR_nanosleep)
/* No clock_nanosleep: use the nanosleep syscall directly. */
#define __NR___nanosleep_nocancel __NR_nanosleep
_syscall2(int, __nanosleep_nocancel, const struct timespec *, req,
	  struct timespec *, rem)
#else
int _NC(nanosleep)(const struct timespec *req, struct timespec *rem)
{
	int __ret = clock_nanosleep(CLOCK_REALTIME, 0, req, rem);

	if (__ret != 0) {
		__set_errno(__ret);
		return -1;
	}

	return __ret;
};
#endif

CANCELLABLE_SYSCALL(int, nanosleep,
		    (const struct timespec *req, struct timespec *rem),
		    (req, rem))

lt_libc_hidden(nanosleep)
