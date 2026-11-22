/*
 * timespec_get() for uClibc (ISO C11)
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <time.h>
#include <sys/time.h>

int timespec_get(struct timespec *ts, int base)
{
	if (base != TIME_UTC)
		return 0;
#ifdef __UCLIBC_HAS_REALTIME__
	if (clock_gettime(CLOCK_REALTIME, ts) != 0)
		return 0;
#else
	struct timeval tv;

	if (gettimeofday(&tv, NULL) != 0)
		return 0;
	ts->tv_sec = tv.tv_sec;
	ts->tv_nsec = tv.tv_usec * 1000;
#endif
	return base;
}
