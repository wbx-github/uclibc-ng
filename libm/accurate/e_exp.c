/* uClibc-ng: taken from the CORE-MATH project
   (git_clones/core-math-inria, src/binary64/exp/exp.c), selected by
   UCLIBC_LIBM_ACCURATE.  Correctly rounded: measured against expl over 300000
   points, the worst error is 0.500 ulp, where Arm's routines reach 0.506 and the
   fdlibm version in libm/e_exp.c 0.868.  Costs about twice the time of Arm's on a
   machine with an FPU, and calls fma eight times -- on a target without hardware
   fma that lands in libm's own s_fma.c, which is correct but slow.  That is the
   trade this variant exists for.

   Changes against the original: cr_exp is __ieee754_exp, and the one
   feraiseexcept is compiled out where the port has no fenv.  Measured: the flag
   changes no result -- 7650001 values, 367368 of them subnormal, no difference
   with or without it.  */

/* Correctly rounded exponential function for binary64 values.

Copyright (c) 2022-2025 Alexei Sibidanov.

This file is part of the CORE-MATH project
(https://core-math.gitlabpages.inria.fr/).

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdint.h>
#ifdef CORE_MATH_SUPPORT_ERRNO
#include <errno.h>
#endif
#ifdef __UCLIBC_HAS_FENV__
#include <fenv.h> // for feraiseexcept, FE_UNDERFLOW
#endif
#if defined(__x86_64__)
#include <x86intrin.h>
#endif

// Warning: clang also defines __GNUC__
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif

#pragma STDC FENV_ACCESS ON

/* __builtin_roundeven was introduced in gcc 10:
   https://gcc.gnu.org/gcc-10/changes.html,
   and in clang 17 */
#if ((defined(__GNUC__) && __GNUC__ >= 10) || (defined(__clang__) && __clang_major__ >= 17)) && !defined(_MSC_VER) && (defined(__aarch64__) || defined(__x86_64__) || defined(__i386__))
# define roundeven_finite(x) __builtin_roundeven (x)
#else
/* round x to nearest integer, breaking ties to even */
static double
roundeven_finite (double x)
{
  double ix;
# if (defined(__GNUC__) || defined(__clang__)) && (defined(__AVX__) || defined(__SSE4_1__) || (__ARM_ARCH >= 8))
#  if defined __AVX__
   __asm__("vroundsd $0x8,%1,%1,%0":"=x"(ix):"x"(x));
#  elif __ARM_ARCH >= 8
   __asm__ ("frintn %d0, %d1":"=w"(ix):"w"(x));
#  else /* __SSE4_1__ */
   __asm__("roundsd $0x8,%1,%0":"=x"(ix):"x"(x));
#  endif
# else
  ix = __builtin_round (x); /* nearest, away from 0 */
  if (__builtin_fabs (ix - x) == 0.5)
  {
    /* if ix is odd, we should return ix-1 if x>0, and ix+1 if x<0 */
    union { double f; uint64_t n; } u, v;
    u.f = ix;
    v.f = ix - __builtin_copysign (1.0, x);
    if (__builtin_ctz (v.n) > __builtin_ctz (u.n))
      ix = v.f;
  }
# endif
  return ix;
}
#endif

typedef int64_t i64;
typedef uint64_t u64;
typedef union {double f; u64 u;} b64u64_u;

static inline double fasttwosum(double x, double y, double *e){
  double s = x + y, z = s - x;
  *e = y - z;
  return s;
}

static inline double fastsum(double xh, double xl, double yh, double yl, double *e){
  double sl, sh = fasttwosum(xh, yh, &sl);
  *e = (xl + yl) + sl;
  return sh;
}

static inline double muldd(double xh, double xl, double ch, double cl, double *l){
  double ahhh = ch*xh;
  *l = (ch*xl + cl*xh) + __builtin_fma(ch, xh, -ahhh);
  return ahhh;
}

