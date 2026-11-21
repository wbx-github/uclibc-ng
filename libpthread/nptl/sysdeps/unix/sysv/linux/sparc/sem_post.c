/* sem_post -- post to a POSIX semaphore.  SPARC version.
   Copyright (C) 2003, 2004, 2006, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2003.

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

int
sem_post (sem_t *sem)
{
  struct sparc_new_sem *isem = (struct sparc_new_sem *) sem;
  int full = 0;

  if (__atomic_is_v9)
    {
      __typeof (isem->value) cur;
      do
	{
	  cur = isem->value;
	  if (cur == SEM_VALUE_MAX)
	    {
	      full = 1;
	      break;
	    }
	}
      while (atomic_compare_and_exchange_bool_acq (&isem->value, cur + 1,
						    cur));
    }
  else
    {
      __sparc32_atomic_do_lock24 (&isem->lock);
      if (isem->value == SEM_VALUE_MAX)
	full = 1;
      else
	isem->value++;
      __sparc32_atomic_do_unlock24 (&isem->lock);
    }
  if (full)
    {
      __set_errno (EOVERFLOW);
      return -1;
    }
  atomic_full_barrier ();
  if (isem->nwaiters > 0)
    {
      int err = lll_futex_wake (&isem->value, 1,
				isem->private ^ FUTEX_PRIVATE_FLAG);
      if (__builtin_expect (err, 0) < 0)
	{
	  __set_errno (-err);
	  return -1;
	}
    }
  return 0;
}
