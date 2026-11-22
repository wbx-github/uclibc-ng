/*
 * Copyright © 2005-2026 Rich Felker, et al.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Imported from musl C library, adapted to uClibc-ng
 */

#ifndef _UCHAR_H
#define _UCHAR_H

#ifdef __cplusplus
extern "C" {
#endif

#if __cplusplus < 201103L
typedef unsigned short char16_t;
typedef unsigned char32_t;
#endif

#include <wchar.h>

#ifdef __USE_ISOC23
# define __STDC_VERSION_UCHAR_H__	202311L
#endif

size_t c16rtomb(char *__restrict, char16_t, mbstate_t *__restrict);
size_t mbrtoc16(char16_t *__restrict, const char *__restrict, size_t, mbstate_t *__restrict);

size_t c32rtomb(char *__restrict, char32_t, mbstate_t *__restrict);
size_t mbrtoc32(char32_t *__restrict, const char *__restrict, size_t, mbstate_t *__restrict);

#if defined __USE_ISOC23 || defined __cpp_char8_t
# ifndef __cpp_char8_t
typedef unsigned char char8_t;
# endif
size_t c8rtomb(char *__restrict, char8_t, mbstate_t *__restrict);
size_t mbrtoc8(char8_t *__restrict, const char *__restrict, size_t, mbstate_t *__restrict);
#endif

#ifdef __cplusplus
}
#endif

#endif
