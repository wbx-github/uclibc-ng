/* sem_timedwait -- wait on a semaphore.  Generic futex-using version.
   Copyright (C) 2003, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Paul Mackerras <paulus@au.ibm.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sysdep.h>
#include <lowlevellock.h>
#include <internaltypes.h>
#include <semaphore.h>
#include <time.h>

#include <pthreadP.h>


#ifdef SEM_CLOCKWAIT_ONLY
/* sem_wait is in assembly and has no C cleanup handler.  */
static void
__sem_wait_cleanup (void *arg)
{
  struct new_sem *isem = (struct new_sem *) arg;

  atomic_decrement (&isem->nwaiters);
}
#else
extern void __sem_wait_cleanup (void *arg) attribute_hidden;
#endif


static int
__sem_clockwait (sem_t *sem, clockid_t clockid, const struct timespec *abstime)
{
  struct new_sem *isem = (struct new_sem *) sem;
  int err;
  int cancelhandling;

  /* A cancellation point even when it does not block, as POSIX requires.  */
  CANCELLATION_P (THREAD_SELF);

  if (atomic_decrement_if_positive (&isem->value) > 0)
    return 0;

  if (abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000)
    {
      __set_errno (EINVAL);
      return -1;
    }

  atomic_increment (&isem->nwaiters);

  pthread_cleanup_push (__sem_wait_cleanup, isem);

  while (1)
    {
      struct timespec now;
      struct timespec rt;
      int sec, nsec;

      /* Get the current time.  */
      clock_gettime (clockid, &now);

      /* Compute relative timeout.  */
      sec = abstime->tv_sec - now.tv_sec;
      nsec = abstime->tv_nsec - now.tv_nsec;
      if (nsec < 0)
	{
	  nsec += 1000000000;
	  --sec;
	}

      /* Already timed out?  */
      err = -ETIMEDOUT;
      if (sec < 0)
	{
	  __set_errno (ETIMEDOUT);
	  err = -1;
	  break;
	}

      /* Do wait.  */
      rt.tv_sec = sec;
      rt.tv_nsec = nsec;

      /* Enable asynchronous cancellation.  Required by the standard.  */
      int oldtype = __pthread_enable_asynccancel ();

      err = lll_futex_timed_wait (&isem->value, 0, &rt,
				  isem->private ^ FUTEX_PRIVATE_FLAG);

      /* Disable asynchronous cancellation.  */
      __pthread_disable_asynccancel (oldtype);

      if (err != 0 && err != -EWOULDBLOCK)
	{
	  __set_errno (-err);
	  err = -1;
	  break;
	}

      if (atomic_decrement_if_positive (&isem->value) > 0)
	{
	  err = 0;
	  break;
	}
    }

  pthread_cleanup_pop (0);

  atomic_decrement (&isem->nwaiters);

  return err;
}

#ifndef SEM_CLOCKWAIT_ONLY
int
sem_timedwait (sem_t *sem, const struct timespec *abstime)
{
  return __sem_clockwait (sem, CLOCK_REALTIME, abstime);
}
#endif

int
sem_clockwait (sem_t *sem, clockid_t clockid, const struct timespec *abstime)
{
  if (clockid != CLOCK_REALTIME && clockid != CLOCK_MONOTONIC)
    {
      __set_errno (EINVAL);
      return -1;
    }

  return __sem_clockwait (sem, clockid, abstime);
}
