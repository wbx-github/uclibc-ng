/*
 * wait4() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <errno.h>

#if defined(__NR_waitid)
/* wait4() expressed via the waitid(2) syscall.  Used as the implementation
 * when the kernel/arch does not provide wait4, and as a runtime fallback
 * when wait4 returns ENOSYS (e.g. rv32 kernels which only wire up waitid
 * even though the uapi headers still define __NR_wait4).
 *
 * We invoke the syscall directly here instead of calling uClibc-ng's
 * POSIX waitid() from waitid.c, because:
 *   1. waitid(3) is 4-arg (idtype, id, infop, options) and passes NULL as
 *      the 5th syscall argument; we need the rusage out-parameter, which
 *      only the Linux waitid(2) syscall (not POSIX waitid) provides.
 *   2. wait4 semantics require translating the pid argument into idtype/id
 *      and re-encoding siginfo_t (si_code/si_status) into the 'int *status'
 *      bit pattern wait/waitpid callers expect.  That logic does not belong
 *      in waitid().
 *   3. waitid() is wrapped as a CANCELLABLE_SYSCALL; this is the nocancel
 *      path. */
static pid_t __wait4_via_waitid(pid_t pid, int *status, int opts, struct rusage *rusage)
{
	idtype_t type;
	siginfo_t info;
	int res;

	info.si_pid = 0;

	if (pid < -1) {
		type = P_PGID;
		pid = -pid;
	} else if (pid == -1) {
		type = P_ALL;
	} else if (pid == 0) {
		type = P_PGID;
	} else {
		type = P_PID;
	}

	res = INLINE_SYSCALL(waitid, 5, type, pid, &info, opts | WEXITED, rusage);
	if (res < 0)
		return res;

	if (info.si_pid && status) {
		int sw = 0;
		switch (info.si_code) {
		case CLD_CONTINUED:
			sw = 0xffff;
			break;
		case CLD_DUMPED:
			sw = (info.si_status & 0x7f) | 0x80;
			break;
		case CLD_EXITED:
			sw = (info.si_status & 0xff) << 8;
			break;
		case CLD_KILLED:
			sw = info.si_status & 0x7f;
			break;
		case CLD_STOPPED:
		case CLD_TRAPPED:
			sw = (info.si_status << 8) + 0x7f;
			break;
		}
		*status = sw;
	}

	return info.si_pid;
}
#endif

#if defined(__NR_wait4)
# define __NR___syscall_wait4 __NR_wait4
static __always_inline _syscall4(int, __syscall_wait4, __kernel_pid_t, pid,
				 int *, status, int, opts, struct rusage *, rusage)

pid_t __wait4_nocancel(pid_t pid, int *status, int opts, struct rusage *rusage)
{
	int ret;
#if defined(__UCLIBC_USE_TIME64__)
	/* Kernel returns 32-bit timevals in the rusage prefix; convert below. */
	char *kru = rusage ? (char *)&rusage->ru_maxrss - 4 * sizeof(__S32_TYPE) : NULL;
	ret = __syscall_wait4(pid, status, opts, (struct rusage *)kru);
#else
	ret = __syscall_wait4(pid, status, opts, rusage);
#endif

#if defined(__NR_waitid)
	/* Some kernels (e.g. rv32) reject wait4 with ENOSYS; emulate via waitid. */
	if (ret == -1 && errno == ENOSYS)
		return __wait4_via_waitid(pid, status, opts, rusage);
#endif

#if defined(__UCLIBC_USE_TIME64__)
	if (ret > 0 && rusage) {
		__S32_TYPE r32[4];
		memcpy(r32, kru, 4 * sizeof(__S32_TYPE));
		rusage->ru_utime = (struct timeval){ .tv_sec = r32[0], .tv_usec = r32[1] };
		rusage->ru_stime = (struct timeval){ .tv_sec = r32[2], .tv_usec = r32[2] };
	}
#endif
	return ret;
}

#elif defined(__NR_waitid)
pid_t __wait4_nocancel(pid_t pid, int *status, int opts, struct rusage *rusage)
{
	return __wait4_via_waitid(pid, status, opts, rusage);
}
#endif

#ifdef __USE_BSD
strong_alias(__wait4_nocancel,wait4)
#endif
