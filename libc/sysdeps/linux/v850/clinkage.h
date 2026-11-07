/*
 * libc/sysdeps/linux/v850/clinkage.h -- Macros for C symbols in assembler
 *
 *  Copyright (C) 2001  NEC Corporation
 *  Copyright (C) 2001  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 */

#ifndef __V850_CLINKAGE_H__
#define __V850_CLINKAGE_H__

#define macrology_paste(arg1, arg2)	macrology_paste_1(arg1, arg2)
#define macrology_paste_1(arg1, arg2)	arg1 ## arg2
#define macrology_stringify(sym)	macrology_stringify_1(sym)
#define macrology_stringify_1(sym)	#sym

#define G_ENTRY(name)							      \
   .balign 4;								      \
   .globl name;								      \
   .type  name,@function;						      \
   name
#define G_DATA(name)							      \
   .globl name;								      \
   .type  name,@object;							      \
   name
#define END(name)							      \
   .size  name,.-name

#define L_ENTRY(name)							      \
   .balign 4;								      \
   .type  name,@function;						      \
   name
#define L_DATA(name)							      \
   .type  name,@object;							      \
   name

#define C_SYMBOL_NAME(name) 	macrology_paste(_, name)
#define C_SYMBOL_STRING(name)	macrology_stringify(C_SYMBOL_NAME(name))
#define C_ENTRY(name)		G_ENTRY(C_SYMBOL_NAME(name))
#define C_DATA(name)		G_DATA(C_SYMBOL_NAME(name))
#define C_END(name)		END(C_SYMBOL_NAME(name))

#endif /* __V850_CLINKAGE_H__ */
