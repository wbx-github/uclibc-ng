/*
 * catopen, catgets and catclose, XSI message catalogs.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <errno.h>
#include <nl_types.h>

/* uClibc reads no catalogs: catopen finds none, as on any system where
   none is installed, and catgets gives the program's own string.  */

nl_catd catopen(const char *name, int flag)
{
	(void) name;
	(void) flag;
	__set_errno(ENOENT);
	return (nl_catd) -1;
}

char *catgets(nl_catd catd, int set_id, int msg_id, const char *s)
{
	(void) catd;
	(void) set_id;
	(void) msg_id;
	__set_errno(EBADF);
	return (char *) s;
}

int catclose(nl_catd catd)
{
	(void) catd;
	__set_errno(EBADF);
	return -1;
}
