/*
 * Copyright (C) 2026 Ramin Moussavi <ramin.moussavi@yacoub.de>
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#ifndef	_SYS_SIGNALFD_H
# error "Never use <bits/signalfd.h> directly; include <sys/signalfd.h> instead."
#endif

/* Flags for signalfd.  */
enum
  {
    SFD_CLOEXEC = 010000000,
#define SFD_CLOEXEC SFD_CLOEXEC
    SFD_NONBLOCK = 00200004
#define SFD_NONBLOCK SFD_NONBLOCK
  };
