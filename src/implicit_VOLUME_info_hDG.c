// Copyright 2017 Philip Zwanenburg
// MIT License (https://github.com/PhilipZwanenburg/DPGSolver/blob/master/LICENSE)

#include "implicit_VOLUME_info_hDG.h"

#include <stdlib.h>
#include <stdio.h>

#include "S_DB.h"
#include "S_VOLUME.h"
/*
#include <string.h>

#include "Parameters.h"
#include "Macros.h"

#include "solver_functions.h"
#include "fluxes_structs.h"
#include "fluxes_inviscid.h"
#include "fluxes_viscous.h"
#include "jacobian_fluxes_inviscid.h"
#include "jacobian_fluxes_viscous.h"
#include "array_free.h"
#include "array_print.h"
*/

#include "solver_functions_hDG.h"

/*
 *	Purpose:
 *		Evaluate the VOLUME contributions to the RHS and LHS terms for the hDG scheme.
 *
 *	Comments:
 *
 *	Notation:
 *
 *	References:
 */

static void compute_Inviscid_VOLUME_hDG (void);

void implicit_VOLUME_info_hDG (void)
{
	compute_Inviscid_VOLUME_hDG();
}

static void compute_Inviscid_VOLUME_hDG (void)
{
	for (struct S_VOLUME *VOLUME = DB.VOLUME; VOLUME; VOLUME = VOLUME->next) {
		struct S_OPERATORS_V *OPS = init_ops_VOLUME(VOLUME);
printf("%p\n",OPS);

		// Obtain W_vI
		if (DB.Collocated) {
			// move constructor
		} else {
			// copy constructor + interpolation
		}

		// Compute Flux and its Jacobian in reference space

		// Convert to reference space

		// Compute RHS and LHS terms

		// RHS

		// LHS
	}
}
