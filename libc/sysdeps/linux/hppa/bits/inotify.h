/*
 * Copyright (C) 2026 Ramin Moussavi <ramin.moussavi@yacoub.de>
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#ifndef	_SYS_INOTIFY_H
# error "Never use <bits/inotify.h> directly; include <sys/inotify.h> instead."
#endif

/* Flags for the parameter of inotify_init1.  */
enum
  {
    IN_CLOEXEC = 010000000,
#define IN_CLOEXEC IN_CLOEXEC
    IN_NONBLOCK = 00200004
#define IN_NONBLOCK IN_NONBLOCK
  };
