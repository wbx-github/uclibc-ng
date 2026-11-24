/* Formatting of monetary values, POSIX.1-2008 (XSI).
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _MONETARY_H
#define _MONETARY_H	1

#include <features.h>

#define __need_size_t
#include <stddef.h>
#include <bits/types.h>

#ifndef __ssize_t_defined
typedef __ssize_t ssize_t;
# define __ssize_t_defined
#endif

#if defined __USE_XOPEN2K8 && defined __UCLIBC_HAS_XLOCALE__
# include <xlocale.h>
#endif

__BEGIN_DECLS

/* Format the values given after FORMAT into at most MAXSIZE bytes
   at S, following the LC_MONETARY category of the current locale.  */
extern ssize_t strfmon (char *__restrict __s, size_t __maxsize,
			const char *__restrict __format, ...)
     __THROW __attribute_format_strfmon__ (3, 4);

#if defined __USE_XOPEN2K8 && defined __UCLIBC_HAS_XLOCALE__
/* Likewise, following the locale LOC.  */
extern ssize_t strfmon_l (char *__restrict __s, size_t __maxsize,
			  __locale_t __loc, const char *__restrict __format, ...)
     __THROW __attribute_format_strfmon__ (4, 5);
#endif

__END_DECLS

#endif /* monetary.h */
