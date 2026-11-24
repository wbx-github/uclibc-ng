/*
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <termios.h>
#include <sys/ioctl.h>

int tcsetwinsize(int fd, const struct winsize *wsp)
{
	return ioctl(fd, TIOCSWINSZ, wsp);
}
