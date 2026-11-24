/*
 * strfmon and strfmon_l, POSIX.1-2008 (XSI).
 *
 * The layout of sign, currency symbol and padding follows C99 7.11.2.1
 * and gives the same results as glibc, except where a left precision
 * did not line up the columns: with '(' a positive value ends in a
 * space (as in FreeBSD), and the padding counts what is printed in
 * front of the number, not what would be with all of the parts.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <monetary.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* What a conversion needs from LC_MONETARY (and the decimal point
   from LC_NUMERIC); the one-character values are -1 if unset.  */
struct mon {
	const char *decimal_point;
	const char *int_curr_symbol;
	const char *currency_symbol;
	const char *mon_decimal_point;
	const char *mon_thousands_sep;
	const char *mon_grouping;
	const char *positive_sign;
	const char *negative_sign;
	signed char frac_digits[2];		/* national, international */
	signed char p_cs_precedes[2];
	signed char p_sep_by_space[2];
	signed char n_cs_precedes[2];
	signed char n_sep_by_space[2];
	signed char p_sign_posn[2];
	signed char n_sign_posn[2];
};

#ifdef __LOCALE_C_ONLY

static void get_mon(struct mon *m)
{
	memset(m, -1, sizeof(*m));
	m->decimal_point = ".";
	m->int_curr_symbol = m->currency_symbol = m->mon_decimal_point
		= m->mon_thousands_sep = m->mon_grouping = m->positive_sign
		= m->negative_sign = "";
}

#else

/* Unset is CHAR_MAX, or 0xff stored in a signed char.  */
static signed char val(const char *p)
{
	return *p < 0 || *p == CHAR_MAX ? -1 : *p;
}

static void get_mon(struct mon *m, const struct __uclibc_locale_struct *l)
{
	m->decimal_point = l->decimal_point;
	m->int_curr_symbol = l->int_curr_symbol;
	m->currency_symbol = l->currency_symbol;
	m->mon_decimal_point = l->mon_decimal_point;
	m->mon_thousands_sep = l->mon_thousands_sep;
	m->mon_grouping = l->mon_grouping;
	m->positive_sign = l->positive_sign;
	m->negative_sign = l->negative_sign;
	m->frac_digits[0] = val(l->frac_digits);
	m->frac_digits[1] = val(l->int_frac_digits);
	m->p_cs_precedes[0] = val(l->p_cs_precedes);
	m->p_cs_precedes[1] = val(l->int_p_cs_precedes);
	m->p_sep_by_space[0] = val(l->p_sep_by_space);
	m->p_sep_by_space[1] = val(l->int_p_sep_by_space);
	m->n_cs_precedes[0] = val(l->n_cs_precedes);
	m->n_cs_precedes[1] = val(l->int_n_cs_precedes);
	m->n_sep_by_space[0] = val(l->n_sep_by_space);
	m->n_sep_by_space[1] = val(l->int_n_sep_by_space);
	m->p_sign_posn[0] = val(l->p_sign_posn);
	m->p_sign_posn[1] = val(l->int_p_sign_posn);
	m->n_sign_posn[0] = val(l->n_sign_posn);
	m->n_sign_posn[1] = val(l->int_n_sign_posn);
}

#endif

/* LEN counts what the result would be; P only ever walks inside the
   buffer.  Moving P past END to measure the length would form a pointer
   outside the object, and on 32-bit it could wrap all the way round.  */
struct out {
	char *s;
	size_t max;
	size_t len;
};

static void put(struct out *o, char c)
{
	if (o->len < o->max)
		o->s[o->len] = c;
	/* Saturate: the length a caller asks for across all fields can
	   exceed what size_t holds, and wrapping would turn a result that
	   does not fit into a short one.  */
	if (o->len != (size_t) -1)
		o->len++;
}

static void puts_n(struct out *o, const char *s, size_t n)
{
	while (n--)
		put(o, *s++);
}

