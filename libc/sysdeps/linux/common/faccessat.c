/*
 * faccessat() for uClibc
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#ifdef __NR_faccessat
_syscall4(int, faccessat, int, fd, const char *, file, int, type, int, flag)
#else
/* Minimal fallback: only the AT_FDCWD case can be mapped to access().
   Anything else would need emulation via /proc/self/fd/. */
int faccessat(int fd, const char *file, int type, int flag)
{
	if (fd == AT_FDCWD && !(flag & ~AT_EACCESS))
		return access(file, type);
	__set_errno(ENOSYS);
	return -1;
}
#endif
libc_hidden_def(faccessat)
