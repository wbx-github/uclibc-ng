/*
 * The generation counter of pthread_once, which fork advances in the
 * child through __fork_generation_pointer, and the stack reclaimer fork
 * calls there.  Defined on their own: init.c registers the counter at
 * startup and pthread_create sets the reclaimer; in pthread_once.c and
 * fork.c they brought those into every program.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <fork.h>

unsigned long int __fork_generation attribute_hidden = 0;

#ifdef __ARCH_USE_MMU__
unsigned long int *__fork_generation_pointer attribute_hidden = NULL;
void (*__fork_reclaim) (void) attribute_hidden = NULL;
#endif
