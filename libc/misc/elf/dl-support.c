/*
 * Support for dynamic linking code in static libc.
 * Copyright (C) 1996-2002, 2003, 2004, 2005 Free Software Foundation, Inc.
 *
 * Partially based on GNU C Library (file: libc/elf/dl-support.c)
 *
 * Copyright (C) 2008 STMicroelectronics Ltd.
 * Author: Carmelo Amoroso <carmelo.amoroso@st.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 */

#include <link.h>
#include <ldso.h>
#include <elf.h>
#include <bits/uClibc_page.h>

ElfW(Phdr) *_dl_phdr;
size_t _dl_phnum;
size_t _dl_pagesize = (1UL << PAGE_SHIFT);	/* fallback; overridden from AT_PAGESZ if present */

ElfW(auxv_t) _dl_auxvt[AUX_MAX_AT_ID];
ElfW(auxv_t) *_dl_auxv_start;

void internal_function _dl_aux_init (ElfW(auxv_t) *av);
void internal_function _dl_aux_init (ElfW(auxv_t) *av)
{
   _dl_auxv_start = av;

   for (; av->a_type != AT_NULL; av++)
     {
       if (av->a_type < AUX_MAX_AT_ID)
         _dl_auxvt[av->a_type] = *av;
     }

   /* Get the program headers base address from the aux vect */
   _dl_phdr = (ElfW(Phdr) *) _dl_auxvt[AT_PHDR].a_un.a_val;

   /* Get the number of program headers from the aux vect */
   _dl_phnum = (size_t) _dl_auxvt[AT_PHNUM].a_un.a_val;

   /* Override the fallback only if AT_PAGESZ is present and non-zero */
   if (_dl_auxvt[AT_PAGESZ].a_type == AT_PAGESZ && _dl_auxvt[AT_PAGESZ].a_un.a_val)
      _dl_pagesize = (size_t) _dl_auxvt[AT_PAGESZ].a_un.a_val;
}

