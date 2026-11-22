/* getpayload, setpayload and setpayloadsig (ISO C23): the payload of a
   NaN, i.e. its significand without the quiet bit, as an integer.

   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#include "c23_private.h"

#define PAYLOAD_BITS	51
#define PAYLOAD_BITSF	22

double getpayload(const double *x)
{
	uint64_t i = c23_asuint64(*x);

	if ((i & 0x7ff0000000000000ULL) != 0x7ff0000000000000ULL
	    || !(i & 0x000fffffffffffffULL))
		return -1;
	return (double) (i & ((1ULL << PAYLOAD_BITS) - 1));
}

/* The high significand bit: set for quiet NaNs, or for signaling ones
   where C23_SNAN_HIGH_BIT.  */
#define HIGH(quiet)	(C23_SNAN_HIGH_BIT ? !(quiet) : (quiet))

/* PL must be an integer that fits; without the high bit it must not be
   zero, or the result would be an infinity.  Otherwise *X becomes +0
   and the result is nonzero.  */
static int setpayload_common(double *x, double pl, int quiet)
{
	uint64_t i;

	/* -0 is not a payload either, as in glibc.  */
	if (!(pl >= (HIGH(quiet) ? 0 : 1) && pl < 0x1p51 && pl == trunc(pl))
	    || signbit(pl)) {
		*x = 0;
		return 1;
	}
	/* Stored as bits: on x87 a signaling NaN that passes through the
	   FPU comes out quiet.  */
	i = 0x7ff0000000000000ULL | ((uint64_t) HIGH(quiet) << PAYLOAD_BITS)
	    | (uint64_t) pl;
	memcpy(x, &i, sizeof i);
	return 0;
}

int setpayload(double *x, double pl)
{
	return setpayload_common(x, pl, 1);
}

int setpayloadsig(double *x, double pl)
{
	return setpayload_common(x, pl, 0);
}

float getpayloadf(const float *x)
{
	uint32_t i = c23_asuint32(*x);

	if ((i & 0x7f800000U) != 0x7f800000U || !(i & 0x007fffffU))
		return -1;
	return (float) (i & ((1U << PAYLOAD_BITSF) - 1));
}

static int setpayloadf_common(float *x, float pl, int quiet)
{
	uint32_t i;

	if (!(pl >= (HIGH(quiet) ? 0 : 1) && pl < 0x1p22f && pl == truncf(pl))
	    || signbit(pl)) {
		*x = 0;
		return 1;
	}
	i = 0x7f800000U | ((uint32_t) HIGH(quiet) << PAYLOAD_BITSF)
	    | (uint32_t) pl;
	memcpy(x, &i, sizeof i);
	return 0;
}

int setpayloadf(float *x, float pl)
{
	return setpayloadf_common(x, pl, 1);
}

int setpayloadsigf(float *x, float pl)
{
	return setpayloadf_common(x, pl, 0);
}

#if C23_LDBL
# if C23_LDBL_MANT_DIG == 53
long double getpayloadl(const long double *x)
{
	double d;

	memcpy(&d, x, sizeof d);
	return getpayload(&d);
}

static int setpayloadl_common(long double *x, long double pl, int quiet)
{
	double d;
	int r = setpayload_common(&d, pl, quiet);

	memcpy(x, &d, sizeof d);
	return r;
}
# elif defined C23_LDBL_BITS
/* The payload is the significand below the quiet bit: 62 bits in the
   80-bit format (the explicit integer bit stays set), 111 bits in
   binary128.  */
long double getpayloadl(const long double *x)
{
	struct c23_ldbits b;

	c23_ldget(x, &b);
#  if C23_LDBL_MANT_DIG == 64
	if (b.hi != 0x7fff || !(b.lo & ((1ULL << 63) - 1)))
		return -1;
	return (long double) (b.lo & ((1ULL << 62) - 1));
#  else
	if ((b.hi >> 48) != 0x7fff || !((b.hi & ((1ULL << 48) - 1)) | b.lo))
		return -1;
	return (long double) (b.hi & ((1ULL << 47) - 1)) * 0x1p64L
	       + (long double) b.lo;
#  endif
}

static int setpayloadl_common(long double *x, long double pl, int quiet)
{
	struct c23_ldbits b;

	b.sign = 0;
	if (signbit(pl))
		goto fail;
#  if C23_LDBL_MANT_DIG == 64
	if (pl >= (quiet ? 0 : 1) && pl < 0x1p62L
	    && (long double) (uint64_t) pl == pl) {
		b.hi = 0x7fff;
		b.lo = (1ULL << 63) | ((uint64_t) quiet << 62) | (uint64_t) pl;
		c23_ldput(x, &b);
		return 0;
	}
#  else
	if (pl >= (HIGH(quiet) ? 0 : 1) && pl < 0x1p111L) {
		uint64_t h = pl * 0x1p-64L;
		uint64_t l = pl - (long double) h * 0x1p64L;

		if ((long double) h * 0x1p64L + (long double) l == pl) {
			b.hi = (0x7fffULL << 48)
			       | ((uint64_t) HIGH(quiet) << 47) | h;
			b.lo = l;
			c23_ldput(x, &b);
			return 0;
		}
	}
#  endif
fail:
	*x = 0;
	return 1;
}
# else
#  error "payload: unknown long double format"
# endif

int setpayloadl(long double *x, long double pl)
{
	return setpayloadl_common(x, pl, 1);
}

int setpayloadsigl(long double *x, long double pl)
{
	return setpayloadl_common(x, pl, 0);
}
#endif
