/*
 * quick_exit() and at_quick_exit() for uClibc (ISO C11)
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdlib.h>
#include <bits/uClibc_mutex.h>

/* C11 asks for room for at least 32; musl and glibc's static part
   stop there too.  */
#define QUICK_EXIT_MAX 32

static void (*funcs[QUICK_EXIT_MAX])(void);
static int count;
__UCLIBC_MUTEX_STATIC(lock, PTHREAD_MUTEX_INITIALIZER);

int at_quick_exit(void (*func)(void))
{
	int r = -1;

	__UCLIBC_MUTEX_LOCK(lock);
	if (count < QUICK_EXIT_MAX) {
		funcs[count++] = func;
		r = 0;
	}
	__UCLIBC_MUTEX_UNLOCK(lock);
	return r;
}

void quick_exit(int status)
{
	void (*func)(void);

	/* Last registered first; a handler may register another.  */
	for (;;) {
		func = NULL;
		__UCLIBC_MUTEX_LOCK(lock);
		if (count > 0)
			func = funcs[--count];
		__UCLIBC_MUTEX_UNLOCK(lock);
		if (!func)
			break;
		func();
	}
	_Exit(status);
}
