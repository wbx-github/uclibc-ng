/*
 * time() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <time.h>

#if defined(__NR_time) && !defined(__UCLIBC_USE_TIME64__)
_syscall_noerr1(time_t, time, time_t *, t)
#else
# include <sys/time.h>
time_t time(time_t * t)
{
	time_t result;
	struct timeval tv;

	if (gettimeofday(&tv, NULL) < 0)
		return (time_t) -1;
	result = (time_t) tv.tv_sec;
	if (t != NULL)
		*t = result;
	return result;
}
#endif
libc_hidden_def(time)
