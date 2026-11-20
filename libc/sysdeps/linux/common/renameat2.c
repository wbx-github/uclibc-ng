/*
 * renameat2() for uClibc-ng
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdio.h>
#include <fcntl.h>
#include <sysdep.h>
#include <errno.h>

int
renameat2 (int oldfd, const char *old, int newfd, const char *new, unsigned int flags)
{
#ifdef __NR_renameat2
  return INLINE_SYSCALL (renameat2, 5, oldfd, old, newfd, new, flags);
#else
  __set_errno(ENOSYS);
  return -1;
#endif
}
libc_hidden_def (renameat2)
