/*
 * mbrtoc8() for uClibc (ISO C23)
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <uchar.h>
#include <wchar.h>
#include <errno.h>

/* The UTF-8 units still to be returned wait in the state: their count
   (1-3) in __mask, which mbrtowc only ever sets to multiples of 0x40,
   and the units themselves in __wc, next one lowest.  */
size_t mbrtoc8(char8_t *__restrict pc8, const char *__restrict s, size_t n,
	       mbstate_t *__restrict ps)
{
	static mbstate_t internal_state;
	unsigned char u[4];
	unsigned int pending;
	wchar_t wc;
	size_t r;
	int k;

	if (!ps)
		ps = &internal_state;
	if (!s) {
		pc8 = NULL;
		s = "";
		n = 1;
	}

	if ((unsigned int) ps->__mask - 1 < 3) {
		pending = (unsigned int) ps->__wc;
		if (pc8)
			*pc8 = pending & 0xff;
		ps->__wc = (wchar_t) (pending >> 8);
		ps->__mask = (wchar_t) ((unsigned int) ps->__mask - 1);
		return (size_t) -3;
	}

	r = mbrtowc(&wc, s, n, ps);
	if (r == (size_t) -1 || r == (size_t) -2)
		return r;

	if ((unsigned int) wc < 0x80) {
		u[0] = wc;
		k = 1;
	} else if ((unsigned int) wc < 0x800) {
		u[0] = 0xc0 | (wc >> 6);
		u[1] = 0x80 | (wc & 0x3f);
		k = 2;
	} else if ((unsigned int) wc - 0xd800 < 0x800) {
		/* A surrogate has no UTF-8 form; the C locale maps the
		   bytes 0x80-0xff into that range.  */
		__set_errno(EILSEQ);
		return (size_t) -1;
	} else if ((unsigned int) wc < 0x10000) {
		u[0] = 0xe0 | (wc >> 12);
		u[1] = 0x80 | ((wc >> 6) & 0x3f);
		u[2] = 0x80 | (wc & 0x3f);
		k = 3;
	} else if ((unsigned int) wc < 0x110000) {
		u[0] = 0xf0 | (wc >> 18);
		u[1] = 0x80 | ((wc >> 12) & 0x3f);
		u[2] = 0x80 | ((wc >> 6) & 0x3f);
		u[3] = 0x80 | (wc & 0x3f);
		k = 4;
	} else {
		__set_errno(EILSEQ);
		return (size_t) -1;
	}

	if (pc8)
		*pc8 = u[0];
	if (k > 1) {
		ps->__mask = (wchar_t) (k - 1);
		pending = 0;
		while (--k > 0)
			pending = (pending << 8) | u[k];
		ps->__wc = (wchar_t) pending;
	}
	return r;
}
