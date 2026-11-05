/*
 * timerfd_create() / timerfd_settime() / timerfd_gettime() for uClibc
 *
 * Copyright (C) 2009 Stephan Raue <stephan@openelec.tv>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/timerfd.h>

#if defined(__UCLIBC_USE_TIME64__)
#include "internal/time64_helpers.h"
#endif

/*
 * timerfd_create()
 */
#ifdef __NR_timerfd_create
_syscall2(int, timerfd_create, int, clockid, int, flags)
#endif

/*
 * timerfd_settime()
 */
#if defined(__NR_timerfd_settime) || defined(__NR_timerfd_settime64)
#if defined(__UCLIBC_USE_TIME64__) && defined(__NR_timerfd_settime64)
int timerfd_settime(int ufd, int flags, const struct itimerspec *utmr, struct itimerspec *otmr)
{
    struct __its64_struct __old;
    int __ret = INLINE_SYSCALL(timerfd_settime64, 4, ufd, flags, TO_ITS64_P(utmr),
                               otmr ? &__old : NULL);

    if (__ret == 0 && otmr)
        __its64_to_its(&__old, otmr);
    return __ret;
}
#else
_syscall4(int, timerfd_settime, int, ufd, int, flags, const struct itimerspec *, utmr, struct itimerspec *, otmr)
#endif
#endif

/*
 * timerfd_gettime()
 */
#if defined(__NR_timerfd_gettime) || defined(__NR_timerfd_gettime64)
#if defined(__UCLIBC_USE_TIME64__) && defined(__NR_timerfd_gettime64)
int timerfd_gettime(int ufd, struct itimerspec *otmr)
{
    struct __its64_struct __cur;
    int __ret = INLINE_SYSCALL(timerfd_gettime64, 2, ufd, otmr ? &__cur : NULL);

    if (__ret == 0 && otmr)
        __its64_to_its(&__cur, otmr);
    return __ret;
}
#else
_syscall2(int, timerfd_gettime, int, ufd, struct itimerspec *, otmr)
#endif
#endif
