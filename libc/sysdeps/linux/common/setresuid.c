/*
 * setresuid() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <internal/setxid.h>
#if defined __USE_GNU && defined __UCLIBC_LINUX_SPECIFIC__
#include <unistd.h>

#if defined(__NR_setresuid32)
# undef __NR_setresuid
# define __NR_setresuid __NR_setresuid32

int setresuid(uid_t ruid, uid_t euid, uid_t suid)
{
	return INLINE_SETXID_SYSCALL(setresuid, 3, ruid, euid, suid);
}
libc_hidden_weak(setresuid)

#elif defined(__NR_setresuid)

int setresuid(uid_t ruid, uid_t euid, uid_t suid)
{
	if (((ruid + 1) > (uid_t) ((__kernel_uid_t) - 1U))
		|| ((euid + 1) > (uid_t) ((__kernel_uid_t) - 1U))
		|| ((suid + 1) > (uid_t) ((__kernel_uid_t) - 1U))) {
		__set_errno(EINVAL);
		return -1;
	}
	return INLINE_SETXID_SYSCALL(setresuid, 3, ruid, euid, suid);
}
libc_hidden_weak(setresuid)

#endif

#endif
