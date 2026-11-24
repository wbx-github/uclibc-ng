/* struct winsize, for <sys/ioctl.h> and <termios.h>.
   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#ifndef _BITS_WINSIZE_H
#define _BITS_WINSIZE_H	1

struct winsize
  {
    unsigned short int ws_row;
    unsigned short int ws_col;
    unsigned short int ws_xpixel;
    unsigned short int ws_ypixel;
  };

#endif
