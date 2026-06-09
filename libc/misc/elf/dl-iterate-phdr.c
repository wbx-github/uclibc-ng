/* Get loaded objects program headers.

  Based on GNU C library (file: libc/elf/dl-iteratephdr.c)

  Copyright (C) 2001,2002,2003,2004,2006,2007 Free Software Foundation, Inc.
  Contributed by Jakub Jelinek <jakub@redhat.com>, 2001.

  Copyright (C) 2008 STMicroelectronics Ltd.
  Author: Carmelo Amoroso <carmelo.amoroso@st.com>

  Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
*/


#include <link.h>
#include <ldso.h>
#include <bits/uClibc_mutex.h>

/* we want this in libc but nowhere else */
#ifdef __USE_GNU

/* Serialize the callback invocations.  glibc holds the loader's
   dl_load_write_lock across dl_iterate_phdr; some callers (notably the
   libgcc unwinder's _Unwind_IteratePhdrCallback) rely on that and mutate
   shared state from the callback without locking themselves.  Without
   serialization, concurrent unwinds (e.g. many threads cancelling at once)
   corrupt that state and spin.  */
__UCLIBC_MUTEX_STATIC(_dl_iterate_phdr_lock,
		      PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP);

static int
__dl_iterate_phdr (int (*callback) (struct dl_phdr_info *info, size_t size, void *data), void *data)
{
	int ret = 0;
#ifndef __ARCH_HAS_NO_SHARED__
	struct elf_resolve *l;
	struct dl_phdr_info info;

	__UCLIBC_MUTEX_CONDITIONAL_LOCK(_dl_iterate_phdr_lock, 1);
	for (l = _dl_loaded_modules; l != NULL; l = l->next) {
		info.dlpi_addr = l->loadaddr;
		info.dlpi_name = l->libname;
		info.dlpi_phdr = l->ppnt;
		info.dlpi_phnum = l->n_phent;
		info.dlpi_adds = _dl_load_adds;
		info.dlpi_subs = _dl_load_subs;
#if defined(USE_TLS) && USE_TLS
		info.dlpi_tls_modid = l->l_tls_modid;
# ifdef SHARED
		/* Resolves into ld.so at runtime; static libc has no DTV.  */
		info.dlpi_tls_data = _dl_tls_get_addr_soft((struct link_map *) l);
# else
		info.dlpi_tls_data = NULL;
# endif
#else
		info.dlpi_tls_modid = 0;
		info.dlpi_tls_data = NULL;
#endif
		ret = callback (&info, sizeof (struct dl_phdr_info), data);
		if (ret)
			break;
	}
	__UCLIBC_MUTEX_CONDITIONAL_UNLOCK(_dl_iterate_phdr_lock, 1);
#endif
	return ret;
}

# ifdef SHARED

weak_alias(__dl_iterate_phdr, dl_iterate_phdr)

# else

/* dl-support.c defines these and initializes them early on.  */
extern ElfW(Phdr) *_dl_phdr;
extern size_t _dl_phnum;

int
dl_iterate_phdr (int (*callback) (struct dl_phdr_info *info,
                                  size_t size, void *data), void *data)
{
  if (_dl_phnum != 0)
    {
      /* This entry describes this statically-linked program itself.  */
      struct dl_phdr_info info;
      int ret;
#if defined(__FRV_FDPIC__) || defined(__BFIN_FDPIC__) || defined(__FDPIC__)
      info.dlpi_addr.map = NULL;
      info.dlpi_addr.got_value = NULL;
#elif defined(__DSBT__)
      info.dlpi_addr.map = NULL;
#else
      info.dlpi_addr = 0;
#endif
      info.dlpi_name = "";
      info.dlpi_phdr = _dl_phdr;
      info.dlpi_phnum = _dl_phnum;
      info.dlpi_adds = 0;
      info.dlpi_subs = 0;
      /* No easy access to TLS info on the static fallback path; report
         0 / NULL.  libsanitizer will skip TLS tracking for this entry,
         which is acceptable for a single statically linked program.  */
      info.dlpi_tls_modid = 0;
      info.dlpi_tls_data = NULL;
      ret = (*callback) (&info, sizeof (struct dl_phdr_info), data);
      if (ret)
        return ret;
    }
   /* Then invoke callback on loaded modules, if any */
  return __dl_iterate_phdr (callback, data);
}

# endif
#endif
