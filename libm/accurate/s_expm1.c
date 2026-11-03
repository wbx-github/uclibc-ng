/* uClibc-ng: taken from the CORE-MATH project
   (git_clones/core-math-inria, src/binary64/expm1/expm1.c), selected by
   UCLIBC_LIBM_ACCURATE.  Correctly rounded.  Needs neither a 128-bit integer nor
   fenv, so it ports unchanged apart from the name -- but it calls fma five times,
   which without hardware fma lands in libm's own s_fma.c.

   Changes against the original: cr_expm1 is expm1.  */

/* Correctly rounded e^x-1 function for binary64 values.

Copyright (c) 2024 Alexei Sibidanov.

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

#include <math.h>
#include <stdint.h>
#include "math_private.h"
#ifdef CORE_MATH_SUPPORT_ERRNO
#include <errno.h>
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

static inline double mulddd(double xh, double xl, double c, double *l){
  double h = c*xh;
  *l = c*xl + __builtin_fma(c, xh, -h);
  return h;
}

static inline double opolydd(double xh, double xl, int n, const double c[][2], double *l){
  int i = n-1;
  double ch = c[i][0], cl = c[i][1];
  while(--i>=0){
    ch = muldd(xh,xl, ch,cl, &cl);
    ch = fastsum(c[i][0],c[i][1], ch,cl, &cl);
  }
  *l = cl;
  return ch;
}

static inline double opolyddd(double x, int n, const double c[][2], double *l){
  int i = n-1;
  double ch = fasttwosum(c[i][0], *l, l), cl = c[i][1] + *l;
  while(--i>=0){
    ch = mulddd(ch,cl, x, &cl);
    ch = fastsum(c[i][0],c[i][1], ch,cl, &cl);
  }
  *l = cl;
  return ch;
}

static inline double as_ldexp(double x, i64 i){
#ifdef __x86_64__
  __m128i sb; sb[0] = (u64)i<<52;
#if defined(__clang__)
  __m128d r = _mm_set_sd(x);
#else
  __m128d r; asm("":"=x"(r):"0"(x));
#endif
  r = (__m128d)_mm_add_epi64(sb, (__m128i)r);
  return r[0];
#else
  b64u64_u ix = {.f = x};
  ix.u += (u64)i<<52;
  return ix.f;
#endif
}

static const double db[] =
  {0x1.e923c188ea79bp-4, 0x1.1a0408712e00ap-2, 0x1.1c38132777b26p-2, 0x1.27f4980d511ffp-2,
   0x1.8172a0e02f90ep-2, 0x1.8bbe2fb45c151p-2, 0x1.bcab27d05abdep-2, 0x1.005ae04256babp-1,
   0x1.accfbe46b4efp-1, 0x1.d086543694c5ap-1, 0x1.273c188aa7b14p+2, 0x1.83d4bcdebb3f4p+2,
   0x1.08f51434652c3p+4, 0x1.1d5c2daebe367p+4, 0x1.c44ce0d716a1ap+4, 0x1.2ee70220fb1c5p+5,
   0x1.89d56a0c38e6fp+5, 0x1.7a60ee15e3e9dp+6, 0x1.1f0da93354198p+7, 0x1.54cd1fea7663ap+7,
   0x1.556c678d5e976p+7, 0x1.2da9e5e6af0bp+8, 0x1.9e7b643238a14p+8, 0x1.d6479eba7c971p+8,
   0x1.0bc04af1b09f5p+9, -0x1.ab86cb1743b75p-4, -0x1.119aae6072d39p-2, -0x1.175693a03b59p-2,
   -0x1.474d4de7c14bbp-2, -0x1.789d025948efap-2, -0x1.82b5dfaf59b4cp-2, -0x1.9d871e078ebcep-2,
   -0x1.1397add4538acp-1, -0x1.22e24fa3d5cf9p-1, -0x1.dc2b5df1f7d3dp-1, -0x1.0a54d87783d6fp+0,
   -0x1.2a9cad9998262p+0, -0x1.e42a2abb1bf0fp+0, };

static double __attribute__((noinline)) as_expm1_database(double x, double f){
  b64u64_u ix = {.f = x};
  int a = 0, b = sizeof(db)/sizeof(db[0]) - 1, m = (a + b)/2;
  const b64u64_u *c = (const b64u64_u*)db;
  while (a <= b) {
    if (c[m].u < ix.u){
      a = m + 1;
    } else if (__builtin_expect(c[m].u == ix.u, 0)) {
      static const u64 s2[2] = {0x76f58b0d65bd5553ull, 0xc06ull};
      const u64 s = 0x300e81651cull;
      b64u64_u jf = {.f = f}, dr = {.u = ((s>>m)<<63)| (((jf.u>>52)&0x7ff) - 54)<<52};
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
static const double tz[][2] = {
  {-0x1.797d4686c5393p-57, -0x1.c5041854df7d4p-3}, {-0x1.ea1cb9d163339p-55, -0x1.b881a23aebb48p-3},
  {0x1.f483a3e8cd60fp-55, -0x1.abe60e1f21838p-3}, {0x1.dffd920f493dbp-56, -0x1.9f3129931fabp-3},
  {-0x1.51bfdbb129094p-55, -0x1.9262c1c3430ap-3}, {0x1.cd3e5225e2206p-55, -0x1.857aa375db4e4p-3},
  {0x1.e3a6bdaece8f9p-58, -0x1.78789b0a5e0cp-3}, {-0x1.daf2ae0c2d3d4p-55, -0x1.6b5c7478983d8p-3},
  {-0x1.fd36226fadd44p-56, -0x1.5e25fb4fde21p-3}, {0x1.d887cd0341abp-56, -0x1.50d4fab639758p-3},
  {-0x1.676a52a1a618bp-55, -0x1.43693d679612cp-3}, {0x1.9776b420ad283p-56, -0x1.35e28db4ecd9cp-3},
  {0x1.3d5fd7d70a5edp-56, -0x1.2840b5836cf68p-3}, {0x1.a94ad2c8fa0bfp-58, -0x1.1a837e4ba376p-3},
  {0x1.6ad4c353465bp-61, -0x1.0caab118a1278p-3}, {-0x1.8bba170e59b65p-56, -0x1.fd6c2d0e3d91p-4},
  {-0x1.e1e0a76cb0685p-55, -0x1.e14aed893eefp-4}, {0x1.fe131f55e75f8p-55, -0x1.c4f1331d22d4p-4},
  {-0x1.b5beee8bcee31p-55, -0x1.a85e8c62d9c1p-4}, {-0x1.7fe9b02c25e9bp-56, -0x1.8b92870fa2b58p-4},
  {-0x1.32ae7bdaf1116p-55, -0x1.6e8caff341fe8p-4}, {0x1.a6cfe58cbd73bp-56, -0x1.514c92f634788p-4},
  {0x1.8798de3138a56p-57, -0x1.33d1bb17df2e8p-4}, {-0x1.589321a7ef10bp-60, -0x1.161bb26cbb59p-4},
  {-0x1.8d0e700fcfb65p-56, -0x1.f0540438fd5cp-5}, {0x1.473ef07d5dd3bp-55, -0x1.b3f864c08p-5},
  {-0x1.38e62149c16e2p-55, -0x1.77239501304p-5}, {-0x1.08bb6309bd394p-58, -0x1.39d4a1a77e05p-5},
  {-0x1.bad3fd501a227p-55, -0x1.f8152aee945p-6}, {0x1.3d27ac39ed253p-57, -0x1.7b88f290230ep-6},
  {-0x1.b60bbd08aac55p-55, -0x1.fc055004416cp-7}, {-0x1.a00d03b3359dep-59, -0x1.fe0154aaeed8p-8},
  {0x0p+0, 0x0p+0},
  {0x1.861931c15e39bp-55, 0x1.0100ab00222cp-7}, {0x1.7ab864b3e9045p-56, 0x1.0202ad5778e4p-6},
  {0x1.4e5659d75e95bp-56, 0x1.84890d904374p-6}, {0x1.8e0bd083aba81p-56, 0x1.040ac0224fd9p-5},
  {0x1.45cc1cf959b1bp-60, 0x1.465509d383ebp-5}, {-0x1.eb6980ce14da7p-55, 0x1.89246d053d18p-5},
  {0x1.7324137d6c342p-56, 0x1.cc79f4f5613ap-5}, {-0x1.5272ff30eed1bp-59, 0x1.082b577d34ed8p-4},
  {-0x1.1280f19dace1cp-55, 0x1.2a5dd543ccc5p-4}, {-0x1.d550af31c8ec3p-55, 0x1.4cd4fc989cd68p-4},
  {0x1.7923b72aa582dp-55, 0x1.6f9157587069p-4}, {-0x1.76c2e732457f1p-56, 0x1.92937074e0cd8p-4},
  {0x1.81f5c92a5200fp-55, 0x1.b5dbd3f68122p-4}, {0x1.e8ac7a4d3206cp-55, 0x1.d96b0eff0e79p-4},
  {-0x1.12db6f4bbe33bp-56, 0x1.fd41afcba45e8p-4}, {-0x1.8c4a5df1ec7e5p-58, 0x1.10b022db7ae68p-3},
  {-0x1.bd4b1c37ea8a2p-57, 0x1.22e3b09dc54d8p-3}, {0x1.5aeb9860044dp-55, 0x1.353bc9fb00b2p-3},
  {-0x1.4c26602c63fdap-57, 0x1.47b8b853aafecp-3}, {-0x1.7f644c1f9d314p-55, 0x1.5a5ac59b963ccp-3},
  {0x1.f5aa8ec61fc2dp-55, 0x1.6d223c5b10638p-3}, {0x1.7ab912c69ffebp-61, 0x1.800f67b00d7b8p-3},
  {-0x1.b3564bc0ec9cdp-58, 0x1.9322934f54148p-3}, {0x1.6a7062465be33p-55, 0x1.a65c0b85ac1a8p-3},
  {-0x1.85718d2ff1bf4p-55, 0x1.b9bc1d3910094p-3}, {-0x1.045cb0c685e08p-55, 0x1.cd4315e9e0834p-3},
  {-0x1.6e7fb859d5055p-62, 0x1.e0f143b41a554p-3}, {0x1.51bbdee020603p-55, 0x1.f4c6f5508ee5cp-3},
  {0x1.e17611afc42c5p-57, 0x1.04623d0b0f8c8p-2}, {-0x1.1c5b2e8735a43p-56, 0x1.0e7510fd7c564p-2},
  {-0x1.25fe139c4cffdp-55, 0x1.189c1ecaeb084p-2}, {-0x1.89843c4964554p-56, 0x1.22d78f0fa061ap-2},
};

static double __attribute__((noinline)) as_expm1_accurate(double x){
  b64u64_u ix = {.f = x};
  if(__builtin_expect(__builtin_fabs(x)<0.25, 0)){
    static const double cl[] =
      {0x1.93974a8ca5354p-37, 0x1.ae7f3e71e4908p-41, 0x1.ae7f357341648p-45, 0x1.952c7f96664cbp-49,
       0x1.686f8ce633aaep-53, 0x1.2f49b2fbfb5b6p-57};
    static const double ch[][2] = {
      {0x1.5555555555555p-3, 0x1.5555555555554p-57}, {0x1.5555555555555p-5, 0x1.5555555555123p-59},
      {0x1.1111111111111p-7, 0x1.1111111118167p-63}, {0x1.6c16c16c16c17p-10, -0x1.f49f49e220ceap-65},
      {0x1.a01a01a01a01ap-13, 0x1.a019eff6f919cp-73}, {0x1.a01a01a01a01ap-16, 0x1.9fcff48a75b41p-76},
      {0x1.71de3a556c734p-19, -0x1.c14f73758cd7fp-73}, {0x1.27e4fb7789f5cp-22, 0x1.dfce97931018fp-76},
      {0x1.ae64567f544e3p-26, 0x1.c513da9e4c9c5p-80}, {0x1.1eed8eff8d831p-29, 0x1.ca00af84f2b6p-83},
      {0x1.6124613a86e8fp-33, 0x1.f27ac6000898fp-87}
    };

    double fl = x*(cl[0] + x*(cl[1] + x*(cl[2] + x*(cl[3] + x*(cl[4] + x*(cl[5]))))));
    double fh = opolyddd(x, 11,ch, &fl);
    fh = mulddd(fh,fl, x, &fl);
    fh = mulddd(fh,fl, x, &fl);
    fh = mulddd(fh,fl, x, &fl);
    double hx = 0.5*x, x2h = x*hx, x2l = __builtin_fma(x,hx,-x2h);
    fh = fastsum(x2h,x2l, fh,fl, &fl);
    double v2, v0 = fasttwosum(x, fh, &v2), v1 = fasttwosum(v2, fl, &v2);
    v0 = fasttwosum(v0,v1, &v1);
    v1 = fasttwosum(v1,v2, &v2);
    ix.f = v1;
    if((ix.u&(~(u64)0>>12))==0) {
      if(!(ix.u<<1)) return as_expm1_database(x,v0);
      b64u64_u v = {.f = v2};
      i64 d = ((u64)(((i64)ix.u>>63)^((i64)v.u>>63))<<1) + 1;
      ix.u += d;
      v1 = ix.f;
    }
    return v0 + v1;
  } else {
    static const double ch[][2] =
      {{0x1p+0, 0}, {0x1p-1, 0x1.712f72ecec2cfp-99}, {0x1.5555555555555p-3, 0x1.5555555554d07p-57},
       {0x1.5555555555555p-5, 0x1.55194d28275dap-59}, {0x1.1111111111111p-7, 0x1.12faa0e1c0f7bp-63},
       {0x1.6c16c16da6973p-10, -0x1.4ba45ab25d2a3p-64}, {0x1.a01a019eb7f31p-13, -0x1.9091d845ecd36p-67}};
    const double s = 0x1.71547652b82fep+12;
    double t = roundeven_finite(x*s);
    i64 jt = t, i0 = (jt>>6)&0x3f, i1 = jt&0x3f, ie = jt>>12;
    double t0h = t0[i0][1], t0l = t0[i0][0];
    double t1h = t1[i1][1], t1l = t1[i1][0];
    double tl, th = muldd(t0h,t0l, t1h,t1l, &tl);

    const double l2h = 0x1.62e42ffp-13, l2l = 0x1.718432a1b0e26p-47, l2ll = 0x1.9ff0342542fc3p-102;
    double dx = x - l2h*t, dxl = l2l*t, dxll = l2ll*t + __builtin_fma(l2l,t,-dxl);
    double dxh = dx + dxl; dxl = (dx - dxh) + dxl + dxll;
    double fl, fh = opolydd(dxh,dxl, 7,ch, &fl);
    fh = muldd(dxh,dxl, fh,fl, &fl);
    fh = muldd(fh,fl, th,tl, &fl);
    fh = fastsum(th,tl, fh,fl, &fl);
    b64u64_u off = {.u = (u64)(2048+1023-ie)<<52};
    double e;
    if(__builtin_expect(ie<53, 1))
      fh = fasttwosum(off.f, fh, &e);
    else{
      if(ie<104)
	fh = fasttwosum(fh, off.f, &e);
      else
	e = 0;
    }
    fl += e;
    fh = fasttwosum(fh,fl, &fl);
    ix.f = fl;
    u64 d = (ix.u + 8)&(~(u64)0>>12);
    if(__builtin_expect(d<=8, 0)) fh = as_expm1_database(x, fh);
    fh = as_ldexp(fh, ie);
    return fh;
  }
}

double expm1(double x){
  b64u64_u ix = {.f = x};
  u64 aix = ix.u & (~(u64)0>>1);
  if(__builtin_expect(aix < 0x3fd0000000000000ull, 1)){ // |x| < 0.25
    if( __builtin_expect(aix < 0x3ca0000000000000ull, 0)) { // |x| < 2^-53
      if( !aix ) return x;
      double res = __builtin_fma(0x1p-54, __builtin_fabs(x), x);
#ifdef CORE_MATH_SUPPORT_ERRNO
      /* we have underflow for |x| < 2^-1022 and for x=-0x1p-1022 and
         rounding towards zero */
      if (aix < 0x10000000000000ull || __builtin_fabs (res) < 0x1p-1022)
        errno = ERANGE; // underflow
