/*
 * timespec_getres() for uClibc (ISO C23)
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <time.h>

int timespec_getres(struct timespec *ts, int base)
{
	if (base != TIME_UTC)
		return 0;
	if (ts != NULL) {
#ifdef __UCLIBC_HAS_REALTIME__
		if (clock_getres(CLOCK_REALTIME, ts) != 0)
			return 0;
#else
		/* timespec_get falls back to gettimeofday there */
		ts->tv_sec = 0;
		ts->tv_nsec = 1000;
#endif
	}
	return base;
}
