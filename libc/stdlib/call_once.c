/*
 * call_once() for uClibc (ISO C11 <threads.h>, C23 also <stdlib.h>)
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdlib.h>

#ifdef __UCLIBC_HAS_THREADS__
# include <pthread.h>

_Static_assert (sizeof (once_flag) == sizeof (pthread_once_t),
		"once_flag does not match pthread_once_t");
_Static_assert (__alignof__ (once_flag) >= 4,
		"once_flag is a futex word and must be 4-byte aligned");

void call_once(once_flag *flag, void (*func)(void))
{
	pthread_once((pthread_once_t *) flag, func);
}
#else
/* Without threads there is no one to race with.  */
void call_once(once_flag *flag, void (*func)(void))
{
	if (flag->__data == 0) {
		flag->__data = 1;
		func();
	}
}
#endif
