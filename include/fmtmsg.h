/* Formatted message display, POSIX.1-2008 (XSI).
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _FMTMSG_H
#define _FMTMSG_H	1

#include <features.h>

/* Classification; the values are glibc's.  */
#define MM_HARD		0x001	/* Source: hardware.  */
#define MM_SOFT		0x002	/* Source: software.  */
#define MM_FIRM		0x004	/* Source: firmware.  */
#define MM_APPL		0x008	/* Detected by an application.  */
#define MM_UTIL		0x010	/* Detected by a utility.  */
#define MM_OPSYS	0x020	/* Detected by the operating system.  */
#define MM_RECOVER	0x040	/* Recoverable.  */
#define MM_NRECOV	0x080	/* Not recoverable.  */
#define MM_PRINT	0x100	/* Display on standard error.  */
#define MM_CONSOLE	0x200	/* Display on the system console.  */

/* Severity.  */
#define MM_NOSEV	0
#define MM_HALT		1
#define MM_ERROR	2
#define MM_WARNING	3
#define MM_INFO		4

/* Null values of the arguments.  */
#define MM_NULLLBL	((char *) 0)
#define MM_NULLSEV	0
#define MM_NULLMC	0L
#define MM_NULLTXT	((char *) 0)
#define MM_NULLACT	((char *) 0)
#define MM_NULLTAG	((char *) 0)

/* Return values.  */
#define MM_NOTOK	-1
#define MM_OK		0
#define MM_NOMSG	1
#define MM_NOCON	4

__BEGIN_DECLS

/* Display a message made of LABEL, SEVERITY, TEXT, ACTION and TAG on
   standard error or the console, as CLASSIFICATION says.  */
extern int fmtmsg (long int __classification, const char *__label,
		   int __severity, const char *__text,
		   const char *__action, const char *__tag);

__END_DECLS

#endif /* fmtmsg.h */
