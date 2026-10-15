/* Copyright (C) 1998-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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

#include "math.h"
#include "math_private.h"
#include <float.h>

/* ln(10) as two doubles: LN10_HI + LN10_LO is ln(10) to 106 bits, and
   LN10_HH + LN10_HL is LN10_HI split in halves so a product with a split
   argument is exact.  */
static const double LN10_HI = 0x1.26bb1bbb55516p+1;
static const double LN10_LO = -0x1.f48ad494ea3e9p-53;
static const double LN10_HH = 0x1.26bb1b8p+1;
static const double LN10_HL = 0x1.daaa8bp-26;

double __ieee754_exp10 (double arg)
{
  double p, e, corr, r, t, xh, xl;

  if (isfinite (arg) && arg < DBL_MIN_10_EXP - DBL_DIG - 10)
    return DBL_MIN * DBL_MIN;

  /* arg * LN10_HI as the exact sum p + e, by Dekker's product: the halves of
     each factor carry 26 bits, so xh * LN10_HH fits in a double, and the
     subtraction of p from it is exact.  */
  p = arg * LN10_HI;
  t = arg * (0x1p27 + 1.0);
  xh = t - (t - arg);
  xl = arg - xh;
  e = ((xh * LN10_HH - p) + xh * LN10_HL + xl * LN10_HH) + xl * LN10_HL;

  /* What the rounded product lost, plus the tail of ln(10).  */
  corr = e + arg * LN10_LO;

  r = __ieee754_exp (p);
  if (!isfinite (r) || r == 0)
    return r;
  return r + r * corr;
}
