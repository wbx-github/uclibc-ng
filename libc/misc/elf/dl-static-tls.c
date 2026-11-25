/*
 * The hook that sets up the static TLS of a module loaded by dlopen,
 * and its default for a program with a single thread; pthread_create
 * installs its own.  Defined apart from dl-support.c, which every
 * static program needs, since only dlopen and pthread_create use it.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <link.h>
#include <ldso.h>

#if defined(USE_TLS) && USE_TLS
#include <assert.h>
#include <tls.h>
#include <ldsodefs.h>
#include <string.h>

/* Initialize static TLS area and DTV for current (only) thread.
   libpthread implementations should provide their own hook
   to handle all threads.  */
void
attribute_hidden
_dl_nothread_init_static_tls (struct link_map *map)
{
# if defined(TLS_TCB_AT_TP)
  void *dest = (char *) THREAD_SELF - map->l_tls_offset;
# elif defined(TLS_DTV_AT_TP)
  void *dest = (char *) THREAD_SELF + map->l_tls_offset + TLS_PRE_TCB_SIZE;
# else
#  error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
# endif

  /* Fill in the DTV slot so that a later LD/GD access will find it.  */
  dtv_t *dtv = THREAD_DTV ();
  assert (map->l_tls_modid <= dtv[-1].counter);
  dtv[map->l_tls_modid].pointer.val = dest;
  dtv[map->l_tls_modid].pointer.is_static = true;

  /* Initialize the memory.  */
  memset (mempcpy (dest, map->l_tls_initimage, map->l_tls_initimage_size),
	  '\0', map->l_tls_blocksize - map->l_tls_initimage_size);
}

void (*_dl_init_static_tls) (struct link_map *) = &_dl_nothread_init_static_tls;

#endif
