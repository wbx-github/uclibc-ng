/* <devctl.h>, POSIX.1-2024.
   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#ifndef _DEVCTL_H
#define _DEVCTL_H	1

#include <features.h>
#define __need_size_t
#include <stddef.h>

__BEGIN_DECLS

/* Send the device command DCMD to FD, with NBYTE bytes of data at
   DEV_DATA_PTR going in or out as the command has it.  On Linux this is
   ioctl; its result goes to *DEV_INFO_PTR unless that is null.  Returns
   0 or an error number.  */
extern int posix_devctl (int __fd, int __dcmd, void *__restrict __dev_data_ptr,
			 size_t __nbyte, int *__restrict __dev_info_ptr);

__END_DECLS

#endif
