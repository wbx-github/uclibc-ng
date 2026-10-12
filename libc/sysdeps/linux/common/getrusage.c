/*
 * getrusage() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <wait.h>
#include <sys/resource.h>

#if defined(__UCLIBC_USE_TIME64__)
#include <string.h>

/* There is no getrusage with a 64-bit time_t: the kernel writes struct
   rusage with 32-bit timevals, which is 16 bytes shorter than ours.  Do
   what wait4() does and let it write starting 4 32-bit words before
   ru_maxrss, so the long fields land where they belong, then build ru_utime
   and ru_stime from those 4 words.  */
# define __NR___syscall_getrusage __NR_getrusage
static __always_inline _syscall2(int, __syscall_getrusage, __rusage_who_t, who,
				 struct rusage *, usage)

int getrusage(__rusage_who_t who, struct rusage *usage)
{
	char *kru = usage ? (char *)&usage->ru_maxrss - 4 * sizeof(__S32_TYPE) : NULL;
	int ret = __syscall_getrusage(who, (struct rusage *)kru);

	if (ret == 0 && usage) {
		__S32_TYPE r32[4];
		memcpy(r32, kru, sizeof(r32));
		usage->ru_utime = (struct timeval){ .tv_sec = r32[0], .tv_usec = r32[1] };
		usage->ru_stime = (struct timeval){ .tv_sec = r32[2], .tv_usec = r32[3] };
	}
	return ret;
}
#else
_syscall2(int, getrusage, __rusage_who_t, who, struct rusage *, usage)
#endif
