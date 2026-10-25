/* Startup code for SH & ELF.
   Copyright (C) 1999 Free Software Foundation, Inc.
   Copyright (C) 2001 Hewlett-Packard Australia
   Copyright (C) 2002 Stefan Allius

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU Library General Public License as published by the Free
 Software Foundation; either version 2 of the License, or (at your option) any
 later version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for more
 details.

 You should have received a copy of the GNU Library General Public License
 along with this program; if not, see <http://www.gnu.org/licenses/>.
*/


/* This is the canonical entry point, usually the first thing in the text
   segment.

	At this entry point, most registers' values are unspecified, except:

   r4	Contains a function pointer to be registered with `atexit'.
		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs.
		For FDPIC, this will be a function descriptor.

   sp		The stack contains the arguments and environment:
   		0(sp)			argc
		4(sp)			argv[0]
		...
		(4*argc)(sp)		NULL
		(4*(argc+1))(sp)	envp[0]
		...
					NULL

	FDPIC only:

   r8	Contains the address of the load map.
		The type is "const struct elf32_fdpic_loadmap *".
		See sh/bits/elf-fdpic.h.
*/

#include <features.h>

	.text
	.globl _start
	.type _start,%function
	.type main,%function
_start:
	/* Clear the frame pointer since this is the outermost frame. */
	mov #0, r14

#if defined __ARCH_HAS_NO_MMU__ && !defined __SH_FDPIC__
	/* ucLinux doesn't seem to provide a valid loader finalization
	   function pointer.  */
	mov #0, r4
#endif

#ifdef __SH_FDPIC__
	mov   r4, r10  /* move to callee save */

	/* GOT = __self_reloc (load_map, rofixup, rofixupend)  */
	mov   r8, r4
	mova  L_rofixup, r0
	mov.l L_rofixup, r5
	add   r0, r5
	mova  L_rofixupend, r0
	mov.l L_rofixupend, r6
	add   r0, r6

	mova  L_self_reloc, r0
	mov.l L_self_reloc, r1
	add   r0, r1
	jsr   @r1
	  nop
	mov   r0, r12
	/* r12 now contains the GOT/FDPIC address.  */

#ifdef __SH2A_DOUBLE__
	/* The SH2A kernel doesn't initialize the FPSCR to double precision.  */
	mov   r12, r13

#ifdef __FMOVD_ENABLED__
	movi20s #0x180000, r4
#else
	movi20s #0x80000, r4
#endif
	mov.l L_set_fpscr, r0
	mov.l @(r0,r12), r1
	mov.l @(0,r1), r0
	jsr   @r0
	 mov.l @(4,r1), r12

	mov   r13, r12
#endif

	/* Pop argc off the stack and save a pointer to argv */
	mov.l @r15+,r5
	mov   r15, r6

	/* In FDPIC, the GOT register is caller save.  */
	mov.l r12, @-r15

	/* Push the stack_end, rtld_fini and fini func onto the stack */
	mov.l r6,@-r15
	mov.l r10,@-r15
	mov.l L_fini,r0
	mov.l @(r0,r12),r0	/* descriptor pointer for _fini */
	mov.l r0,@-r15

	/* Set up the main/init funcs that go in registers */
	mov.l L_main,r0
	mov.l @(r0,r12),r4	/* descriptor pointer for main */
	mov.l L_init,r0
	mov.l @(r0,r12),r7	/* descriptor pointer for _init */

	/* __uClibc_main (main, argc, argv, init, fini, rtld_fini, stack_end) */

	/* Let the libc call main and exit with its return code.  */
	mov.l L_uClibc_main,r0
	mov.l @(r0,r12), r0
	mov.l @(0,r0),r1
	jsr   @r1
	  mov.l @(4,r0), r12

	/* We should not get here. */
	add   #12, r15		/* Pop parameters from stack.  */
	mov.l @r15+, r12
	mov.l L_abort,r0
	mov.l @(r0,r12), r0
	mov.l @(0,r0),r1
	jsr   @r1
	  mov.l @(4,r0), r12

#elif defined __PIC__
	/* Pop argc off the stack and save a pointer to argv */
	mov.l @r15+,r5
	mov r15, r6

	/* Push the stack_end, rtld_fini and fini func onto the stack */
	mov.l r6,@-r15
	mov.l r4,@-r15

	mova  L_got, r0
	mov.l L_got, r12
	add r0, r12

	mov.l L_fini,r0
	add r12, r0
	mov.l r0,@-r15

	/* Set up the main/init funcs that go in registers */
	mov.l L_main, r4
	add r12, r4
	mov.l L_init, r7
	add r12, r7

	/* __uClibc_main (main, argc, argv, init, fini, rtld_fini, stack_end) */

	/* Let the libc call main and exit with its return code.  */
	mov.l L_uClibc_main,r0
	mov.l @(r0,r12),r1
	jsr @r1
	nop
	/* We should not get here. */
	mov.l L_abort,r0
	mov.l @(r0,r12),r1
	jsr @r1
	nop
#else
	/* Pop argc off the stack and save a pointer to argv */
	mov.l @r15+,r5
	mov r15, r6

	/* Push the stack_end, rtld_fini and fini func onto the stack */
	mov.l r6,@-r15
	mov.l r4,@-r15

	mov.l L_fini,r0
	mov.l r0,@-r15

	/* Set up the main/init funcs that go in registers */
	mov.l L_main,r4
	mov.l L_init,r7

	/* __uClibc_main (main, argc, argv, init, fini, rtld_fini, stack_end) */

	/* Let the libc call main and exit with its return code.  */
	mov.l L_uClibc_main,r1
	jsr @r1
	nop
	/* We should not get here. */
	mov.l L_abort,r1
	jmp @r1
	nop
#endif

	.size _start,.-_start
	.align	2

#ifdef __SH_FDPIC__
/* These are used before the FDPIC register is set, so must be relative.
   The functions must be implemented without using r12.  */
L_self_reloc:
	.long   __self_reloc@PCREL
L_rofixup:
	.long   __ROFIXUP_LIST__@PCREL
L_rofixupend:
	.long   __ROFIXUP_END__@PCREL

/* These must use FDPIC relocations.  */
#ifdef __SH2A_DOUBLE__
L_set_fpscr:
	.long	__set_fpscr@GOTFUNCDESC
#endif
L_main:
	.long	main@GOTFUNCDESC
L_init:
	.long   _init@GOTFUNCDESC
L_fini:
	.long   _fini@GOTFUNCDESC
L_uClibc_main:
	.long   __uClibc_main@GOTFUNCDESC
L_abort:
	.long   abort@GOTFUNCDESC
#elif defined __PIC__
L_got:
	.long	_GLOBAL_OFFSET_TABLE_
L_main:
	.long	main@GOTOFF
L_init:
	.long   _init@GOTOFF
L_fini:
	.long   _fini@GOTOFF
L_uClibc_main:
	.long   __uClibc_main@GOT
L_abort:
	.long   abort@GOT
#else
L_main:
	.long   main
L_init:
	.long   _init
L_fini:
	.long   _fini
L_uClibc_main:
	.long   __uClibc_main
L_abort:
	.long   abort
#endif

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
