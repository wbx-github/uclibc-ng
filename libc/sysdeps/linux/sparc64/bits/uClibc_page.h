/*
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Supply an architecture specific value for PAGE_SIZE and friends.  */

#ifndef _UCLIBC_PAGE_H
#define _UCLIBC_PAGE_H

/* PAGE_SHIFT determines the page size -- in this case 8192 */
#define PAGE_SHIFT	13

#define MMAP2_PAGE_SHIFT PAGE_SHIFT

#endif /* _UCLIBC_PAGE_H */
