/* Copyright (C) 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2006.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sys/syscall.h>

#if defined __UCLIBC_LINUX_SPECIFIC__ && defined __USE_GNU

#define __need_NULL
#include <stddef.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <sys/poll.h>
#include <cancel.h>

#if defined(__UCLIBC_USE_TIME64__)
#include "internal/time64_helpers.h"
#endif

static int
__NC(ppoll)(struct pollfd *fds, nfds_t nfds, const struct timespec *timeout,
	    const sigset_t *sigmask)
{
#if defined(__NR_ppoll) || defined(__NR_ppoll_time64)
	/* The Linux kernel can in some situations update the timeout value.
	   We do not want that so use a local variable.  */
	struct timespec tval;
	if (timeout != NULL) {
		tval = *timeout;
		timeout = &tval;
	}
#if defined(__UCLIBC_USE_TIME64__) && defined(__NR_ppoll_time64)
	return INLINE_SYSCALL(ppoll_time64, 5, fds, nfds, TO_TS64_P(timeout), sigmask, __SYSCALL_SIGSET_T_SIZE);
#else
	return INLINE_SYSCALL(ppoll, 5, fds, nfds, timeout, sigmask, __SYSCALL_SIGSET_T_SIZE);
#endif
#else
	int tmo = -1;
	int retval;
	int saved_errno;
	sigset_t savemask;

	if (timeout != NULL) {
		if (timeout->tv_sec < 0 || timeout->tv_nsec < 0
		    || timeout->tv_nsec >= 1000000000L) {
			__set_errno(EINVAL);
			return -1;
		}
		/* Round up to whole milliseconds, clamp to what poll() takes.  */
		if (timeout->tv_sec >= INT_MAX / 1000)
			tmo = INT_MAX;
		else
			tmo = timeout->tv_sec * 1000
			      + (timeout->tv_nsec + 999999) / 1000000;
	}

	/* The setting and restoring of the signal mask and the poll call
	   should be an atomic operation.  This can't be done without kernel
	   help, so this is racy in the same way as the pselect() fallback.  */
	if (sigmask != NULL)
		sigprocmask(SIG_SETMASK, sigmask, &savemask);

	retval = poll(fds, nfds, tmo);

	if (sigmask != NULL) {
		saved_errno = errno;
		sigprocmask(SIG_SETMASK, &savemask, NULL);
		__set_errno(saved_errno);
	}

	return retval;
#endif
}

CANCELLABLE_SYSCALL(int, ppoll, (struct pollfd *fds, nfds_t nfds, const struct timespec *timeout,
				 const sigset_t *sigmask),
		    (fds, nfds, timeout, sigmask))
#endif
