/*
 * posix_typed_mem_open, posix_typed_mem_get_info and posix_mem_offset,
 * POSIX.1-2008 (TYM).
 *
 * Linux has no typed memory objects, so no name names one, no
 * descriptor refers to one and no mapping comes from one.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

int posix_typed_mem_open(const char *name, int oflag, int tflag)
{
	(void) name;
	(void) oflag;
	/* At most one of the three flags; none of them is allowed too.  */
	if (tflag != 0
	    && tflag != POSIX_TYPED_MEM_ALLOCATE
	    && tflag != POSIX_TYPED_MEM_ALLOCATE_CONTIG
	    && tflag != POSIX_TYPED_MEM_MAP_ALLOCATABLE)
		__set_errno(EINVAL);
	else
		__set_errno(ENOENT);
	return -1;
}

int posix_typed_mem_get_info(int fildes, struct posix_typed_mem_info *info)
{
	(void) info;
	return fcntl(fildes, F_GETFD) < 0 ? EBADF : ENODEV;
}

int posix_mem_offset(const void *__restrict addr, size_t len,
		     off_t *__restrict off, size_t *__restrict contig_len,
		     int *__restrict fildes)
{
	(void) addr;
	(void) len;
	(void) off;
	(void) contig_len;
	(void) fildes;
	return EACCES;
}
