/* Copyright (C) 1995, 1996, 1997, 2000 Free Software Foundation, Inc.
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

#ifndef _SYS_MSG_H
# error "Never use <bits/msq.h> directly; include <sys/msg.h> instead."
#endif

#include <bits/types.h>

/* Define options for message queue functions.  */
#define MSG_NOERROR	010000	/* no error if message is too big */
#ifdef __USE_GNU
# define MSG_EXCEPT	020000	/* recv any msg except of specified type */
#endif

/* Types used in the structure definition.  */
typedef unsigned long int msgqnum_t;
typedef unsigned long int msglen_t;


/* Structure of record for one message inside the kernel.
   The type `struct msg' is opaque.  */
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

struct msqid_ds
{
  struct ipc_perm msg_perm;	/* structure describing operation permission */
  unsigned long int msg_stime_internal_2;	/* time of last msgsnd command */
  unsigned long int msg_stime_internal_1;
  unsigned long int msg_rtime_internal_2;	/* time of last msgrcv command */
  unsigned long int msg_rtime_internal_1;
  unsigned long int msg_ctime_internal_2;	/* time of last change */
  unsigned long int msg_ctime_internal_1;
  unsigned long int __msg_cbytes; /* current number of bytes on queue */
  msgqnum_t msg_qnum;		/* number of messages currently on queue */
  msglen_t msg_qbytes;		/* max number of bytes allowed on queue */
  __pid_t msg_lspid;		/* pid of last msgsnd() */
  __pid_t msg_lrpid;		/* pid of last msgrcv() */
  /* The kernel fills only the pairs above.  msgctl() composes the __time_t
     fields below from them once the call has returned -- and only then,
     because the kernel's copy still covers the first of them.  */
  __time_t msg_stime;
  __time_t msg_rtime;
  __time_t msg_ctime;
  unsigned long int __uclibc_unused4;
  unsigned long int __uclibc_unused5;
};

# define __MSQID_DS_TIME64_SPLIT 1

#else

struct msqid_ds
{
  struct ipc_perm msg_perm;	/* structure describing operation permission */
  unsigned int __uclibc_pad1;
  __time_t msg_stime;		/* time of last msgsnd command */
  unsigned int __uclibc_pad2;
  __time_t msg_rtime;		/* time of last msgrcv command */
  unsigned int __uclibc_pad3;
  __time_t msg_ctime;		/* time of last change */
  unsigned long int __msg_cbytes; /* current number of bytes on queue */
  msgqnum_t msg_qnum;		/* number of messages currently on queue */
  msglen_t msg_qbytes;		/* max number of bytes allowed on queue */
  __pid_t msg_lspid;		/* pid of last msgsnd() */
  __pid_t msg_lrpid;		/* pid of last msgrcv() */
  unsigned long int __uclibc_unused4;
  unsigned long int __uclibc_unused5;
};

#endif

#ifdef __USE_MISC

# define msg_cbytes	__msg_cbytes

/* ipcs ctl commands */
# define MSG_STAT 11
# define MSG_INFO 12
# define MSG_STAT_ANY 13

/* buffer for msgctl calls IPC_INFO, MSG_INFO */
struct msginfo
  {
    int msgpool;
    int msgmap;
    int msgmax;
    int msgmnb;
    int msgmni;
    int msgssz;
    int msgtql;
    unsigned short int msgseg;
  };

#endif /* __USE_MISC */