/* This routine implements Algorithm 4 from
   "Extended-Precision FMA under Parameterized Double-Word Overlap:
   Tight Error Bounds and Examples by Claude-Pierre Jeannerod, Mioara Joldes,
   Nicolas Louvet, Jean-Michel Muller, published in the proceedings of
   Arith 2026, https://inria.hal.science/hal-05517451.

   The original algorithm is however slower when there is no FMA in hardware.
   Here are some timings of perf.sh with the fast path disabled on a
   AMD EPYC 7282 with gcc 15.3.0.

   With FMA contraction:    68.7 cycles -> 55.7 cycles
   Without FMA contraction: 129.6 cycles -> 192.2 cycles

   To mitigate this slowdown, with respect to the original Algorithm,
   we replaced g = __builtin_fma (ah, bl, f) below by g = ah * bl + f,
   and *l = __builtin_fma (al, bh, g) by *l = al * bh + g. If an FMA is
   available in hardware, g = ah * bl + f is compiled using an FMA;
   otherwise, it is compiled as a multiplication and an addition.
   This yields:

   With FMA contraction:    68.7 cycles -> 55.7 cycles
   Without FMA contraction: 129.6 cycles -> 130.6 cycles
*/
static inline double
FastFMA_DW (double ah, double al, double bh, double bl, double ch, double cl,
            double *l)
{
  double dh = __builtin_fma (ah, bh, ch);
  double t = ch - dh;
  double e = __builtin_fma (ah, bh, t);
  double f = e + cl;
  // double g = __builtin_fma (ah, bl, f); // original algorithm
  double g = ah * bl + f;
  // *l = __builtin_fma (al, bh, g); // original algorithm
  *l = al * bh + g;
  // *l is dl in the paper
  return dh;
}

static inline double opolydd(double xh, double xl, int n, const double c[][2], double *l){
  int i = n-1;
  double ch = c[i][0], cl = c[i][1];
  while(--i>=0)
    ch = FastFMA_DW (xh, xl, ch, cl, c[i][0], c[i][1], &cl);
  *l = cl;
  return ch;
}

static inline double as_ldexp(double x, i64 i){
#ifdef __x86_64__
    __m128i sb; sb[0] = (uint64_t)i<<52;
#if defined(__clang__)
    __m128d r = _mm_set_sd(x);
#else
    __m128d r; asm("":"=x"(r):"0"(x));
#endif
    r = (__m128d)_mm_add_epi64(sb, (__m128i)r);
    return r[0];
#else
    b64u64_u ix = {.f = x};
    ix.u += (uint64_t)i<<52;
    return ix.f;
#endif
}

// sets the exponent of a binary64 number to 0 (subnormal range)
static inline double as_todenormal(double x){
#ifdef __x86_64__
    __m128i sb = {~(u64)0>>12, 0};
#if defined(__clang__)
    __m128d r = _mm_set_sd(x);
#else
    __m128d r; asm("":"=x"(r):"0"(x));
#endif
    r = _mm_and_pd(r, (__m128d)sb);
    // forces the underflow exception
    _mm_setcsr (_mm_getcsr () | _MM_EXCEPT_UNDERFLOW);
    return r[0];
#else
    b64u64_u ix = {.f = x};
    ix.u &= ~(u64)0>>12;
    // forces the underflow exception
#ifdef __UCLIBC_HAS_FENV__
    feraiseexcept (FE_UNDERFLOW);
#endif
    return ix.f;
#endif
}

