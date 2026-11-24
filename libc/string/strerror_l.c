/*
 * strerror_l, POSIX.1-2008.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>
#include <string.h>
#include "_syserrmsg.h"

/* The messages exist in one language only, so the locale changes
   nothing.  POSIX wants strerror_l thread-safe, and for every error
   number the library knows it is: the message is a constant and comes
   back as it stands.  Only "Unknown error N" has to be built, and that
   goes into a buffer per thread where there is TLS.  */
char *strerror_l(int errnum, __locale_t loc)
{
	/* __thread and the attribute both fall away without TLS.  */
	static __thread char buf[_STRERROR_BUFSIZE] attribute_tls_model_ie;
	const char *s;

	(void) loc;

	s = __strerror_const(errnum);
	if (s)
		return (char *) s;

	__xpg_strerror_r(errnum, buf, sizeof(buf));
	return buf;
}
