/* faddl, fsubl, fmull, fdivl, ffmal, fsqrtl and daddl, dsubl, dmull,
   ddivl, dfmal, dsqrtl (ISO C23): long double operations rounded once
   to float or double.  See c23_narrow.h.

   libm's fmal and sqrtl compute in double, so the exact product comes
   from Dekker's splitting and the square root from Newton steps checked
   against the exact residual.

   Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.  */

#include "c23_narrow.h"

#if C23_LDBL
# if C23_LDBL_MANT_DIG == 53
/* long double is double.  */
float faddl(long double x, long double y) { return fadd(x, y); }
float fsubl(long double x, long double y) { return fsub(x, y); }
float fmull(long double x, long double y) { return fmul(x, y); }
float fdivl(long double x, long double y) { return fdiv(x, y); }
float ffmal(long double x, long double y, long double z)
{ return ffma(x, y, z); }
float fsqrtl(long double x) { return fsqrt(x); }
double daddl(long double x, long double y) { return (double) x + (double) y; }
double dsubl(long double x, long double y) { return (double) x - (double) y; }
double dmull(long double x, long double y) { return (double) x * (double) y; }
double ddivl(long double x, long double y) { return (double) x / (double) y; }
double dfmal(long double x, long double y, long double z)
{ return fma(x, y, z); }
double dsqrtl(long double x) { return sqrt(x); }
# elif defined C23_LDBL_IBM
/* double-double is no IEEE format: compute in it and round once more,
   as glibc's ldbl-128ibm does (NARROW_*_TRIVIAL).  */
float faddl(long double x, long double y) { return (float) (x + y); }
float fsubl(long double x, long double y) { return (float) (x - y); }
float fmull(long double x, long double y) { return (float) (x * y); }
float fdivl(long double x, long double y) { return (float) (x / y); }
float ffmal(long double x, long double y, long double z)
{ return (float) (x * y + z); }
float fsqrtl(long double x) { return (float) sqrtl(x); }
double daddl(long double x, long double y) { return (double) (x + y); }
double dsubl(long double x, long double y) { return (double) (x - y); }
double dmull(long double x, long double y) { return (double) (x * y); }
double ddivl(long double x, long double y) { return (double) (x / y); }
double dfmal(long double x, long double y, long double z)
{ return (double) (x * y + z); }
double dsqrtl(long double x) { return (double) sqrtl(x); }
# elif defined C23_LDBL_BITS

typedef long double LD;

#  define LDABS(v)	((v) < 0 ? -(v) : (v))
#  define FINITE(v)	c23_isfinite(v)
/* Beyond double, below double: the result then overflows or underflows
   either narrow type, and only its sign (and size) matters.  */
#  define BIG		0x1p1100L
#  define TINY		0x1p-1200L
/* Far from the long double limits, where the splitting stays exact.  */
#  define HUGE_LD	0x1p16000L
#  define TINY_LD	0x1p-16000L
#  if C23_LDBL_MANT_DIG == 64
#   define SPLIT	(0x1p32L + 1)
#  else
#   define SPLIT	(0x1p57L + 1)
#  endif
#  define STICKY(x, y)	((((x) < 0) != ((y) < 0)) ? -TINY : TINY)

/* One unit in the last place up (UP) or down in magnitude.  */
static void incmag(LD *v, int up)
{
	struct c23_ldbits b;

	c23_ldget(v, &b);
#  if C23_LDBL_MANT_DIG == 64
	/* The explicit integer bit: 1.11..1 wraps to 1.00..0 one up.  */
	if (up) {
		if (++b.lo == 0) {
			b.lo = 1ULL << 63;
			b.hi++;
		} else if (b.hi == 0 && (b.lo >> 63))
			b.hi = 1;
	} else if (b.lo == (1ULL << 63) && b.hi > 1) {
		b.lo = ~0ULL;
		b.hi--;
	} else if (--b.lo >> 63 == 0 && b.hi == 1)
		b.hi = 0;
#  else
	if (up) {
		if (++b.lo == 0)
			b.hi++;
	} else if (b.lo-- == 0)
		b.hi--;
#  endif
	c23_ldput(v, &b);
}

static int is_odd(LD v)
{
	struct c23_ldbits b;

	c23_ldget(&v, &b);
	return b.lo & 1;
}

static LD round_odd(LD hi, LD lo)
{
	if (lo != 0 && !is_odd(hi))
		incmag(&hi, (lo < 0) == (hi < 0));
	return hi;
}

/* Dekker: ph + pl == a b exactly, for |a|, |b| <= HUGE_LD and
   |a b| >= TINY_LD.  */
