/*
 * timer_gettime.c - get the timer value.
 */

#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <sys/syscall.h>

#include "kernel-posix-timers.h"

#if defined(__NR_timer_gettime) || defined(__NR_timer_gettime64)

#if defined(__UCLIBC_USE_TIME64__) && defined(__NR_timer_gettime64)
#include "internal/time64_helpers.h"

int timer_gettime(timer_t timerid, struct itimerspec *value)
{
	struct timer *kt = (struct timer *)timerid;
	struct __its64_struct __cur;
	int __ret = INLINE_SYSCALL(timer_gettime64, 2, kt->ktimerid, value ? &__cur : NULL);

	if (__ret == 0 && value)
		__its64_to_its(&__cur, value);
	return __ret;
}
#else
#define __NR___syscall_timer_gettime __NR_timer_gettime
static __inline__ _syscall2(int, __syscall_timer_gettime, kernel_timer_t, ktimerid,
			void *, value);

/* Get the amount of time left on a timer */
int timer_gettime(timer_t timerid, struct itimerspec *value)
{
	struct timer *kt = (struct timer *)timerid;

	/* Get timeout from the kernel */
	return __syscall_timer_gettime(kt->ktimerid, value);
}
#endif

#endif
