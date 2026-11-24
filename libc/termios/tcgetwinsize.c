/*
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <termios.h>
#include <sys/ioctl.h>

int tcgetwinsize(int fd, struct winsize *wsp)
{
	return ioctl(fd, TIOCGWINSZ, wsp);
}
