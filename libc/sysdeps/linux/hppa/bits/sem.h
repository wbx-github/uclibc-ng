/* Copyright (C) 1995, 1996, 1997, 1998, 2000 Free Software Foundation, Inc.
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

#ifndef _SYS_SEM_H
# error "Never include <bits/sem.h> directly; use <sys/sem.h> instead."
#endif

#include <sys/types.h>
#include <bits/wordsize.h>

/* Flags for `semop'.  */
#define SEM_UNDO	0x1000		/* undo the operation on exit */

/* Commands for `semctl'.  */
#define GETPID		11		/* get sempid */
#define GETVAL		12		/* get semval */
#define GETALL		13		/* get all semval's */
#define GETNCNT		14		/* get semncnt */
#define GETZCNT		15		/* get semzcnt */
#define SETVAL		16		/* set semval */
#define SETALL		17		/* set all semval's */


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

struct semid_ds
{
  struct ipc_perm sem_perm;		/* operation permission struct */
  unsigned long int __sem_otime_internal_2;	/* last semop() time */
  unsigned long int __sem_otime_internal_1;
  unsigned long int __sem_ctime_internal_2;	/* last time changed by semctl() */
  unsigned long int __sem_ctime_internal_1;
  unsigned long int sem_nsems;		/* number of semaphores in set */
  /* The kernel fills only the pairs above.  semctl() composes the __time_t
     fields below from them once the call has returned -- and only then,
     because the kernel's copy still covers the first of them.  */
  __time_t sem_otime;
  __time_t sem_ctime;
  unsigned long int __uclibc_unused1;
  unsigned long int __uclibc_unused2;
};

# define __SEMID_DS_TIME64_SPLIT 1

#else

struct semid_ds
{
  struct ipc_perm sem_perm;		/* operation permission struct */
  unsigned int __uclibc_pad1;
  __time_t sem_otime;			/* last semop() time */
  unsigned int __uclibc_pad2;
  __time_t sem_ctime;			/* last time changed by semctl() */
  unsigned long int sem_nsems;		/* number of semaphores in set */
  unsigned long int __uclibc_unused1;
  unsigned long int __uclibc_unused2;
};

#endif

/* The user should define a union like the following to use it for arguments
   for `semctl'.

   union semun
   {
     int val;				<= value for SETVAL
     struct semid_ds *buf;		<= buffer for IPC_STAT & IPC_SET
     unsigned short int *array;		<= array for GETALL & SETALL
     struct seminfo *__buf;		<= buffer for IPC_INFO
   };

   Previous versions of this file used to define this union but this is
   incorrect.  One can test the macro _SEM_SEMUN_UNDEFINED to see whether
   one must define the union or not.  */
#define _SEM_SEMUN_UNDEFINED	1

#ifdef __USE_MISC

/* ipcs ctl cmds */
# define SEM_STAT 18
# define SEM_INFO 19
# define SEM_STAT_ANY 20

struct  seminfo
{
  int semmap;
  int semmni;
  int semmns;
  int semmnu;
  int semmsl;
  int semopm;
  int semume;
  int semusz;
  int semvmx;
  int semaem;
};

#endif /* __USE_MISC */