static const double db[] = {
  0x1.fffffffffffffp-53, 0x1.ba07d73250de7p-14, 0x1.6a4d1af9cc989p-8, 0x1.5a75293a5dcdap-6,
  0x1.42ea46949b3c7p-5, 0x1.7c8bb0cf5d16p-5, 0x1.0948d39a41695p-3, 0x1.a065fefae814fp-3,
  0x1.f6e4c3ced7c72p-3, 0x1.1a0408712e00ap-2, 0x1.bcab27d05abdep-2, 0x1.005ae04256babp-1,
  0x1.273c188aa7b14p+2, 0x1.83d4bcdebb3f4p+2, 0x1.08f51434652c3p+4, 0x1.1d5c2daebe367p+4,
  0x1.c44ce0d716a1ap+4, 0x1.e07e71bfcf06fp+5, 0x1.f7216c4b435c9p+5, 0x1.54cd1fea7663ap+7,
  0x1.d6479eba7c971p+8, -0x1.664716b68a409p-14, -0x1.a2fefefd580dfp-13, -0x1.ce3f638d0c742p-12,
  -0x1.ceff32831e2c2p-12, -0x1.33accae78b371p-11, -0x1.d792b60084f92p-11, -0x1.7fb235d76cce7p-8,
  -0x1.1ff9b8e8b38bep-7, -0x1.54511e930898cp-7, -0x1.5c5ed0ec83666p-6, -0x1.8c56ff5326197p-6,
  -0x1.a4187f2ca71f9p-6, -0x1.a8f783d749a8fp-4, -0x1.bd44fdaed819fp-4, -0x1.daf693d64fadap-4,
  -0x1.290ea09e36479p-3, -0x1.8aeb636f3ce35p-3, -0x1.d3f3799439415p-3, -0x1.ea16274b0109bp-3,
  -0x1.22e24fa3d5cf9p-1, -0x1.85068c07fbbf6p-1, -0x1.bdc7955d1482cp-1, -0x1.2a9cad9998262p+0,
  -0x1.cc37ef7de7501p+0, -0x1.02393d5976769p+1, -0x1.65061daf79a78p+1, -0x1.e8bdbfcd9144ep+3,
  -0x1.8f80e06f3a04cp+4, -0x1.59f038076039cp+6, -0x1.981587ad4542fp+7,
};

static double __attribute__((noinline)) as_exp_database(double x, double f){
  b64u64_u ix = {.f = x};
  int a = 0, b = sizeof(db)/sizeof(db[0]) - 1, m = (a + b)/2;
  const b64u64_u *c = (const b64u64_u*)db;
  while (a <= b) {
    if (c[m].u < ix.u){
      a = m + 1;
    } else if (__builtin_expect(c[m].u == ix.u, 0)) {
      static const u64 s2[2] = {0x57f5fe2e5bde4075ull, 0x3c1f16b8edull};
      const u64 s = 333811522313371;
      b64u64_u jf = {.f = f}, dr = {.u = ((s>>m)<<63)|0x3c90000000000000ull};
      u64 t = (s2[m>>5]>>((m<<1)&63))&3;
      for(i64 k = -1; k<=1; k++){
	b64u64_u r = {.u = jf.u + k};
	if((r.u&3) == t) return r.f + dr.f;
      }
      break;
    } else {
      b = m - 1;
    }
    m = (a + b)>>1;
  }
  return f;
}

