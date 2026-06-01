/*
 * Copyright (C) 2026 Ramin Moussavi <ramin.moussavi@yacoub.de>
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#ifndef	_SYS_TIMERFD_H
# error "Never use <bits/timerfd.h> directly; include <sys/timerfd.h> instead."
#endif

/* Bits to be set in the FLAGS parameter of `timerfd_create'.  */
enum
  {
    TFD_CLOEXEC = 010000000,
#define TFD_CLOEXEC TFD_CLOEXEC
    TFD_NONBLOCK = 00200004
#define TFD_NONBLOCK TFD_NONBLOCK
  };
