/*
 * sched_rr_get_interval() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sched.h>
#include <sys/types.h>
#include <sys/syscall.h>

#if defined(__UCLIBC_USE_TIME64__) && defined(__NR_sched_rr_get_interval_time64)
#include "internal/time64_helpers.h"

int sched_rr_get_interval(pid_t pid, struct timespec *tp)
{
	struct __ts64_struct __ts64;
	int __ret = INLINE_SYSCALL(sched_rr_get_interval_time64, 2, pid, tp ? &__ts64 : NULL);

	if (__ret == 0 && tp)
		__ts64_to_ts(&__ts64, tp);
	return __ret;
}
#else
#define __NR___syscall_sched_rr_get_interval __NR_sched_rr_get_interval

static __inline__ _syscall2(int, __syscall_sched_rr_get_interval,
		__kernel_pid_t, pid, struct timespec *, tp)

int sched_rr_get_interval(pid_t pid, struct timespec *tp)
{
	return (__syscall_sched_rr_get_interval(pid, tp));
}
#endif
