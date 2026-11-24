/* Copyright (C) 1991, 1996, 1997, 2000-2002, 2003, 2004
   Free Software Foundation, Inc.
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

#include <stddef.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <bits/wordsize.h>


#define CS_PATH "/bin:/usr/bin"

/* If BUF is not NULL and LEN > 0, fill in at most LEN - 1 bytes
   of BUF with the value corresponding to NAME and zero-terminate BUF.
   Return the number of bytes required to hold NAME's entire value.  */
size_t confstr (int name, char *buf, size_t len)
{
  const char *string;
  size_t string_len;

  switch (name)
    {
    case _CS_PATH:
      {
	static const char cs_path[] = CS_PATH;
	string = cs_path;
	string_len = sizeof (cs_path);
      }
      break;
    /* The one environment the toolchain builds for, with no extra
       flags; off_t is always 64 bits wide.  */
    case _CS_V5_WIDTH_RESTRICTED_ENVS:
    case _CS_V6_WIDTH_RESTRICTED_ENVS:
    case _CS_V7_WIDTH_RESTRICTED_ENVS:
      {
#if __WORDSIZE == 64
# define ENV_NAME "LP64_OFF64"
#else
# define ENV_NAME "ILP32_OFFBIG"
#endif
	static const char v5[] = "XBS5_" ENV_NAME;
	static const char v6[] = "POSIX_V6_" ENV_NAME;
	static const char v7[] = "POSIX_V7_" ENV_NAME;
	string = name == _CS_V5_WIDTH_RESTRICTED_ENVS ? v5
		 : name == _CS_V6_WIDTH_RESTRICTED_ENVS ? v6 : v7;
	string_len = strlen (string) + 1;
      }
      break;
    /* Two ranges, not one: 1008..1099 between _CS_LFS64_LINTFLAGS and
       _CS_XBS5_ILP32_OFF32_CFLAGS is unassigned, and an unassigned name
       has to give EINVAL rather than an empty string.  */
    case _CS_LFS_CFLAGS ... _CS_LFS64_LINTFLAGS:
    case _CS_XBS5_ILP32_OFF32_CFLAGS ... _CS_POSIX_V7_LPBIG_OFFBIG_LINTFLAGS:
    case _CS_V6_ENV:
    case _CS_V7_ENV:
    case _CS_POSIX_V7_THREADS_CFLAGS:
      string = "";
      string_len = 1;
      break;
    case _CS_POSIX_V7_THREADS_LDFLAGS:
#ifdef __UCLIBC_HAS_THREADS__
      string = "-lpthread";
#else
      string = "";
#endif
      string_len = strlen (string) + 1;
      break;
#ifdef __UCLIBC_HAS_THREADS__
    case _CS_GNU_LIBPTHREAD_VERSION:
# if defined __UCLIBC_HAS_LINUXTHREADS__
      string = "linuxthreads-0.01";
      string_len = sizeof("linuxthreads-x.xx");
# elif defined __UCLIBC_HAS_THREADS_NATIVE__
#  define __NPTL_VERSION ("NPTL " \
		__stringify(__UCLIBC_MAJOR__) "." \
		__stringify(__UCLIBC_MINOR__) "." \
		__stringify(__UCLIBC_SUBLEVEL__))
      string = __NPTL_VERSION;
      string_len = sizeof(__NPTL_VERSION);
# else
#  error unable to determine thread impl
# endif
      break;
#endif
    default:
      __set_errno (EINVAL);
      return 0;
    }

  if (len > 0 && buf != NULL)
    {
      if (string_len <= len)
	memcpy (buf, string, string_len);
      else
	{
	  memcpy (buf, string, len - 1);
	  buf[len - 1] = '\0';
	}
    }
  return string_len;
}
