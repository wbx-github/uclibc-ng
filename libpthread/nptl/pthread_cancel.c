/* Copyright (C) 2002, 2003, 2004, 2009 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <errno.h>
#include <signal.h>
#include "pthreadP.h"
#include "atomic.h"
#include <sysdep.h>
#include <unistd.h>
#include <bits/kernel-features.h>


/* For asynchronous cancellation we use a signal.  This is the handler.  */
static void
sigcancel_handler (int sig, siginfo_t *si, void *ctx)
{

  /* Safety check.  It would be possible to call this function for
     other signals and send a signal from another process.  This is not
     correct and might even be a security problem.  Try to catch as
     many incorrect invocations as possible.  */
  if (sig != SIGCANCEL
      || si->si_pid != getpid()
      || si->si_code != SI_TKILL)
    return;

  struct pthread *self = THREAD_SELF;

  int oldval = THREAD_GETMEM (self, cancelhandling);
  while (1)
    {
      /* We are canceled now.  When canceled by another thread this flag
	 is already set but if the signal is directly send (internally or
	 from another process) is has to be done here.  */
      int newval = oldval | CANCELING_BITMASK | CANCELED_BITMASK;

      if (oldval == newval || (oldval & EXITING_BITMASK) != 0)
	/* Already canceled or exiting.  */
	break;

      int curval = THREAD_ATOMIC_CMPXCHG_VAL (self, cancelhandling, newval,
					      oldval);
      if (curval == oldval)
	{
	  /* Set the return value.  */
	  THREAD_SETMEM (self, result, PTHREAD_CANCELED);

	  /* Make sure asynchronous cancellation is still enabled.  */
	  if ((newval & CANCELTYPE_BITMASK) != 0)
	    /* Run the registered destructors and terminate the thread.  */
	    __do_cancel ();

	  break;
	}

      oldval = curval;
    }
}


/* Install the handler for SIGCANCEL.  Only pthread_cancel and a failed
   pthread_create send the signal, so a program that calls neither does
   not get the handler, nor the unwinder behind __do_cancel.  */
void
__pthread_sigcancel_init (void)
{
  static volatile int installed;
  struct sigaction sa;

  if (installed)
    {
      /* Pairs with the barrier below: whoever sees the flag has the
	 handler in place and may send the signal.  Two threads can get
	 here at once -- pthread_cancel, unlike the setxid handler, is
	 not set up before the first clone -- and installing it twice
	 does no harm.  */
      atomic_read_barrier ();
      return;
    }
  sa.sa_sigaction = sigcancel_handler;
  sa.sa_flags = SA_SIGINFO;
  __sigemptyset (&sa.sa_mask);
  (void) __libc_sigaction (SIGCANCEL, &sa, NULL);
  atomic_write_barrier ();
  installed = 1;
}


int
pthread_cancel (
     pthread_t th)
{
  volatile struct pthread *pd = (volatile struct pthread *) th;

  /* Make sure the descriptor is valid.  */
  if (INVALID_TD_P (pd))
    /* Not a valid thread handle.  */
    return ESRCH;

#ifdef SHARED
  pthread_cancel_init ();
#endif
  __pthread_sigcancel_init ();
  int result = 0;
  int oldval;
  int newval;
  do
    {
    again:
      oldval = pd->cancelhandling;
      newval = oldval | CANCELING_BITMASK | CANCELED_BITMASK;

      /* Avoid doing unnecessary work.  The atomic operation can
	 potentially be expensive if the bug has to be locked and
	 remote cache lines have to be invalidated.  */
      if (oldval == newval)
	break;

      /* If the cancellation is handled asynchronously just send a
	 signal.  We avoid this if possible since it's more
	 expensive.  */
      if (CANCEL_ENABLED_AND_CANCELED_AND_ASYNCHRONOUS (newval))
	{
	  /* Cancelling ourselves needs no signal: deliver the cancellation
	     directly instead of bouncing through tgkill+SIGCANCEL.  This
	     avoids the syscall, the kernel-built signal frame and the extra
	     (signal-frame) unwind step.  */
	  if (pd == (volatile struct pthread *) THREAD_SELF)
	    {
	      if (atomic_compare_and_exchange_bool_acq (&pd->cancelhandling,
							newval, oldval))
		goto again;
	      THREAD_SETMEM (THREAD_SELF, result, PTHREAD_CANCELED);
	      __do_cancel ();
	      /* NOTREACHED */
	    }

	  /* Mark the cancellation as "in progress".  */
	  if (atomic_compare_and_exchange_bool_acq (&pd->cancelhandling,
						    oldval | CANCELING_BITMASK,
						    oldval))
	    goto again;

	  /* The cancellation handler will take care of marking the
	     thread as canceled.  */
	  INTERNAL_SYSCALL_DECL (err);

	  /* One comment: The PID field in the TCB can temporarily be
	     changed (in fork).  But this must not affect this code
	     here.  Since this function would have to be called while
	     the thread is executing fork, it would have to happen in
	     a signal handler.  But this is no allowed, pthread_cancel
	     is not guaranteed to be async-safe.  */
	  pid_t pid = getpid ();
	  int val;
	  val = INTERNAL_SYSCALL (tgkill, err, 3, pid, pd->tid, SIGCANCEL);

	  if (INTERNAL_SYSCALL_ERROR_P (val, err))
	    result = INTERNAL_SYSCALL_ERRNO (val, err);

	  break;
	}
    }
  /* Mark the thread as canceled.  This has to be done
     atomically since other bits could be modified as well.  */
  while (atomic_compare_and_exchange_bool_acq (&pd->cancelhandling, newval,
					       oldval));

  return result;
}

PTHREAD_STATIC_FN_REQUIRE (pthread_create)
