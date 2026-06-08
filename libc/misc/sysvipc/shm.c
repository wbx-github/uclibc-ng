/* Copyright (C) 1995, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, August 1995.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   see <http://www.gnu.org/licenses/>.  */

/* SHMLBA uses it on most of the archs (not mips) */
#define __getpagesize getpagesize

#include <stdlib.h>
#include <errno.h>
#include <sys/shm.h>
#include <syscall.h>
#include "ipc.h"

#if defined(__UCLIBC_USE_TIME64__)
union shmun {
    struct shmid_ds* buff;
    void *__pad;
};
#endif

#ifdef L_shmat
/* Attach the shared memory segment associated with SHMID to the data
   segment of the calling process.  SHMADDR and SHMFLG determine how
   and where the segment is attached.  */
#if defined(__NR_osf_shmat)
# define __NR_shmat  __NR_osf_shmat
#endif
#ifdef __NR_shmat
_syscall3(void *, shmat, int, shmid, const void *,shmaddr, int, shmflg)
#else
/* psm: don't remove this, else mips will fail */
#include <unistd.h>

void * shmat (int shmid, const void *shmaddr, int shmflg)
{
    int retval;
    unsigned long raddr;

    retval = __syscall_ipc(IPCOP_shmat, shmid, shmflg, (int) &raddr, (void *) shmaddr, 0);
    return ((unsigned long int) retval > -(unsigned long int) SHMLBA
	    ? (void *) retval : (void *) raddr);
}
#endif
#endif

#ifdef L_shmctl
/* Provide operations to control over shared memory segments.  */
#ifdef __NR_shmctl
#define __NR___syscall_shmctl __NR_shmctl
static __always_inline _syscall3(int, __syscall_shmctl, int, shmid, int, cmd, struct shmid_ds *, buf)
#endif
int shmctl(int shmid, int cmd, struct shmid_ds *buf)
{
#ifdef __NR_shmctl
	int __ret = __syscall_shmctl(shmid, cmd | __IPC_64, buf);
#if (__WORDSIZE == 32) && defined(__mips) && defined(__UCLIBC_USE_TIME64__)
	union shmun arg = {.buff = buf};
	// When cmd is IPC_RMID, buf should be NULL.
        if (arg.__pad != NULL) {
		arg.buff->shm_atime = (__time_t)arg.buff->shm_atime_internal_1 | (__time_t)(arg.buff->shm_atime_internal_2) << 32;
		arg.buff->shm_dtime = (__time_t)arg.buff->shm_dtime_internal_1 | (__time_t)(arg.buff->shm_dtime_internal_2) << 32;
		arg.buff->shm_ctime = (__time_t)arg.buff->shm_ctime_internal_1 | (__time_t)(arg.buff->shm_ctime_internal_2) << 32;
	}
#elif defined(__SHMID_DS_TIME64_SPLIT)
	// combine the kernel's split time words
	// When cmd is IPC_RMID, buf should be NULL.
	if (buf != NULL) {
		buf->shm_atime = (__time_t)buf->__shm_atime_internal_1 | (__time_t)(buf->__shm_atime_internal_2) << 32;
		buf->shm_dtime = (__time_t)buf->__shm_dtime_internal_1 | (__time_t)(buf->__shm_dtime_internal_2) << 32;
		buf->shm_ctime = (__time_t)buf->__shm_ctime_internal_1 | (__time_t)(buf->__shm_ctime_internal_2) << 32;
	}
#endif
	return __ret;
#else
    return __syscall_ipc(IPCOP_shmctl, shmid, cmd | __IPC_64, 0, buf, 0);
#endif
}
#endif


#ifdef L_shmdt
/* Detach shared memory segment starting at address specified by SHMADDR
   from the caller's data segment.  */
#ifdef __NR_shmdt
_syscall1(int, shmdt, const void *, shmaddr)
#else
int shmdt (const void *shmaddr)
{
    return __syscall_ipc(IPCOP_shmdt, 0, 0, 0, (void *) shmaddr, 0);
}
#endif
#endif

#ifdef L_shmget
/* Return an identifier for an shared memory segment of at least size SIZE
   which is associated with KEY.  */
#ifdef __NR_shmget
_syscall3(int, shmget, key_t, key, size_t, size, int, shmflg)
#else
int shmget (key_t key, size_t size, int shmflg)
{
    return __syscall_ipc(IPCOP_shmget, key, size, shmflg, NULL, 0);
}
#endif
#endif
