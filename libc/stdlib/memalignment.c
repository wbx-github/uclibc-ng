/*
 * memalignment() for uClibc (ISO C23)
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdlib.h>
#include <stdint.h>

/* The largest power of two dividing the address; 0 for a null pointer.  */
size_t memalignment(const void *p)
{
	uintptr_t a = (uintptr_t) p;

	return a & -a;
}