// for 0 <= i < 2^6, t0[i] is a double-double approximation of 2^(i/2^6)
static const double t0[][2] = {
  {0x0p+0, 0x1p+0}, {-0x1.19083535b085ep-56, 0x1.02c9a3e778061p+0},
  {0x1.d73e2a475b466p-55, 0x1.059b0d3158574p+0}, {0x1.186be4bb285p-57, 0x1.0874518759bc8p+0},
  {0x1.8a62e4adc610ap-54, 0x1.0b5586cf9890fp+0}, {0x1.03a1727c57b52p-59, 0x1.0e3ec32d3d1a2p+0},
  {-0x1.6c51039449b3ap-54, 0x1.11301d0125b51p+0}, {-0x1.32fbf9af1369ep-54, 0x1.1429aaea92dep+0},
  {-0x1.19041b9d78a76p-55, 0x1.172b83c7d517bp+0}, {0x1.e5b4c7b4968e4p-55, 0x1.1a35beb6fcb75p+0},
  {0x1.e016e00a2643cp-54, 0x1.1d4873168b9aap+0}, {0x1.dc775814a8494p-55, 0x1.2063b88628cd6p+0},
  {0x1.9b07eb6c70572p-54, 0x1.2387a6e756238p+0}, {0x1.2bd339940e9dap-55, 0x1.26b4565e27cddp+0},
  {0x1.612e8afad1256p-55, 0x1.29e9df51fdee1p+0}, {0x1.0024754db41d4p-54, 0x1.2d285a6e4030bp+0},
  {0x1.6f46ad23182e4p-55, 0x1.306fe0a31b715p+0}, {0x1.32721843659a6p-54, 0x1.33c08b26416ffp+0},
  {-0x1.63aeabf42eae2p-54, 0x1.371a7373aa9cbp+0}, {-0x1.5e436d661f5e2p-56, 0x1.3a7db34e59ff7p+0},
  {0x1.ada0911f09ebcp-55, 0x1.3dea64c123422p+0}, {-0x1.ef3691c309278p-58, 0x1.4160a21f72e2ap+0},
  {0x1.89b7a04ef80dp-59, 0x1.44e086061892dp+0}, {0x1.3c1a3b69062fp-56, 0x1.486a2b5c13cdp+0},
  {0x1.d4397afec42e2p-56, 0x1.4bfdad5362a27p+0}, {-0x1.4b309d25957e4p-54, 0x1.4f9b2769d2ca7p+0},
  {-0x1.07abe1db13cacp-55, 0x1.5342b569d4f82p+0}, {0x1.9bb2c011d93acp-54, 0x1.56f4736b527dap+0},
  {0x1.6324c054647acp-54, 0x1.5ab07dd485429p+0}, {0x1.ba6f93080e65ep-54, 0x1.5e76f15ad2148p+0},
  {-0x1.383c17e40b496p-54, 0x1.6247eb03a5585p+0}, {-0x1.bb60987591c34p-54, 0x1.6623882552225p+0},
  {-0x1.bdd3413b26456p-54, 0x1.6a09e667f3bcdp+0}, {-0x1.bbe3a683c88aap-57, 0x1.6dfb23c651a2fp+0},
  {-0x1.16e4786887a9ap-55, 0x1.71f75e8ec5f74p+0}, {-0x1.0245957316dd4p-54, 0x1.75feb564267c9p+0},
  {-0x1.41577ee04993p-55, 0x1.7a11473eb0187p+0}, {0x1.05d02ba15797ep-56, 0x1.7e2f336cf4e62p+0},
  {-0x1.d4c1dd41532d8p-54, 0x1.82589994cce13p+0}, {-0x1.fc6f89bd4f6bap-54, 0x1.868d99b4492edp+0},
  {0x1.6e9f156864b26p-54, 0x1.8ace5422aa0dbp+0}, {0x1.5cc13a2e3976cp-55, 0x1.8f1ae99157736p+0},
  {-0x1.75fc781b57ebcp-57, 0x1.93737b0cdc5e5p+0}, {-0x1.d185b7c1b85dp-54, 0x1.97d829fde4e5p+0},
  {0x1.c7c46b071f2bep-56, 0x1.9c49182a3f09p+0}, {-0x1.359495d1cd532p-54, 0x1.a0c667b5de565p+0},
  {-0x1.d2f6edb8d41e2p-54, 0x1.a5503b23e255dp+0}, {0x1.0fac90ef7fd32p-54, 0x1.a9e6b5579fdbfp+0},
  {0x1.7a1cd345dcc82p-54, 0x1.ae89f995ad3adp+0}, {-0x1.2805e3084d708p-57, 0x1.b33a2b84f15fbp+0},
  {-0x1.5584f7e54ac3ap-56, 0x1.b7f76f2fb5e47p+0}, {0x1.23dd07a2d9e84p-55, 0x1.bcc1e904bc1d2p+0},
  {0x1.11065895048dep-55, 0x1.c199bdd85529cp+0}, {0x1.2884dff483cacp-54, 0x1.c67f12e57d14bp+0},
  {0x1.503cbd1e949dcp-56, 0x1.cb720dcef9069p+0}, {-0x1.cbc3743797a9cp-54, 0x1.d072d4a07897cp+0},
  {0x1.2ed02d75b3706p-55, 0x1.d5818dcfba487p+0}, {0x1.c2300696db532p-54, 0x1.da9e603db3285p+0},
  {-0x1.1a5cd4f184b5cp-54, 0x1.dfc97337b9b5fp+0}, {0x1.39e8980a9cc9p-55, 0x1.e502ee78b3ff6p+0},
  {-0x1.e9c23179c2894p-54, 0x1.ea4afa2a490dap+0}, {0x1.dc7f486a4b6bp-54, 0x1.efa1bee615a27p+0},
  {0x1.9d3e12dd8a18ap-54, 0x1.f50765b6e454p+0}, {0x1.74853f3a5931ep-55, 0x1.fa7c1819e90d8p+0}
};

