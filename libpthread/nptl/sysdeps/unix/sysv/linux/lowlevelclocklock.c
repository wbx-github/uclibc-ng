/* Lock waits with a timeout on a given clock, for pthread_mutex_clocklock.
   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.

   Plain C on every target, so the timed waits written in assembler need no
   variant; those stay on CLOCK_REALTIME for pthread_mutex_timedlock.  */

#include <errno.h>
#include <time.h>
#include <sysdep.h>
#include <lowlevellock.h>
#include <pthreadP.h>

/* sparc32 has 24 bits of the lock word for the value.  */
#ifndef atomic_compare_and_exchange_val_24_acq
# define atomic_compare_and_exchange_val_24_acq(mem, newval, oldval) \
  atomic_compare_and_exchange_val_acq (mem, newval, oldval)
#endif

/* ABSTIME on CLOCKID less the time now, in *RT; -1 once it has passed.  */
static int
remaining (clockid_t clockid, const struct timespec *abstime,
	   struct timespec *rt)
{
  struct timespec now;

  clock_gettime (clockid, &now);

  /* Compare before subtracting: with a time_t far from now the
     difference overflows, and a deadline long past would come back as
     a long wait.  */
  if (abstime->tv_sec < now.tv_sec
      || (abstime->tv_sec == now.tv_sec && abstime->tv_nsec <= now.tv_nsec))
    return -1;

  rt->tv_sec = abstime->tv_sec - now.tv_sec;
  rt->tv_nsec = abstime->tv_nsec - now.tv_nsec;
  if (rt->tv_nsec < 0)
    {
      rt->tv_nsec += 1000000000;
      --rt->tv_sec;
    }
  return 0;
}

int
__lll_clocklock_wait (int *futex, clockid_t clockid,
		      const struct timespec *abstime, int private)
{
  if (atomic_compare_and_exchange_val_24_acq (futex, 1, 0) == 0)
    return 0;

  if (abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000)
    return EINVAL;

  do
    {
      struct timespec rt;

      /* Mark the lock as contended before looking at the clock.  Giving
	 up with the futex still at 1 would drop the waiters flag, and
	 __lll_unlock only wakes when it reads more than 1 -- a waiter
	 already asleep would then never be woken.  */
      if (atomic_compare_and_exchange_val_24_acq (futex, 2, 1) == 0)
	continue;

      if (remaining (clockid, abstime, &rt))
	return ETIMEDOUT;

      lll_futex_timed_wait (futex, 2, &rt, private);
    }
  while (atomic_compare_and_exchange_val_24_acq (futex, 2, 0) != 0);

  return 0;
}

int
__lll_robust_clocklock_wait (int *futex, int id, clockid_t clockid,
			     const struct timespec *abstime, int private)
{
  int oldval = atomic_compare_and_exchange_val_acq (futex, id, 0);

  if (oldval == 0)
    return 0;

  if (abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000)
    return EINVAL;

  do
    {
      struct timespec rt;

      if (__builtin_expect (oldval & FUTEX_OWNER_DIED, 0))
	return oldval;

      /* Set FUTEX_WAITERS before looking at the clock, as the plain
	 lock above does: giving up with the bit clear would leave a
	 waiter asleep that the next unlock no longer wakes.  */
      int newval = oldval | FUTEX_WAITERS;
      if (oldval != newval
	  && atomic_compare_and_exchange_bool_acq (futex, newval, oldval))
	continue;

      if (remaining (clockid, abstime, &rt))
	return ETIMEDOUT;

      lll_futex_timed_wait (futex, newval, &rt, private);
    }
  while ((oldval = atomic_compare_and_exchange_val_acq (futex,
							id | FUTEX_WAITERS,
							0)) != 0);

  return 0;
}
