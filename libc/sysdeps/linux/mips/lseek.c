/*
 * lseek()/lseek64() for MIPS uClibc
 *
 * Copyright (C) 2026 Ramin Moussavi <ramin.moussavi@yacoub.de>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <_lfs_64.h>
#include <sys/syscall.h>
#include <sgidefs.h>

#if _MIPS_SIM == _MIPS_SIM_NABI32 && defined __NR_lseek

# include <unistd.h>
# include <errno.h>
# include <cancel.h>

/* n32 has neither _llseek nor lseek64, so the generic lseek.c builds the
 * wrapper with off_t -- 32 bit here -- and aliases lseek64 onto it.  The n32
 * lseek syscall takes and returns a full 64-bit offset, so turn it around the
 * way glibc does: lseek64 is the real call and lseek narrows it.  */
off64_t __NC(lseek64)(int fd, off64_t offset, int whence)
{
	return (off64_t) INLINE_SYSCALL(lseek, 3, fd, offset, whence);
}
CANCELLABLE_SYSCALL(off64_t, lseek64, (int fd, off64_t offset, int whence),
		    (fd, offset, whence))
lt_libc_hidden(lseek64)

off_t __NC(lseek)(int fd, off_t offset, int whence)
{
	off64_t res = __NC(lseek64)(fd, offset, whence);

	if (res != (off64_t) (off_t) res) {
		__set_errno(EOVERFLOW);
		return (off_t) -1;
	}
	return (off_t) res;
}
CANCELLABLE_SYSCALL(off_t, lseek, (int fd, off_t offset, int whence),
		    (fd, offset, whence))
lt_libc_hidden(lseek)

#else
# include "../common/lseek.c"
#endif
