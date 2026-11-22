/* stdc_first_leading_one_* (ISO C23 <stdbit.h>),
   from glibc's stdlib/stdc_first_leading_one_*.c.
   Copyright (C) 2024-2026 Free Software Foundation, Inc.
   The inline versions in the header do the work; these are the
   functions behind them.

   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#include <stdbit.h>

unsigned int
(stdc_first_leading_one_uc) (unsigned char x)
{
  return stdc_first_leading_one_uc (x);
}

unsigned int
(stdc_first_leading_one_us) (unsigned short x)
{
  return stdc_first_leading_one_us (x);
}

unsigned int
(stdc_first_leading_one_ui) (unsigned int x)
{
  return stdc_first_leading_one_ui (x);
}

unsigned int
(stdc_first_leading_one_ul) (unsigned long int x)
{
  return stdc_first_leading_one_ul (x);
}

unsigned int
(stdc_first_leading_one_ull) (unsigned long long int x)
{
  return stdc_first_leading_one_ull (x);
}
