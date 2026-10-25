/*
 * Built with -funwind-tables, objects in libc.so refer to the C6000
 * EABI personality routines, but libc.so is not linked against
 * libgcc_eh.a.  The unwinder handles these compact models itself and
 * never calls them, so dummy definitions do, as on ARM.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#define PR(n)						\
attribute_hidden void __c6xabi_unwind_cpp_pr##n (void);	\
attribute_hidden void __c6xabi_unwind_cpp_pr##n (void)	\
{							\
}

PR(0)
PR(1)
PR(2)
PR(3)
PR(4)
