/*
 * strfromd(), strfromf() and strfroml() for uClibc (ISO C23)
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>

/* C23 allows only "%[.precision]conversion" with one of aAeEfFgG.
   Return the conversion and set *PREC (-1 without a precision, which
   "%.*" takes as none given); return 0 for anything else.  */
static int strfrom_format(const char *p, int *prec)
{
	*prec = -1;
	if (*p++ != '%')
		goto bad;
	if (*p == '.') {
		*prec = 0;
		for (++p; *p >= '0' && *p <= '9'; ++p) {
			if (*prec > (INT_MAX - 9) / 10)
				goto bad;
			*prec = *prec * 10 + (*p - '0');
		}
	}
	switch (*p) {
	case 'a': case 'A': case 'e': case 'E':
	case 'f': case 'F': case 'g': case 'G':
		if (!p[1])
			return *p;
	}
bad:
	__set_errno(EINVAL);
	return 0;
}

#define STRFROM(L)							\
	switch (strfrom_format(format, &prec)) {			\
	case 'a': return snprintf(s, n, "%.*" L "a", prec, fp);		\
	case 'A': return snprintf(s, n, "%.*" L "A", prec, fp);		\
	case 'e': return snprintf(s, n, "%.*" L "e", prec, fp);		\
	case 'E': return snprintf(s, n, "%.*" L "E", prec, fp);		\
	case 'f': return snprintf(s, n, "%.*" L "f", prec, fp);		\
	case 'F': return snprintf(s, n, "%.*" L "F", prec, fp);		\
	case 'g': return snprintf(s, n, "%.*" L "g", prec, fp);		\
	case 'G': return snprintf(s, n, "%.*" L "G", prec, fp);		\
	}								\
	return -1

int strfromd(char *__restrict s, size_t n, const char *__restrict format,
	     double fp)
{
	int prec;

	STRFROM("");
}

int strfromf(char *__restrict s, size_t n, const char *__restrict format,
	     float f)
{
	double fp = f;
	int prec;

	STRFROM("");
}

int strfroml(char *__restrict s, size_t n, const char *__restrict format,
	     long double fp)
{
	int prec;

	STRFROM("L");
}
