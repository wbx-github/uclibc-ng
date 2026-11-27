/* Note a change to floating-point exceptions.
   Copyright (C) 2013-2019 Free Software Foundation, Inc.

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
#include <syscall.h>

/* Setting PR_FP_EXC_SW_ENABLE makes the kernel take the current SPEFSCR
   as the sticky state it must keep when emulating an instruction.  */
void
__fe_note_change (void)
{
  int pflags, r;
  INTERNAL_SYSCALL_DECL (err);

  r = INTERNAL_SYSCALL (prctl, err, 2, PR_GET_FPEXC, &pflags);
  if (INTERNAL_SYSCALL_ERROR_P (r, err))
    return;
  if ((pflags & PR_FP_EXC_SW_ENABLE) == 0)
    INTERNAL_SYSCALL (prctl, err, 2, PR_SET_FPEXC,
		      pflags | PR_FP_EXC_SW_ENABLE);
}
