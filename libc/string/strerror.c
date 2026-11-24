/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>
#include <string.h>
#include "_syserrmsg.h"


/* The messages are constants, so every error number the library knows
   comes back as it stands and two threads cannot get in each other's
   way.  Only "Unknown error N" has to be built, and C and POSIX both
   allow that one to be overwritten by a later call.  glibc and musl
   answer from the same constants.  */
char *strerror(int errnum)
{
    static char buf[_STRERROR_BUFSIZE];
	const char *s;

	s = __strerror_const(errnum);
	if (s)
		return (char *) s;

	__xpg_strerror_r(errnum, buf, sizeof(buf));

	return buf;
}
libc_hidden_def(strerror)
