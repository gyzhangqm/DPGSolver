// Copyright 2017 Philip Zwanenburg
// MIT License (https://github.com/PhilipZwanenburg/DPGSolver/blob/master/LICENSE)

#ifndef DPG__solver_functions_h__INCLUDED
#define DPG__solver_functions_h__INCLUDED

#include <stdbool.h>
#include <complex.h>

#include "S_VOLUME.h"
#include "S_FACE.h"
#include "S_OpCSR.h"


// VOLUME structs and functions

struct S_Dxyz {
	unsigned int dim, Nbf, Nn;
	double const *const *D, *C;
};

extern double *compute_Dxyz(struct S_Dxyz *DxyzInfo, unsigned int d);


struct S_OPERATORS_V {
	// Standard
	unsigned int NvnI, NvnS;
	double const *ChiS_vI, *I_Weak, *const *D_Weak;

	// Sparse
	struct S_OpCSR const *const *D_Weak_sp;

	// (S)um (F)actorized
	unsigned int NvnS_SF, NvnI_SF;
	double const *ChiS_vI_SF, *I_Weak_SF, *const *D_Weak_SF;
};

struct S_VDATA {
	unsigned int P, Eclass;
	double       *W_vI, **Q_vI;

	struct S_OPERATORS_V const *const *OPS;
	struct S_VOLUME      const *VOLUME;

	// Only used for verification of equivalence between real and complex functions.
	double complex *W_vI_c, **Q_vI_c;
};

extern void init_ops_VOLUME    (struct S_OPERATORS_V *const OPS, struct S_VOLUME const *const VOLUME,
                                unsigned int const IndClass);
extern void init_VDATA         (struct S_VDATA *const VDATA, struct S_VOLUME const *const VOLUME);
extern void coef_to_values_vI  (struct S_VDATA const *const VDATA, char const coef_type);
extern void convert_between_rp (unsigned int const Nn, unsigned int const Nrc, double const *const C, double *const Ap,
                                double *const Ar, char const *const conv_type);

extern void finalize_VOLUME_Inviscid_Weak (unsigned int const Nrc, double const *const Ar_vI, double *const RLHS,
                                           char const imex_type, struct S_VDATA const *const VDATA);
extern void finalize_VOLUME_Viscous_Weak  (unsigned int const Nrc, double *const Ar_vI, double *const RLHS,
                                           char const imex_type, struct S_VDATA const *const VDATA);
extern void initialize_VOLUME_LHSQ_Weak   (unsigned int const Nrc, double const *const *const dFrdQ_vI,
                                           double *const *const LHSQ, struct S_VDATA const *const VDATA);
extern void finalize_VOLUME_LHSQV_Weak    (struct S_VOLUME *const VOLUME);


// FACE structs and functions

struct S_OPERATORS_F {
	// Standard
	unsigned int       NvnS, NfnI;
	unsigned int const *nOrdLR, *nOrdRL;

	double const *w_fI, *const *ChiS_fI, *const *I_Weak_FF;

	// Sparse
	struct S_OpCSR const *const *ChiS_fI_sp, *const *I_Weak_FF_sp;

	// (S)um (F)actorized
	unsigned int NvnS_SF, NfnI_SF, NvnI_SF;
	double const *const *ChiS_fI_SF, *const *ChiS_vI_SF, *const *I_Weak_FF_SF, *const *I_Weak_VV_SF;
};

struct S_NumericalFlux {
	double const *WL_fIL, *WR_fIL;
	double       *nFluxNum_fI,     *dnFluxNumdWL_fI,     *dnFluxNumdWR_fI,
	             **nSolNum_fI,     **dnSolNumdWL_fI,     **dnSolNumdWR_fI,
	             *nFluxViscNum_fI, *dnFluxViscNumdWL_fI, *dnFluxViscNumdWR_fI,
	             **dnFluxViscNumdQL_fI, **dnFluxViscNumdQR_fI;

	// Only used for verification.
	double complex const *WL_fIL_c, *WR_fIL_c;
	double complex       *nFluxNum_fI_c, **nSolNum_fI_c, *nFluxViscNum_fI_c;
};

struct S_FDATA {
	char         side;
	unsigned int P, Vf, f, SpOp, Eclass, IndFType;
	double       *W_fIL, **GradW_fIL, **QhatF, **Q_WhatLL, **Q_WhatRL, **Q_WhatLR, **Q_WhatRR;

	struct S_OPERATORS_F const *const *OPS;
	struct S_VOLUME      const *VOLUME;
	struct S_FACE        const *FACE;

	struct S_NumericalFlux const *NFluxData;

	// Only used for verification.
	double complex *W_fIL_c, **GradW_fIL_c, **QhatF_c;
};

extern void init_ops_FACE             (struct S_OPERATORS_F *const OPS, struct S_VOLUME const *const VOLUME,
                                       struct S_FACE const *const FACE, unsigned int const IndFType);
extern void init_FDATA                (struct S_FDATA *const FDATA, struct S_FACE const *const FACE, char const side);
extern void coef_to_values_fI         (struct S_FDATA *const FDATA, char const coef_type, char const imex_type);
extern void compute_WR_fIL            (struct S_FDATA const *const FDATA, double const *const WL_fIL,
                                       double *const WR_fIL);
extern void compute_WR_GradWR_fIL     (struct S_FDATA const *const FDATA, double const *const WL_fIL,
                                       double *const WR_fIL, double const *const *const GradWL_fIL,
                                       double *const *const GradWR_fIL, char const imex_type);

extern void compute_numerical_flux         (struct S_FDATA const *const FDATA, char const imex_type);
extern void compute_numerical_solution     (struct S_FDATA const *const FDATA, char const imex_type);
extern void compute_numerical_flux_viscous (struct S_FDATA const *const FDATAL, struct S_FDATA const *const FDATAR,
                                            char const imex_type);
extern void add_Jacobian_scaling_FACE (struct S_FDATA const *const FDATA, char const imex_type, char const coef_type);

extern void finalize_FACE_Inviscid_Weak (struct S_FDATA const *const FDATAL, struct S_FDATA const *const FDATAR,
                                         double const *const nANumL_fI, double const *const nANumR_fI, char const side,
                                         char const imex_type, char const coef_type);
extern void finalize_QhatF_Weak         (struct S_FDATA const *const FDATAL, struct S_FDATA const *const FDATAR,
                                         char const side, char const imex_type, char const FORM_MF1);
extern void finalize_FACE_Viscous_Weak  (struct S_FDATA const *const FDATAL, struct S_FDATA const *const FDATAR,
                                         double *const nANumL_fI, double *const nANumR_fI, char const side,
                                         char const imex_type, char const coef_type);
extern void finalize_implicit_FACE_Q_Weak (struct S_FDATA const *const FDATAL, struct S_FDATA const *const FDATAR,
                                           char const side);
extern void finalize_VOLUME_LHSQF_Weak  (struct S_FACE *const FACE);

#endif // DPG__solver_functions_h__INCLUDED
