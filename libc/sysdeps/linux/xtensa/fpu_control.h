/* FPU control word bits.  Xtensa version.
   Copyright (C) 2026 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#ifndef _FPU_CONTROL_H
#define _FPU_CONTROL_H

/* Xtensa FPU floating point control register bits.
 *
 *  6     -> enable exception for invalid exception
 *  5     -> enable exception for division by zero exception
 *  4     -> enable exception for overflow exception
 *  3     -> enable exception for underflow exception
 *  2     -> enable exception for inexact exception
 *  1-0   -> rounding control
 *
 * Rounding Control:
 * 00 - rounding to nearest (RN)
 * 01 - rounding toward zero (RZ)
 * 10 - rounding (up) toward plus infinity (RP)
 * 11 - rounding (down)toward minus infinity (RM)
 */

#include <features.h>

#if __XCHAL_HAVE_FP || __XCHAL_HAVE_DFP

/* Masks for interrupts.  */
#define _FPU_MASK_V             0x0040  /* Invalid operation */
#define _FPU_MASK_Z             0x0020  /* Division by zero  */
#define _FPU_MASK_O             0x0010  /* Overflow          */
#define _FPU_MASK_U             0x0008  /* Underflow         */
#define _FPU_MASK_I             0x0004  /* Inexact operation */

/* Rounding control.  */
#define _FPU_RC_NEAREST         0x0     /* RECOMMENDED */
#define _FPU_RC_ZERO            0x1
#define _FPU_RC_UP              0x2
#define _FPU_RC_DOWN            0x3
/* Mask for rounding control.  */
#define _FPU_RC_MASK            0x3

#define _FPU_RESERVED           0xfffff000  /* Reserved bits in FCR.  */
#define _FPU_FPSR_RESERVED      0xfffff000  /* Reserved bits in FSR.  */

#define _FPU_DEFAULT            0x00000000
#define _FPU_FPSR_DEFAULT       0x00000000
/* IEEE: same as above, but exceptions.  */
#define _FPU_FPCR_IEEE          0x00000F80
#define _FPU_FPSR_IEEE          0x00000000

typedef unsigned int fpu_control_t;
typedef unsigned int fpu_fpsr_t;

/* Macros for accessing the hardware control word.  */
#define _FPU_GETCW(cw) __asm__ volatile ("rur.fcr %0" : "=a" (cw))
#define _FPU_SETCW(cw) __asm__ volatile ("wur.fcr %0" : : "r" (cw))
#define _FPU_GETFPSR(fpsr) __asm__ volatile ("rur.fsr %0" : "=a" (fpsr))
#define _FPU_SETFPSR(fpsr) __asm__ volatile ("wur.fsr %0" : : "r" (fpsr))

/* Default control word set at startup.  */
extern fpu_control_t __fpu_control;

#else /* __XTENSA_HAVE_FP || __XTENSA_HAVE_DFP */

#define _FPU_RC_MASK            0x0
#define _FPU_RESERVED           0xffffffff
#define _FPU_FPSR_RESERVED      0xffffffff
#define _FPU_DEFAULT            0x00000000
#define _FPU_FPSR_DEFAULT       0x00000000
#define _FPU_FPCR_IEEE          0x00000000
#define _FPU_FPSR_IEEE          0x00000000

typedef unsigned int fpu_control_t;
typedef unsigned int fpu_fpsr_t;

#define _FPU_GETCW(cw)          (cw) = 0
#define _FPU_SETCW(cw)          (void) (cw)
#define _FPU_GETFPSR(fpsr)      (fpsr) = 0
#define _FPU_SETFPSR(fpsr)      (void) (fpsr)

extern fpu_control_t __fpu_control;

#endif /* __XTENSA_HAVE_FP || __XTENSA_HAVE_DFP */

#endif	/* fpu_control.h */
