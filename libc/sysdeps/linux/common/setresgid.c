/*
 * setresgid() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <internal/setxid.h>
#ifdef __USE_GNU
#include <unistd.h>

#if defined(__NR_setresgid32)
# undef __NR_setresgid
# define __NR_setresgid __NR_setresgid32

int setresgid(gid_t rgid, gid_t egid, gid_t sgid)
{
	return INLINE_SETXID_SYSCALL(setresgid, 3, rgid, egid, sgid);
}
libc_hidden_weak(setresgid)

#elif defined(__NR_setresgid)

int setresgid(gid_t rgid, gid_t egid, gid_t sgid)
{
	if (((rgid + 1) > (gid_t) ((__kernel_gid_t) - 1U))
		|| ((egid + 1) > (gid_t) ((__kernel_gid_t) - 1U))
		|| ((sgid + 1) > (gid_t) ((__kernel_gid_t) - 1U))) {
		__set_errno(EINVAL);
		return -1;
	}
	return INLINE_SETXID_SYSCALL(setresgid, 3, rgid, egid, sgid);
}
libc_hidden_weak(setresgid)

#endif

#endif
