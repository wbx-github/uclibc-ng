/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>

#define JB_REGS   0
#define JB_DREGS  0
#define JB_AREGS  24
#define JB_PC     48
#define JB_FPREGS 52

/* JB_SIZE == sizeof(__jmp_buf): 12 regs + PC (= 52), plus the saved
   %fp2-%fp7 (6 * 12 bytes) when an FPU is present.  */
#if defined __HAVE_68881__ || defined __UCLIBC_HAS_FPU__
# define JB_SIZE 124
#else
# define JB_SIZE 52
#endif
