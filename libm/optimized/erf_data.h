/*
 * Copyright (c) 2022, Arm Limited.
 * SPDX-License-Identifier: MIT
 */
/* The declarations erf.c needs from Arm's math_config.h, nothing more. */
#ifndef _ERF_DATA_H
#define _ERF_DATA_H

#include <features.h>
#include <stdint.h>

#define ERF_POLY_A_NCOEFFS 10
#define ERFC_POLY_C_NCOEFFS 16
#define ERFC_POLY_D_NCOEFFS 18
#define ERFC_POLY_E_NCOEFFS 14
#define ERFC_POLY_F_NCOEFFS 17
extern const struct erf_data
{
  double erf_poly_A[ERF_POLY_A_NCOEFFS];
  double erf_ratio_N_A[5];
  double erf_ratio_D_A[5];
  double erf_ratio_N_B[7];
  double erf_ratio_D_B[6];
  double erfc_poly_C[ERFC_POLY_C_NCOEFFS];
  double erfc_poly_D[ERFC_POLY_D_NCOEFFS];
  double erfc_poly_E[ERFC_POLY_E_NCOEFFS];
  double erfc_poly_F[ERFC_POLY_F_NCOEFFS];
} __erf_data;

#endif
