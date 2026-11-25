/* POSIX asynchronous I/O: submit a list of requests.
   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.

   Adapted from musl libc (src/aio/lio_listio.c), MIT licensed,
   Copyright (c) 2005-2020 Rich Felker, et al.  */

#include <aio.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "aio_misc.h"

struct lio_state {
	/* By value: with LIO_NOWAIT the caller's sigevent may be gone by
	   the time wait_thread looks at it.  */
	struct sigevent sev;
	int cnt;
	struct aiocb *cbs[];
};

static int lio_wait(struct lio_state *st)
{
	int i, err, got_err = 0;
	int cnt = st->cnt;
	struct aiocb **cbs = st->cbs;

	for (;;) {
		int pending = 0;

		/* Every entry, not just up to the first one still running:
		   aio_suspend returns as soon as any request in the list is
		   finished, so leaving a finished one in would make it
		   return at once and the loop spin.  */
		for (i = 0; i < cnt; i++) {
			if (!cbs[i])
				continue;
			err = aio_error(cbs[i]);
			if (err == EINPROGRESS) {
				pending = 1;
				continue;
			}
			if (err)
				got_err = 1;
			cbs[i] = 0;
		}
		if (!pending) {
			if (got_err) {
				__set_errno(EIO);
				return -1;
			}
			return 0;
		}
		if (aio_suspend((void *)cbs, cnt, 0))
			return -1;
	}
}

static void notify_signal(struct sigevent *sev)
{
	siginfo_t si = {
		.si_signo = sev->sigev_signo,
		.si_code = SI_ASYNCIO,
	};

	si.si_value = sev->sigev_value;
	si.si_pid = getpid();
	si.si_uid = getuid();
	INTERNAL_SYSCALL_DECL(err);
	INTERNAL_SYSCALL(rt_sigqueueinfo, err, 3, si.si_pid, si.si_signo, &si);
}

static void *wait_thread(void *p)
{
	struct lio_state *st = p;
	struct sigevent sev = st->sev;
	sigset_t nomask;

	lio_wait(st);
	free(st);
	switch (sev.sigev_notify) {
	case SIGEV_SIGNAL:
		notify_signal(&sev);
		break;
	case SIGEV_THREAD:
		/* It is the caller's code from here on, so it gets the
		   signals back that this thread was started with blocked.  */
		sigemptyset(&nomask);
		pthread_sigmask(SIG_SETMASK, &nomask, 0);
		sev.sigev_notify_function(sev.sigev_value);
		break;
	}
	return 0;
}

int lio_listio(int mode, struct aiocb *const cbs[__restrict_arr],
	       int cnt, struct sigevent *__restrict sev)
{
	int i, ret, bad = 0, live = 0;
	struct lio_state *st = 0;

	/* A count whose byte size does not fit is over any limit this
	   implementation could have; POSIX reports that as EINVAL.  */
	if (cnt < 0 || (size_t)cnt > (SIZE_MAX - sizeof *st) / sizeof *cbs
	    || (mode != LIO_WAIT && mode != LIO_NOWAIT)) {
		__set_errno(EINVAL);
		return -1;
	}

	if (mode == LIO_WAIT || (sev && sev->sigev_notify != SIGEV_NONE)) {
		if (!(st = malloc(sizeof *st + cnt * sizeof *cbs))) {
			__set_errno(EAGAIN);
			return -1;
		}
		st->cnt = cnt;
		if (sev)
			st->sev = *sev;
		else
			st->sev.sigev_notify = SIGEV_NONE;
		memcpy(st->cbs, (void *)cbs, cnt * sizeof *cbs);
	}

	for (i = 0; i < cnt; i++) {
		if (!cbs[i])
			continue;
		switch (cbs[i]->aio_lio_opcode) {
		case LIO_READ:
			ret = aio_read(cbs[i]);
			break;
		case LIO_WRITE:
			ret = aio_write(cbs[i]);
			break;
		case LIO_NOP:
			/* Nothing to wait for, and the block may still hold
			   the state of an older request.  */
			if (st)
				st->cbs[i] = 0;
			continue;
		default:
			/* POSIX: the operation fails with EINVAL and the
			   list with EIO.  */
			cbs[i]->__ret = -1;
			cbs[i]->__err = EINVAL;
			if (!bad)
				bad = EINVAL;
			if (st)
				st->cbs[i] = 0;
			continue;
		}
		if (ret) {
			/* The request did not start, and its own block says
			   why.  POSIX wants the rest submitted all the same
			   and one answer at the end.  */
			if (!bad)
				bad = errno;
			if (st)
				st->cbs[i] = 0;
		} else
			live++;
	}

	if (mode == LIO_WAIT) {
		int saved;

		/* aio_suspend inside lio_wait is a cancellation point, and
		   the list belongs to this call.  */
		pthread_cleanup_push(free, st);
		ret = lio_wait(st);
		saved = errno;
		pthread_cleanup_pop(1);
		__set_errno(saved);
		/* An interrupted wait leaves the outcome of the list open,
		   so it is the answer -- EIO would claim knowledge of a
		   failure that may not have happened yet.  */
		if (ret && errno == EINTR)
			return -1;
		if (bad || ret) {
			__set_errno(bad == EAGAIN ? EAGAIN : EIO);
			return -1;
		}
		return 0;
	}

	if (st && !live && bad) {
		/* Nothing was submitted, so nothing will ever complete:
		   there is no later point for the notification, and the
		   return value already tells the caller that the list
		   failed.  A list that is merely empty -- no entries, only
		   NULL ones or only LIO_NOP -- succeeds, and its
		   notification is due right away.  */
		free(st);
		st = 0;
	}

	if (st) {
		pthread_attr_t a;
		sigset_t set, set_old;
		pthread_t td;

		if (sev->sigev_notify == SIGEV_THREAD) {
			if (sev->sigev_notify_attributes)
				a = *(pthread_attr_t *)sev->sigev_notify_attributes;
			else
				pthread_attr_init(&a);
		} else {
			pthread_attr_init(&a);
			pthread_attr_setstacksize(&a, __pthread_get_minstack(&a));
			pthread_attr_setguardsize(&a, 0);
		}
		pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);
		sigfillset(&set);
		pthread_sigmask(SIG_BLOCK, &set, &set_old);
		ret = pthread_create(&td, &a, wait_thread, st);
		pthread_sigmask(SIG_SETMASK, &set_old, 0);
		if (ret) {
			free(st);
			__set_errno(EAGAIN);
			return -1;
		}
	}

	if (bad) {
		__set_errno(bad == EAGAIN ? EAGAIN : EIO);
		return -1;
	}
	return 0;
}

strong_alias(lio_listio,lio_listio64)
