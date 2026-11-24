/*
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _DL_OSINFO_H
#define _DL_OSINFO_H 1

#include <features.h>

#ifdef __UCLIBC_HAS_SSP__
# if defined IS_IN_libc || defined IS_IN_rtld

#  if defined __SSP__ || defined __SSP_ALL__
#   error "file must not be compiled with stack protection enabled on it. Use -fno-stack-protector"
#  endif

#  include <stdint.h>

#  ifdef IS_IN_libc
#   include <fcntl.h>
#   include <unistd.h>
#   include <string.h>
#   include <sysdep.h>
/* Raw system calls: this runs before main, and the wrappers are
   cancellation points that bring their machinery along.  */
#   ifdef __NR_open
#    define OPEN(path, flags, mode) INLINE_SYSCALL (open, 3, path, flags, mode)
#   else
#    define OPEN(path, flags, mode) \
	INLINE_SYSCALL (openat, 4, AT_FDCWD, path, flags, mode)
#   endif
#   define READ(fd, buf, n) INLINE_SYSCALL (read, 3, fd, buf, n)
#   define CLOSE(fd) INLINE_SYSCALL (close, 1, fd)
#   define MEMCPY memcpy
#  else
#   include <dl-string.h>
#   define OPEN _dl_open
#   define READ _dl_read
#   define CLOSE _dl_close
#   define MEMCPY _dl_memcpy
#  endif

static __always_inline uintptr_t
_dl_setup_stack_chk_guard(void *dl_random)
{
	/* Fallback just the "terminator canary". */
	uintptr_t ret = 0xFF0A0D00UL;

	/*
	 * Linux supplies random data through AT_RANDOM.
	 * Use it directly when available.
	 */
	if (dl_random != NULL) {
		MEMCPY(&ret, dl_random, sizeof(ret));
	} else {
		int fd = OPEN("/dev/urandom", O_RDONLY, 0);
		if (fd < 0)
			goto out;
		uintptr_t tmp;
		size_t size = READ(fd, &tmp, sizeof(tmp));
		CLOSE(fd);
		if (size != sizeof(tmp))
			goto out;
		ret = tmp;
	}
out:

/* Make it harder to leak the canary by ensuring
 * that the byte with the lowest address is a zero
 * byte that will stop a rogue strcpy, printf %s, etc*/
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret &= ~(uintptr_t)0xff;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret &= ~((uintptr_t)0xff << (8 * (sizeof(ret) - 1)));
#else
# error "Unknown byte order"
#endif

	return ret;
}
# endif /* libc || rtld */
#endif /* __UCLIBC_HAS_SSP__ */

#endif /* _DL_OSINFO_H */
