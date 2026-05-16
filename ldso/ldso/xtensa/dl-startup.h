/*
 * Xtensa ELF code used by dl-startup.c.
 *
 * Copyright (C) 2007 Tensilica Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 * Parts taken from glibc/sysdeps/xtensa/dl-machine.h.
 */

#include <bits/uClibc_page.h>

#if defined(__FDPIC__)
__asm__ (
    "	.text\n"
    "	.align  4\n"
    "   .literal_position\n"
    "	.global _start\n"
    "	.type   _start, @function\n"
    "	.hidden _start\n"
    "_start:\n"
    "	.begin	no-transform\n"
    "	_call0  1f\n"
    "2:\n"
    "	.end	no-transform\n"
    "	.align  4\n"
    "1:\n"
#if defined(__XTENSA_CALL0_ABI__)
    "	movi	a15, 2b\n"
    "	sub	a15, a0, a15\n"

    /* Save FDPIC pointers in callee-saved registers */
    "	mov	a12, a4\n"
    "	mov	a13, a5\n"
    "	mov	a14, a6\n"

    /* Call __self_reloc */
    "	mov	a2, a5\n"
    "	movi	a3, __ROFIXUP_LIST__\n"
    "	add	a3, a3, a15\n"
    "	movi	a4, __ROFIXUP_END__\n"
    "	add	a4, a4, a15\n"
    "	movi    a0, __self_reloc\n"
    "	add     a0, a0, a15\n"
    "	callx0  a0\n"

    /* call _dl_start */
    "	mov	a3, a12\n"
    "	mov	a4, a13\n"
    "	mov	a5, a14\n"
    "	mov	a7, sp\n"
    "	addi	sp, sp, -16\n"
    "	mov	a6, sp\n"
    "	mov	a11, a2\n"
    /* a13, interpreter map is no longer needed, save interpreter GOT there */
    "	mov	a13, a2\n"
    "	movi	a0, _dl_start\n"
    "	add	a0, a0, a15\n"
    "	callx0	a0\n"

    /* call main */
    "	l32i	a0, sp, 0\n"
    "	l32i	a11, sp, 4\n"
    "	addi	sp, sp, 16\n"
    "	mov	a4, a12\n"
    "	movi	a5, _dl_fini@GOTOFFFUNCDESC\n"
    "	add	a5, a5, a13\n"
    "	mov	a6, a14\n"
    "	jx	a0\n"
#else
#error Unsupported Xtensa ABI
#endif
   );
#else /* __FDPIC__ */
#ifndef L_rcrt1
__asm__ (
    "	.text\n"
    "	.align  4\n"
    "   .literal_position\n"
    "	.global _start\n"
    "	.type   _start, @function\n"
    "	.hidden _start\n"
    "_start:\n"
    "	# Compute load offset in a2: the GOT has not yet been relocated\n"
    "	# but the entries for local symbols contain the relative offsets\n"
    "	# and we can explicitly add the load offset in this code.\n"
    "	_call0  0f\n"
    "	.align  4\n"
    "0:	movi    a3, _start+3\n"
    "	sub     a2, a0, a3\n"
#if defined(__XTENSA_WINDOWED_ABI__)
    "	# Make sure a0 is cleared to mark the top of stack.\n"
    "	movi    a0, 0\n"
    "	# user_entry_point = _dl_start(pointer to argument block)\n"
    "	movi    a4, _dl_start\n"
    "	mov     a6, sp\n"
    "	add     a4, a4, a2\n"
    "	callx4  a4\n"
    "	# Save user_entry_point so we can jump to it.\n"
    "	mov     a3, a6\n"
#elif defined(__XTENSA_CALL0_ABI__)
    "	# user_entry_point = _dl_start(pointer to argument block)\n"
    "	movi    a0, _dl_start\n"
    "	add     a0, a0, a2\n"
    "	mov     a2, sp\n"
    "	callx0  a0\n"
    "	# Save user_entry_point so we can jump to it.\n"
    "	mov     a3, a2\n"
#else
#error Unsupported Xtensa ABI
#endif
    "	l32i    a7, sp, 0   # load argc\n"
    "	# Load _dl_skip_args into a4.\n"
    "	movi    a4, _dl_skip_args\n"
    "	l32i    a4, a4, 0\n"
    "	bnez    a4, .Lfixup_stack\n"
    ".Lfixup_stack_ret:\n"
    "	# Pass finalizer (_dl_fini) in a2 to the user entry point.\n"
    "	movi    a2, _dl_fini\n"
    "	# Jump to user's entry point (_start).\n"
    "	jx      a3\n"
    ".Lfixup_stack:\n"
    "	# argc -= _dl_skip_args (with argc @ sp+0)\n"
    "	sub     a7, a7, a4\n"
    "	s32i    a7, sp, 0\n"
    "	# Shift everything by _dl_skip_args.\n"
    "	addi    a5, sp, 4   # a5 = destination ptr = argv\n"
    "	add     a4, a5, a4  # a4 = source ptr = argv + _dl_skip_args\n"
    "	# Shift argv.\n"
    "1:	l32i    a6, a4, 0\n"
    "	addi    a4, a4, 4\n"
    "	s32i    a6, a5, 0\n"
    "	addi    a5, a5, 4\n"
    "	bnez    a6, 1b\n"
    "	# Shift envp.\n"
    "2:	l32i    a6, a4, 0\n"
    "	addi    a4, a4, 4\n"
    "	s32i    a6, a5, 0\n"
    "	addi    a5, a5, 4\n"
    "	bnez    a6, 2b\n"
    "	# Shift auxiliary table.\n"
    "3:	l32i    a6, a4, 0\n"
    "	l32i    a8, a4, 4\n"
    "	addi    a4, a4, 8\n"
    "	s32i    a6, a5, 0\n"
    "	s32i    a8, a5, 4\n"
    "	addi    a5, a5, 8\n"
    "	bnez    a6, 3b\n"
    "	j      .Lfixup_stack_ret");
