/* Provide kernel hint to read ahead.
   Copyright (C) 2002, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sys/syscall.h>

#if defined __NR_readahead && defined __USE_GNU

# include <fcntl.h>
# include <bits/wordsize.h>

/* __WORDSIZE answers how wide a long is, not how wide an argument register
   is.  On an ILP32 ABI with 64-bit registers the two differ, and there the
   kernel takes the offset whole -- splitting it would shift the length into
   the register the kernel reads the offset from.  */
# if __WORDSIZE == 64 || defined __UCLIBC_SYSCALL_ARG_64BIT__

_syscall3(ssize_t, readahead, int, fd, __off64_t, offset, size_t, count)

# else

ssize_t readahead(int fd, off64_t offset, size_t count)
{
	return INLINE_SYSCALL(readahead,
#  if defined(__UCLIBC_SYSCALL_ALIGN_64BIT__)
		5, fd, 0,
#  else
		4, fd,
#  endif
		OFF64_HI_LO(offset), count);
}

# endif

#endif