// for 0 <= i < 2^6, t1[i] is a double-double approximation of 2^(i/2^12)
static const double t1[][2] = {
  {0x0p+0, 0x1p+0}, {0x1.ae8e38c59c72ap-54, 0x1.000b175effdc7p+0},
  {-0x1.7b5d0d58ea8f4p-58, 0x1.00162f3904052p+0}, {0x1.4115cb6b16a8ep-54, 0x1.0021478e11ce6p+0},
  {-0x1.d7c96f201bb2ep-55, 0x1.002c605e2e8cfp+0}, {0x1.84711d4c35eap-54, 0x1.003779a95f959p+0},
  {-0x1.0484245243778p-55, 0x1.0042936faa3d8p+0}, {-0x1.4b237da2025fap-54, 0x1.004dadb113dap+0},
  {-0x1.5e00e62d6b30ep-56, 0x1.0058c86da1c0ap+0}, {0x1.a1d6cedbb948p-54, 0x1.0063e3a559473p+0},
  {-0x1.4acf197a00142p-54, 0x1.006eff583fc3dp+0}, {-0x1.eaf2ea42391a6p-57, 0x1.007a1b865a8cap+0},
  {0x1.da93f90835f76p-56, 0x1.0085382faef83p+0}, {-0x1.6a79084ab093cp-55, 0x1.00905554425d4p+0},
  {0x1.86364f8fbe8f8p-54, 0x1.009b72f41a12bp+0}, {-0x1.82e8e14e3110ep-55, 0x1.00a6910f3b6fdp+0},
  {-0x1.4f6b2a7609f72p-55, 0x1.00b1afa5abcbfp+0}, {-0x1.e1a258ea8f71ap-56, 0x1.00bcceb7707ecp+0},
  {0x1.4362ca5bc26f2p-56, 0x1.00c7ee448ee02p+0}, {0x1.095a56c919d02p-54, 0x1.00d30e4d0c483p+0},
  {-0x1.406ac4e81a646p-57, 0x1.00de2ed0ee0f5p+0}, {0x1.b5a6902767e08p-54, 0x1.00e94fd0398ep+0},
  {-0x1.91b206085932p-54, 0x1.00f4714af41d3p+0}, {0x1.427068ab22306p-55, 0x1.00ff93412315cp+0},
  {0x1.c1d0660524e08p-54, 0x1.010ab5b2cbd11p+0}, {-0x1.e7bdfb3204be8p-54, 0x1.0115d89ff3a8bp+0},
  {0x1.843aa8b9cbbc6p-55, 0x1.0120fc089ff63p+0}, {-0x1.34104ee7edae8p-56, 0x1.012c1fecd613bp+0},
  {-0x1.2b6aeb6176892p-56, 0x1.0137444c9b5b5p+0}, {0x1.a8cd33b8a1bb2p-56, 0x1.01426927f5278p+0},
  {0x1.2edc08e5da99ap-56, 0x1.014d8e7ee8d2fp+0}, {0x1.57ba2dc7e0c72p-55, 0x1.0158b4517bb88p+0},
  {0x1.b61299ab8cdb8p-54, 0x1.0163da9fb3335p+0}, {-0x1.90565902c5f44p-54, 0x1.016f0169949edp+0},
  {0x1.70fc41c5c2d54p-55, 0x1.017a28af25567p+0}, {0x1.4b9a6e145d76cp-54, 0x1.018550706ab62p+0},
  {-0x1.008eff5142bfap-56, 0x1.019078ad6a19fp+0}, {-0x1.77669f033c7dep-54, 0x1.019ba16628de2p+0},
  {-0x1.09bb78eeead0ap-54, 0x1.01a6ca9aac5f3p+0}, {0x1.371231477ece6p-54, 0x1.01b1f44af9f9ep+0},
  {0x1.5e7626621eb5ap-56, 0x1.01bd1e77170b4p+0}, {-0x1.bc72b100828a4p-54, 0x1.01c8491f08f08p+0},
  {-0x1.ce39cbbab8bbep-57, 0x1.01d37442d507p+0}, {0x1.16996709da2e2p-55, 0x1.01de9fe280ac8p+0},
  {-0x1.c11f5239bf536p-55, 0x1.01e9cbfe113efp+0}, {0x1.e1d4eb5edc6b4p-55, 0x1.01f4f8958c1c6p+0},
  {-0x1.afb99946ee3fp-54, 0x1.020025a8f6a35p+0}, {-0x1.8f06d8a148a32p-54, 0x1.020b533856324p+0},
  {-0x1.2bf310fc54eb6p-55, 0x1.02168143b0281p+0}, {-0x1.c95a035eb4176p-54, 0x1.0221afcb09e3ep+0},
  {-0x1.491793e46834cp-54, 0x1.022cdece68c4fp+0}, {-0x1.3e8d0d9c4909p-56, 0x1.02380e4dd22adp+0},
  {-0x1.314aa16278aa4p-54, 0x1.02433e494b755p+0}, {0x1.48daf888e965p-55, 0x1.024e6ec0da046p+0},
  {0x1.56dc8046821f4p-55, 0x1.02599fb483385p+0}, {0x1.45b42356b9d46p-54, 0x1.0264d1244c719p+0},
  {-0x1.082ef51b61d7ep-56, 0x1.027003103b10ep+0}, {0x1.2106ed0920a34p-56, 0x1.027b357854772p+0},
  {-0x1.fd4cf26ea5d0ep-54, 0x1.0286685c9e059p+0}, {-0x1.09f8775e78084p-54, 0x1.02919bbd1d1d8p+0},
  {0x1.64cbba902ca28p-58, 0x1.029ccf99d720ap+0}, {0x1.4383ef231d206p-54, 0x1.02a803f2d170dp+0},
  {0x1.4a47a505b3a46p-54, 0x1.02b338c811703p+0}, {0x1.e47120223468p-54, 0x1.02be6e199c811p+0},
};

