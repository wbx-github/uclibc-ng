/*
 * fetestexceptflag() for uClibc (ISO C23)
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <fenv.h>

/* Every port's fegetexceptflag keeps the flags at their FE_* bits.  */
int fetestexceptflag(const fexcept_t *flagp, int excepts)
{
	return *flagp & excepts & FE_ALL_EXCEPT;
}
