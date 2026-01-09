/* Copyright (C) 2025 Waldemar Brodkorb <wbx@uclibc-ng.org> */

#include <unistd.h>
#include <tls.h>
#include <sysdep.h>

pid_t
gettid (void)
{
  INTERNAL_SYSCALL_DECL (err);
  pid_t result = INTERNAL_SYSCALL (gettid, err, 0);
  return result;
}
