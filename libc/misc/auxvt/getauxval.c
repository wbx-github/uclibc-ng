/*  Copyright (C) 2022     Ramin Seyed Moussavi
 *  An getauxval() function compatible with the glibc auxv.h
 *  that is used by uClibc-ng.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, see
 *  <http://www.gnu.org/licenses/>.
 */

#include "errno.h"
#include "ldso.h"
#include "sys/auxv.h"


/*
 *
 * aarch64 gcc 11 uses __getauxval() in init_have_lse_atomics()
 *
 */
unsigned long int __getauxval (unsigned long int __type)
{
	if ( __type >= AUX_MAX_AT_ID ){
		__set_errno (ENOENT);
		return 0;
	}

	if ( _dl_auxvt[__type].a_type == __type){
		return _dl_auxvt[__type].a_un.a_val;
	}

	__set_errno (ENOENT);
	return 0;
}

unsigned long int getauxval (unsigned long int __type){
	return __getauxval( __type );
}

