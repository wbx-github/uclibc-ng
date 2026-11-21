/* Copyright (C) 2004, 2005 Free Software Foundation, Inc.
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

#ifndef _NETLINKACCESS_H
#define _NETLINKACCESS_H 1

#include <features.h>
#include <stdint.h>
#include <unistd.h>
#if defined __ASSUME_NETLINK_SUPPORT || defined __UCLIBC_USE_NETLINK__
#include <asm/types.h>
/* Older kernel headers (e.g. Linux 2.6.26) only hide the userspace copies
   of struct sockaddr, struct iovec, struct ucred, the CMSG macros etc. in
   <linux/socket.h> if __GLIBC__ >= 2.  features.h does not define __GLIBC__
   while building libc itself, so pretend to be glibc for these includes.  */
#ifndef __GLIBC__
# define __GLIBC__ 2
# define __UCLIBC_NETLINK_FAKE_GLIBC
#endif
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#ifdef __UCLIBC_NETLINK_FAKE_GLIBC
# undef __GLIBC__
# undef __UCLIBC_NETLINK_FAKE_GLIBC
#endif

struct netlink_res
{
  struct netlink_res *next;
  struct nlmsghdr *nlh;
  size_t size;			/* Size of response.  */
  uint32_t seq;			/* sequential number we used.  */
};


struct netlink_handle
{
  int fd;			/* Netlink file descriptor.  */
  pid_t pid;			/* Process ID.  */
  uint32_t seq;			/* The sequence number we use currently.  */
  struct netlink_res *nlm_list;	/* Pointer to list of responses.  */
  struct netlink_res *end_ptr;	/* For faster append of new entries.  */
};


#ifndef __ASSUME_NETLINK_SUPPORT
#define __ASSUME_NETLINK_SUPPORT 1
#endif

extern int __netlink_open (struct netlink_handle *h) attribute_hidden;
extern void __netlink_close (struct netlink_handle *h) attribute_hidden;
extern void __netlink_free_handle (struct netlink_handle *h) attribute_hidden;
extern int __netlink_request (struct netlink_handle *h, int type) attribute_hidden;

#else
#define __ASSUME_NETLINK_SUPPORT 0
#endif

#endif /* _NETLINKACCESS_H */
