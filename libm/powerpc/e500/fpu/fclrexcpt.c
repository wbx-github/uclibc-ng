/* Clear given exceptions in current floating-point environment.  e500 version.
   Copyright (C) 2004-2019 Free Software Foundation, Inc.

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


#include "fenv_libc.h"

int
feclearexcept (int excepts)
{
  unsigned int fpescr;

  fpescr = fegetenv_register ();
  fpescr &= ~(excepts & FE_ALL_EXCEPT);
  fesetenv_register (fpescr);

  /* Let the kernel know if the "invalid" or "underflow" bit was
     cleared.  */
  if (excepts & (FE_INVALID | FE_UNDERFLOW))
    __fe_note_change ();

  return 0;
}
