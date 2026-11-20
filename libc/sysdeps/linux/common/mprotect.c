/*
 * mprotect() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/mman.h>

#if defined __NR_mprotect
_syscall3(int, mprotect, void *, addr, size_t, len, int, prot)
#else
/* asm-generic leaves the number out on __ARCH_NOMMU (h8300); every no-MMU
   kernel answers ENOSYS anyway, which this does without the call.  */
#include <errno.h>
int mprotect(void *addr, size_t len, int prot)
{
	(void) addr;
	(void) len;
	(void) prot;
	__set_errno(ENOSYS);
	return -1;
}
#endif
