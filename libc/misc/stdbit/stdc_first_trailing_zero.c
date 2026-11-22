/* stdc_first_trailing_zero_* (ISO C23 <stdbit.h>),
   from glibc's stdlib/stdc_first_trailing_zero_*.c.
   Copyright (C) 2024-2026 Free Software Foundation, Inc.
   The inline versions in the header do the work; these are the
   functions behind them.

   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#include <stdbit.h>

unsigned int
(stdc_first_trailing_zero_uc) (unsigned char x)
{
  return stdc_first_trailing_zero_uc (x);
}

unsigned int
(stdc_first_trailing_zero_us) (unsigned short x)
{
  return stdc_first_trailing_zero_us (x);
}

unsigned int
(stdc_first_trailing_zero_ui) (unsigned int x)
{
  return stdc_first_trailing_zero_ui (x);
}

unsigned int
(stdc_first_trailing_zero_ul) (unsigned long int x)
{
  return stdc_first_trailing_zero_ul (x);
}

unsigned int
(stdc_first_trailing_zero_ull) (unsigned long long int x)
{
  return stdc_first_trailing_zero_ull (x);
}
