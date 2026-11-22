/* ISO C11 threads on top of the POSIX threads.
   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.

   Modelled on glibc's nptl/thrd_*.c, mtx_*.c, cnd_*.c and tss_*.c.  */

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>

_Static_assert (sizeof (mtx_t) == sizeof (pthread_mutex_t),
		"mtx_t does not match pthread_mutex_t");
_Static_assert (sizeof (cnd_t) == sizeof (pthread_cond_t),
		"cnd_t does not match pthread_cond_t");
_Static_assert (__alignof__ (mtx_t) == __alignof__ (pthread_mutex_t),
		"mtx_t is not aligned like pthread_mutex_t");
_Static_assert (__alignof__ (cnd_t) == __alignof__ (pthread_cond_t),
		"cnd_t is not aligned like pthread_cond_t");
_Static_assert (sizeof (thrd_t) == sizeof (pthread_t),
		"thrd_t does not match pthread_t");
_Static_assert (sizeof (tss_t) == sizeof (pthread_key_t),
		"tss_t does not match pthread_key_t");

static int
thrd_err_map (int err)
{
  switch (err)
    {
    case 0:
      return thrd_success;
    case ENOMEM:
      return thrd_nomem;
    case ETIMEDOUT:
      return thrd_timedout;
    case EBUSY:
      return thrd_busy;
    default:
      return thrd_error;
    }
}

/* A C11 thread function returns int, a POSIX one void *; on some ABIs
   (m68k) the two come back in different registers, so the function
   cannot simply be cast.  */
struct thrd_start
{
  thrd_start_t func;
  void *arg;
};

static void *
thrd_trampoline (void *p)
{
  struct thrd_start start = *(struct thrd_start *) p;

  free (p);
  return (void *) (intptr_t) start.func (start.arg);
}

int
thrd_create (thrd_t *thr, thrd_start_t func, void *arg)
{
  struct thrd_start *start = malloc (sizeof *start);
  int err;

  if (start == NULL)
    return thrd_nomem;
  start->func = func;
  start->arg = arg;
  /* Let pthread_create write the id, so that it is there before the
     thread runs.  */
  err = pthread_create ((pthread_t *) thr, NULL, thrd_trampoline, start);
  if (err != 0)
    {
      free (start);
      /* EAGAIN is pthread_create's "no stack": memory, for C11.  */
      return err == EAGAIN ? thrd_nomem : thrd_err_map (err);
    }
  return thrd_success;
}

int
thrd_equal (thrd_t lhs, thrd_t rhs)
{
  return pthread_equal (lhs, rhs);
}

thrd_t
thrd_current (void)
{
  return pthread_self ();
}

int
thrd_sleep (const struct timespec *time_point, struct timespec *remaining)
{
  int saved = errno;
  int ret = nanosleep (time_point, remaining);

  if (ret == 0)
    return 0;
  ret = errno == EINTR ? -1 : -2;
  __set_errno (saved);
  return ret;
}

void
thrd_exit (int res)
{
  pthread_exit ((void *) (intptr_t) res);
}

int
thrd_detach (thrd_t thr)
{
  return thrd_err_map (pthread_detach (thr));
}

int
thrd_join (thrd_t thr, int *res)
{
  void *ret;
  int err = pthread_join (thr, &ret);

  if (err == 0 && res != NULL)
    *res = (int) (intptr_t) ret;
  return thrd_err_map (err);
}

void
thrd_yield (void)
{
  sched_yield ();
}

int
mtx_init (mtx_t *mutex, int type)
{
  pthread_mutexattr_t attr;
  int err;

  switch (type)
    {
    case mtx_plain:
    case mtx_timed:
    case mtx_plain | mtx_recursive:
    case mtx_timed | mtx_recursive:
      break;
    default:
      return thrd_error;
    }

  pthread_mutexattr_init (&attr);
  if (type & mtx_recursive)
    pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);
  err = pthread_mutex_init ((pthread_mutex_t *) mutex, &attr);
  pthread_mutexattr_destroy (&attr);
  return thrd_err_map (err);
}

int
mtx_lock (mtx_t *mutex)
{
  return thrd_err_map (pthread_mutex_lock ((pthread_mutex_t *) mutex));
}

int
mtx_timedlock (mtx_t *__restrict mutex,
	       const struct timespec *__restrict time_point)
{
  return thrd_err_map (pthread_mutex_timedlock ((pthread_mutex_t *) mutex,
						time_point));
}

int
mtx_trylock (mtx_t *mutex)
{
  return thrd_err_map (pthread_mutex_trylock ((pthread_mutex_t *) mutex));
}

int
mtx_unlock (mtx_t *mutex)
{
  return thrd_err_map (pthread_mutex_unlock ((pthread_mutex_t *) mutex));
}

void
mtx_destroy (mtx_t *mutex)
{
  pthread_mutex_destroy ((pthread_mutex_t *) mutex);
}

int
cnd_init (cnd_t *cond)
{
  return thrd_err_map (pthread_cond_init ((pthread_cond_t *) cond, NULL));
}

int
cnd_signal (cnd_t *cond)
{
  return thrd_err_map (pthread_cond_signal ((pthread_cond_t *) cond));
}

int
cnd_broadcast (cnd_t *cond)
{
  return thrd_err_map (pthread_cond_broadcast ((pthread_cond_t *) cond));
}

int
cnd_wait (cnd_t *cond, mtx_t *mutex)
{
  return thrd_err_map (pthread_cond_wait ((pthread_cond_t *) cond,
					  (pthread_mutex_t *) mutex));
}

int
cnd_timedwait (cnd_t *__restrict cond, mtx_t *__restrict mutex,
	       const struct timespec *__restrict time_point)
{
  return thrd_err_map (pthread_cond_timedwait ((pthread_cond_t *) cond,
					       (pthread_mutex_t *) mutex,
					       time_point));
}

void
cnd_destroy (cnd_t *cond)
{
  pthread_cond_destroy ((pthread_cond_t *) cond);
}

int
tss_create (tss_t *tss_id, tss_dtor_t destructor)
{
  return thrd_err_map (pthread_key_create (tss_id, destructor));
}

void *
tss_get (tss_t tss_id)
{
  return pthread_getspecific (tss_id);
}

int
tss_set (tss_t tss_id, void *val)
{
  return thrd_err_map (pthread_setspecific (tss_id, val));
}

void
tss_delete (tss_t tss_id)
{
  pthread_key_delete (tss_id);
}