static void put_s(struct out *o, const char *s)
{
	puts_n(o, s, strlen(s));
}

static void pad(struct out *o, char c, long n)
{
	while (n-- > 0)
		put(o, c);
}

/* Whether a separator goes left of the digit that has K digits right
   of it, for GROUPING as in localeconv.  */
static int sep_at(const char *g, size_t k)
{
	size_t s = 0;

	for (;;) {
		if (*g <= 0 || *g == CHAR_MAX)
			return 0;
		s += *g;
		if (k <= s)
			return k == s;
		if (g[1] == 0)
			return (k - s) % *g == 0;
		g++;
	}
}

static size_t count_seps(const char *g, size_t digits)
{
	size_t k, n = 0;

	for (k = 1; k < digits; k++)
		n += sep_at(g, k);
	return n;
}

/* The number: NUM as printed by "%.*f", grouped and with the monetary
   decimal point, right-aligned with FILL in WIDTH characters.  */
static void put_number(struct out *o, const struct mon *m, const char *num,
		       int prec, int group, char fill, long width)
{
	const char *dp = *m->mon_decimal_point ? m->mon_decimal_point
					       : m->decimal_point;
	const char *ts = m->mon_thousands_sep;
	size_t digits = strspn(num, "0123456789");
	size_t seps = 0, total;

	if (digits == 0) {		/* inf or nan: glibc pads these with
					   spaces, whatever the fill is */
		pad(o, ' ', width - (long) strlen(num));
		put_s(o, num);
		return;
	}

	if (!group || !*ts)
		group = 0;
	else
		seps = count_seps(m->mon_grouping, digits);
	total = digits + seps + (prec ? 1 + prec : 0);
	pad(o, fill, width - (long) total);

	for (; digits; digits--) {
		put(o, *num++);
		if (group && digits > 1 && sep_at(m->mon_grouping, digits - 1))
			put_s(o, ts);
	}
	if (prec) {
		put_s(o, dp);
		put_s(o, num + strlen(num) - prec);
	}
}

/* sep_by_space 2 only makes sense if sign and symbol are adjacent.  */
static int fix_sep(int sep, int posn, int cs_precedes)
{
	if (sep < 0)
		return 0;
	if (sep == 2 && (posn == 0 || (posn == 1 && !cs_precedes)
			 || (posn == 2 && cs_precedes)))
		return 0;
	return sep;
}

/* Bytes in front of the number for one sign, as written below.  */
static int lead_bytes(int cs, int sep, int posn, int print_cur,
		      size_t cur_len, const char *sign, int paren)
{
	int n = 0;

	if (posn == 0)
		n += paren;
	if (cs) {
		if (posn == 1 || posn == 3)
			n += strlen(sign) + (sep == 2);
		if (print_cur)
			n += cur_len;
		if (posn == 4)
			n += (print_cur && sep == 2) + strlen(sign) + (sep == 1);
		else
			n += print_cur && sep == 1;
	} else if (posn == 1)
		n += strlen(sign);
	return n;
}