#endif
      return res;
    }
    double sx = 0x1p7*x, fx = roundeven_finite(sx), z = sx - fx, z2 = z*z;
    i64 i = fx;
    double th = tz[i+32][1], tl = tz[i+32][0];
    static const double c[] =
      {0x1p-7, 0x1p-15, 0x1.55555555551adp-24, 0x1.555555555599cp-33, 0x1.11111ad1ad69dp-42, 0x1.6c16c168b1fb5p-52};
    double fh = z*c[0], fl = z2*((c[1] + z*c[2]) + z2*(c[3] + z*(c[4] + z*c[5])));
    double e0 = 0x1.ap-65, eps = z2*e0 + 0x1p-104;
    double rl, rh = fasttwosum(th,fh,&rl); rl += tl + fl;
    fh = muldd(th,tl, fh,fl, &fl);
    fh = fastsum(rh,rl, fh,fl, &fl);
    double ub = fh + (fl + eps), lb = fh + (fl - eps);
    if(__builtin_expect( ub != lb, 0)) return as_expm1_accurate(x);
    return lb;
  } else { // |x| >= 0.25
    if(__builtin_expect(aix>=0x40862e42fefa39f0ull, 0)){
      // |x| >= 0x1.62e42fefa39fp+9
      if(aix>0x7ff0000000000000ull) return x + x; // nan
      if(aix==0x7ff0000000000000ull){ // +/-inf
	if(ix.u>>63) // -inf
	  return -1.0;
	else
	  return x; // +inf
      }
      if(!(ix.u>>63)){ // x >= 0x1.62e42fefa39fp+9
#ifdef CORE_MATH_SUPPORT_ERRNO
        errno = ERANGE; // overflow
#endif
	volatile double z = 0x1p1023;
	return z*z;
      }
    }
    if(__builtin_expect(ix.u>=0xc0425e4f7b2737faull, 0)){
      // x <= -0x1.25e4f7b2737fap+5
      if(ix.u>=0xc042b708872320e2ull) // x <= -0x1.2b708872320e2p+5
	return -1.0 + 0x1p-55;
      return (0x1.25e4f7b2737fap+5 + x + 0x1.8486612173c69p-51)*0x1.71547652b82fep-54 - 0x1.fffffffffffffp-1;
    }

    const double s = 0x1.71547652b82fep+12; // s approximates 2^12/log(2)
    double t = roundeven_finite(x*s);
    i64 jt = t, i0 = (jt>>6)&0x3f, i1 = jt&0x3f, ie = jt>>12;
    double t0h = t0[i0][1], t0l = t0[i0][0];
    double t1h = t1[i1][1], t1l = t1[i1][0];
    double tl, th = muldd(t0h,t0l, t1h,t1l, &tl);
    const double l2h = 0x1.62e42ffp-13, l2l = 0x1.718432a1b0e26p-47;
    double dx = (x - l2h*t) + l2l*t, dx2 = dx*dx;
    static const double ch[] = {0x1p+0, 0x1p-1, 0x1.55555557e54ffp-3, 0x1.55555553a12f4p-5};
    double p = (ch[0] + dx*ch[1]) + dx2*(ch[2] + dx*ch[3]);
    double fh = th, tx = th*dx, fl = tl + tx*p;
    double eps = 1.64e-19*th;
    b64u64_u off = {.u = (u64)(2048+1023-ie)<<52};
    double e;
    if(__builtin_expect(ie<53, 1)){
      fh = fasttwosum(off.f, fh, &e);
    } else {
      if(ie<75){
	fh = fasttwosum(fh, off.f, &e);
      } else
	e = 0;
    }
    fl += e;
    double ub = fh + (fl + eps), lb = fh + (fl - eps);
    if(__builtin_expect( ub != lb, 0)) return as_expm1_accurate(x);
    return as_ldexp(lb, ie);
  }
}
libm_hidden_def(expm1)
