/*
 * msync() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#include <sys/mman.h>
#include <cancel.h>

#if defined __NR_msync && defined __ARCH_USE_MMU__
# define __NR___msync_nocancel __NR_msync
static _syscall3(int, __NC(msync), void *, addr, size_t, length, int, flags)
#else
/* Without an MMU a shared mapping is the page cache itself, so there is
   nothing to write back; the kernel has no msync then.  msync is still a
   cancellation point.  */
static int __NC(msync)(void *addr, size_t length, int flags)
{
	(void) addr;
	(void) length;
	(void) flags;
	return 0;
}
#endif

CANCELLABLE_SYSCALL(int, msync, (void *addr, size_t length, int flags),
		    (addr, length, flags))
