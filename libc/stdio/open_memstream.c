/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include <features.h>

#ifdef __USE_GNU
#include "_stdio.h"


#ifndef __UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__
#error no custom streams!
#endif

#define COOKIE ((__oms_cookie *) cookie)

#define MEMSTREAM_BUFSIZ	256

typedef struct {
	char *buf;
	size_t len;
	size_t pos;
	size_t eof;
	char **bufloc;
	size_t *sizeloc;
} __oms_cookie;

/* POSIX appends the null at the end of the data, not at the position, and
 * wants the size after every fflush().  A cookie stream has no flush callback
 * (__stdio_wcommit() only reaches oms_write() when the FILE buffer holds
 * something), so both are kept current at all times.  The size is the smaller
 * of length and position, which is the position: a seek past the end extends
 * the length.  */
static void oms_term(register __oms_cookie *c)
{
	c->buf[c->eof] = 0;
	*c->sizeloc = c->pos;
}

/* Nothing to do here, as memstreams are write-only. */
/*  static ssize_t oms_read(void *cookie, char *buf, size_t bufsize) */
/*  { */
/*  } */

static ssize_t oms_write(register void *cookie, const char *buf, size_t bufsize)
{
	register char *newbuf;
	size_t count;

	/* Note: we already know bufsize < SSIZE_MAX... */

	count = COOKIE->len - COOKIE->pos - 1;
	assert(COOKIE->pos < COOKIE->len); /* Always nul-terminate! */

	if (bufsize > count) {
		newbuf = realloc(COOKIE->buf, COOKIE->len + bufsize - count);
		if (newbuf) {
			*COOKIE->bufloc = COOKIE->buf = newbuf;
			COOKIE->len += (bufsize - count);
		} else {
			bufsize = count;
			if (count == 0) {
				__set_errno(EFBIG);	/* TODO: check glibc errno setting... */
				return -1;
			}
		}
	}

	memcpy(COOKIE->buf + COOKIE->pos, buf, bufsize);
	COOKIE->pos += bufsize;

	if (COOKIE->pos > COOKIE->eof) {
		COOKIE->eof = COOKIE->pos;	/* Buffer defined this far. */
	}
	oms_term(COOKIE);

	return bufsize;
}

static int oms_seek(register void *cookie, __offmax_t *pos, int whence)
{
	__offmax_t p = *pos;
	register char *buf;
	size_t leastlen;

	/* Note: fseek already checks that whence is legal, so don't check here
	 * unless debugging. */
	assert(((unsigned int) whence) <= 2);

	/* POSIX leaves it implementation-defined whether SEEK_END refers to the
	 * buffer length or to the size an fflush() would report; glibc uses the
	 * latter, which for a write-only stream is the position, so both bases are
	 * the same here.  musl uses the former. */
	if (whence != SEEK_SET) {
		p += COOKIE->pos;
	}

	/* Note: glibc only allows seeking in the buffer.  We'll actually restrict
	 * to the data. */
	/* Check for offset < 0, offset >= too big (need nul), or overflow... */
	if (((uintmax_t) p) >= SIZE_MAX - 1) {
		__set_errno(EINVAL);
		return -1;
	}

	leastlen = ((size_t) p) + 1; /* New pos + 1 for nul if necessary. */

	if (leastlen >= COOKIE->len) { /* Need to grow buffer... */
		buf = realloc(COOKIE->buf, leastlen);
		if (buf) {
			*COOKIE->bufloc = COOKIE->buf = buf;
			COOKIE->len = leastlen;
			memset(buf + COOKIE->eof, 0, leastlen - COOKIE->eof); /* 0-fill */
		} else {
			/* TODO: check glibc errno setting... */
			return -1;
		}
	}

	*pos = COOKIE->pos = --leastlen;

	if (leastlen > COOKIE->eof) {	/* Seeked past the end: 0-fill the gap. */
		memset(COOKIE->buf + COOKIE->eof, 0, leastlen - COOKIE->eof);
		COOKIE->eof = leastlen;
	}
	oms_term(COOKIE);

	return 0;
}

static int oms_close(void *cookie)
{
	free(cookie);
	return 0;
}

#undef COOKIE

static const cookie_io_functions_t _oms_io_funcs = {
	NULL, oms_write, oms_seek, oms_close
};

/* TODO: If we have buffers enabled, it might be worthwile to add a pointer
 * to the FILE in the cookie and operate directly on the buffer itself
 * (ie replace the FILE buffer with the cookie buffer and update FILE bufstart,
 * etc. whenever we seek). */

FILE *open_memstream(char **bufloc, size_t *sizeloc)
{
	register __oms_cookie *cookie;
	register FILE *fp;

	if ((cookie = malloc(sizeof(__oms_cookie))) != NULL) {
		if ((cookie->buf = malloc(cookie->len = MEMSTREAM_BUFSIZ)) == NULL) {
			goto EXIT_cookie;
		}
		*cookie->buf = 0;		/* Set nul terminator for buffer. */
		*(cookie->bufloc = bufloc) = cookie->buf;
		*(cookie->sizeloc = sizeloc) = cookie->eof = cookie->pos = 0;

#ifndef __BCC__
		fp = fopencookie(cookie, "w", _oms_io_funcs);
#else
		fp = fopencookie(cookie, "w", &_oms_io_funcs);
#endif
		/* Note: We don't need to worry about locking fp in the thread case
		 * as the only possible access would be a close or flush with
		 * nothing currently in the FILE's write buffer. */

		if (fp != NULL) {
			/* POSIX: "The stream associated with a call to
			 * open_memstream() shall be byte-oriented."  fopencookie()
			 * leaves it unoriented. */
			__STDIO_STREAM_SET_NARROW(fp);
			__STDIO_STREAM_VALIDATE(fp);
			return fp;
		}

		free(cookie->buf);
	}

 EXIT_cookie:
	free(cookie);

	return NULL;
}
libc_hidden_def(open_memstream)
#endif
