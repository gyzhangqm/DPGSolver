// Copyright 2017 Philip Zwanenburg
// MIT License (https://github.com/PhilipZwanenburg/DPGSolver/blob/master/LICENSE)

#include "test_regression_fluxes_inviscid.h"

#include <stdlib.h>
#include <stdio.h>

#include "Parameters.h"
#include "Test.h"

#include "test_support.h"
#include "array_norm.h"
#include "fluxes_structs.h"
#include "fluxes_inviscid.h"

/*
 *	Purpose:
 *		Test correctness of implementation of functions relating to inviscid flux computation.
 *
 *	Comments:
 *		Add in test for numerical flux functions through comparison with Godunov (ToBeDeleted).
 *
 *	Notation:
 *
 *	References:
 */

void test_regression_fluxes_inviscid(void)
{
	unsigned int pass;

	/*
	 *	Input:
	 *
	 *	Expected Output:
	 *
	 */

	unsigned int Nn, Nel, d, Neq;
	double       *F;

	struct S_FLUX *const FLUX = malloc(sizeof *FLUX); // free

	Nn  = 3;
	Nel = 2;

	FLUX->PDE_index = PDE_EULER;
	FLUX->Nn  = Nn;
	FLUX->Nel = Nel;

	// d = 1
	d   = 1;
	Neq = d+2;

	double const W1[18] = {  1.01,  1.02,  1.03,
	                         1.11,  1.12,  1.13,
	                         2.01, -2.02,  2.03,
	                        -2.11,  2.12, -2.13,
	                         5.01,  5.02,  5.03,
	                         5.11,  5.12,  5.13},
	             F1[18] = {  2.010000000000000e+00, -2.020000000000000e+00,  2.030000000000000e+00,
	                        -2.110000000000000e+00,  2.120000000000000e+00, -2.130000000000000e+00,
	                         5.204079207920791e+00,  5.208313725490196e+00,  5.212699029126213e+00,
	                         5.252720720720720e+00,  5.258285714285715e+00,  5.263964601769911e+00,
	                         1.236643583962356e+01, -1.233372702806613e+01,  1.230184975021209e+01,
	                        -1.207418001785569e+01,  1.204884693877551e+01, -1.202414942438719e+01};

	F = malloc(Nn*Neq*d*Nel * sizeof *F); // free

	FLUX->d = d;
	FLUX->W = W1;
	FLUX->F = F;
	flux_inviscid(FLUX);

	pass = 0;
	if (array_norm_diff_d(Nn*Neq*d*Nel,F,F1,"Inf") < 10*EPS)
		pass = 1;

	test_print2(pass,"flux_inviscid (d = 1):");

	free(F);

	// d = 2
	d   = 2;
	Neq = d+2;

	double const W2[24] = {  1.01,  1.02,  1.03,
	                         1.11,  1.12,  1.13,
	                         2.01, -2.02,  2.03,
	                        -2.11,  2.12, -2.13,
	                         3.04, -3.05,  3.06,
	                        -3.14,  3.15, -3.16,
	                         5.01,  5.02,  5.03,
	                         5.11,  5.12,  5.13},
	             F2[48] = {  2.010000000000000e+00, -2.020000000000000e+00,  2.030000000000000e+00,
	                        -2.110000000000000e+00,  2.120000000000000e+00, -2.130000000000000e+00,
	                         3.040000000000000e+00, -3.050000000000000e+00,  3.060000000000000e+00,
	                        -3.140000000000000e+00,  3.150000000000000e+00, -3.160000000000000e+00,
	                         3.374059405940593e+00,  3.384294117647060e+00,  3.394524271844660e+00,
	                         3.476216216216216e+00,  3.486410714285715e+00,  3.496601769911504e+00,
	                         6.049900990099009e+00,  6.040196078431372e+00,  6.030873786407766e+00,
	                         5.968828828828828e+00,  5.962499999999999e+00,  5.956460176991150e+00,
	                         6.049900990099009e+00,  6.040196078431372e+00,  6.030873786407766e+00,
	                         5.968828828828828e+00,  5.962499999999999e+00,  5.956460176991150e+00,
	                         8.524059405940594e+00,  8.503999999999998e+00,  8.484524271844661e+00,
	                         8.347837837837838e+00,  8.332928571428569e+00,  8.318460176991152e+00,
	                         8.724515243603568e+00, -8.721452902729720e+00,  8.718456781977567e+00,
	                        -8.697221004788572e+00,  8.694940688775510e+00, -8.692748688229305e+00,
	                         1.319528673659445e+01, -1.316853037293349e+01,  1.314210726741446e+01,
	                        -1.294278386494603e+01,  1.291936941964286e+01, -1.289628443887540e+01};

	F = malloc(Nn*Neq*d*Nel * sizeof *F); // free

	FLUX->d = d;
	FLUX->W = W2;
	FLUX->F = F;
	flux_inviscid(FLUX);

	pass = 0;
	if (array_norm_diff_d(Nn*Neq*d*Nel,F,F2,"Inf") < 10*EPS)
		pass = 1;

	test_print2(pass,"              (d = 2):");

	free(F);

	// d = 3
	d   = 3;
	Neq = d+2;

	double const W3[30] = {  1.01,  1.02,  1.03,
	                         1.11,  1.12,  1.13,
	                         2.01, -2.02,  2.03,
	                        -2.11,  2.12, -2.13,
	                         3.04, -3.05,  3.06,
	                        -3.14,  3.15, -3.16,
	                         4.07, -4.08,  4.09,
	                        -4.17,  4.18, -4.19,
	                         5.01,  5.02,  5.03,
	                         5.11,  5.12,  5.13},
	             F3[90] = { 2.010000000000000e+00, -2.020000000000000e+00,  2.030000000000000e+00,
	                       -2.110000000000000e+00,  2.120000000000000e+00, -2.130000000000000e+00,
	                        3.040000000000000e+00, -3.050000000000000e+00,  3.060000000000000e+00,
	                       -3.140000000000000e+00,  3.150000000000000e+00, -3.160000000000000e+00,
	                        4.070000000000000e+00, -4.080000000000000e+00,  4.090000000000000e+00,
	                       -4.170000000000000e+00,  4.180000000000000e+00, -4.190000000000000e+00,
	                        9.388118811881085e-02,  1.202941176470596e-01,  1.463495145631057e-01,
	                        3.430810810810794e-01,  3.663392857142878e-01,  3.893274336283170e-01,
	                        6.049900990099009e+00,  6.040196078431372e+00,  6.030873786407766e+00,
	                        5.968828828828828e+00,  5.962499999999999e+00,  5.956460176991150e+00,
	                        8.099702970297029e+00,  8.080000000000000e+00,  8.060873786407765e+00,
	                        7.926756756756756e+00,  7.912142857142856e+00,  7.897964601769912e+00,
	                        6.049900990099009e+00,  6.040196078431372e+00,  6.030873786407766e+00,
	                        5.968828828828828e+00,  5.962499999999999e+00,  5.956460176991150e+00,
	                        5.243881188118813e+00,  5.239999999999998e+00,  5.236349514563107e+00,
	                        5.214702702702702e+00,  5.212857142857143e+00,  5.211185840707966e+00,
	                        1.225029702970297e+01,  1.220000000000000e+01,  1.215087378640777e+01,
	                        1.179621621621622e+01,  1.175625000000000e+01,  1.171716814159292e+01,
	                        8.099702970297029e+00,  8.080000000000000e+00,  8.060873786407766e+00,
	                        7.926756756756755e+00,  7.912142857142856e+00,  7.897964601769912e+00,
	                        1.225029702970297e+01,  1.220000000000000e+01,  1.215087378640777e+01,
	                        1.179621621621622e+01,  1.175625000000000e+01,  1.171716814159292e+01,
	                        1.249467326732674e+01,  1.243990196078432e+01,  1.238634951456310e+01,
	                        1.199785585585585e+01,  1.195383928571428e+01,  1.191074336283186e+01,
	                        2.196635820017645e+00, -2.257452902729719e+00,  2.316714299179942e+00,
	                       -2.741441603765927e+00,  2.789091198979595e+00, -2.835674054350378e+00,
	                        3.322275071071463e+00, -3.408530372933486e+00,  3.492190027335282e+00,
	                       -4.079680870059247e+00,  4.144168526785719e+00, -4.206915498472862e+00,
	                        4.447914322125281e+00, -4.559607843137254e+00,  4.667665755490622e+00,
	                       -5.417920136352567e+00,  5.499245854591842e+00, -5.578156942595345e+00};

	F = malloc(Nn*Neq*d*Nel * sizeof *F); // free

	FLUX->d = d;
	FLUX->W = W3;
	FLUX->F = F;
	flux_inviscid(FLUX);

	pass = 0;
	if (array_norm_diff_d(Nn*Neq*d*Nel,F,F3,"Inf") < 10*EPS)
		pass = 1;

	test_print2(pass,"              (d = 3):");

	free(F);
	free(FLUX);
}
