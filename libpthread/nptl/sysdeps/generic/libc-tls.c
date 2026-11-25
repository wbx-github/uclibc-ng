/* Initialization code for TLS in statically linked application.
   Copyright (C) 2002, 2003, 2004, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <ldsodefs.h>
#include <tls.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/param.h>
#include <elf.h>
#include <link.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/syscall.h>
#if !defined __NR_getpid && defined __NR_getxpid
# define __NR_getpid __NR_getxpid	/* alpha */
#endif

#ifdef SHARED
 #error makefile bug, this file is for static only
#endif

#if USE_TLS
extern ElfW(Phdr) *_dl_phdr;
extern size_t _dl_phnum;

#ifdef __FDPIC__
/* phdr->p_vaddr is not valid in FDPIC mode. To find tdata start we
   use the linker script defined symbol __tdata_start.  */
extern int __tdata_start;
#endif

#ifdef SHARED
static
#endif
dtv_t static_dtv[2 + TLS_SLOTINFO_SURPLUS];


static struct
{
  struct dtv_slotinfo_list si;
  /* The dtv_slotinfo_list data structure does not include the actual
     information since it is defined as an array of size zero.  We define
     here the necessary entries.  Note that it is not important whether
     there is padding or not since we will always access the information
     through the 'si' element.  */
  struct dtv_slotinfo info[2 + TLS_SLOTINFO_SURPLUS];
} static_slotinfo;

/* Fake link map for the application.  */
static struct link_map static_map;


/* Highest dtv index currently needed.  */
size_t _dl_tls_max_dtv_idx;
/* Flag signalling whether there are gaps in the module ID allocation.  */
bool _dl_tls_dtv_gaps;
/* Information about the dtv slots.  */
struct dtv_slotinfo_list *_dl_tls_dtv_slotinfo_list;
/* Number of modules in the static TLS block.  */
size_t _dl_tls_static_nelem;
/* Size of the static TLS block.  Initialized, so that it is no common
   symbol: one would make the linker pull in libdl.os for its definition.  */
size_t _dl_tls_static_size = 0;
/* Surplus for modules with IE-model TLS loaded by dlopen; libdl.c
   defines it, so a program without dlopen has none.  */
extern const size_t _dl_tls_static_surplus attribute_hidden
	__attribute__ ((weak));
/* Size actually allocated in the static TLS block.  */
size_t _dl_tls_static_used;
/* Alignment requirement of the static TLS block.  */
size_t _dl_tls_static_align;

/* Generation counter for the dtv.  */
size_t _dl_tls_generation;


/* Additional definitions needed by TLS initialization.  */
#ifdef TLS_INIT_HELPER
TLS_INIT_HELPER
#endif

static inline void
init_slotinfo (void)
{
  /* Create the slotinfo list.  */
  static_slotinfo.si.len = (((char *) (&static_slotinfo + 1)
			     - (char *) &static_slotinfo.si.slotinfo[0])
			    / sizeof static_slotinfo.si.slotinfo[0]);
  // static_slotinfo.si.next = NULL;	already zero

  /* The slotinfo list.  Will be extended by the code doing dynamic
     linking.  */
  GL(dl_tls_max_dtv_idx) = 1;
  GL(dl_tls_dtv_slotinfo_list) = &static_slotinfo.si;
}

static inline void
init_static_tls (size_t memsz, size_t align)
{
  /* That is the size of the TLS memory for this object.  The value of
     _dl_tls_static_size set in __libc_setup_tls is the surplus that
     permits dynamic loading of modules with IE-model TLS.  */
  GL(dl_tls_static_size) = roundup (memsz + GL(dl_tls_static_size),
				    TLS_TCB_ALIGN);
  GL(dl_tls_static_used) = memsz;
  /* The alignment requirement for the static TLS block.  */
  GL(dl_tls_static_align) = align;
  /* Number of elements in the static TLS block.  */
  GL(dl_tls_static_nelem) = GL(dl_tls_max_dtv_idx);
}

#if !defined(__FDPIC__) && !defined(SHARED) && defined(STATIC_PIE)
ElfW(Addr) _dl_load_base;
#endif

