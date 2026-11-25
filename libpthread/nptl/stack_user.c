/*
 * The list of thread descriptors whose stacks the program provided,
 * the main thread's among them.  Defined on its own: init.c puts the
 * main thread on it, and in allocatestack.c, which pthread_create.c
 * includes, it brought pthread_create into every program.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <pthreadP.h>

list_t __stack_user __attribute__ ((nocommon));
hidden_data_def (__stack_user)
