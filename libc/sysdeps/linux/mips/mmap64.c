/*
 * mmap64() for MIPS uClibc
 *
 * Copyright (C) 2026 Ramin Moussavi <ramin.moussavi@yacoub.de>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <_lfs_64.h>
#include <sys/syscall.h>
#include <sgidefs.h>

#if _MIPS_SIM == _MIPS_SIM_NABI32 && defined __NR_mmap

# include <errno.h>
# include <sys/mman.h>

/* n32 has no mmap2 and a 32-bit off_t, so the generic mmap64() (which either
 * needs mmap2 or truncates the offset to off_t and rejects anything that does
 * not fit) cannot express a >2GB offset.  But the n32 mmap syscall takes a
 * full 64-bit byte offset, so pass it straight through.  */
void *mmap64(void *addr, size_t len, int prot, int flags, int fd,
	     __off64_t offset)
{
	return (void *) INLINE_SYSCALL(mmap, 6, addr, len, prot, flags, fd,
				       offset);
}

#else
# include "../common/mmap64.c"
#endif
