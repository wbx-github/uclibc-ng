/*
 * truncate64() for MIPS uClibc
 *
 * Copyright (C) 2026 Ramin Moussavi <ramin.moussavi@yacoub.de>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <_lfs_64.h>
#include <sys/syscall.h>
#include <sgidefs.h>

#if _MIPS_SIM == _MIPS_SIM_NABI32 && defined __NR_truncate

# include <unistd.h>

/* Same as ftruncate64(): n32 has no truncate64, but its truncate syscall
 * takes a full 64-bit length.  */
int truncate64(const char *path, __off64_t length)
{
	return INLINE_SYSCALL(truncate, 2, path, length);
}
libc_hidden_def(truncate64)

#else
# include "../common/truncate64.c"
#endif
