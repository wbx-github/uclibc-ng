/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/*
 * Manuel Novoa III       Dec 2000
 *
 * Converted to use my new (un)signed long (long) to string routines, which
 * are smaller than the previous functions and don't require static buffers.
 * In the process, removed the reference to strcat and cut object size of
 * inet_ntoa in half (from 190 bytes down to 94).
 *
 * Manuel Novoa III       Feb 2002
 *
 * Changed to use _int10tostr.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bits/uClibc_uintmaxtostr.h>

#ifdef L_inet_aton
/*
 * The forms POSIX gives for inet_addr:
 *
 * a.b.c.d    - {a,b,c,d} -> 1 byte
 * a.b.c      - {a,b}     -> 1 byte    {c} -> 2 bytes
 * a.b        - {a}       -> 1 byte    {b} -> 3 bytes
 * a          -                        {a} -> 4 bytes
 *
 * Each part may be decimal, octal, or hexadecimal as in ISO C.
 * 0x or 0X    -> hexadecimal
 * leading 0   -> octal
 * all else    -> decimal
 */
int inet_aton(const char *cp, struct in_addr *addrptr)
{
	unsigned long part[4];
	in_addr_t addr;
	char *end;
	int n, saved_errno = errno;

	if (cp == NULL) {
		return 0;
	}

	__set_errno(0);
	for (n = 0; ; n++) {
		/* strtoul would also take a sign or leading space */
		if (!isdigit(*cp))
			goto fail;
		part[n] = strtoul(cp, &end, 0);
		cp = end;
		if (*cp != '.' || n == 3)
			break;
		cp++;
	}
	/* on 32 bit an overflow would read as 255.255.255.255 */
	if (errno == ERANGE || (*cp != '\0' && !isspace(*cp)))
		goto fail;
	__set_errno(saved_errno);

	/* the last part fills the bytes the others leave */
	if (part[n] > (0xffffffffUL >> (8 * n)))
		return 0;
	addr = part[n];
	while (n--) {
		if (part[n] > 0xff)
			return 0;
		addr |= (in_addr_t) part[n] << (8 * (3 - n));
	}

	/*  W. Richard Stevens in his book UNIX Network Programming,
	 *  Volume 1, second edition, on page 71 says:
	 *
	 *  An undocumented feature of inet_aton is that if addrptr is
	 *  a null pointer, the function still performs it validation
	 *  of the input string, but does not store the result.
	 */
	if (addrptr) {
		addrptr->s_addr = htonl(addr);
	}

	return 1;

fail:
	__set_errno(saved_errno);
	return 0;
}
libc_hidden_def(inet_aton)
#endif

#ifdef L_inet_addr

in_addr_t inet_addr(const char *cp)
{
	struct in_addr a;

	if (!inet_aton(cp, &a))
		return INADDR_NONE;
	else
		return a.s_addr;
}
libc_hidden_def(inet_addr)
#endif

#ifdef L_inet_ntoa

#define INET_NTOA_MAX_LEN	16	/* max 12 digits + 3 '.'s + 1 nul */

static char *__inet_ntoa_r(struct in_addr in, char buf[INET_NTOA_MAX_LEN])
{
	in_addr_t addr = ntohl(in.s_addr);
	int i;
	char *p, *q;

	q = 0;
	p = buf + INET_NTOA_MAX_LEN - 1; /* cannot use sizeof(buf) here */
	for (i = 0; i < 4; i++ ) {
		p = _int10tostr(p, addr & 0xff) - 1;
		addr >>= 8;
		if (q) {
			*q = '.';
		}
		q = p;
	}

	return p+1;
}
strong_alias(__inet_ntoa_r,inet_ntoa_r)

char *inet_ntoa(struct in_addr in)
{
	static char buf[INET_NTOA_MAX_LEN];
	return __inet_ntoa_r(in, buf);
}
libc_hidden_def(inet_ntoa)
#endif

#ifdef L_inet_makeaddr

/* for some reason it does not remove the jump relocation */

/*
 * Formulate an Internet address from network + host.  Used in
 * building addresses stored in the ifnet structure.
 */
struct in_addr inet_makeaddr(in_addr_t net, in_addr_t host)
{
	struct in_addr in;

	if (net < 128)
		in.s_addr = (net << IN_CLASSA_NSHIFT) | (host & IN_CLASSA_HOST);
	else if (net < 65536)
		in.s_addr = (net << IN_CLASSB_NSHIFT) | (host & IN_CLASSB_HOST);
	else if (net < 16777216UL)
		in.s_addr = (net << IN_CLASSC_NSHIFT) | (host & IN_CLASSC_HOST);
	else
		in.s_addr = net | host;
	in.s_addr = htonl(in.s_addr);
	return in;
}
libc_hidden_def(inet_makeaddr)
#endif

#ifdef L_inet_lnaof
/*
 * Return the local network address portion of an
 * internet address; handles class a/b/c network
 * number formats.
 */
in_addr_t inet_lnaof(struct in_addr in)
{
	in_addr_t i = ntohl(in.s_addr);

	if (IN_CLASSA(i))
		return (i & IN_CLASSA_HOST);
	else if (IN_CLASSB(i))
		return (i & IN_CLASSB_HOST);
	else
		return (i & IN_CLASSC_HOST);
}
#endif

#ifdef L_inet_netof

/*
 * Return the network number from an internet
 * address; handles class a/b/c network #'s.
 */
in_addr_t
inet_netof(struct in_addr in)
{
	in_addr_t i = ntohl(in.s_addr);

	if (IN_CLASSA(i))
		return ((i & IN_CLASSA_NET) >> IN_CLASSA_NSHIFT);
	else if (IN_CLASSB(i))
		return ((i & IN_CLASSB_NET) >> IN_CLASSB_NSHIFT);
	else
		return ((i & IN_CLASSC_NET) >> IN_CLASSC_NSHIFT);
}
libc_hidden_def(inet_netof)
#endif
