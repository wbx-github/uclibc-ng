/*
 * renameat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sysdep.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

int
renameat (int oldfd, const char *old, int newfd, const char *new)
{
#if defined(__NR_renameat)
	return INLINE_SYSCALL(renameat, 4, oldfd, old, newfd, new);
#elif defined(__NR_renameat2)
	return INLINE_SYSCALL(renameat2, 5, oldfd, old, newfd, new, 0);
#else
	if (oldfd == AT_FDCWD && newfd == AT_FDCWD)
		return rename(old, new);
	__set_errno(ENOSYS);
	return -1;
#endif
}
libc_hidden_def (renameat)
