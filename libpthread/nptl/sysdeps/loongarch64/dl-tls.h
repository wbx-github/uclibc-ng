/* Copyright (C) 2024 uClibc-ng contributors.

   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
*/

/* Type used for the representation of TLS information in the GOT.  */
typedef struct
{
  unsigned long int ti_module;
  unsigned long int ti_offset;
} tls_index;

/* The thread pointer points to the first static TLS block.  */
#define TLS_TP_OFFSET           0

/* Dynamic thread vector pointers point to the start of each TLS block;
   unlike riscv, LoongArch has no DTV bias.  */
#define TLS_DTV_OFFSET          0

extern void *__tls_get_addr (tls_index *ti);

#define GET_ADDR_OFFSET        (ti->ti_offset + TLS_DTV_OFFSET)
#define __TLS_GET_ADDR(__ti)	(__tls_get_addr (__ti) - TLS_DTV_OFFSET)

/* Value used for dtv entries for which the allocation is delayed.  */
#define TLS_DTV_UNALLOCATED    ((void *) -1l)