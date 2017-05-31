// Copyright 2017 Philip Zwanenburg
// MIT License (https://github.com/PhilipZwanenburg/DPGSolver/blob/master/LICENSE)

#include "setup_operators_hDG.h"

#include <stdlib.h>
#include <stdio.h>

#include "Parameters.h"
#include "Macros.h"
#include "S_DB.h"
#include "S_ELEMENT.h"
#include "matrix_structs.h"

#include "element_functions.h"
#include "select_functions.h"
#include "adaptation.h"
#include "matrix_functions.h"
#include "array_free.h"

#include "cubature.h" // Needed only for the definitions of the structs
#include "bases.h"

/*
 *	Purpose:
 *		Set up additional operators required for the hDG method.
 *
 *	Comments:
 *		The operators computed here are those required assuming that the traces are in L2 (non-conforming). As it is
 *		currently assumed that the FACE cubature nodes of the parent ELEMENTs are the same as the VOLUME cubature nodes
 *		of the current element, the operators computed below are identical to the corresponding VOLUME operators if the
 *		VOLUME and FACE cubature orders are equal.
 *
 *		For consistency of FACE trace terms with VOLUME terms which are interpolated to the FACE, the cubature order for
 *		the FACE operators is specified as that of PIf of the parent ELEMENT.
 *
 *		The I(s/c)_FF operators currently include the inverse of the cubature weights for Collocated = TRUE but this
 *		does not have an effect on the scheme.
 *
 *	Notation:
 *		Chi(Ref)(1)(2)_(3)(4)(5)[6] : (Ref)erence (optional) Basis functions (Chi) of type (1) which are (2) evaluated
 *		                              at (3) nodes of (4) type which are (5) of order [6]
 *		                              (1/4): TR(ace)S(olution), I(ntegration)
 *		                              (2/5): (s)traight, (c)urved
 *		                              (3)  : (v)olume,
 *
 *	References:
 */

static void setup_operators_hDG_std(EType)
{
	char const *const *const *const NodeTypeIfs = (char const *const *const *const) DB.NodeTypeIfs,
	           *const *const *const NodeTypeIfc = (char const *const *const *const) DB.NodeTypeIfc;

	unsigned int const              Eclass = get_Eclass(EType),
	                   *const       PTRS   = DB.PTRS,
	                   *const *const PIfs   = (unsigned int const *const *const) DB.PIfs;
//	                   *const *const PIfc   = (unsigned int const *const *const) DB.PIfc;

	struct S_ELEMENT *const ELEMENT = get_ELEMENT_type(EType);

	unsigned int const dE = ELEMENT->d;

	// Returned operators
	struct S_MATRIX **ChiTRS_vIs = ELEMENT->ChiTRS_vIs;
//	                **Is_FF      = ELEMENT->Is_FF;

	cubature_tdef cubature;
	basis_tdef    basis;

	select_functions_cubature(&cubature,EType);
	select_functions_basis(&basis,EType);

	struct S_CUBATURE *CUBDATA = malloc(sizeof *CUBDATA); // free

	unsigned int PSMin, PSMax;
	get_PS_range(&PSMin,&PSMax);
	for (size_t P = PSMin; P <= PSMax; P++) {

		unsigned int NvnTRS = 0;
		double       *rst_vTRS = NULL,
		set_cubdata(CUBDATA,false,false,NodeTypeS[P][Eclass],dE,PTRS[P],cubature); // free
		set_from_cubdata(CUBDATA,&NvnTRS,NULL,&rst_vTRS,NULL,NULL);

		struct S_MATRIX *ChiRefTRS_vTRS    = basis_mat(PTRS[P],dE,NvnTRS,rst_vTRS,basis), // tbd
		                *ChiRefInvTRS_vTRS = inverse_mat(ChiRefTRS_vTRS); // tbd
		struct S_MATRIX *TTRS = mm_Alloc_Mat_d('R',ChiRefInvTRS_vTRS,ChiTRS_vTRS); // free

		unsigned int NvnIs = 0;
		double       *rst_vIs = NULL, *w_vIs = NULL;
		set_cubdata(CUBDATA,true,false,NodeTypeIfs[P][Eclass],dE,PIfs[P][Eclass],cubature); // free
		set_from_cubdata(CUBDATA,&NvnIs,NULL,&rst_vIs,&w_vIs,NULL);

		struct S_MATRIX *ChiRefTRS_vIs  = basis_mat(PTRS[P],dE,NvnIs,rst_vIs,basis); // tbd


		ChiTRS_vIs[P] = mm_Alloc_Mat_d('R',ChiRefTRS_vIs,TTRS); // keep

		matrix_free(TTRS);

printf("%p\n",ChiRefTRS_vIs);

		free(rst_vTRS);

		free(rst_vIs);
		free(w_vIs);
	}

	free(CUBDATA);
	free(BASISDATA);

if (0) {
	printf("%p %p\n",NodeTypeIfs,NodeTypeIfc);
	printf("%d\n",EType);
}
}

static void setup_operators_hDG_TP(EType)
{
	if (EType != QUAD)
		EXIT_UNSUPPORTED;

	EXIT_UNSUPPORTED;
}

void setup_operators_hDG(void)
{
	unsigned int const d = DB.d;

	unsigned int EType;

	// LINE
	EType = LINE;
	setup_operators_hDG_std(EType);

	if (d == 3) {
		if (is_ELEMENT_present(TET) || is_ELEMENT_present(WEDGE) || is_ELEMENT_present(PYR)) {
			EType = TRI;
			setup_operators_hDG_std(EType);
		} else if (is_ELEMENT_present(HEX) || is_ELEMENT_present(WEDGE) || is_ELEMENT_present(PYR)) {
			EType = QUAD;
			setup_operators_hDG_TP(EType);
		}
	}
}
