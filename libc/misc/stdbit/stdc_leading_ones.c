/* stdc_leading_ones_* (ISO C23 <stdbit.h>),
   from glibc's stdlib/stdc_leading_ones_*.c.
   Copyright (C) 2024-2026 Free Software Foundation, Inc.
   The inline versions in the header do the work; these are the
   functions behind them.

   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#include <stdbit.h>

unsigned int
(stdc_leading_ones_uc) (unsigned char x)
{
  return stdc_leading_ones_uc (x);
}

unsigned int
(stdc_leading_ones_us) (unsigned short x)
{
  return stdc_leading_ones_us (x);
}

unsigned int
(stdc_leading_ones_ui) (unsigned int x)
{
  return stdc_leading_ones_ui (x);
}

unsigned int
(stdc_leading_ones_ul) (unsigned long int x)
{
  return stdc_leading_ones_ul (x);
}

unsigned int
(stdc_leading_ones_ull) (unsigned long long int x)
{
  return stdc_leading_ones_ull (x);
}
