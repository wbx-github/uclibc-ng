	.align 4
__syscall_error:
	/* Call errno_location, store '-r4' in errno and return -1 */
#ifdef __SH_FDPIC__
	neg	r4, r4
	sts.l	pr, @-r15
	mov.l	r4, @-r15
	mov.l	1f, r0
	mov.l	@(r0,r12), r0
	mov.l	@(0,r0), r1
	jsr	@r1
	  mov.l	@(4,r0), r12
	mov.l	@r15+, r4
	lds.l	@r15+, pr
	mov.l	r4, @r0
#else
	mov.l	r12, @-r15
	sts.l	pr, @-r15
#ifdef SHARED
	mova	.LG, r0
	mov.l	.LG, r12
	add	r0, r12
	mov.l	1f, r0
	mov.l	@(r0,r12),r0
 	jsr	@r0
	 neg	r4, r12
#else	
	mov.l	1f, r0
	bsrf	r0
	 neg	r4, r12
.jmp_loc:		
#endif
	mov.l	r12, @r0
	lds.l	@r15+, pr
	mov.l	@r15+,r12
#endif

	/* And just kick back a -1.  */
	rts
	 mov	#-1, r0

	.align	4

#ifdef __SH_FDPIC__
1:	.long   __errno_location@GOTFUNCDESC
#elif defined __HAVE_SHARED__
1:	.long   __errno_location@GOT
.LG:	.long	_GLOBAL_OFFSET_TABLE_
#else
1:	.long   __errno_location - .jmp_loc
#endif		