#endif
#endif /* __FDPIC__ */

/* Get a pointer to the argv value.  */
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long *) ARGS) + 1)

/* Function calls are not safe until the GOT relocations have been done.  */
#define NO_FUNCS_BEFORE_BOOTSTRAP

#if defined(__ARCH_USE_MMU__) && !defined(__FDPIC__)
#define PERFORM_BOOTSTRAP_GOT(tpnt) \
do { \
	xtensa_got_location *got_loc; \
	unsigned long l_addr = tpnt->loadaddr; \
	Elf32_Addr prev_got_start = 0, prev_got_end = 0; \
	int x; \
\
	got_loc = (xtensa_got_location *) \
		(tpnt->dynamic_info[DT_XTENSA (GOT_LOC_OFF)] + l_addr); \
\
	for (x = 0; x < tpnt->dynamic_info[DT_XTENSA (GOT_LOC_SZ)]; x++) { \
		Elf32_Addr got_start, got_end; \
		got_start = got_loc[x].offset & ~((1UL << PAGE_SHIFT) - 1); \
		got_end = ((got_loc[x].offset + got_loc[x].length \
			    + (1UL << PAGE_SHIFT) - 1) \
			   & ~((1UL << PAGE_SHIFT) - 1)); \
		if (got_end >= prev_got_start && got_start <= prev_got_end) { \
			if (got_end > prev_got_end) \
				prev_got_end = got_end; \
			if (got_start < prev_got_start) \
				prev_got_start = got_start; \
			continue; \
		} else if (prev_got_start != prev_got_end) { \
			_dl_mprotect ((void *)(prev_got_start + l_addr), \
						  prev_got_end - prev_got_start, \
						  PROT_READ | PROT_WRITE | PROT_EXEC); \
		} \
		prev_got_start = got_start; \
		prev_got_end = got_end; \
	} \
\
	if (prev_got_start != prev_got_end) { \
		_dl_mprotect ((void *)(prev_got_start + l_addr), \
					  prev_got_end - prev_got_start, \
					  PROT_READ | PROT_WRITE | PROT_EXEC); \
	} \
} while (0)
#endif

#ifdef __FDPIC__
#undef DL_START
#define DL_START(X)   \
static void  __attribute__ ((used)) \
_dl_start (Elf32_Addr dl_boot_got_pointer, \
          struct elf32_fdpic_loadmap *dl_boot_progmap, \
          struct elf32_fdpic_loadmap *dl_boot_ldsomap, \
          Elf32_Dyn *dl_boot_ldso_dyn_pointer, \
          struct funcdesc_value *dl_main_funcdesc, \
          X)

/*
 * Transfer control to the user's application, once the dynamic loader
 * is done.  We return the address of the function's entry point to
 * _dl_boot, see boot1_arch.h.
 */
#define START()	do {							\
  struct elf_resolve *exec_mod = _dl_loaded_modules;			\
  dl_main_funcdesc->entry_point = _dl_elf_main;				\
  while (exec_mod->libtype != elf_executable)				\
    exec_mod = exec_mod->next;						\
  dl_main_funcdesc->got_value = exec_mod->loadaddr.got_value;		\
  return;								\
} while (0)
#endif /* __FDPIC__ */
