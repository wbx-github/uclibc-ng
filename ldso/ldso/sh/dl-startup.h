/* Any assembly language/system dependent hacks needed to setup boot1.c so it
 * will work as expected and cope with whatever platform specific wierdness is
 * needed for this architecture.  */

#ifndef __SH_FDPIC__
__asm__(
    "	.text\n"
    "	.globl	_start\n"
    "	.type	_start,@function\n"
    "	.hidden	_start\n"
    "_start:\n"
    "	mov	r15, r4\n"
    "	mov.l   .L_dl_start, r0\n"
    "	bsrf    r0\n"
    "	add	#4, r4\n"
    ".jmp_loc:\n"
    "	mov     r0, r8        ! Save the user entry point address in r8\n"
    "	mov.l   .L_got, r12   ! Load the GOT on r12\n"
    "	mova    .L_got, r0\n"
    "	add     r0, r12\n"
    "	mov.l .L_dl_skip_args,r0\n"
    "	mov.l @(r0,r12),r0\n"
    "	mov.l @r0,r0\n"
    "	mov.l @r15,r5         ! Get the original argument count\n"
    "	sub r0,r5             ! Subtract _dl_skip_args from it\n"
    "	shll2 r0\n"
    "	add r0,r15 ! Adjust the stack pointer to skip _dl_skip_args words\n"
    "	mov.l r5,@r15         ! Store back the modified argument count\n"
    "	mov.l   .L_dl_fini, r0\n"
    "	mov.l   @(r0,r12), r4 ! Pass the finalizer in r4\n"
    "	jmp     @r8\n"
    "	nop\n"
    ".L_dl_start:\n"
    "	.long   _dl_start-.jmp_loc\n"
    ".L_dl_skip_args:\n"
    "	.long   _dl_skip_args@GOT\n"
    ".L_dl_fini:\n"
    "	.long	_dl_fini@GOT\n"
    ".L_got:\n"
    "	.long _GLOBAL_OFFSET_TABLE_\n"
    "	.size	_start,.-_start\n"
    "	.previous\n"
);
#else /* __SH_FDPIC__  */
	/* On entry:
		R8  = load map for user executable.
		R9  = load map for interpreter (this program), or zero if
		      the user executed the interpreter directly.
		R10 = address of _DYNAMIC section.  */

__asm__(
    "	.text\n"
    "	.globl	_start\n"
    "	.type	_start,@function\n"
    "_start:\n"

    /* Select the correct load map.  */
    "	tst	r9, r9\n"
    "	bf/s	1f\n"
    "	  mov	r9, r4\n"
    "	mov	r8, r4\n"
    "1:\n"

    /* GOT = __self_reloc (load_map, rofixup, rofixupend)  */
    "	mova	.L_rofixup, r0\n"
    "	mov.l	.L_rofixup, r5\n"
    "	add	r0, r5\n"
    "	mova	.L_rofixupend, r0\n"
    "	mov.l	.L_rofixupend, r6\n"
    "	add	r0, r6\n"

    "	mov.l	.L_self_reloc, r1\n"
    "	bsrf	r1\n"
    "	  nop\n"
    ".L_self_reloc_jmp_loc:\n"
    "	mov	r0, r12\n"
    /* r12 now contains the GOT/FDPIC address.  */

    /* entry_point = _dl_start (dl_boot_got_pointer, dl_boot_progmap,
     *				dl_boot_ldsomap, dl_boot_ldso_dyn_pointer,
     *				argv)  */
    "	mov	r12, r4\n"
    "	mov	r8, r5\n"
    "	mov	r9, r6\n"
    "	mov	r10, r7\n"
    "	mov	r15, r11\n"
    "	add	#4, r11\n"
    "	mov.l   .L_dl_start, r0\n"
    "	bsrf    r0\n"
    "	  mov.l	r11, @-r15\n"		/* arg5 on the stack. */
    ".L_dl_start_jmp_loc:\n"
    "	add	#4, r15\n"		/* Pop arg5 from stack.  */

    /* Start the user program.  R8, R9 and R10 should remain as on entry.  */
    "	mov	r0, r1\n"
    "	mov.l   .L_dl_fini, r0\n"
    "	mov.l   @(r0,r12), r4 ! Pass the finalizer FD in r4\n"
    "	mov.l	@(0,r1), r2\n"
    "	jmp     @r2\n"
    "	  mov.l	@(4,r1), r12\n"

    ".L_self_reloc:\n"
    "	.long   __self_reloc-.L_self_reloc_jmp_loc\n"
    ".L_rofixup:\n"
    "	.long   __ROFIXUP_LIST__@PCREL\n"
    ".L_rofixupend:\n"
    "	.long   __ROFIXUP_END__@PCREL\n"
    ".L_dl_start:\n"
    "	.long	_dl_start-.L_dl_start_jmp_loc\n"
    ".L_dl_fini:\n"
    "	.long	_dl_fini@GOTFUNCDESC\n"

    "	.size	_start,.-_start\n"
    "	.previous\n"
);

