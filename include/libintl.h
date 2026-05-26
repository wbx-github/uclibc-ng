#ifndef LIBINTL_H
#define LIBINTL_H

/* BBS: __attribute__((format_arg(N))) tells GCC that this function
   returns a format-string equivalent to argument N.  -Wformat-security
   uses this to see through gettext/dgettext/... calls and detect that
   the msgid argument was a string literal.  glibc's libintl.h ships the
   same annotation; without it, programs that build with
   -Werror=format-security (e.g. distro-style hardening flags) would
   fail on every printf(_("fmt %d"), n).  */
#if defined __GNUC__ && __GNUC__ >= 3
# define __libintl_format_arg(N) __attribute__ ((__format_arg__ (N)))
#else
# define __libintl_format_arg(N)
#endif

char *gettext(const char *msgid)
	__libintl_format_arg(1);
char *dgettext(const char *domainname, const char *msgid)
	__libintl_format_arg(2);
char *dcgettext(const char *domainname, const char *msgid, int category)
	__libintl_format_arg(2);
char *ngettext(const char *msgid1, const char *msgid2, unsigned long n)
	__libintl_format_arg(1) __libintl_format_arg(2);
char *dngettext(const char *domainname, const char *msgid1, const char *msgid2, unsigned long n)
	__libintl_format_arg(2) __libintl_format_arg(3);
char *dcngettext(const char *domainname, const char *msgid1, const char *msgid2, unsigned long n, int category)
	__libintl_format_arg(2) __libintl_format_arg(3);

char *textdomain(const char *domainname);
char *bind_textdomain_codeset(const char *domainname, const char *codeset);
char *bindtextdomain(const char *domainname, const char *dirname);

/* BBS: gettext_noop is just compile-time identity; many GNU programs
   (e.g. elfutils' lib/system.h line 157) ship
       #define gettext_noop(Str) Str
   verbatim.  Use the same token form so a redefinition is recognised
   as identical by the preprocessor and does not trigger
   -Werror=macro-redefined.  Also gate with #ifndef so a consumer that
   defines it first keeps its definition (which the previous
   "#undef gettext_noop" would have wiped).  */
#ifndef gettext_noop
#define gettext_noop(Str) Str
#endif

#ifndef LIBINTL_NO_MACROS
/* if these macros are defined, configure checks will detect libintl as
 * built into the libc because test programs will work without -lintl.
 * for example:
 * checking for ngettext in libc ... yes
 * the consequence is that -lintl will not be added to the LDFLAGS.
 * so if for some reason you want that libintl.a gets linked,
 * add -DLIBINTL_NO_MACROS=1 to your CPPFLAGS.
 *
 * BBS: the previous implementation expanded to comma expressions like
 *   #define dgettext(dom, X) ((void)(dom), (char*) (X))
 * which silenced -Wunused-value on the unused dom argument but defeated
 * -Wformat-security: GCC cannot see through a comma expression to
 * decide that the result is still a string literal.  Route through
 * static-inline stubs decorated with __attribute__((format_arg))
 * instead:
 *   - function-call semantics evaluate every argument, so side effects
 *     in callers like dgettext(read_dom(), "fmt") are preserved;
 *   - the inline body collapses to (char*) msgid at -O1+ for literal
 *     msgid, so there is zero runtime overhead in the common case;
 *   - format_arg tells -Wformat-security how to follow the call back
 *     to the original literal.
 * The static-inline stub names match the convention __libintl_stub_*.
 * The user-visible macros shadow the extern declarations only at call
 * sites (foo(...)), so taking &gettext still resolves to the extern
 * function declared above.  */

static inline __libintl_format_arg(1)
char *__libintl_stub_gettext(const char *__msgid)
{
	return (char *) __msgid;
}
#define gettext(X) __libintl_stub_gettext(X)

static inline __libintl_format_arg(2)
char *__libintl_stub_dgettext(const char *__dom, const char *__msgid)
{
	(void) __dom;
	return (char *) __msgid;
}
#define dgettext(dom, X) __libintl_stub_dgettext((dom), (X))

static inline __libintl_format_arg(2)
char *__libintl_stub_dcgettext(const char *__dom, const char *__msgid, int __cat)
{
	(void) __dom; (void) __cat;
	return (char *) __msgid;
}
#define dcgettext(dom, X, cat) __libintl_stub_dcgettext((dom), (X), (cat))

static inline __libintl_format_arg(1) __libintl_format_arg(2)
char *__libintl_stub_ngettext(const char *__s1, const char *__s2, unsigned long __n)
{
	return (char *) ((__n == 1) ? __s1 : __s2);
}
#define ngettext(X, Y, N) __libintl_stub_ngettext((X), (Y), (N))

static inline __libintl_format_arg(2) __libintl_format_arg(3)
char *__libintl_stub_dngettext(const char *__dom, const char *__s1, const char *__s2, unsigned long __n)
{
	(void) __dom;
	return (char *) ((__n == 1) ? __s1 : __s2);
}
#define dngettext(dom, X, Y, N) __libintl_stub_dngettext((dom), (X), (Y), (N))

static inline __libintl_format_arg(2) __libintl_format_arg(3)
char *__libintl_stub_dcngettext(const char *__dom, const char *__s1, const char *__s2, unsigned long __n, int __cat)
{
	(void) __dom; (void) __cat;
	return (char *) ((__n == 1) ? __s1 : __s2);
}
#define dcngettext(dom, X, Y, N, cat) __libintl_stub_dcngettext((dom), (X), (Y), (N), (cat))

static inline char *__libintl_stub_bindtextdomain(const char *__dom, const char *__dir)
{
	(void) __dom; (void) __dir;
	return (char *) "/";
}
#define bindtextdomain(X, Y) __libintl_stub_bindtextdomain((X), (Y))

static inline char *__libintl_stub_bind_textdomain_codeset(const char *__dom, const char *__codeset)
{
	(void) __dom; (void) __codeset;
	return (char *) 0;
}
#define bind_textdomain_codeset(dom, codeset) __libintl_stub_bind_textdomain_codeset((dom), (codeset))

static inline char *__libintl_stub_textdomain(const char *__dom)
{
	(void) __dom;
	return (char *) "messages";
}
#define textdomain(X) __libintl_stub_textdomain(X)

#undef ENABLE_NLS
#undef DISABLE_NLS
#define DISABLE_NLS 1

/* BBS: the previous version had
 *   #pragma GCC diagnostic ignored "-Wunused-value"
 * here because the comma-expression macros produced a (char*)"..."
 * trailing rvalue that callers discarded as a statement.  With
 * function-call semantics (static-inline stubs) the discarded return
 * value is a regular function call statement, not an unused expression
 * value, so -Wunused-value never fires and the pragma is no longer
 * needed.  */

#endif

/* to supply LC_MESSAGES and other stuff GNU expects to be exported when
   including libintl.h */
#include <locale.h>

#endif

