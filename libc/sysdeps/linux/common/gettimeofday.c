/*
 * gettimeofday() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/time.h>
#include <time.h>

#ifdef __VDSO_SUPPORT__
#include "ldso.h"
#endif

extern int clock_gettime (clockid_t __clock_id, struct timespec *__tp) __THROW;

int gettimeofday(struct timeval * tv, __timezone_ptr_t tz) {
    if (!tv)
        return 0;
#if defined(__VDSO_SUPPORT__) && defined(ARCH_VDSO_GETTIMEOFDAY)
    return ARCH_VDSO_GETTIMEOFDAY(tv, tz);
#elif !defined(__NR_clock_gettime) && !defined(__NR_clock_gettime64) \
      && defined(__NR_gettimeofday)
    return INLINE_SYSCALL(gettimeofday, 2, tv, tz);
#else
    struct timespec __ts;
    int __ret = clock_gettime(CLOCK_REALTIME, &__ts);
    if (__ret == 0) {
        tv->tv_sec = __ts.tv_sec;
        tv->tv_usec = (suseconds_t)__ts.tv_nsec / 1000;
    }
    return __ret;
#endif
}

libc_hidden_def(gettimeofday)