static void two_prod(LD a, LD b, LD *ph, LD *pl)
{
	LD p = a * b, t, ah, al, bh, bl;

	t = SPLIT * a;
	ah = t - (t - a);
	al = a - ah;
	t = SPLIT * b;
	bh = t - (t - b);
	bl = b - bh;
	*ph = p;
	*pl = ((ah * bh - p) + ah * bl + al * bh) + al * bl;
}

/* Move a power of two from the larger factor to the smaller one, so
   that neither is near the limits; the product stays the same.  */
static void balance(LD *a, LD *b)
{
	if (LDABS(*a) > HUGE_LD || LDABS(*b) < TINY_LD) {
		*a *= 0x1p-1000L;
		*b *= 0x1p1000L;
	} else if (LDABS(*b) > HUGE_LD || LDABS(*a) < TINY_LD) {
		*b *= 0x1p-1000L;
		*a *= 0x1p1000L;
	}
}

/* The workers set *R to the odd-rounded result and return 0, or return
   1 when the caller is to compute it in its own rounding mode, or 2
   for an exact zero sum of two nonzero terms of opposite sign.  */

static int add_odd(LD x, LD y, LD *r)
{
	LD s, e;

	TWO_SUM(LD, x, y, s, e);
	if (!FINITE(s) || s == 0)
		return 1;
	*r = round_odd(s, e);
	return 0;
}

static int mul_odd(LD x, LD y, LD *r)
{
	LD p = x * y, ph, pl;

	if (!FINITE(p) || x == 0 || y == 0 || LDABS(p) > BIG)
		return 1;
	if (LDABS(p) < TINY) {
		*r = p != 0 ? p : STICKY(x, y);
		return 0;
	}
	balance(&x, &y);
	two_prod(x, y, &ph, &pl);
	*r = round_odd(ph, pl);
	return 0;
}

static int div_odd(LD x, LD y, LD *r)
{
	LD q = x / y, ph, pl, d;

	if (!FINITE(q) || x == 0 || !FINITE(y) || LDABS(q) > BIG)
		return 1;
	if (LDABS(q) < TINY) {
		*r = q != 0 ? q : STICKY(x, y);
		return 0;
	}
	if (LDABS(x) < TINY_LD) {
		x *= 0x1p1000L;
		y *= 0x1p1000L;
	} else if (LDABS(y) > HUGE_LD) {
		x *= 0x1p-1000L;
		y *= 0x1p-1000L;
	}
	/* x - q y = (x - ph) - pl, and x - ph is exact (Sterbenz).  */
	two_prod(q, y, &ph, &pl);
	d = x - ph;
	*r = round_odd(q, d == pl ? 0 : ((d > pl) == (y > 0) ? 1 : -1));
	return 0;
}

/* The sign of x - s^2.  */
static int cmp_sq(LD s, LD x)
{
	LD ph, pl, d;

	two_prod(s, s, &ph, &pl);
	d = x - ph;
	return (d > pl) - (d < pl);
}

static int sqrt_odd(LD x, LD *r)
{
	LD s, t;
	int k = 0, c, i;

	if (!(x > 0) || !FINITE(x))
		return 1;
	if (x > 0x1p2200L) {
		*r = BIG;
		return 0;
	}
	if (x < 0x1p-2200L) {
		*r = TINY;
		return 0;
	}
	while (x > 0x1p900L) {
		x *= 0x1p-1000L;
		k += 500;
	}
	while (x < 0x1p-900L) {
		x *= 0x1p1000L;
		k -= 500;
	}
	s = sqrt((double) x);
	s = 0.5L * (s + x / s);
#  if C23_LDBL_MANT_DIG > 64
	s = 0.5L * (s + x / s);
#  endif
	/* Make s the floor of the root: s^2 <= x < next(s)^2.  */
	c = cmp_sq(s, x);
	for (i = 0; c < 0 && i < 4; i++) {
		incmag(&s, 0);
		c = cmp_sq(s, x);
	}
	for (i = 0; c > 0 && i < 4; i++) {
		int ct;

		t = s;
		incmag(&t, 1);
		ct = cmp_sq(t, x);
		if (ct < 0)
			break;
		s = t;
		c = ct;
	}
	if (c != 0 && !is_odd(s))
		incmag(&s, 1);
	for (; k > 0; k -= 500)
		s *= 0x1p500L;
	for (; k < 0; k += 500)
		s *= 0x1p-500L;
	*r = s;
	return 0;
}

