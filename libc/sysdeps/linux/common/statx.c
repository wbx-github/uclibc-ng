/* Linux statx implementation. modified for uClibc-ng.
   Copyright (C) 2018-2026 Free Software Foundation, Inc.

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
   <https://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sys/stat.h>
#include <sysdep.h>

#if defined(__NR_statx)
int
statx (int fd, const char *path, int flags,
       unsigned int mask, struct statx *buf)
{
  int ret = INLINE_SYSCALL (statx, 5, fd, path, flags, mask, buf);
  return ret;
}
#endif
