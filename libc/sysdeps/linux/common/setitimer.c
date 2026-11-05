/*
 * setitimer() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/time.h>


#if defined(__UCLIBC_USE_TIME64__)
#include "internal/time64_helpers.h"

int setitimer(__itimer_which_t which, const struct itimerval *restrict new, struct itimerval *restrict old)
{
	struct __itimerval32_struct __itv32 = {
		.__interval_sec = new->it_interval.tv_sec,
		.__interval_usec = new->it_interval.tv_usec,
		.__value_sec = new->it_value.tv_sec,
		.__value_usec = new->it_value.tv_usec
	};
	struct __itimerval32_struct __itv32_old;

	int __ret = INLINE_SYSCALL(setitimer, 3, which, &__itv32, &__itv32_old);
	if (__ret == 0 && old) {
		old->it_interval.tv_sec = __itv32_old.__interval_sec;
		old->it_interval.tv_usec = __itv32_old.__interval_usec;
		old->it_value.tv_sec = __itv32_old.__value_sec;
		old->it_value.tv_usec = __itv32_old.__value_usec;
	}

	return __ret;
}
#else
_syscall3(int, setitimer, __itimer_which_t, which,
		  const struct itimerval *, new, struct itimerval *, old)

#endif
libc_hidden_def(setitimer)
