/*
 * Stolen from glibc-2.2.2 by Eddie C. Dost <ecd@atecom.com>
 */

	.text
	.globl	_dl_linux_resolver
	.globl	_dl_linux_resolve
	.type	_dl_linux_resolve, @function
	.balign	16
_dl_linux_resolve:
	mov.l	r2, @-r15	! see Note2 below
	mov.l	r3, @-r15
	mov.l	r4, @-r15
	mov.l	r5, @-r15
	mov.l	r6, @-r15
	mov.l	r7, @-r15
	mov.l	r12, @-r15
	movt	r4		! Save T flag
	mov.l	r4, @-r15

#ifdef HAVE_FPU
	sts.l	fpscr, @-r15
	mov	#8,r4
	swap.w	r4, r4
	lds	r4, fpscr
	fmov.s	fr11, @-r15
	fmov.s	fr10, @-r15
	fmov.s	fr9, @-r15
	fmov.s	fr8, @-r15
	fmov.s	fr7, @-r15
	fmov.s	fr6, @-r15
	fmov.s	fr5, @-r15
	fmov.s	fr4, @-r15
#endif
	sts.l	pr, @-r15

#ifdef __SH_FDPIC__
	/* On entry,
	     R0   the address of the resolver itself (this function)
	     R3   the GOT address (FDPIC value) for the resolver's GOT
	     R1   the address of the lazy PLT entry being resolved
	     R12  the GOT address for the caller's GOT
		  or sometimes for the called function's GOT

	   The uncertainty in R12 is due to a race condition when overwriting
	   the function descriptor. This code should not rely on R12.
	
	   Since we do not confidently know the value of the arguments to
	   _dl_linux_resolver, we pass the information we do have, and let it
	   work it out for itself ... it's easier to do this in C.  */

	mov.l	@(8,r12), r4	! load map
	mov	r1, r5		! Lazy PLT stub

	mov.l   .L_dl_linux_resolver, r0
	mov.l	@(r0, r3), r6
	mov.l	@(0,r6), r7
	jsr	@r7
	  mov.l	@(4,r6), r12
#else
/* Note - The PLT entries have been "optimised" not to use r2.  r2 is used by
   GCC to return the address of large structures, so it should not be
   corrupted here.  This does mean however, that those PLTs does not conform
   to the SH PIC ABI.  That spec says that r0 contains the type of the PLT
   and r2 contains the GOT id.  The GNU Plt version stores the GOT id in r0 and
   ignores the type.  We can easily detect this difference however,
   since the type will always be 0 or 8, and the GOT ids will always be
   greater than or equal to 12.

   Found in binutils/bfd/elf32-sh.c by Stefan Allius <allius@atecom.com>

   Note2 - we also have to preserve r2 on the stack as the call into
   C code (_dl_linux_resolver) will use r2 as a scratch register and we
   need it for the address for returning structures,
   David McCullough <davidm@snapgear.com>.
 */
	mov     #8 ,r5
	cmp/gt  r5, r0
	bt      1f
	mov     r2, r0          ! link map address in r2 (SH PIC ABI)
1:
	mov     r0, r4          ! link map address in r0 (GNUs PLT)

	mova    .LG, r0
	mov.l   .LG, r5
	add     r5, r0
	mov.l   .L_dl_linux_resolver, r5
	mov.l   @(r0, r5),r5
	jsr     @r5
	 mov	r1, r5		! Reloc offset
#endif

	lds.l	@r15+, pr	! Get register content back

#ifdef HAVE_FPU
	fmov.s	@r15+, fr4
	fmov.s	@r15+, fr5
	fmov.s	@r15+, fr6
	fmov.s	@r15+, fr7
	fmov.s	@r15+, fr8
	fmov.s	@r15+, fr9
	fmov.s	@r15+, fr10
	fmov.s	@r15+, fr11
	lds.l	@r15+, fpscr
#endif

	mov.l	@r15+, r3
	shal	r3		! Load T flag
	mov.l	@r15+, r12
	mov.l	@r15+, r7
	mov.l	@r15+, r6
	mov.l	@r15+, r5
	mov.l	@r15+, r4
	mov.l	@r15+, r3
#ifdef __SH_FDPIC__
	mov.l	@(4,r0), r12
	mov.l	@(0,r0), r0
#endif
	jmp	@r0		! Jump to function address
	 mov.l	@r15+, r2	! see Note2 above

	.balign	4

#ifdef __SH_FDPIC__
.L_dl_linux_resolver:
	.long   _dl_linux_resolver@GOTFUNCDESC
#else
.L_dl_linux_resolver:
	.long   _dl_linux_resolver@GOT
.LG:
	.long	_GLOBAL_OFFSET_TABLE_
#endif
	.size	_dl_linux_resolve, . - _dl_linux_resolve

