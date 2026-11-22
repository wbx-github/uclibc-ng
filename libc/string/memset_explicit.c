/*
 * memset_explicit() for uClibc (ISO C23)
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <string.h>

/* The empty asm takes the buffer as an input that reads memory, so the
   stores cannot be dropped as dead, as explicit_bzero does it.  */
void *memset_explicit(void *s, int c, size_t n)
{
	s = memset(s, c, n);
	__asm__ __volatile__ ("" : : "r"(s) : "memory");
	return s;
}
