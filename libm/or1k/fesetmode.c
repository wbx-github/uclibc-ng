/* Install given floating-point control modes.  OpenRISC version.
   Copyright (C) 2024-2025 Free Software Foundation, Inc.

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
   <https://www.gnu.org/licenses/>.  */

#include <fenv.h>
#include <fpu_control.h>

int
fesetmode (const femode_t *modep)
{
  fpu_control_t cw;
  fpu_control_t cw_new;

  _FPU_GETCW (cw);
  cw_new = cw & ~_FPU_FPCSR_RM_MASK;
  if (modep == FE_DFL_MODE)
    cw_new |= (_FPU_DEFAULT & _FPU_FPCSR_RM_MASK);
  else
    cw_new |= (*modep & _FPU_FPCSR_RM_MASK);
  if (cw != cw_new)
    _FPU_SETCW (cw_new);

  /* Success.  */
  return 0;
}
