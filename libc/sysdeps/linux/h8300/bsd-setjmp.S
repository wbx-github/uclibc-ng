/* BSD `_setjmp' entry point to `sigsetjmp (..., 1)'. H8/300 version. */

#include <jmpbuf-offsets.h>

#ifdef __H8300S__
	.h8300s
#else
	.h8300h
#endif
	.text

.global setjmp

setjmp:
	add.l	#JB_SIZE,er0
	mov.l	@sp,er1		; return PC
	mov.l	er1,@-er0
	mov.l	sp,@-er0
	mov.l	er6,@-er0
	mov.l	er5,@-er0
	mov.l	er4,@-er0
	sub.l	er1,er1
	inc.l	#1,er1		; __sigjmp_save (env, 1), returns 0
#if !defined(__PIC__)
	jmp	@__sigjmp_save
#else
	mov.l	@(__sigjmp_save@GOTOFF,er5),er3
	jmp	@er3
#endif
	.end
