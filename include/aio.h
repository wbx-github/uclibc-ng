/* POSIX asynchronous I/O.
   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#ifndef _AIO_H
#define _AIO_H	1

#include <features.h>
#include <sys/types.h>
/* Not <signal.h> and <time.h> alone: those give struct sigevent and
   struct timespec only when a feature test macro asks for POSIX.1b,
   and the header has to work under -std=c99 too.  <mqueue.h> asks the
   same way.  */
#include <signal.h>
#define __need_sigevent_t
#include <bits/siginfo.h>
#define __need_timespec
#include <time.h>

__BEGIN_DECLS

struct aiocb
{
  int aio_fildes;
  int aio_lio_opcode;
  int aio_reqprio;
  volatile void *aio_buf;
  size_t aio_nbytes;
  struct sigevent aio_sigevent;
  __off64_t aio_offset;

  /* Private.  __err is a futex word, which must be 4-byte aligned; on
     m68k an int alone is only 2-byte aligned.  */
  volatile int __err __attribute__ ((__aligned__ (4)));
  ssize_t __ret;
};

/* Return values of aio_cancel.  */
#define AIO_CANCELED	0
#define AIO_NOTCANCELED	1
#define AIO_ALLDONE	2

/* Operation codes for aio_lio_opcode.  */
#define LIO_READ	0
#define LIO_WRITE	1
#define LIO_NOP		2

/* Modes of lio_listio.  */
#define LIO_WAIT	0
#define LIO_NOWAIT	1

extern int aio_read (struct aiocb *__aiocbp);
extern int aio_write (struct aiocb *__aiocbp);
extern int aio_fsync (int __op, struct aiocb *__aiocbp);
extern int aio_error (const struct aiocb *__aiocbp) __THROW;
extern ssize_t aio_return (struct aiocb *__aiocbp) __THROW;
extern int aio_cancel (int __fildes, struct aiocb *__aiocbp);
extern int aio_suspend (const struct aiocb *const __list[], int __nent,
			const struct timespec *__restrict __timeout);
extern int lio_listio (int __mode,
		       struct aiocb *const __list[__restrict_arr],
		       int __nent, struct sigevent *__restrict __sig);

#ifdef __USE_LARGEFILE64
/* aio_offset is 64 bits wide already, so there is one type and these
   are the same functions.  Real symbols, not macros: the rest of the
   library's *64 interfaces are symbols, a configure probe links one
   without including this header, and dlsym has to find it.  */
# define aiocb64	aiocb
extern int aio_read64 (struct aiocb64 *__aiocbp);
extern int aio_write64 (struct aiocb64 *__aiocbp);
extern int aio_fsync64 (int __op, struct aiocb64 *__aiocbp);
extern int aio_error64 (const struct aiocb64 *__aiocbp) __THROW;
extern ssize_t aio_return64 (struct aiocb64 *__aiocbp) __THROW;
extern int aio_cancel64 (int __fildes, struct aiocb64 *__aiocbp);
extern int aio_suspend64 (const struct aiocb64 *const __list[], int __nent,
			  const struct timespec *__restrict __timeout);
extern int lio_listio64 (int __mode,
			 struct aiocb64 *const __list[__restrict_arr],
			 int __nent, struct sigevent *__restrict __sig);
#endif

__END_DECLS

#endif /* aio.h */
