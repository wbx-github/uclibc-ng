/*
 * clock_gettime() for uClibc
 *
 * Copyright (C) 2003 by Justus Pendleton <uc@ryoohki.net>
 * Copyright (C) 2005 by Peter Kjellerstedt <pkj@axis.com>
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <time.h>

#ifdef __VDSO_SUPPORT__
#include "ldso.h"
#endif

#if defined(__UCLIBC_USE_TIME64__) && defined(__NR_clock_gettime64)
#include "internal/time64_helpers.h"

int __libc_clock_gettime(clockid_t clock_id, struct timespec *tp)
{
	struct __ts64_struct __ts64;
	int __ret = INLINE_SYSCALL(clock_gettime64, 2, clock_id, &__ts64);
	if (__ret == 0 && tp) {
		tp->tv_sec = __ts64.tv_sec;
		tp->tv_nsec = __ts64.tv_nsec;
	}

	return __ret;
}
#elif defined(__NR_clock_gettime)
int __libc_clock_gettime(clockid_t clock_id, struct timespec *tp)
{
	return INLINE_SYSCALL(clock_gettime, 2, clock_id, tp);
}
#else
# include <sys/time.h>

int __libc_clock_gettime(clockid_t clock_id, struct timespec* tp)
{
	struct timeval tv;
	int retval = -1;

	switch (clock_id) {
		case CLOCK_REALTIME:
			retval = gettimeofday(&tv, NULL);
			if (retval == 0)
				TIMEVAL_TO_TIMESPEC(&tv, tp);
			break;

		default:
			errno = EINVAL;
			break;
	}

	return retval;
}
#endif

int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
#if defined(__VDSO_SUPPORT__) && defined(ARCH_VDSO_CLOCK_GETTIME)
	return ARCH_VDSO_CLOCK_GETTIME(clock_id, tp);
#else
	return __libc_clock_gettime(clock_id, tp);
#endif
}
