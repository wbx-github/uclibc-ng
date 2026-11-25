/*
 * utimensat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/stat.h>
#include <stdint.h>

#if defined(__UCLIBC_USE_TIME64__)
#include "internal/time64_helpers.h"
#endif

#if defined(__NR_utimensat) || defined(__NR_utimensat_time64)
#if defined(__UCLIBC_USE_TIME64__) && defined(__NR_utimensat_time64)
int utimensat(int fd, const char *path, const struct timespec times[2], int flags)
{
    struct __ts64_struct __times64[2] = {
        {
            .tv_sec = times ? times[0].tv_sec : 0,
            .tv_nsec = times ? times[0].tv_nsec : 0
        },
        {
            .tv_sec = times ? times[1].tv_sec : 0,
            .tv_nsec = times ? times[1].tv_nsec : 0
        }
    };

    return INLINE_SYSCALL(utimensat_time64, 4, fd, path, times ? (uintptr_t) &__times64 : 0, flags);
}
#else
int utimensat(int fd, const char *path, const struct timespec times[2], int flags)
{
    return INLINE_SYSCALL(utimensat, 4, fd, path, times, flags);
}
#endif
libc_hidden_def(utimensat)
#else
/* should add emulation with utimens() and /proc/self/fd/ ... */
#endif

