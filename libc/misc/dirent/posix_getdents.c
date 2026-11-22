/*
 * posix_getdents, POSIX.1-2024.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#ifdef __NR_getdents64

/* What the kernel writes, which is struct posix_dent as programs see it;
   inside libc ino_t may be 32 bits wide.  */
struct kernel_dirent64 {
	uint64_t d_ino;
	int64_t d_off;
	unsigned short d_reclen;
	unsigned char d_type;
	char d_name[];
};

ssize_t posix_getdents(int fd, void *buf, size_t nbyte, int flags)
{
	ssize_t n, i;

	if (flags & ~DT_FORCE_TYPE) {
		__set_errno(EINVAL);
		return -1;
	}
	if (nbyte > INT_MAX)
		nbyte = INT_MAX;
	n = INLINE_SYSCALL(getdents64, 3, fd, buf, nbyte);
	if (n <= 0 || !(flags & DT_FORCE_TYPE))
		return n;

	/* Some file systems do not know the type; ask stat.  */
	for (i = 0; i < n; ) {
		struct kernel_dirent64 *d = (void *) ((char *) buf + i);
		struct stat64 st;

		if (d->d_type == DT_UNKNOWN) {
			int saved = errno;

			/* The entries are read; losing them is worse.  */
			if (fstatat64(fd, d->d_name, &st, AT_SYMLINK_NOFOLLOW) == 0)
				d->d_type = IFTODT(st.st_mode);
			else
				__set_errno(saved);
		}
		i += d->d_reclen;
	}
	return n;
}

#endif
