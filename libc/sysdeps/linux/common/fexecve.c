/* Copyright (C) 1994-2019 Free Software Foundation, Inc.

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
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/syscall.h>

/* "/proc/self/fd/" and the decimal fd, without snprintf: fexecve is
   async-signal-safe.  */
static char *
fd_to_filename (int fd, char *buf)
{
  static const char prefix[] = "/proc/self/fd/";
  char digits[3 * sizeof (int)];
  char *p = buf;
  int n = 0;

  memcpy (p, prefix, sizeof prefix - 1);
  p += sizeof prefix - 1;
  do
    digits[n++] = '0' + fd % 10;
  while ((fd /= 10) != 0);
  while (n > 0)
    *p++ = digits[--n];
  *p = '\0';
  return buf;
}

int
fexecve (int fd, char *const argv[], char *const envp[])
{
  if (fd < 0 || argv == NULL || envp == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

#ifdef __NR_execveat
  /* Works without /proc and from Linux 3.19 on.  */
  INLINE_SYSCALL (execveat, 5, fd, "", &argv[0], &envp[0], AT_EMPTY_PATH);
  if (errno != ENOSYS)
    return -1;
#endif

  /* We use the /proc filesystem to get the information.  If it is not
     mounted we fail.  */
  char buf[sizeof "/proc/self/fd/" + sizeof (int) * 3];

  /* We do not need the return value.  */
  execve (fd_to_filename (fd, buf), argv, envp);

  int save = errno;

  /* We come here only if the 'execve' call fails.  Determine whether
     /proc is mounted.  If not we return ENOSYS.  */
  struct stat st;
  if (stat ("/proc/self/fd", &st) != 0 && errno == ENOENT)
    save = ENOSYS;

  __set_errno (save);

  return -1;
}
libc_hidden_def(fexecve)
