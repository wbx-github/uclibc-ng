/* O_*, F_*, FD_* bit values for Linux.
   Copyright (C) 2001-2024 Free Software Foundation, Inc.

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
   <https://www.gnu.org/licenses/>.  */

#ifndef	_FCNTL_H
# error "Never use <bits/fcntl-linux.h> directly; include <fcntl.h> instead."
#endif

#ifdef __USE_GNU
/* Types of seals.  */
# define F_SEAL_SEAL	0x0001	/* Prevent further seals from being set.  */
# define F_SEAL_SHRINK	0x0002	/* Prevent file from shrinking.  */
# define F_SEAL_GROW	0x0004	/* Prevent file from growing.  */
# define F_SEAL_WRITE	0x0008	/* Prevent writes.  */
# define F_SEAL_FUTURE_WRITE	0x0010	/* Prevent future writes while
					   mapped.  */
# define F_SEAL_EXEC	0x0020	/* Prevent chmod modifying exec bits. */

# define F_ADD_SEALS	1033	/* Add seals to file.  */
# define F_GET_SEALS	1034	/* Get seals for file.  */
#endif

#ifdef __USE_GNU
/* Flags for name_to_handle_at.  See comment in fcntl.h about the use
   of the same AT_* flag bits for different purposes in different
   functions.  */
# define AT_HANDLE_FID		AT_REMOVEDIR /* File handle is needed
						to compare object
						identity and may not
						be usable to
						open_by_handle_at.  */
# define AT_HANDLE_MNT_ID_UNIQUE 1 /* Return the 64-bit unique mount
				      ID.  */
# define AT_HANDLE_CONNECTABLE 2 /* Request a connectable file handle */

/* Flags for execveat2(2). */
# define AT_EXECVE_CHECK 0x10000 /* Only perform a check if execution
				    would be allowed */
#endif
