/* Dummy crti file.

   In this configuration, crti.o and crtn.o are both empty because the
   .init_array/.fini_array sections are used exclusively.  No _init or
   _fini either: a label in an empty .init gets DT_INIT pointed at
   whatever the linker places next, the PLT.  */
