/*
 * psiginfo, POSIX.1-2008.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <signal.h>

/* The message psignal prints, for the signal in PINFO.  */
void psiginfo(const siginfo_t *pinfo, const char *message)
{
	psignal(pinfo->si_signo, message);
}
