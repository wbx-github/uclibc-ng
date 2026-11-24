/*
 * posix_close() for uClibc
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <errno.h>
#include <unistd.h>
#include <cancel.h>

/* Linux releases the descriptor even when close is interrupted, so
   there is nothing to restart: POSIX_CLOSE_RESTART is 0, and EINTR
   becomes EINPROGRESS, which POSIX gives for "interrupted, but
   closed".  */
int posix_close(int fd, int flag)
{
	int ret;

	if (flag != 0) {
		__set_errno(EINVAL);
		return -1;
	}
	ret = close(fd);
	if (ret == -1 && errno == EINTR)
		__set_errno(EINPROGRESS);
	return ret;
}
/* close handled cancellation, noop on uClibc */
LIBC_CANCEL_HANDLED();
