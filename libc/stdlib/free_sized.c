/*
 * free_sized() and free_aligned_sized() for uClibc (ISO C23)
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdlib.h>

/* The sizes are a hint the allocator may use; none of uClibc's does.  */
void free_sized(void *p, size_t size)
{
	(void) size;
	free(p);
}

void free_aligned_sized(void *p, size_t alignment, size_t size)
{
	(void) alignment;
	(void) size;
	free(p);
}
