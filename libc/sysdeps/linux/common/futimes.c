/*
 * futimes() implementation for uClibc
 *
 * Copyright (C) 2026 Ramin Moussavi <lordrasmus@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/time.h>

#if defined(__NR_utimensat) || defined(__NR_utimensat_time64)
#include <errno.h>
#include <time.h>

/* No sys/stat.h, as in futimens.c: its utimensat takes a nonnull path.  */
extern int utimensat (int __fd, const char *__path,
	const struct timespec __times[2],
	int __flags) __THROW;
libc_hidden_proto(utimensat)

int futimes(int fd, const struct timeval tvp[2])
{
	struct timespec ts[2];

	if (tvp != NULL) {
		if (tvp[0].tv_usec < 0 || tvp[0].tv_usec >= 1000000
		    || tvp[1].tv_usec < 0 || tvp[1].tv_usec >= 1000000) {
			__set_errno(EINVAL);
			return -1;
		}

		TIMEVAL_TO_TIMESPEC(&tvp[0], &ts[0]);
		TIMEVAL_TO_TIMESPEC(&tvp[1], &ts[1]);
	}

	return utimensat(fd, 0, tvp ? ts : NULL, 0);
}
#endif
