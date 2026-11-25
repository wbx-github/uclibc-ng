/*
 * pipe2() for uClibc
 *
 * Copyright (C) 2011 Bernhard Reutner-Fischer <uclibc@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#ifdef __NR_pipe2
int pipe2(int filedes[2], int flags)
{
	return INLINE_SYSCALL(pipe2, 2, filedes, flags);
}
libc_hidden_def(pipe2)
#endif
