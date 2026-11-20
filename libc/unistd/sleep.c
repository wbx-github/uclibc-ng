/* Implementation of the POSIX sleep function using nanosleep.
   Copyright (C) 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   see <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>


#if defined __UCLIBC_HAS_REALTIME__

/* From musl.  Always through nanosleep, so that sleep(0) is a cancellation
   point too.  */
unsigned int sleep (unsigned int seconds)
{
	/* An unsigned int can hold more seconds than a 32-bit time_t, so
	   sleep in steps where it does not fit.  The compiler drops this
	   where tv_sec is the wider type.  */
	const unsigned int max =
		(unsigned int) (((unsigned long int) (~((time_t) 0))) >> 1);
	struct timespec ts;

	do {
		if (sizeof(ts.tv_sec) <= sizeof(seconds)) {
			ts.tv_sec = (time_t) (seconds < max ? seconds : max);
			seconds -= (unsigned int) ts.tv_sec;
		} else {
			ts.tv_sec = (time_t) seconds;
			seconds = 0;
		}
		ts.tv_nsec = 0;

		if (nanosleep(&ts, &ts))
			return seconds + (unsigned int) ts.tv_sec;
	} while (seconds > 0);

	return 0;
}

#else /* __UCLIBC_HAS_REALTIME__ */

/* no nanosleep, use signals and alarm() */
static void sleep_alarm_handler(int attribute_unused sig)
{
}
unsigned int sleep (unsigned int seconds)
{
    struct sigaction act, oact;
    sigset_t set, oset;
    unsigned int result, remaining;
    time_t before, after;
    int old_errno = errno;

    /* This is not necessary but some buggy programs depend on this.  */
    if (seconds == 0)
	return 0;

    /* block SIGALRM */
    __sigemptyset (&set);
    __sigaddset (&set, SIGALRM);
    sigprocmask (SIG_BLOCK, &set, &oset); /* can't fail */

    act.sa_handler = sleep_alarm_handler;
    act.sa_flags = 0;
    act.sa_mask = oset;
    sigaction(SIGALRM, &act, &oact); /* never fails */

    before = time(NULL);
    remaining = alarm(seconds);
    if (remaining && remaining > seconds) {
	/* restore user's alarm */
	sigaction(SIGALRM, &oact, NULL);
	alarm(remaining); /* restore old alarm */
	sigsuspend(&oset);
	after = time(NULL);
    } else {
	sigsuspend (&oset);
	after = time(NULL);
	sigaction (SIGALRM, &oact, NULL);
    }
    result = after - before;
    alarm(remaining > result ? remaining - result : 0);
    sigprocmask (SIG_SETMASK, &oset, NULL);

    __set_errno(old_errno);

    return result > seconds ? 0 : seconds - result;
}

#endif /* __UCLIBC_HAS_REALTIME__ */

libc_hidden_def(sleep)
