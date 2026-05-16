/*
 * This contains all symbols shared between
 * dynamic linker ld.so and into static libc
 *
 * Copyright (c) 2008  STMicroelectronics Ltd
 * Author: Carmelo Amoroso <carmelo.amoroso@st.com>
 *
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 */

/*
 * This is the start of the linked list that describes all of the files present
 * in the system with pointers to all of the symbol, string, and hash tables,
 * as well as all of the other good stuff in the binary.
 */
#include <ldso.h>

struct elf_resolve *_dl_loaded_modules = NULL;

/* Monotonic counters of load/unload events.  Incremented by the
   dynamic loader whenever an object is added to or removed from
   _dl_loaded_modules.  Exposed via dl_phdr_info::dlpi_adds /
   dlpi_subs to let dl_iterate_phdr callers cache results across
   invocations -- matches the glibc contract.  */
unsigned long long _dl_load_adds = 0;
unsigned long long _dl_load_subs = 0;

