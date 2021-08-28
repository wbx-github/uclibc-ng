/*
 * libc/sysdeps/linux/v850/crt1.S -- Initial program entry point for linux/v850
 *
 *  Copyright (C) 2001,02,03  NEC Electronics Corporation
 *  Copyright (C) 2001,02,03  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 */

#undef C_SYMBOL_NAME
#include "clinkage.h"

/* Upon entry, the stack contains the following data:
	argc, argv[0], ..., argv[argc-1], 0, envp[0], ..., 0
*/

	.text
C_ENTRY(start):
	mov	hilo(C_SYMBOL_NAME(main)), r6	# Arg 0: main
	ld.w	0[sp], r7			# Arg 1: argc
	addi	4, sp, r8			# Arg 2: argv
	mov	0, r9				# Arg 3: app_init (NULL)
	add	-12, sp				# Room for Arg 4 to 6
	st.w	r0, 0[sp]			# Arg 4: app_fini (NULL)
	st.w	r0, 4[sp]			# Arg 5: rtld_fini (NULL)
	st.w	r0, 8[sp]			# Arg 6: stack_end (NULL)

	// Load CTBP register
	mov	hilo(C_SYMBOL_NAME(_ctbp)), r19
	ldsr	r19, ctbp

	// Load GP
	mov	hilo(C_SYMBOL_NAME(_gp)), gp

	// tail-call uclibc's startup routine
	addi	-24, sp, sp		// Stack space reserved for args
	jr	C_SYMBOL_NAME(__uClibc_main)


/* Stick in a dummy reference to `main', so that if an application
   is linking when the `main' function is in a static library (.a)
   we can be sure that `main' actually gets linked in.  */
L_dummy_main_reference:
	.long	C_SYMBOL_NAME(main)

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start

/* The linker doesn't generate the __{preinit,init,fini}_array_{start,end}
   symbols expected by __uClibc_main, so provide dummy versions here.  */

	.text
	.globl	C_SYMBOL_NAME(__preinit_array_start)
C_SYMBOL_NAME(__preinit_array_start):
	.globl	C_SYMBOL_NAME(__preinit_array_end)
C_SYMBOL_NAME(__preinit_array_end):
	.globl	C_SYMBOL_NAME(__init_array_start)
C_SYMBOL_NAME(__init_array_start):
	.globl	C_SYMBOL_NAME(__init_array_end)
C_SYMBOL_NAME(__init_array_end):
	.globl	C_SYMBOL_NAME(__fini_array_start)
C_SYMBOL_NAME(__fini_array_start):
	.globl	C_SYMBOL_NAME(__fini_array_end)
C_SYMBOL_NAME(__fini_array_end):

.section .note.GNU-stack,"",%progbits