#undef DL_START
#define DL_START(X)   \
/*static*/ void * __attribute__ ((used)) \
_dl_start (void *dl_boot_got_pointer, \
	   struct elf32_fdpic_loadmap *dl_boot_progmap, \
	   struct elf32_fdpic_loadmap *dl_boot_ldsomap, \
	   Elf32_Dyn *dl_boot_ldso_dyn_pointer, \
	   X)

#define START() do {						\
  static struct funcdesc_value _dl_elf_main_fd;			\
  struct elf_resolve *exec_mod = _dl_loaded_modules;		\
  while (exec_mod->libtype != elf_executable)			\
    exec_mod = exec_mod->next;					\
  _dl_elf_main_fd.entry_point = _dl_elf_main;			\
  _dl_elf_main_fd.got_value = exec_mod->loadaddr.got_value;	\
  return &_dl_elf_main_fd;					\
} while (0)

#endif  /* __SH_FDPIC__ */

/*
 * Get a pointer to the argv array.  On many platforms this can be just
 * the address of the first argument, on other platforms we need to
 * do something a little more subtle here.
 */
#define GET_ARGV(ARGVP, ARGS) ARGVP = ((unsigned long*)   ARGS)

/* We can't call functions earlier in the dl startup process */
#define NO_FUNCS_BEFORE_BOOTSTRAP

/*
 * Here is a macro to perform a relocation.  This is only used when
 * bootstrapping the dynamic loader.  RELP is the relocation that we
 * are performing, REL is the pointer to the address we are relocating.
 * SYMBOL is the symbol involved in the relocation, and LOAD is the
 * load address.
 */
#ifndef __SH_FDPIC__
#define PERFORM_BOOTSTRAP_RELOC(RELP,REL,SYMBOL,LOAD,SYMTAB)	\
	switch(ELF_R_TYPE((RELP)->r_info)){			\
	case R_SH_REL32:					\
		*(REL)  = (SYMBOL) + (RELP)->r_addend		\
			    - (unsigned long)(REL);		\
		break;						\
	case R_SH_DIR32:					\
	case R_SH_GLOB_DAT:					\
	case R_SH_JMP_SLOT:					\
		*(REL)  = (SYMBOL) + (RELP)->r_addend;		\
		break;						\
	case R_SH_RELATIVE:					\
		*(REL)  = (LOAD) + (RELP)->r_addend;		\
		break;						\
	case R_SH_NONE:						\
		break;						\
	default:						\
		_dl_exit(1);					\
	}
#else
#define BOOT_FD_SLOTS	16	/* must cover all R_SH_FUNCDESC relocs in ld.so (5 now) */
#define PERFORM_BOOTSTRAP_RELOC(RELP,REL,SYMBOL,LOAD,SYMTAB)	\
	switch(ELF32_R_TYPE((RELP)->r_info)){			\
	case R_SH_DIR32:					\
	case R_SH_GLOB_DAT:					\
	case R_SH_JMP_SLOT:					\
		*(REL)  = (SYMBOL) + (RELP)->r_addend;		\
		break;						\
	case R_SH_FUNCDESC_VALUE:				\
		{						\
		  struct funcdesc_value fv = {			\
		    (void*)((SYMBOL) + *(REL)),			\
		    (LOAD).got_value				\
		  };						\
		  *(struct funcdesc_value volatile *)(REL) = fv;\
		  break;					\
		}						\
	case R_SH_FUNCDESC:					\
		{						\
		  /* REL gets a pointer to a descriptor for SYMBOL. */	\
		  static struct funcdesc_value boot_fds[BOOT_FD_SLOTS];	\
		  static unsigned int boot_nfds;		\
		  void *ep = (void*)((SYMBOL) + (RELP)->r_addend);	\
		  unsigned int i;				\
		  for (i = 0; i < boot_nfds; i++)		\
		    if (boot_fds[i].entry_point == ep)		\
		      break;					\
		  if (i == boot_nfds) {				\
		    if (i == BOOT_FD_SLOTS) {			\
		      SEND_EARLY_STDERR_DEBUG("Too many FUNCDESC relocs\n");\
		      _dl_exit(1);				\
		    }						\
		    boot_fds[i].entry_point = ep;		\
		    boot_fds[i].got_value = (LOAD).got_value;	\
		    boot_nfds++;				\
		  }						\
		  *(REL) = (unsigned long)&boot_fds[i];		\
		  break;					\
		}						\
	case R_SH_NONE:						\
		break;						\
	default:						\
		SEND_EARLY_STDERR_DEBUG("Unknown relocation: ");\
		SEND_NUMBER_STDERR_DEBUG(ELF32_R_TYPE((RELP)->r_info), 1);\
		_dl_exit(1);					\
	}
#endif