static ssize_t vstrfmon(char *s, size_t maxsize, const struct mon *m,
			const char *fmt, va_list ap)
{
	struct out o = { s, maxsize, 0 };

	while (*fmt) {
		char fill = ' ', cur_int[4], space_char, numbuf[64], *num;
		const char *cur, *sign, *osign;
		int group = 1, print_cur = 1, left = 0, i, is_ldbl = 0;
		int p_posn = -2, n_posn = -2, posn, oposn;
		int cs, ocs, sep, osep, negative, lpad, len, align;
		long width = 0, lprec = -1, rprec = -1;
		size_t cur_len;
		size_t start;
		long double v;

		if (*fmt != '%' || *++fmt == '%') {
			put(&o, *fmt++);
			continue;
		}

		for (;; fmt++) {
			if (*fmt == '=') {
				if (!(fill = *++fmt))
					goto inval;
			} else if (*fmt == '^')
				group = 0;
			else if (*fmt == '+' || *fmt == '(') {
				if (n_posn != -2)
					goto inval;
				/* -1: from the locale, but '(' excluded.  */
				p_posn = n_posn = *fmt == '(' ? 0 : -1;
			} else if (*fmt == '!')
				print_cur = 0;
			else if (*fmt == '-')
				left = 1;
			else
				break;
		}

		while (*fmt >= '0' && *fmt <= '9') {
			if (width > (LONG_MAX - 9) / 10)
				goto toobig;
			width = width * 10 + (*fmt++ - '0');
		}
		if (*fmt == '#') {
			if (*++fmt < '0' || *fmt > '9')
				goto inval;
			for (lprec = 0; *fmt >= '0' && *fmt <= '9'; ) {
				if (lprec > (INT_MAX / 2 - 9) / 10)
					goto toobig;
				lprec = lprec * 10 + (*fmt++ - '0');
			}
		}
		if (*fmt == '.') {
			if (*++fmt < '0' || *fmt > '9')
				goto inval;
			for (rprec = 0; *fmt >= '0' && *fmt <= '9'; ) {
				if (rprec > (INT_MAX / 2 - 9) / 10)
					goto toobig;
				rprec = rprec * 10 + (*fmt++ - '0');
			}
		}
		if (*fmt == 'L') {
			is_ldbl = 1;
			fmt++;
		}

		if (*fmt == 'i') {
			i = 1;
			strncpy(cur_int, m->int_curr_symbol, 3);
			cur_int[3] = '\0';
			cur = cur_int;
			space_char = strlen(m->int_curr_symbol) > 3
				? m->int_curr_symbol[3] : ' ';
		} else if (*fmt == 'n') {
			i = 0;
			cur = m->currency_symbol;
			space_char = ' ';
		} else
			goto inval;
		fmt++;

		/* The field is padded to width and carries at least lprec
		   digits, so a result that long cannot fit with its nul.
		   Refusing it here keeps the padding loops short.  After
		   the conversion character, so that a bad one still gives
		   EINVAL rather than E2BIG.  */
		if ((unsigned long) width >= maxsize
		    || (lprec >= 0 && (unsigned long) lprec >= maxsize)
		    || (rprec >= 0 && (unsigned long) rprec >= maxsize))
			goto toobig;

		cur_len = strlen(cur);

		if (p_posn != 0) {
			p_posn = m->p_sign_posn[i];
			n_posn = m->n_sign_posn[i];
		}
		if (rprec < 0) {
			rprec = m->frac_digits[i];
			if (rprec < 0)
				rprec = 2;
		}

		if (is_ldbl)
			v = va_arg(ap, long double);
		else
			v = va_arg(ap, double);
		/* signbit, not v < 0: -0.0 and a value that rounds to it
		   carry the negative sign in glibc too.  */
		negative = signbit(v);
		if (negative)
			v = -v;

		if (negative) {
			sign = *m->negative_sign ? m->negative_sign : "-";
			osign = m->positive_sign;
			cs = m->n_cs_precedes[i];
			sep = m->n_sep_by_space[i];
			posn = n_posn;
			ocs = m->p_cs_precedes[i];
			osep = m->p_sep_by_space[i];
			oposn = p_posn;
		} else {
			sign = m->positive_sign;
			osign = *m->negative_sign ? m->negative_sign : "-";
			cs = m->p_cs_precedes[i];
			sep = m->p_sep_by_space[i];
			posn = p_posn;
			ocs = m->n_cs_precedes[i];
			osep = m->n_sep_by_space[i];
			oposn = n_posn;
		}
		cs = cs != 0;
		ocs = ocs != 0;
		if (posn < 0)
			posn = 1;
		if (oposn < 0)
			oposn = 1;
		sep = fix_sep(sep, posn, cs);
		osep = fix_sep(osep, oposn, ocs);

		/* With #n, the digits line up whatever the sign is.  */
		lpad = 0;
		align = lprec >= 0;
		if (align) {
			int a = lead_bytes(cs, sep, posn, print_cur, cur_len, sign,
					   negative);
			int b = lead_bytes(ocs, osep, oposn, print_cur, cur_len,
					   osign, !negative);

			if (b > a)
				lpad = b - a;
			if (group && *m->mon_thousands_sep)
				lprec += count_seps(m->mon_grouping, lprec);
		} else
			lprec = 0;

		num = numbuf;
		len = snprintf(num, sizeof(numbuf), "%.*Lf", (int) rprec, v);
		if (len < 0)
			return -1;
		if ((size_t) len >= sizeof(numbuf)) {
			num = malloc(len + 1);
			if (num == NULL)
				return -1;
			snprintf(num, len + 1, "%.*Lf", (int) rprec, v);
		}

		start = o.len;
		pad(&o, ' ', lpad);
		if (posn == 0 && negative)
			put(&o, '(');
		if (cs) {
			if (posn == 1 || posn == 3) {
				put_s(&o, sign);
				if (sep == 2)
					put(&o, ' ');
			}
			if (print_cur)
				put_s(&o, cur);
			if (posn == 4) {
				if (print_cur && sep == 2)
					put(&o, space_char);
				put_s(&o, sign);
				if (sep == 1)
					put(&o, ' ');
			} else if (print_cur && sep == 1)
				put(&o, space_char);
		} else if (posn == 1)
			put_s(&o, sign);

		put_number(&o, m, num, rprec, group, fill,
			   lprec + (rprec ? rprec + 1 : 0));
		if (num != numbuf)
			free(num);

		if (!cs) {
			if (posn == 3) {
				if (sep == 1)
					put(&o, ' ');
				put_s(&o, sign);
			}
			if (print_cur) {
				if (sep == (posn == 3 ? 2 : 1))
					put(&o, space_char);
				put_s(&o, cur);
			}
			if (posn == 4) {
				if (sep == 2)
					put(&o, ' ');
				put_s(&o, sign);
			}
		}
		if (posn == 2) {
			if (sep == 2)
				put(&o, ' ');
			put_s(&o, sign);
		}
		if (posn == 0 && negative)
			put(&o, ')');
		else if (posn == 0 && align)
			put(&o, ' ');		/* where the ')' would be */

		if (o.len - start < (size_t) width) {
			long n = width - (long) (o.len - start);

			pad(&o, ' ', n);
			/* Strictly less: at o.len == maxsize the result
			   has no room for the nul and fails below, and
			   o.len can be saturated, which would make the
			   length below wrap.  */
			if (!left && o.len < maxsize) {
				memmove(s + start + n, s + start,
					o.len - start - n);
				memset(s + start, ' ', n);
			}
		}
	}

	if (o.len >= maxsize)
		goto toobig;
	s[o.len] = '\0';
	return (ssize_t) o.len;

 toobig:
	__set_errno(E2BIG);
	return -1;
 inval:
	__set_errno(EINVAL);
	return -1;
}

ssize_t strfmon(char *__restrict s, size_t maxsize,
		const char *__restrict format, ...)
{
	struct mon m;
	va_list ap;
	ssize_t r;

#ifdef __LOCALE_C_ONLY
	get_mon(&m);
#else
	get_mon(&m, __UCLIBC_CURLOCALE);
#endif
	va_start(ap, format);
	r = vstrfmon(s, maxsize, &m, format, ap);
	va_end(ap);
	return r;
}

#ifdef __UCLIBC_HAS_XLOCALE__
ssize_t strfmon_l(char *__restrict s, size_t maxsize, locale_t loc,
		  const char *__restrict format, ...)
{
	struct mon m;
	va_list ap;
	ssize_t r;

	get_mon(&m, loc);
	va_start(ap, format);
	r = vstrfmon(s, maxsize, &m, format, ap);
	va_end(ap);
	return r;
}
#endif
