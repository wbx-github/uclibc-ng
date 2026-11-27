/* Copyright (C) 1995,1996,1997,2000,2002,2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _SYS_SHM_H
# error "Never include <bits/shm.h> directly; use <sys/shm.h> instead."
#endif

#include <bits/types.h>

/* Permission flag for shmget.  */
#define SHM_R		0400		/* or S_IRUGO from <linux/stat.h> */
#define SHM_W		0200		/* or S_IWUGO from <linux/stat.h> */

/* Flags for `shmat'.  */
#define SHM_RDONLY	010000		/* attach read-only else read-write */
#define SHM_RND		020000		/* round attach address to SHMLBA */
#define SHM_REMAP	040000		/* take-over region on attach */

/* Commands for `shmctl'.  */
#define SHM_LOCK	11		/* lock segment (root only) */
#define SHM_UNLOCK	12		/* unlock segment (root only) */

__BEGIN_DECLS

/* Segment low boundary address multiple.  */
#define SHMLBA		(__getpagesize ())
extern int __getpagesize (void) __THROW __attribute__ ((__const__));


/* Type to count number of attaches.  */
typedef unsigned long int shmatt_t;

/* Data structure describing a set of semaphores.  */
/* parisc places the upper half of each 64-bit time value BEFORE the word that
   holds the value itself -- see arch/parisc/include/uapi/asm/{msgbuf,sembuf,
   shmbuf}.h.  The generic layout in libc/sysdeps/linux/common/bits has it the
   other way round, which shifted every time field by one word: msgctl() and
   semctl() returned the always-zero upper half, so uClibc reported 1970.

   The structure is spelled out once per configuration rather than stitched
   together with conditionals inside it.  With a 32-bit time_t the field sits
   where the kernel's lower word is and the upper one is padding; with a 64-bit
   time_t the kernel's two words are kept as the pair it writes, _internal_1
   being the lower half, and msgctl()/semctl()/shmctl() compose the value
   behind the end of the kernel's structure as
   "_internal_1 | _internal_2 << 32".  */

#if (__WORDSIZE == 32 && defined(__UCLIBC_USE_TIME64__))

struct shmid_ds
  {
    struct ipc_perm shm_perm;		/* operation permission struct */
    unsigned long int __shm_atime_internal_2;	/* time of last shmat() */
    unsigned long int __shm_atime_internal_1;
    unsigned long int __shm_dtime_internal_2;	/* time of last shmdt() */
    unsigned long int __shm_dtime_internal_1;
    unsigned long int __shm_ctime_internal_2;	/* time of last change by shmctl() */
    unsigned long int __shm_ctime_internal_1;
    unsigned int __uclibc_pad4;		/* the kernel's __pad4, parisc only */
    size_t shm_segsz;			/* size of segment in bytes */
    __pid_t shm_cpid;			/* pid of creator */
    __pid_t shm_lpid;			/* pid of last shmop */
    shmatt_t shm_nattch;		/* number of current attaches */
    unsigned long int __uclibc_unused4;
    unsigned long int __uclibc_unused5;
    /* The kernel fills only the pairs above, and its structure ends exactly
       here.  shmctl() composes the __time_t fields below from them once the
       call has returned.  */
    __time_t shm_atime;
    __time_t shm_dtime;
    __time_t shm_ctime;
  };

# define __SHMID_DS_TIME64_SPLIT 1

#else

struct shmid_ds
  {
    struct ipc_perm shm_perm;		/* operation permission struct */
    unsigned int __uclibc_pad1;
    __time_t shm_atime;			/* time of last shmat() */
    unsigned int __uclibc_pad2;
    __time_t shm_dtime;			/* time of last shmdt() */
    unsigned int __uclibc_pad3;
    __time_t shm_ctime;			/* time of last change by shmctl() */
    unsigned int __uclibc_pad4;		/* the kernel's __pad4, parisc only */
    size_t shm_segsz;			/* size of segment in bytes */
    __pid_t shm_cpid;			/* pid of creator */
    __pid_t shm_lpid;			/* pid of last shmop */
    shmatt_t shm_nattch;		/* number of current attaches */
    unsigned long int __uclibc_unused4;
    unsigned long int __uclibc_unused5;
  };

#endif

#ifdef __USE_MISC

/* ipcs ctl commands */
# define SHM_STAT	13
# define SHM_INFO	14
# define SHM_STAT_ANY	15

/* shm_mode upper byte flags */
# define SHM_DEST	01000	/* segment will be destroyed on last detach */
# define SHM_LOCKED	02000   /* segment will not be swapped */
# define SHM_HUGETLB	04000	/* segment is mapped via hugetlb */
# define SHM_NORESERVE	010000	/* don't check for reservations */

struct	shminfo
  {
    unsigned long int shmmax;
    unsigned long int shmmin;
    unsigned long int shmmni;
    unsigned long int shmseg;
    unsigned long int shmall;
    unsigned long int __uclibc_unused1;
    unsigned long int __uclibc_unused2;
    unsigned long int __uclibc_unused3;
    unsigned long int __uclibc_unused4;
  };

struct shm_info
  {
    int used_ids;
    unsigned long int shm_tot;	/* total allocated shm */
    unsigned long int shm_rss;	/* total resident shm */
    unsigned long int shm_swp;	/* total swapped shm */
    unsigned long int swap_attempts;
    unsigned long int swap_successes;
  };

#endif /* __USE_MISC */

__END_DECLS
