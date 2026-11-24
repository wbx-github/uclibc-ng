/*
 * open_wmemstream, POSIX.1-2008: open_memstream for wide characters.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include "_stdio.h"
#include <wchar.h>

#ifndef __UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__
#error no custom streams!
#endif

#define COOKIE ((__owms_cookie *) cookie)

#define WMEMSTREAM_BUFSIZ	64

/* As open_memstream, in wide characters: positions and the size count
   them, and the wide nul goes at the end of the data.  stdio hands the
   cookie the multibyte form of what the program wrote, so it is turned
   back here; a character split across two writes waits in the state.  */
typedef struct {
	wchar_t *buf;
	size_t len;
	size_t pos;
	size_t eof;
	mbstate_t state;
	wchar_t **bufloc;
	size_t *sizeloc;
} __owms_cookie;

static void owms_term(register __owms_cookie *c)
{
	c->buf[c->eof] = 0;
	*c->sizeloc = c->pos;
}

/* Room for at least NEED wide characters plus the nul.  */
static int owms_grow(register __owms_cookie *c, size_t need)
{
	wchar_t *newbuf;
	size_t want;

	if (need < c->len)
		return 0;
	if (need >= SIZE_MAX / sizeof(wchar_t) - 1) {
		__set_errno(EFBIG);
		return -1;
	}
	/* Double rather than fit exactly.  The stream is unbuffered, so a
	   wide character arrives on its own, and an exact fit would mean
	   one realloc per character.  */
	want = need + 1;
	if (c->len <= (SIZE_MAX / sizeof(wchar_t) - 1) / 2 && want < 2 * c->len)
		want = 2 * c->len;

	newbuf = realloc(c->buf, want * sizeof(wchar_t));
	if (newbuf == NULL)
		return -1;
	*c->bufloc = c->buf = newbuf;
	c->len = want;
	return 0;
}

static ssize_t owms_write(register void *cookie, const char *buf, size_t bufsize)
{
	size_t i, r;
	wchar_t wc;

	/* Each byte gives at most one wide character.  */
	if (owms_grow(COOKIE, COOKIE->pos + bufsize) < 0)
		return -1;

	for (i = 0; i < bufsize; i += r) {
		r = mbrtowc(&wc, buf + i, bufsize - i, &COOKIE->state);
		if (r == (size_t) -2)	/* Rest of a character to come.  */
			break;
		if (r == (size_t) -1) {
			/* Hand back what was converted, but leave the buffer
			   described: pos has moved, so eof and the wide nul
			   have to follow before returning.  */
			if (COOKIE->pos > COOKIE->eof)
				COOKIE->eof = COOKIE->pos;
			owms_term(COOKIE);
			return i ? (ssize_t) i : -1;
		}
		if (r == 0)
			r = 1;
		COOKIE->buf[COOKIE->pos++] = wc;
	}

	if (COOKIE->pos > COOKIE->eof)
		COOKIE->eof = COOKIE->pos;
	owms_term(COOKIE);

	return bufsize;
}

static int owms_seek(register void *cookie, __offmax_t *pos, int whence)
{
	__offmax_t p = *pos;

	assert(((unsigned int) whence) <= 2);

	/* As in open_memstream, SEEK_END counts from the size, which is the
	   position for a write-only stream.  */
	if (whence != SEEK_SET)
		p += COOKIE->pos;

	if (p < 0 || ((uintmax_t) p) >= SIZE_MAX / sizeof(wchar_t) - 1) {
		__set_errno(EINVAL);
		return -1;
	}

	if (owms_grow(COOKIE, (size_t) p) < 0)
		return -1;

	*pos = COOKIE->pos = (size_t) p;
	memset(&COOKIE->state, 0, sizeof(COOKIE->state));

	if (COOKIE->pos > COOKIE->eof) {	/* Seeked past the end: 0-fill.  */
		wmemset(COOKIE->buf + COOKIE->eof, 0, COOKIE->pos - COOKIE->eof);
		COOKIE->eof = COOKIE->pos;
	}
	owms_term(COOKIE);

	return 0;
}

static int owms_close(void *cookie)
{
	free(cookie);
	return 0;
}

#undef COOKIE

static const cookie_io_functions_t _owms_io_funcs = {
	NULL, owms_write, owms_seek, owms_close
};

FILE *open_wmemstream(wchar_t **bufloc, size_t *sizeloc)
{
	register __owms_cookie *cookie;
	register FILE *fp;

	cookie = calloc(1, sizeof(__owms_cookie));
	if (cookie == NULL)
		return NULL;
	cookie->buf = malloc((cookie->len = WMEMSTREAM_BUFSIZ) * sizeof(wchar_t));
	if (cookie->buf == NULL)
		goto EXIT_cookie;
	*cookie->buf = 0;
	*(cookie->bufloc = bufloc) = cookie->buf;
	*(cookie->sizeloc = sizeloc) = 0;

	fp = fopencookie(cookie, "w", _owms_io_funcs);
	if (fp != NULL) {
		/* The cookie counts wide characters, the stdio buffer holds
		   multibyte bytes.  Anything left in that buffer would be
		   added to the position in the wrong unit, so ftell would
		   answer 3 after a single fputwc (L'\u20ac') and 1 after the
		   flush.  There is nothing to gain from buffering in front of
		   a buffer, so write through.  Before the stream is marked
		   wide: setvbuf refuses an oriented stream.  */
		setvbuf(fp, NULL, _IONBF, 0);
		/* POSIX: the stream is wide-oriented.  */
		__STDIO_STREAM_SET_WIDE(fp);
		__STDIO_STREAM_VALIDATE(fp);
		return fp;
	}

	free(cookie->buf);
 EXIT_cookie:
	free(cookie);
	return NULL;
}
