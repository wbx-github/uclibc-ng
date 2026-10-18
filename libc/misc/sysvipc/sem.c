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

#include <errno.h>
#include <sys/sem.h>
#include <stddef.h>
#include <stdlib.h> /* for NULL */

#include "ipc.h"

#if defined(__UCLIBC_USE_TIME64__)
#include "internal/time64_helpers.h"
#endif

#ifdef L_semctl
/* Return identifier for array of NSEMS semaphores associated with
   KEY.  */
#include <stdarg.h>
/* arg for semctl system calls. */
union semun {
    int val;			/* value for SETVAL */
    struct semid_ds *buf;		/* buffer for IPC_STAT & IPC_SET */
    unsigned short *array;		/* array for GETALL & SETALL */
    struct seminfo *__buf;		/* buffer for IPC_INFO */
    void *__pad;
};


#ifdef __NR_semctl
#define __NR___semctl __NR_semctl
static __inline__ _syscall4(int, __semctl, int, semid, int, semnum, int, cmd, void *, arg)
#endif

int semctl(int semid, int semnum, int cmd, ...)
{
    union semun arg = { 0 };
    va_list ap;

    /* Only these commands take a fourth argument.  Reading it anyway is not
       free: where the ABI hands a union over by reference, va_arg becomes a
       load through a pointer the caller never wrote.  */
    switch (cmd) {
    case SETVAL:
    case GETALL:
    case SETALL:
    case IPC_STAT:
    case IPC_SET:
#ifdef IPC_INFO
    case IPC_INFO:
#endif
#ifdef SEM_STAT
    case SEM_STAT:
#endif
#ifdef SEM_INFO
    case SEM_INFO:
#endif
	va_start (ap, cmd);
	arg = va_arg (ap, union semun);
	va_end (ap);
	break;
    }
    if (cmd == IPC_SET && arg.buf != NULL)
	__ipc_mode_to_kernel(&arg.buf->sem_perm.mode);
#ifdef __NR_semctl
    int __ret = __semctl(semid, semnum, cmd | __IPC_64, arg.__pad);
#if (__WORDSIZE == 32) && defined(__UCLIBC_USE_TIME64__)
    /* IPC_STAT and SEM_STAT are the commands the kernel answers with a
       struct semid_ds.  For every other one the fourth argument has a
       different type, or the caller passed none at all.  */
    if ((cmd == IPC_STAT || cmd == SEM_STAT) && arg.buf != NULL) {
        arg.buf->sem_otime = (__time_t)arg.buf->__sem_otime_internal_1 | (__time_t)(arg.buf->__sem_otime_internal_2) << 32;
        arg.buf->sem_ctime = (__time_t)arg.buf->__sem_ctime_internal_1 | (__time_t)(arg.buf->__sem_ctime_internal_2) << 32;
    }
#endif
    if ((cmd == IPC_SET || cmd == IPC_STAT) && arg.buf != NULL)
	__ipc_mode_from_kernel(&arg.buf->sem_perm.mode);
    return __ret;
#else
# if defined __sparc__ && defined __arch64__
    /* sparc64 has its own demultiplexer, sys_sparc_ipc() in
       arch/sparc/kernel/sys_sparc_64.c: for SEMCTL it hands ptr straight to
       sys_semctl() as the semun argument, where the generic sys_ipc() reads
       the argument through it with get_user().  Passing &arg there makes the
       kernel write the semid_ds over the union itself -- and it sets IPC_64
       on its own, so the bit does not matter.  */
    return __syscall_ipc(IPCOP_semctl, semid, semnum, cmd, arg.__pad, NULL);
# else
    {
	int __ret = __syscall_ipc(IPCOP_semctl, semid, semnum, cmd|__IPC_64, &arg, NULL);
	if ((cmd == IPC_SET || cmd == IPC_STAT) && arg.buf != NULL)
		__ipc_mode_from_kernel(&arg.buf->sem_perm.mode);
	return __ret;
    }
# endif
#endif
}
#endif

#ifdef L_semget
#ifdef __NR_semget
_syscall3(int, semget, key_t, key, int, nsems, int, semflg)

#else
/* Return identifier for array of NSEMS semaphores associated
 * with KEY.  */
int semget (key_t key, int nsems, int semflg)
{
    return __syscall_ipc(IPCOP_semget, key, nsems, semflg, NULL, 0);
}
#endif
#endif

#ifdef L_semop

#ifdef __NR_semop
_syscall3(int, semop, int, semid, struct sembuf *, sops, size_t, nsops)

#else
/* Perform user-defined atomical operation of array of semaphores.  */
int semop (int semid, struct sembuf *sops, size_t nsops)
{
    return __syscall_ipc(IPCOP_semop, semid, (int) nsops, 0, sops, NULL);
}
#endif
#endif

#ifdef L_semtimedop

#if defined(__UCLIBC_USE_TIME64__) && defined(__NR_semtimedop_time64)
int semtimedop(int semid, struct sembuf *sops, size_t nsops, const struct timespec *timeout)
{
    return INLINE_SYSCALL(semtimedop_time64, 4, semid, sops, nsops, TO_TS64_P(timeout));
}

#elif defined(__NR_semtimedop)
_syscall4(int, semtimedop, int, semid, struct sembuf *, sops, size_t, nsops, const struct timespec *, timeout)

#else

int semtimedop(int semid, struct sembuf *sops, size_t nsops,
	       const struct timespec *timeout)
{
    return __syscall_ipc(IPCOP_semtimedop, semid, nsops, 0, sops, (void *) timeout);
}
#endif
#endif
