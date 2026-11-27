/* totalorder and totalordermag (ISO C23): the IEEE 754 total order,
   -NaN < -inf < ... < -0 < +0 < ... < +inf < +NaN, with NaNs ordered by
   payload and signaling below quiet.

   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#include "c23_private.h"

/* Flipping the magnitude of negative encodings turns sign-magnitude
   into two's complement order.  Where the high significand bit marks
   signaling NaNs, it is flipped first for two NaNs, so that signaling
   still orders below quiet.  */
int totalorder(const double *x, const double *y)
{
	int64_t a = c23_asuint64(*x), b = c23_asuint64(*y);

#if C23_SNAN_HIGH_BIT
	if ((a & 0x7fffffffffffffffLL) > 0x7ff0000000000000LL
	    && (b & 0x7fffffffffffffffLL) > 0x7ff0000000000000LL) {
		a ^= 1LL << 51;
		b ^= 1LL << 51;
	}
#endif

	a ^= (uint64_t) (a >> 63) >> 1;
	b ^= (uint64_t) (b >> 63) >> 1;
	return a <= b;
}

int totalordermag(const double *x, const double *y)
{
	uint64_t a = c23_asuint64(*x) & ~(1ULL << 63);
	uint64_t b = c23_asuint64(*y) & ~(1ULL << 63);

#if C23_SNAN_HIGH_BIT
	if (a > 0x7ff0000000000000ULL && b > 0x7ff0000000000000ULL) {
		a ^= 1ULL << 51;
		b ^= 1ULL << 51;
	}
#endif
	return a <= b;
}

int totalorderf(const float *x, const float *y)
{
	int32_t a = c23_asuint32(*x), b = c23_asuint32(*y);

#if C23_SNAN_HIGH_BIT
	if ((a & 0x7fffffff) > 0x7f800000 && (b & 0x7fffffff) > 0x7f800000) {
		a ^= 1 << 22;
		b ^= 1 << 22;
	}
#endif

	a ^= (uint32_t) (a >> 31) >> 1;
	b ^= (uint32_t) (b >> 31) >> 1;
	return a <= b;
}

int totalordermagf(const float *x, const float *y)
{
	uint32_t a = c23_asuint32(*x) & 0x7fffffffU;
	uint32_t b = c23_asuint32(*y) & 0x7fffffffU;

#if C23_SNAN_HIGH_BIT
	if (a > 0x7f800000U && b > 0x7f800000U) {
		a ^= 1U << 22;
		b ^= 1U << 22;
	}
#endif
	return a <= b;
}

#if C23_LDBL
# if C23_LDBL_MANT_DIG == 53
int totalorderl(const long double *x, const long double *y)
{
	double a, b;

	memcpy(&a, x, sizeof a);
	memcpy(&b, y, sizeof b);
	return totalorder(&a, &b);
}

int totalordermagl(const long double *x, const long double *y)
{
	double a, b;

	memcpy(&a, x, sizeof a);
	memcpy(&b, y, sizeof b);
	return totalordermag(&a, &b);
}
# elif defined C23_LDBL_IBM
/* glibc's ldbl-128ibm order: the high parts decide; for equal finite
   high parts the low parts do, unless both are zero.  */
static int64_t ordered(double d)
{
	int64_t i = (int64_t) c23_asuint64(d);

	return i ^ (int64_t) ((uint64_t) (i >> 63) >> 1);
}

static int low_le(const long double *x, const long double *y, int mag)
{
	double xh, xl, yh, yl;
	uint64_t hx, lx, ly;

	c23_ldunpack(x, &xh, &xl);
	c23_ldunpack(y, &yh, &yl);
	hx = c23_asuint64(xh);
	if ((hx & 0x7fffffffffffffffULL) >= 0x7ff0000000000000ULL)
		return 1;				/* NaN or infinity */
	lx = c23_asuint64(xl);
	ly = c23_asuint64(yl);
	if (((lx | ly) & 0x7fffffffffffffffULL) == 0)
		return 1;
	if (mag) {		/* the low parts carry the high part's sign */
		lx ^= hx & 0x8000000000000000ULL;
		ly ^= c23_asuint64(yh) & 0x8000000000000000ULL;
	}
	memcpy(&xl, &lx, sizeof xl);
	memcpy(&yl, &ly, sizeof yl);
	return ordered(xl) <= ordered(yl);
}

int totalorderl(const long double *x, const long double *y)
{
	double xh, xl, yh, yl;
	int64_t a, b;

	c23_ldunpack(x, &xh, &xl);
	c23_ldunpack(y, &yh, &yl);
	a = ordered(xh);
	b = ordered(yh);
	if (a != b)
		return a < b;
	return low_le(x, y, 0);
}

int totalordermagl(const long double *x, const long double *y)
{
	double xh, xl, yh, yl;
	uint64_t a, b;

	c23_ldunpack(x, &xh, &xl);
	c23_ldunpack(y, &yh, &yl);
	a = c23_asuint64(xh) & 0x7fffffffffffffffULL;
	b = c23_asuint64(yh) & 0x7fffffffffffffffULL;
	if (a != b)
		return a < b;
	return low_le(x, y, 1);
}
# elif defined C23_LDBL_BITS
static int mag_le(const struct c23_ldbits *a, const struct c23_ldbits *b)
{
	return a->hi < b->hi || (a->hi == b->hi && a->lo <= b->lo);
}

#  if C23_SNAN_HIGH_BIT && C23_LDBL_MANT_DIG == 113
static int isnan_bits(const struct c23_ldbits *b)
{
	return b->hi > (0x7fffULL << 48)
	       || (b->hi == (0x7fffULL << 48) && b->lo);
}

/* For two NaNs, put signaling below quiet as in the other order.  */
static void flip_snan(struct c23_ldbits *a, struct c23_ldbits *b)
{
	if (isnan_bits(a) && isnan_bits(b)) {
		a->hi ^= 1ULL << 47;
		b->hi ^= 1ULL << 47;
	}
}
#  else
#   define flip_snan(a, b)	((void) 0)
#  endif

int totalorderl(const long double *x, const long double *y)
{
	struct c23_ldbits a, b;

	c23_ldget(x, &a);
	c23_ldget(y, &b);
	flip_snan(&a, &b);
	if (a.sign != b.sign)
		return a.sign;
	return a.sign ? mag_le(&b, &a) : mag_le(&a, &b);
}

int totalordermagl(const long double *x, const long double *y)
{
	struct c23_ldbits a, b;

	c23_ldget(x, &a);
	c23_ldget(y, &b);
	flip_snan(&a, &b);
	return mag_le(&a, &b);
}
# else
#  error "totalorderl: unknown long double format"
# endif
#endif
