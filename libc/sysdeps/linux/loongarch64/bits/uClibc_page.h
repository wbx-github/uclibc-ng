/*  Copyright (C) 2024 uClibc-ng contributors.
 *
 *  Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Supply an architecture specific value for PAGE_SIZE and friends.  */

#ifndef _UCLIBC_PAGE_H
#define _UCLIBC_PAGE_H

/* LoongArch supports 4K, 16K, and 64K pages; default is 16K */
#define PAGE_SHIFT	14

/* No PAGE_SIZE for programs: it would be wrong on 4K/64K kernels.  */
#define __UCLIBC_PAGE_SIZE_VARIES__ 1

#define MMAP2_PAGE_SHIFT PAGE_SHIFT

#endif /* _UCLIBC_PAGE_H */