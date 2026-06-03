/*
 * Zero terminator for the .eh_frame section of shared objects.
 *
 * uClibc-ng links its shared objects with -nostdlib/-nostartfiles, so
 * gcc's crtendS.o - which normally contributes this terminator - is
 * never pulled in.  Unwinders that walk .eh_frame linearly (used
 * whenever no .eh_frame_hdr search table is available) rely on a
 * terminating zero length-word to stop; without it they run past the
 * end of the section into unrelated data and crash.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

static const int __FRAME_END__[1]
	__attribute__ ((used, section (".eh_frame"), aligned (4))) = { 0 };
