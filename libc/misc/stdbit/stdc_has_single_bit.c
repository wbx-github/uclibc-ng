/* stdc_has_single_bit_* (ISO C23 <stdbit.h>),
   from glibc's stdlib/stdc_has_single_bit_*.c.
   Copyright (C) 2024-2026 Free Software Foundation, Inc.
   The inline versions in the header do the work; these are the
   functions behind them.

   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#include <stdbit.h>

_Bool
(stdc_has_single_bit_uc) (unsigned char x)
{
  return stdc_has_single_bit_uc (x);
}

_Bool
(stdc_has_single_bit_us) (unsigned short x)
{
  return stdc_has_single_bit_us (x);
}

_Bool
(stdc_has_single_bit_ui) (unsigned int x)
{
  return stdc_has_single_bit_ui (x);
}

_Bool
(stdc_has_single_bit_ul) (unsigned long int x)
{
  return stdc_has_single_bit_ul (x);
}

_Bool
(stdc_has_single_bit_ull) (unsigned long long int x)
{
  return stdc_has_single_bit_ull (x);
}
