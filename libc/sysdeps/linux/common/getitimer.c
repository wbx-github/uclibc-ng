/*
 * getitimer() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/time.h>

#if defined(__UCLIBC_USE_TIME64__)
#include "internal/time64_helpers.h"

int getitimer(__itimer_which_t which, struct itimerval *value)
{
	struct __itimerval32_struct __itv32;
	int __ret = INLINE_SYSCALL(getitimer, 2, which, value ? &__itv32 : NULL);

	if (__ret == 0 && value) {
		value->it_interval.tv_sec = __itv32.__interval_sec;
		value->it_interval.tv_usec = __itv32.__interval_usec;
		value->it_value.tv_sec = __itv32.__value_sec;
		value->it_value.tv_usec = __itv32.__value_usec;
	}

	return __ret;
}
#else
_syscall2(int, getitimer, __itimer_which_t, which, struct itimerval *, value)
#endif