void __libc_setup_tls (size_t tcbsize, size_t tcbalign);
void
__libc_setup_tls (size_t tcbsize, size_t tcbalign)
{
  void *tlsblock;
  size_t memsz = 0;
  size_t filesz = 0;
  void *initimage = NULL;
  size_t align = 0;
  size_t max_align = tcbalign;
  size_t tcb_offset;
  ElfW(Phdr) *phdr;
#if !defined(__FDPIC__) && !defined(SHARED) && defined(STATIC_PIE)
  ElfW(Addr) load_base = _dl_load_base;
#endif

  if (&_dl_tls_static_surplus != NULL)
    GL(dl_tls_static_size) = _dl_tls_static_surplus;

#if !defined(__FDPIC__) && !defined(SHARED) && defined(STATIC_PIE)

  /* reloc_static_pie() sets _dl_load_base only when the program
     relocates itself; one started through ld-uClibc (noMMU ELF) is
     relocated by the loader.  The program headers give the load
     address either way.  */
  if (_dl_phdr != NULL)
    for (phdr = _dl_phdr; phdr < &_dl_phdr[_dl_phnum]; ++phdr)
      if (phdr->p_type == PT_PHDR)
	{
	  load_base = (ElfW(Addr)) _dl_phdr - phdr->p_vaddr;
	  break;
	}
#endif

  /* Look through the TLS segment if there is any.  */
  if (_dl_phdr != NULL)
    for (phdr = _dl_phdr; phdr < &_dl_phdr[_dl_phnum]; ++phdr)
      if (phdr->p_type == PT_TLS)
	{
	  /* Remember the values we need.  */
	  memsz = phdr->p_memsz;
	  filesz = phdr->p_filesz;
#ifdef __FDPIC__
	  initimage = (void *) &__tdata_start;
#else
	  initimage = (void *) phdr->p_vaddr;
#if !defined(SHARED) && defined(STATIC_PIE)
	  initimage += load_base;
#endif
#endif
	  align = phdr->p_align;
	  if (phdr->p_align > max_align)
	    max_align = phdr->p_align;
	  break;
	}

  /* We have to set up the TCB block which also (possibly) contains
     'errno'.  Therefore we avoid 'malloc' which might touch 'errno'.
     Instead we use 'sbrk' which would only uses 'errno' if it fails.
     In this case we are right away out of memory and the user gets
     what she/he deserves.

     _dl_tls_static_size holds the surplus that permits dynamic loading
     of modules with IE-model TLS.  */
  /* Use mmap instead of sbrk since this commit disables sbrk area
     for FDPIC MMU-less platforms:
     fs/binfmt_elf_fdpic.c: fix brk area overlap with stack on NOMMU
     https://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/fs/binfmt_elf_fdpic.c?id=4ac313111018cb44ecc250445de5ccb93026a980
     Loading static PIE ELFs on noMMU is possible since the linux kernel commit
     1bde925d2354 ("fs/binfmt_elf_fdpic.c: provide NOMMU loader for regular ELF binaries")
     and it is subject to the same brk restriction.
   */
# if defined(TLS_TCB_AT_TP)
  /* The thread pointer is the TLS block's end; the block below it is
     laid out for a thread pointer aligned like its most aligned part.
     The program's own block is rounded up first and the surplus added
     to that, the way the loader counts: rounding the sum instead can
     swallow the whole surplus when p_align is large -- 128 bytes of
     TLS at an alignment of 4096 leaves none of 2048.  */
  tcb_offset = roundup (roundup (memsz, align ?: 1)
			+ GL(dl_tls_static_size), max_align);
#  if defined(__FDPIC__) || (!defined(__ARCH_USE_MMU__) && defined(STATIC_PIE))
  tlsblock = mmap (NULL, tcb_offset + tcbsize + max_align,
                   PROT_READ|PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#  else
  tlsblock = sbrk (tcb_offset + tcbsize + max_align);
#  endif
# elif defined(TLS_DTV_AT_TP)
  tcb_offset = roundup (tcbsize, align ?: 1);
  /* Likewise the rounded size here: the layout below uses it, and
     reserving the raw one leaves the surplus partly outside the
     block that was asked for.  */
#  if defined(__FDPIC__) || (!defined(__ARCH_USE_MMU__) && defined(STATIC_PIE))
  tlsblock = mmap (NULL, tcb_offset + roundup (memsz, align ?: 1) + max_align
                   + TLS_PRE_TCB_SIZE + GL(dl_tls_static_size),
                   PROT_READ|PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#  else
  tlsblock = sbrk (tcb_offset + roundup (memsz, align ?: 1) + max_align
		     + TLS_PRE_TCB_SIZE + GL(dl_tls_static_size));
#  endif
  memset(tlsblock, '\0', tcb_offset + roundup (memsz, align ?: 1) + max_align
         + TLS_PRE_TCB_SIZE + GL(dl_tls_static_size));
  tlsblock += TLS_PRE_TCB_SIZE;
# else
  /* In case a model with a different layout for the TCB and DTV
     is defined add another #elif here and in the following #ifs.  */
#  error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
# endif

  /* Align the TLS block.  */
  tlsblock = (void *) (((uintptr_t) tlsblock + max_align - 1)
		       & ~(max_align - 1));

  /* Initialize the dtv.  [0] is the length, [1] the generation counter.  */
  static_dtv[0].counter = (sizeof (static_dtv) / sizeof (static_dtv[0])) - 2;
  // static_dtv[1].counter = 0;		would be needed if not already done

  /* Initialize the TLS block.  */
# if defined(TLS_TCB_AT_TP)
  static_dtv[2].pointer.val = ((char *) tlsblock + tcb_offset
			       - roundup (memsz, align ?: 1));
  static_map.l_tls_offset = roundup (memsz, align ?: 1);
# elif defined(TLS_DTV_AT_TP)
  static_dtv[2].pointer.val = (char *) tlsblock + tcb_offset;
  static_map.l_tls_offset = tcb_offset;
# else
#  error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
# endif
  static_dtv[2].pointer.is_static = true;
  /* sbrk gives us zero'd memory, so we don't need to clear the remainder.  */
  memcpy (static_dtv[2].pointer.val, initimage, filesz);

  /* Install the pointer to the dtv.  */

  /* Initialize the thread pointer.  */
# if defined(TLS_TCB_AT_TP)
  INSTALL_DTV ((char *) tlsblock + tcb_offset, static_dtv);

  const char *lossage = TLS_INIT_TP ((char *) tlsblock + tcb_offset, 0);
# elif defined(TLS_DTV_AT_TP)
  INSTALL_DTV (tlsblock, static_dtv);
  const char *lossage = (char *)TLS_INIT_TP (tlsblock, 0);
# else
#  error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
# endif
  if (__builtin_expect (lossage != NULL, 0))
    {
      /* Not abort(): without a thread pointer nothing that uses TLS can
	 run, and abort brings sigaction and sigprocmask along.  The pid
	 goes in a variable of its own: a system call in the argument of
	 another overwrites the number register.  */
      INTERNAL_SYSCALL_DECL (err);
      int pid = INTERNAL_SYSCALL (getpid, err, 0);
      INTERNAL_SYSCALL (kill, err, 2, pid, SIGKILL);
      INTERNAL_SYSCALL (exit_group, err, 1, 127);
    }

  /* We have to create a fake link map which normally would be created
     by the dynamic linker.  It just has to have enough information to
     make the TLS routines happy.  */
  static_map.l_tls_align = align;
  static_map.l_tls_blocksize = memsz;
  static_map.l_tls_initimage = initimage;
  static_map.l_tls_initimage_size = filesz;
  static_map.l_tls_modid = 1;

  init_slotinfo ();
  // static_slotinfo.si.slotinfo[1].gen = 0; already zero
  static_slotinfo.si.slotinfo[1].map = &static_map;

  memsz = roundup (memsz, align ?: 1);

# if defined(TLS_TCB_AT_TP)
  /* Count it as _dl_determine_tlsoffset does: _dl_tls_static_used is
     what lies below the thread pointer, and the TCB above it only adds
     to the size.  The size is the main thread's block, so that
     _dl_try_allocate_static_tls hands out exactly its surplus.  */
  GL(dl_tls_static_size) = tcb_offset + TLS_TCB_SIZE;
  GL(dl_tls_static_used) = memsz;
  GL(dl_tls_static_align) = MAX (TLS_TCB_ALIGN, max_align);
  GL(dl_tls_static_nelem) = GL(dl_tls_max_dtv_idx);
# elif defined(TLS_DTV_AT_TP)
  memsz += tcb_offset;
  init_static_tls (memsz, MAX (TLS_TCB_ALIGN, max_align));
# endif
}

/* Get size and alignment requirements of the static TLS block.  Here,
   next to the variables, rather than in dl-tls.c, which the thread setup
   would otherwise bring in with malloc for every program.  */
void
internal_function
_dl_get_tls_static_info (size_t *sizep, size_t *alignp)
{
  *sizep = GL(dl_tls_static_size);
  *alignp = GL(dl_tls_static_align);
}


#endif
