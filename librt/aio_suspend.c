/* POSIX asynchronous I/O: wait for completion.
   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.

   Adapted from musl libc (src/aio/aio_suspend.c), MIT licensed,
   Copyright (c) 2005-2020 Rich Felker, et al.  */

#include <aio.h>
#include <errno.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include <atomic.h>
#include <cancel.h>
#include "aio_misc.h"

int aio_suspend(const struct aiocb *const cbs[], int cnt,
		const struct timespec *ts)
{
	int i, ret, expect = 0, nzcnt = 0, oldtype;
	struct timespec at, now, rel;
	volatile int dummy_fut __attribute__ ((__aligned__ (4))) = 0;
	volatile int *pfut;
	const struct aiocb *cb = 0;

	pthread_testcancel();

	if (cnt < 0) {
		__set_errno(EINVAL);
		return -1;
	}

	for (i = 0; i < cnt; i++)
		if (cbs[i]) {
			if (aio_error(cbs[i]) != EINPROGRESS)
				return 0;
			nzcnt++;
			cb = cbs[i];
		}

	if (ts) {
		/* POSIX asks for EINVAL here.  Without the check an out of
		   range tv_nsec survives the one subtraction below, the
		   futex call then fails with EINVAL, and the loop spins at
		   full speed until the deadline.  */
		if (ts->tv_sec < 0 || ts->tv_nsec < 0
		    || ts->tv_nsec >= 1000000000L) {
			__set_errno(EINVAL);
			return -1;
		}
		/* time_t is signed, so this is its largest value.  */
		const time_t max = (time_t)
			(((uintmax_t)1 << (sizeof(time_t) * 8 - 1)) - 1);

		clock_gettime(CLOCK_MONOTONIC, &at);
		if (ts->tv_sec > max - at.tv_sec - 1) {
			at.tv_sec = max;
			at.tv_nsec = 999999999L;
		} else {
			at.tv_sec += ts->tv_sec;
			if ((at.tv_nsec += ts->tv_nsec) >= 1000000000) {
				at.tv_nsec -= 1000000000;
				at.tv_sec++;
			}
		}
	}

	for (;;) {
		for (i = 0; i < cnt; i++)
			if (cbs[i] && aio_error(cbs[i]) != EINPROGRESS)
				return 0;

		switch (nzcnt) {
		case 0:
			pfut = &dummy_fut;
			break;
		case 1:
			/* bit 31 tells the worker that someone waits */
			pfut = (volatile int *)&cb->__err;
			expect = EINPROGRESS | 0x80000000;
			atomic_compare_and_exchange_val_acq(pfut, expect, EINPROGRESS);
			break;
		default:
			pfut = &__aio_fut;
			expect = atomic_compare_and_exchange_val_acq(pfut, 1, 0);
			if (!expect)
				expect = 1;
			/* The other half of the store-buffering pair in
			   publish(): the flag has to be in place before
			   the states are read.  A failed compare and
			   exchange orders nothing at all.  */
			atomic_full_barrier();
			/* recheck before sleeping */
			for (i = 0; i < cnt; i++)
				if (cbs[i] && aio_error(cbs[i]) != EINPROGRESS)
					return 0;
			break;
		}

		if (ts) {
			clock_gettime(CLOCK_MONOTONIC, &now);
			rel.tv_sec = at.tv_sec - now.tv_sec;
			rel.tv_nsec = at.tv_nsec - now.tv_nsec;
			if (rel.tv_nsec < 0) {
				rel.tv_nsec += 1000000000;
				rel.tv_sec--;
			}
			if (rel.tv_sec < 0) {
				__set_errno(EAGAIN);
				return -1;
			}
		}

		oldtype = LIBC_CANCEL_ASYNC();
		ret = __aio_futex_wait(pfut, expect, ts ? &rel : NULL);
		LIBC_CANCEL_RESET(oldtype);

		switch (ret) {
		case ETIMEDOUT:
			__set_errno(EAGAIN);
			return -1;
		case EINTR:
			__set_errno(EINTR);
			return -1;
		}
	}
}

strong_alias(aio_suspend,aio_suspend64)
