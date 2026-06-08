/*
 * fallocate64() for MIPS uClibc
 *
 * Copyright (C) 2026 Ramin Moussavi <ramin.moussavi@yacoub.de>
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <_lfs_64.h>
#include <sys/syscall.h>
#include <bits/wordsize.h>

#if defined __NR_fallocate && __WORDSIZE == 32
# include <fcntl.h>
# include <endian.h>
# include <errno.h>
# include <sgidefs.h>

extern __typeof(fallocate64) __libc_fallocate64 attribute_hidden;
int attribute_hidden __libc_fallocate64(int fd, int mode, __off64_t offset,
		__off64_t len)
{
	int ret;
	INTERNAL_SYSCALL_DECL(err);
# if _MIPS_SIM == _ABIO32
	/* o32 splits each 64-bit argument into a register pair.  */
	ret = (int) (INTERNAL_SYSCALL(fallocate, err, 6, fd, mode,
		OFF64_HI_LO (offset), OFF64_HI_LO (len)));
# else /* N32: 64-bit registers, pass each 64-bit argument whole.  */
	ret = (int) (INTERNAL_SYSCALL(fallocate, err, 4, fd, mode, offset, len));
# endif
	if (unlikely(INTERNAL_SYSCALL_ERROR_P (ret, err))) {
		__set_errno(INTERNAL_SYSCALL_ERRNO (ret, err));
		ret = -1;
	}
	return ret;
}

# if defined __UCLIBC_LINUX_SPECIFIC__ && defined __USE_GNU
strong_alias(__libc_fallocate64,fallocate64)
# endif
#endif
