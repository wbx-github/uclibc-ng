/*
 * adjtimex() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/timex.h>
#include <time.h>

int adjtimex(struct timex *buf)
{
#ifdef __NR_clock_adjtime
    return clock_adjtime(CLOCK_REALTIME, buf);
#elif defined(__NR_adjtimex)
    return syscall(__NR_adjtimex, buf);
#else
    errno = ENOSYS;
    return -1;
#endif
}

libc_hidden_def(adjtimex)
weak_alias(adjtimex,__adjtimex)
#if defined __UCLIBC_NTP_LEGACY__
strong_alias(adjtimex,ntp_adjtime)
#endif
