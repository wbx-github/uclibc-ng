#ifndef _DL_VDSO_CALLS_H
#define _DL_VDSO_CALLS_H

#include <sys/time.h>
#include <sys/syscall.h>
#include <errno.h>
#include <time.h>

void __attribute__((weak)) *_get__dl__vdso_clock_gettime(void);
#if defined(__UCLIBC_USE_TIME64__)
#include "internal/time64_helpers.h"
void __attribute__((weak)) *_get__dl__vdso_clock_gettime64(void);
typedef int (*clock_gettime_func)(clockid_t clock_id, struct __ts64_struct *tp);
#else
typedef int (*clock_gettime_func)(clockid_t clock_id, struct timespec *tp);
#endif

extern int __libc_clock_gettime(clockid_t clock_id, struct timespec *tp);

static int __attribute__ ((used)) __generic_vdso_clock_gettime(clockid_t clock_id, struct timespec *tp);
static int __attribute__ ((used)) __generic_vdso_clock_gettime(clockid_t clock_id, struct timespec *tp)
{
    void *impl = NULL;
#if defined(__UCLIBC_USE_TIME64__)
    if (&_get__dl__vdso_clock_gettime64 && (impl = _get__dl__vdso_clock_gettime64())) {
        struct __ts64_struct __ts64;
        int __ret = ((clock_gettime_func)impl)(clock_id, &__ts64);
        if (__ret != 0) {
            __set_errno(-__ret);
            return -1;
        }

        if (tp) {
            tp->tv_sec = __ts64.tv_sec;
            tp->tv_nsec = __ts64.tv_nsec;
        }
        return 0;
    }

    /* fallback to syscall */
    return __libc_clock_gettime(clock_id, tp);
#else
    if (&_get__dl__vdso_clock_gettime && (impl = _get__dl__vdso_clock_gettime())) {
        int __ret = ((clock_gettime_func)impl)(clock_id, tp);
        if (__ret != 0) {
            __set_errno(-__ret);
            return -1;
        }

        return 0;
    }

    /* fallback to syscall */
    return __libc_clock_gettime(clock_id, tp);
#endif
}

static int __attribute__ ((used)) __generic_vdso_gettimeofday(struct timeval *tv, __timezone_ptr_t tz)
{
    struct timespec ts;
    int __res = __generic_vdso_clock_gettime(CLOCK_REALTIME, &ts);
    if (__res == 0) {
        tv->tv_sec = ts.tv_sec;
        tv->tv_usec = (suseconds_t)ts.tv_nsec / 1000;
    }

    return __res;
}

#endif /* _DL_VDSO_CALLS_H */