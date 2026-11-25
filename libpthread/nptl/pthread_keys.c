/*
 * Table of the key information, 16 KB with PTHREAD_KEYS_MAX keys.
 * Defined on its own: in vars.c it went into every program, since the
 * thread setup at startup uses the other variables there.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <pthreadP.h>

struct pthread_key_struct __pthread_keys[PTHREAD_KEYS_MAX]
  __attribute__ ((nocommon));
hidden_data_def (__pthread_keys)
