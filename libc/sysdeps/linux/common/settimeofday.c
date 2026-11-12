/*
 * settimeofday() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#if defined(__NR_settimeofday) && \
    !defined(__NR_clock_settime) && !defined(__NR_clock_settime64)
# include <sys/time.h>
_syscall2(int, settimeofday, const struct timeval *, tv,
	  const struct timezone *, tz)
libc_hidden_def(settimeofday)

#elif defined(__USE_BSD)

# include <sys/time.h>
# include <time.h>

extern int clock_settime (clockid_t __clock_id, const struct timespec *__tp)
     __THROW;

int settimeofday(const struct timeval *tv, const struct timezone *tz)
{
	if (!tv)
		return 0;

	struct timespec __ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * 1000
	};

	return clock_settime(CLOCK_REALTIME, &__ts);
}
libc_hidden_def(settimeofday)
#endif
