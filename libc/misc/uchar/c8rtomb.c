/*
 * c8rtomb() for uClibc (ISO C23)
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <uchar.h>
#include <wchar.h>
#include <errno.h>

/* A started sequence waits in the state: its length times 4 plus the
   units still missing in __mask (at most 19, below the multiples of
   0x40 mbrtowc uses), the bits so far in __wc.  */
size_t c8rtomb(char *__restrict s, char8_t c8, mbstate_t *__restrict ps)
{
	static mbstate_t internal_state;
	unsigned int st, len, wc;

	if (!ps)
		ps = &internal_state;
	if (!s) {
		if (ps->__mask)
			goto ilseq;
		return 1;
	}

	st = (unsigned int) ps->__mask;
	if (!st) {
		if (c8 < 0x80)
			return wcrtomb(s, c8, NULL);
		if (c8 >= 0xc2 && c8 <= 0xdf) {
			len = 2;
			wc = c8 & 0x1f;
		} else if (c8 >= 0xe0 && c8 <= 0xef) {
			len = 3;
			wc = c8 & 0x0f;
		} else if (c8 >= 0xf0 && c8 <= 0xf4) {
			len = 4;
			wc = c8 & 0x07;
		} else
			goto ilseq;
		ps->__wc = (wchar_t) wc;
		ps->__mask = (wchar_t) (len * 4 + len - 1);
		return 0;
	}

	if ((c8 & 0xc0) != 0x80)
		goto ilseq;
	len = st / 4;
	wc = ((unsigned int) ps->__wc << 6) | (c8 & 0x3f);
	/* Refuse a bad second unit right away, as glibc does: overlong,
	   surrogate or past U+10FFFF already shows in the top bits.  */
	if (len > 2 && st % 4 == len - 1
	    && (len == 3 ? wc < 0x20 || (wc >= 0x360 && wc < 0x380)
			 : wc < 0x10 || wc > 0x10f))
		goto ilseq;
	if (st % 4 > 1) {
		ps->__wc = (wchar_t) wc;
		ps->__mask = (wchar_t) (st - 1);
		return 0;
	}
	ps->__mask = 0;
	/* No overlong forms, no surrogates, nothing past U+10FFFF.  */
	if ((len == 3 && wc < 0x800) || (len == 4 && wc < 0x10000)
	    || (wc >= 0xd800 && wc < 0xe000) || wc > 0x10ffff)
		goto ilseq;
	return wcrtomb(s, (wchar_t) wc, NULL);

ilseq:
	ps->__mask = 0;
	__set_errno(EILSEQ);
	return (size_t) -1;
}