static int fma_odd(LD x, LD y, LD z, LD *r)
{
	LD ph, pl, s1, e1, s2, e2, t, te, h, l;
	int scaled = 0;

	if (!FINITE(x) || !FINITE(y) || x == 0 || y == 0)
		return 1;
	if (!FINITE(z)) {
		if (c23_isnan(z))
			return 1;	/* the fallback quiets it */
		/* The exact product of two finite numbers is finite and
		   cannot cancel an infinity.  */
		*r = z;
		return 0;
	}
	balance(&x, &y);
	ph = x * y;
	if (!FINITE(ph) || LDABS(ph) > HUGE_LD || LDABS(z) > HUGE_LD) {
		if (LDABS(x) > LDABS(y))
			x *= 0x1p-1000L;
		else
			y *= 0x1p-1000L;
		z *= 0x1p-1000L;
		ph = x * y;
		if (!FINITE(ph)) {
			/* Past the long double range even scaled: the
			   product dwarfs z, so the narrow result
			   overflows with the product's sign.  */
			*r = c23_signbit(ph) ? -BIG : BIG;
			return 0;
		}
		scaled = 1;
	}
	if (LDABS(ph) < TINY_LD) {
		if (LDABS(z) < 0x1p-15000L) {
			/* Both so far below the narrow format that only
			   the sign is left; TINY carries it.  */
			LD s = z + ph;

			if (s != 0)
				*r = s < 0 ? -TINY : TINY;
			else if (z != 0 || ph != 0)
				*r = 0;		/* exact cancellation */
			else
				*r = STICKY(x, y);
			return 0;
		}
		*r = round_odd(z, ph != 0 ? ph : STICKY(x, y));
	} else {
		two_prod(x, y, &ph, &pl);
		TWO_SUM(LD, z, ph, s1, e1);
		TWO_SUM(LD, s1, pl, s2, e2);
		TWO_SUM(LD, e1, e2, t, te);
		TWO_SUM(LD, s2, t, h, l);
		if (h == 0) {
			if (te == 0)
				return 2;
			*r = te;
		} else
			*r = round_odd(h, l != 0 ? l : te);
	}
	if (scaled) {
		LD s = *r * 0x1p1000L;

		/* Scaling back can leave the long double range where the
		   narrow result is just an overflow; BIG carries the sign
		   and narrows right in every rounding mode.  */
		*r = FINITE(s) ? s : (c23_signbit(*r) ? -BIG : BIG);
	}
	return 0;
}

/* ZERO is the exact zero in the caller's mode, for status 2.  */
#  define NARROW(T, call, fallback, zero) {				\
	LD r = 0;							\
	volatile LD vr;							\
	int st;								\
	{								\
		RN_BEGIN();						\
		st = call;						\
		vr = r;							\
		RN_END();						\
	}								\
	if (st == 2)							\
		return (T) (zero);					\
	if (st)								\
		return (T) (fallback);					\
	return (T) vr;							\
}

float faddl(LD x, LD y)
	NARROW(float, add_odd(x, y, &r), LATE(x) + LATE(y), 0)
float fsubl(LD x, LD y)
	NARROW(float, add_odd(x, -y, &r), LATE(x) - LATE(y), 0)
float fmull(LD x, LD y)
	NARROW(float, mul_odd(x, y, &r), LATE(x) * LATE(y), 0)
float fdivl(LD x, LD y)
	NARROW(float, div_odd(x, y, &r), LATE(x) / LATE(y), 0)
float fsqrtl(LD x)
	/* x < 0 must be a domain error; narrowing it first could make it
	   -0, and sqrt(-0) is -0.  */
	NARROW(float, sqrt_odd(x, &r), sqrt(x < 0 ? -1.0 : LATE(x)), 0)
float ffmal(LD x, LD y, LD z)
	NARROW(float, fma_odd(x, y, z, &r), LATE(x) * LATE(y) + LATE(z),
	       LATE(z) - LATE(z))
double daddl(LD x, LD y)
	NARROW(double, add_odd(x, y, &r), LATE(x) + LATE(y), 0)
double dsubl(LD x, LD y)
	NARROW(double, add_odd(x, -y, &r), LATE(x) - LATE(y), 0)
double dmull(LD x, LD y)
	NARROW(double, mul_odd(x, y, &r), LATE(x) * LATE(y), 0)
double ddivl(LD x, LD y)
	NARROW(double, div_odd(x, y, &r), LATE(x) / LATE(y), 0)
double dsqrtl(LD x)
	NARROW(double, sqrt_odd(x, &r), sqrt(x < 0 ? -1.0 : LATE(x)), 0)
double dfmal(LD x, LD y, LD z)
	NARROW(double, fma_odd(x, y, z, &r), LATE(x) * LATE(y) + LATE(z),
	       LATE(z) - LATE(z))
# else
#  error "narrowing: unknown long double format"
# endif
#endif
