/* Copyright (C) 2002 by  Red Hat, Incorporated. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

#include "math.h"
#include "math_private.h"
#include <errno.h>

double fdim(double x, double y)
{
  /* islessequal, not <=: the quiet comparison, so a NaN argument does not
     raise invalid.  It is false for a NaN, so such a call falls through to
     x - y, which is the NaN the caller wants and raises nothing either --
     no test for it is needed.  */
  if (islessequal(x, y))
	  return .0;

  double z = x - y;
  if (isinf(z) && !isinf(x) && !isinf(y))
	  __set_errno(ERANGE);

  return z;
}
libm_hidden_def(fdim)
