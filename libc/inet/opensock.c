/* Copyright (C) 1999, 2001, 2002 Free Software Foundation, Inc.
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

#include <sys/socket.h>
#include <bits/kernel-features.h>

/* Return a socket of any type.  The socket can be used in subsequent
   ioctl calls to talk to the kernel.

   The order matters, and it is glibc's: a family the kernel was built
   without costs a round trip through the usermode helper, because
   socket() makes the kernel call request_module("net-pf-N") and wait for
   /sbin/modprobe to fail before it returns EAFNOSUPPORT.  AF_UNIX is
   therefore first -- it is present wherever anything else works -- and
   AF_INET6 last, since a kernel without IPv6 is the common case.  The
   interface ioctls these sockets exist for do not care which family it
   is: they are handled by the socket layer, not by the protocol.  */
int __opensock(void) attribute_hidden;
int
__opensock(void)
{
	int fd = socket(AF_UNIX, SOCK_DGRAM, 0);

#ifdef __UCLIBC_HAS_IPV4__
	if (fd < 0)
		fd = socket(AF_INET, SOCK_DGRAM, 0);
#endif
#ifdef __UCLIBC_HAS_IPV6__
	if (fd < 0)
		fd = socket(AF_INET6, SOCK_DGRAM, 0);
#endif
	return fd;
}
