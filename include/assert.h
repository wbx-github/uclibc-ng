/* Copyright (C) 1991,1992,1994-2001,2003,2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/*
 *	ISO C99 Standard: 7.2 Diagnostics	<assert.h>
 */

#ifdef	_ASSERT_H

# undef	_ASSERT_H
# undef	assert
# undef __ASSERT_VOID_CAST
# undef __ASSERT_VARIADIC

#endif /* assert.h	*/

#define	_ASSERT_H	1
#include <features.h>

#if defined __cplusplus && __GNUC_PREREQ (2,95)
# define __ASSERT_VOID_CAST static_cast<void>
#else
# define __ASSERT_VOID_CAST (void)
#endif

/* C23 makes assert variadic, so that an expression with a comma outside
   parentheses (a compound literal, say) can be passed.  The sizeof of
   a call to __assert_single_arg, which is never defined, still refuses
   more than one expression.  Only for C, as in glibc.  */
#if defined __USE_ISOC23 && !defined __cplusplus
# define __ASSERT_VARIADIC 1
#else
# define __ASSERT_VARIADIC 0
#endif

/* void assert (int expression);

   If NDEBUG is defined, do nothing.
   If not, and EXPRESSION is zero, print an error message and abort.  */

#ifdef	NDEBUG

# if __ASSERT_VARIADIC
#  define assert(...)		(__ASSERT_VOID_CAST (0))
# else
#  define assert(expr)		(__ASSERT_VOID_CAST (0))
# endif

#else /* Not NDEBUG.  */

__BEGIN_DECLS

/* This prints an "Assertion failed" message and aborts.  */
extern void __assert(const char *, const char *, unsigned int, const char *)
	__THROW __attribute__ ((__noreturn__));
libc_hidden_proto(__assert)

# if __ASSERT_VARIADIC
extern _Bool __assert_single_arg (_Bool);
# endif

__END_DECLS

# if __ASSERT_VARIADIC
#  define assert(...) \
  ((void) sizeof (__assert_single_arg (__VA_ARGS__)),				\
   __ASSERT_VOID_CAST ((__VA_ARGS__) ? 0 :					\
		       (__assert (#__VA_ARGS__, __FILE__, __LINE__, __ASSERT_FUNCTION), 0)))
# else
#  define assert(expr) \
  (__ASSERT_VOID_CAST ((expr) ? 0 :								\
		       (__assert (__STRING(expr), __FILE__, __LINE__, __ASSERT_FUNCTION), 0)))
# endif

/* Version 2.4 and later of GCC define a magical variable `__PRETTY_FUNCTION__'
   which contains the name of the function currently being defined.
   This is broken in G++ before version 2.6.
   C9x has a similar variable called __func__, but prefer the GCC one since
   it demangles C++ function names.  */
# if defined __cplusplus ? __GNUC_PREREQ (2, 6) : __GNUC_PREREQ (2, 4)
#   define __ASSERT_FUNCTION	__PRETTY_FUNCTION__
# else
#  if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#   define __ASSERT_FUNCTION	__func__
#  else
#   define __ASSERT_FUNCTION	((const char *) 0)
#  endif
# endif

#endif /* NDEBUG.  */

#if (defined __USE_ISOC11				\
     && (!defined __STDC_VERSION__			\
	 || __STDC_VERSION__ <= 201710L			\
	 || !__GNUC_PREREQ (13, 0))			\
     && !defined __cplusplus)
# undef static_assert
# define static_assert _Static_assert
#endif

/* assert.h may be included again; the version stays defined.  */
#if defined __USE_ISOC23 && !defined __STDC_VERSION_ASSERT_H__
# define __STDC_VERSION_ASSERT_H__	202311L
#endif
