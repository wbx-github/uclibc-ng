/* Internal declarations for POSIX asynchronous I/O.
   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#ifndef _AIO_MISC_H
#define _AIO_MISC_H

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <time.h>
#include <sys/syscall.h>
#include "internal/time64_helpers.h"

#define AIO_FUTEX_WAIT		(0 | 128)	/* FUTEX_WAIT | FUTEX_PRIVATE_FLAG */
#define AIO_FUTEX_WAKE		(1 | 128)	/* FUTEX_WAKE | FUTEX_PRIVATE_FLAG */

#if defined(__UCLIBC_USE_TIME64__) && defined(__NR_futex_time64)
# define AIO_FUTEX(addr, op, val, ts) \
	INTERNAL_SYSCALL (futex_time64, __err, 4, (addr), (op), (val), TO_TS64_P (ts))
#else
# define AIO_FUTEX(addr, op, val, ts) \
	INTERNAL_SYSCALL (futex, __err, 4, (addr), (op), (val), (ts))
#endif

/* Wake every waiter on ADDR.  */
static inline void __aio_wake (volatile int *addr)
{
	INTERNAL_SYSCALL_DECL (__err);
	AIO_FUTEX (addr, AIO_FUTEX_WAKE, INT_MAX, NULL);
}

/* Sleep while *ADDR is VAL, for at most TS (relative, CLOCK_MONOTONIC).
   Returns 0, or ETIMEDOUT, EINTR or EAGAIN.  */
static inline int __aio_futex_wait (volatile int *addr, int val,
				    const struct timespec *ts)
{
	INTERNAL_SYSCALL_DECL (__err);
	long r = AIO_FUTEX (addr, AIO_FUTEX_WAIT, val, ts);

	return INTERNAL_SYSCALL_ERROR_P (r, __err)
		? INTERNAL_SYSCALL_ERRNO (r, __err) : 0;
}

/* The stack an io thread asks for.  NPTL carves the static TLS block and
   the thread descriptor out of the size a caller gives, so PTHREAD_STACK_MIN
   alone leaves a program with TLS less than the minimum; musl adds its TLS
   in pthread_create instead.  __pthread_get_minstack returns the sum.  */
extern size_t __pthread_get_minstack (const pthread_attr_t *attr);

/* Bumped and woken whenever an operation completes; aio_suspend with
   more than one request waits on it.  Futex words must be 4-byte
   aligned, which an int is not on m68k.  */
extern volatile int __aio_fut __attribute__ ((__aligned__ (4))) attribute_hidden;

#endif