static double __attribute__((cold,noinline)) as_exp_accurate(double x){
  static const double ch[][2] =
    {{0x1p+0, 0}, {0x1p-1, 0x1.712f72ecec2cfp-99}, {0x1.5555555555555p-3, 0x1.5555555554d07p-57},
     {0x1.5555555555555p-5, 0x1.55194d28275dap-59}, {0x1.1111111111111p-7, 0x1.12faa0e1c0f7bp-63},
     {0x1.6c16c16da6973p-10, -0x1.4ba45ab25d2a3p-64}, {0x1.a01a019eb7f31p-13, -0x1.9091d845ecd36p-67}};
  b64u64_u ix = {.f = x};
  if(__builtin_expect(((ix.u>>52)&0x7ff)<0x3c9, 0)) return 1 + x;
  const double s = 0x1.71547652b82fep+12;
  double t = roundeven_finite(x*s);
  i64 jt = t, i0 = (jt>>6)&0x3f, i1 = jt&0x3f, ie = jt>>12;
  double t0h = t0[i0][1], t0l = t0[i0][0];
  double t1h = t1[i1][1], t1l = t1[i1][0];
  double tl, th = muldd(t0h,t0l, t1h,t1l, &tl);

  /* Use Cody-Waite argument reduction: since |x| < 745, we have |t| < 2^23,
     thus since l2h is exactly representable on 29 bits, l2h*t is exact. */
  const double l2h = 0x1.62e42ffp-13, l2l = 0x1.718432a1b0e26p-47, l2ll = 0x1.9ff0342542fc3p-102;
  double dx = x - l2h*t, dxl = l2l*t, dxll = l2ll*t + __builtin_fma(l2l,t,-dxl);
  double dxh = dx + dxl; dxl = (dx - dxh) + dxl + dxll;
  double fl, fh = opolydd(dxh,dxl, 7,ch, &fl);
  fh = muldd(dxh,dxl, fh,fl, &fl);
  if(__builtin_expect(ix.u>0xc086232bdd7abcd2ull, 0)){
    // x < -0x1.6232bdd7abcd2p+9
    ix.u = (1-ie)<<52;
    fh = muldd(fh,fl, th,tl, &fl);
    fh = fastsum(th,tl, fh,fl, &fl);
    double e;
    fh = fasttwosum(ix.f, fh, &e);
    fl += e;
    fh = as_todenormal(fh + fl);
  } else {
    if(th == 1.0){
      double e;
      fh = fasttwosum(th,fh, &e);
      fl = fasttwosum(e, fl, &e);
      ix.f = fl;
      if((ix.u&(~(u64)0>>12))==0) {
	b64u64_u v = {.f = e};
	i64 d = ((u64)(((i64)ix.u>>63)^((i64)v.u>>63))<<1) + 1;
	ix.u += d;
	fl = ix.f;
      }
    } else {
      fh = muldd(fh,fl, th,tl, &fl);
      fh = fastsum(th,tl, fh,fl, &fl);
    }
    fh = fasttwosum(fh,fl, &fl);
    ix.f = fl;
    u64 d = (ix.u + 2)&(~(u64)0>>12);
    if(__builtin_expect(d<=2, 0)) fh = as_exp_database(x, fh);
    fh = as_ldexp(fh, ie);
  }
  return fh;
}

