/*
 * The generation counter of pthread_once, advanced by fork in the child.
 * Defined on its own: init.c registers it at startup, and in
 * pthread_once.c it brought pthread_once and the unwinder into every
 * program.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <fork.h>

unsigned long int __fork_generation attribute_hidden = 0;
