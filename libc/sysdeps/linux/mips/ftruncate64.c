/*
 * ftruncate64() for MIPS uClibc
 *
 * Copyright (C) 2026 Ramin Moussavi <ramin.moussavi@yacoub.de>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <_lfs_64.h>
#include <sys/syscall.h>
#include <sgidefs.h>

#if _MIPS_SIM == _MIPS_SIM_NABI32 && defined __NR_ftruncate

# include <unistd.h>

/* n32 has no ftruncate64 and a 32-bit off_t, so the generic ftruncate64()
 * truncates the length to off_t and rejects with EFBIG anything that does not
 * fit.  But the n32 ftruncate syscall takes a full 64-bit length, so pass it
 * straight through.  */
int ftruncate64(int fd, __off64_t length)
{
	return INLINE_SYSCALL(ftruncate, 2, fd, length);
}
libc_hidden_def(ftruncate64)

#else
# include "../common/ftruncate64.c"
#endif
