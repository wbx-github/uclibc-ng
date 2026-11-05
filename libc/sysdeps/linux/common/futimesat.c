/*
 * futimesat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/time.h>

#if defined __NR_futimesat && !defined __UCLIBC_USE_TIME64__
_syscall3(int, futimesat, int, fd, const char *, file, const struct timeval *, tvp)
#elif defined __NR_utimensat || defined __NR_utimensat_time64
#include <errno.h>
#define __need_NULL
#include <stddef.h>

int futimesat(int dirfd, const char *file, const struct timeval tvp[2])
{
	struct timespec ts[2];

	if (tvp != NULL)
	{
		if (tvp[0].tv_usec < 0 || tvp[0].tv_usec >= 1000000
		    || tvp[1].tv_usec < 0 || tvp[1].tv_usec >= 1000000)
		{
			__set_errno(EINVAL);
			return -1;
		}

		TIMEVAL_TO_TIMESPEC(&tvp[0], &ts[0]);
		TIMEVAL_TO_TIMESPEC(&tvp[1], &ts[1]);
	}

	return utimensat(dirfd, file, tvp ? ts : NULL, 0);
}
#endif
