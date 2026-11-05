/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <sysdep.h>
#include <bits/kernel-features.h>
#include "kernel-posix-timers.h"

#if defined(__UCLIBC_USE_TIME64__)
#include "internal/time64_helpers.h"
#endif


#if defined(__NR_timer_gettime) || defined(__NR_timer_gettime64)
# ifndef __ASSUME_POSIX_TIMERS
static int compat_timer_gettime (timer_t timerid, struct itimerspec *value);
#  define timer_gettime static compat_timer_gettime
#  include <nptl/sysdeps/pthread/timer_gettime.c>
#  undef timer_gettime
# endif

# ifdef timer_gettime_alias
#  define timer_gettime timer_gettime_alias
# endif


int
timer_gettime (
     timer_t timerid,
     struct itimerspec *value)
{
# undef timer_gettime
# ifndef __ASSUME_POSIX_TIMERS
  if (__no_posix_timers >= 0)
# endif
    {
      struct timer *kt = (struct timer *) timerid;

      /* Read the kernel timer.  */
# if defined(__UCLIBC_USE_TIME64__) && defined(__NR_timer_gettime64)
      struct __its64_struct cur64;
      int res = INLINE_SYSCALL (timer_gettime64, 2, kt->ktimerid,
				value ? &cur64 : NULL);

      if (res == 0 && value)
	__its64_to_its (&cur64, value);
# else
      int res = INLINE_SYSCALL (timer_gettime, 2, kt->ktimerid, value);
# endif

# ifndef __ASSUME_POSIX_TIMERS
      if (res != -1 || errno != ENOSYS)
	{
	  /* We know the syscall support is available.  */
	  __no_posix_timers = 1;
# endif
	  return res;
# ifndef __ASSUME_POSIX_TIMERS
	}
# endif

# ifndef __ASSUME_POSIX_TIMERS
      __no_posix_timers = -1;
# endif
    }

# ifndef __ASSUME_POSIX_TIMERS
  return compat_timer_gettime (timerid, value);
# endif
}
#else
# ifdef timer_gettime_alias
#  define timer_gettime timer_gettime_alias
# endif
/* The new system calls are not available.  Use the userlevel
   implementation.  */
# include <nptl/sysdeps/pthread/timer_gettime.c>
#endif