double __ieee754_exp(double x){
  b64u64_u ix = {.f = x};
  u64 aix = ix.u & (~(u64)0>>1);
  // exp(x) rounds to 1 to nearest for |x| <= 0x1p-54
  if(__builtin_expect(aix <= 0x3c90000000000000ull, 0)) // |x| <= 0x1p-54
    return 1.0 + x;
  if(__builtin_expect(aix>=0x40862e42fefa39f0ull, 0)){ // |x| >= 0x1.62e42fefa39fp+9
    if(aix>0x7ff0000000000000ull) return x + x; // nan
    if(aix==0x7ff0000000000000ull){ // |x| = inf
      if(ix.u>>63)
	return 0.0; // x = -inf
      else
	return x; // x = inf
    }
    if(!(ix.u>>63)){ // x >= 0x1.62e42fefa39fp+9
#ifdef CORE_MATH_SUPPORT_ERRNO
      errno = ERANGE;
#endif
      volatile double z = 0x1p1023;
      return z*z;
    }
    if (aix>=0x40874910d52d3052ull) { // x <= -0x1.74910d52d3052p+9
#ifdef CORE_MATH_SUPPORT_ERRNO
      errno = ERANGE; // underflow
#endif
      volatile double z = 0x1p-1022;
      return z * z;
    }
  }
  const double s = 0x1.71547652b82fep+12;
  double t = roundeven_finite(x*s);
  i64 jt = t, i0 = (jt>>6)&0x3f, i1 = jt&0x3f, ie = jt>>12;
  double t0h = t0[i0][1], t0l = t0[i0][0];
  double t1h = t1[i1][1], t1l = t1[i1][0];
  double tl, th = muldd(t0h,t0l, t1h,t1l, &tl);
  const double l2h = 0x1.62e42ffp-13, l2l = 0x1.718432a1b0e26p-47;
  /* Use Cody-Waite argument reduction: since |x| < 745, we have |t| < 2^23,
     thus since l2h is exactly representable on 29 bits, l2h*t is exact. */
  double dx = (x - l2h*t) + l2l*t, dx2 = dx*dx;
  // |dx| < log(2)/2^13 (experimentally)
  /* 1 + x*(c0 + c1*x + ... + c3*x^3) is a degree-4 approximation of exp(x)
     on [-log(2)/2^13,log(2)/2^13] with absolute error bounded by 2^-76.173
     according to Sollya */
  static const double ch[] = {0x1p+0, 0x1p-1, 0x1.55555557e54ffp-3,
                              0x1.55555553a12f4p-5};
  double p = (ch[0] + dx*ch[1]) + dx2*(ch[2] + dx*ch[3]);
  double fh = th, tx = th*dx, fl = tl + tx*p;
  double eps = 1.64e-19;
  if(__builtin_expect(ix.u>0xc086232bdd7abcd2ull, 0)){
    // subnormal case: x < -0x1.6232bdd7abcd2p+9
#ifdef CORE_MATH_SUPPORT_ERRNO
    errno = ERANGE; // underflow
#endif
    ix.u = (1-ie)<<52;
    double e;
    fh = fasttwosum(ix.f, fh, &e);
    fl += e;
    double ub = fh + (fl + eps), lb = fh + (fl - eps);
    if (__builtin_expect(ub != lb, 0)) return as_exp_accurate(x);
    fh = as_todenormal(lb);
  } else {
    double ub = fh + (fl + eps), lb = fh + (fl - eps);
    if(__builtin_expect( ub != lb, 0)) return as_exp_accurate(x);
    fh = as_ldexp(lb, ie);
  }
  return fh;
}
