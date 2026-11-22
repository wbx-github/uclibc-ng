/*
 * posix_devctl, POSIX.1-2024.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <devctl.h>
#include <errno.h>
#include <sys/ioctl.h>

int posix_devctl(int fd, int dcmd, void *__restrict dev_data_ptr,
		 size_t nbyte, int *__restrict dev_info_ptr)
{
	int saved = errno, r;

	(void) nbyte;
	/* Linux request numbers use all 32 bits; do not sign-extend.  */
	r = ioctl(fd, (unsigned int) dcmd, dev_data_ptr);
	if (r == -1) {
		r = errno;
		__set_errno(saved);
		return r;
	}
	if (dev_info_ptr)
		*dev_info_ptr = r;
	return 0;
}
