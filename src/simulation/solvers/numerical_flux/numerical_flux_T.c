/* {{{
This file is part of DPGSolver.

DPGSolver is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or any later version.

DPGSolver is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with DPGSolver.  If not, see
<http://www.gnu.org/licenses/>.
}}} */
/** \file
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "macros.h"
#include "definitions_core.h"


#include "def_templates_numerical_flux.h"

#include "def_templates_multiarray.h"

#include "def_templates_boundary.h"
#include "def_templates_math_functions.h"
#include "def_templates_test_case.h"

// Static function declarations ************************************************************************************* //

/** \brief Add the linearized contribution from "right" volume to the linearized numerical flux terms if on a domain
 *         boundary. */
static void combine_num_flux_boundary_T
	(struct Numerical_Flux_Input_T* num_flux_i, ///< Defined for \ref compute_Numerical_Flux_fptr_T.
	 struct mutable_Numerical_Flux_T* num_flux  ///< Defined for \ref compute_Numerical_Flux_fptr_T.
	);

// Interface functions ********************************************************************************************** //

struct Numerical_Flux_Input_T* constructor_Numerical_Flux_Input_T (const struct Simulation* sim)
{
	struct Numerical_Flux_Input_T* num_flux_i = calloc(1,sizeof *num_flux_i); // returned

	const_cast_c1(&num_flux_i->bv_l.input_path,sim->input_path);

	struct Test_Case_T* test_case = (struct Test_Case_T*)sim->test_case_rc->tc;
	const_cast_i(&num_flux_i->method,sim->method);
	const_cast_b(&num_flux_i->has_1st_order,test_case->has_1st_order);
	const_cast_b(&num_flux_i->has_2nd_order,test_case->has_2nd_order);

	const_cast_i(&num_flux_i->bv_l.n_eq,test_case->n_eq);
	const_cast_i(&num_flux_i->bv_l.n_var,test_case->n_var);

	num_flux_i->compute_Numerical_Flux = test_case->compute_Numerical_Flux;
	switch (test_case->solver_method_curr) {
	case 'e':
		num_flux_i->bv_l.compute_member = test_case->boundary_value_comp_mem_e;
		num_flux_i->compute_Numerical_Flux_1st = test_case->compute_Numerical_Flux_e[0];
		num_flux_i->compute_Numerical_Flux_2nd = test_case->compute_Numerical_Flux_e[1];
		break;
	case 'i':
		num_flux_i->bv_l.compute_member = test_case->boundary_value_comp_mem_i;
		num_flux_i->compute_Numerical_Flux_1st = test_case->compute_Numerical_Flux_i[0];
		num_flux_i->compute_Numerical_Flux_2nd = test_case->compute_Numerical_Flux_i[1];
		break;
	default:
		EXIT_ERROR("Unsupported: %c.\n",test_case->solver_method_curr);
		break;
	}

	return num_flux_i;
}

void destructor_Numerical_Flux_Input_T (struct Numerical_Flux_Input_T* num_flux_i)
{
	free(num_flux_i);
}

struct Numerical_Flux_T* constructor_Numerical_Flux_T (const struct Numerical_Flux_Input_T* num_flux_i)
{
	assert(((num_flux_i->bv_l.s != NULL && num_flux_i->bv_l.s->layout == 'C') &&
	        (num_flux_i->bv_r.s != NULL && num_flux_i->bv_r.s->layout == 'C')) ||
	       ((num_flux_i->bv_l.g != NULL && num_flux_i->bv_l.g->layout == 'C') &&
	        (num_flux_i->bv_r.g != NULL && num_flux_i->bv_r.g->layout == 'C')));

	const bool* c_m = num_flux_i->bv_l.compute_member;
	assert(c_m[0] || c_m[1] || c_m[2]);

	const int n_eq = num_flux_i->bv_l.n_eq,
	          n_vr = num_flux_i->bv_l.n_var;
	const ptrdiff_t n_n = ( num_flux_i->bv_l.s != NULL ? num_flux_i->bv_l.s->extents[0]
	                                                   : num_flux_i->bv_l.g->extents[0] );

	struct mutable_Numerical_Flux_T* num_flux = calloc(1,sizeof *num_flux); // returned

	num_flux->nnf = (c_m[0] ? constructor_zero_Multiarray_T('C',2,(ptrdiff_t[]){n_n,n_eq}) : NULL); // destructed
	for (int i = 0; i < 2; ++i) {
		struct m_Neigh_Info_NF_T* n_i = &num_flux->neigh_info[i];
		n_i->dnnf_ds = (c_m[1] ? constructor_zero_Multiarray_T('C',3,(ptrdiff_t[]){n_n,n_eq,n_vr})   : NULL); // destructed
		n_i->dnnf_dg = (c_m[2] ? constructor_zero_Multiarray_T('C',4,(ptrdiff_t[]){n_n,n_eq,n_vr,DIM}) : NULL); // destructed
	}

	assert(num_flux_i->bv_l.s->extents[0] == num_flux_i->bv_l.normals->extents[0]);
	num_flux_i->compute_Numerical_Flux(num_flux_i,num_flux);

	return (struct Numerical_Flux_T*) num_flux;
}

void destructor_Numerical_Flux_T (struct Numerical_Flux_T* num_flux)
{
	destructor_conditional_const_Multiarray_T(num_flux->nnf);
	for (int i = 0; i < 2; ++i) {
		struct Neigh_Info_NF_T n_i = num_flux->neigh_info[i];
		destructor_conditional_const_Multiarray_T(n_i.dnnf_ds);
		destructor_conditional_const_Multiarray_T(n_i.dnnf_dg);
	}
	free(num_flux);
}

void compute_Numerical_Flux_1_T
	(const struct Numerical_Flux_Input_T* num_flux_i, struct mutable_Numerical_Flux_T* num_flux)
{
	num_flux_i->compute_Numerical_Flux_1st(num_flux_i,num_flux);
	combine_num_flux_boundary_T((struct Numerical_Flux_Input_T*)num_flux_i,num_flux);
}

void compute_Numerical_Flux_12_T
	(const struct Numerical_Flux_Input_T* num_flux_i, struct mutable_Numerical_Flux_T* num_flux)
{
// make sure to use '+=' for the numerical fluxes.
	num_flux_i->compute_Numerical_Flux_1st(num_flux_i,num_flux);
	num_flux_i->compute_Numerical_Flux_2nd(num_flux_i,num_flux);
EXIT_ERROR("Ensure that all is working correctly.");
}

// Static functions ************************************************************************************************* //
// Level 0 ********************************************************************************************************** //

static void combine_num_flux_boundary_T
	(struct Numerical_Flux_Input_T* num_flux_i, struct mutable_Numerical_Flux_T* num_flux)
{
	// Only performed for the linearized terms.
	if (!(num_flux_i->bv_l.compute_member[1] == true || num_flux_i->bv_l.compute_member[2] == true))
		return;

	// Only performed if on a boundary.
	if (num_flux_i->bv_r.ds_ds == NULL)
		return;

	struct Multiarray_T* dnnf_ds_l = num_flux->neigh_info[0].dnnf_ds;
	const struct const_Multiarray_T* dnnf_ds_r = (struct const_Multiarray_T*) num_flux->neigh_info[1].dnnf_ds,
	                               * ds_ds     = num_flux_i->bv_r.ds_ds;

	const int n_n  = (int)dnnf_ds_l->extents[0],
	          n_eq = (int)dnnf_ds_l->extents[1],
	          n_vr = (int)dnnf_ds_l->extents[2];

	for (int eq = 0; eq < n_eq; ++eq) {
	for (int vr_l = 0; vr_l < n_vr; ++vr_l) {
		const int ind_dnnf_ds_l = eq+n_eq*(vr_l);
		for (int vr_r = 0; vr_r < n_vr; ++vr_r) {
			const int ind_dnnf_ds_r = eq+n_eq*(vr_r);
			const int ind_ds_ds     = vr_r+n_vr*(vr_l);
			z_yxpz_T(n_n,get_col_const_Multiarray_T(ind_dnnf_ds_r,dnnf_ds_r),
			             get_col_const_Multiarray_T(ind_ds_ds,ds_ds),
			             get_col_Multiarray_T(ind_dnnf_ds_l,dnnf_ds_l));
		}
	}}

	destructor_const_Multiarray_T(num_flux_i->bv_r.ds_ds);
	num_flux_i->bv_r.ds_ds = NULL;

	destructor_Multiarray_T(num_flux->neigh_info[1].dnnf_ds);
	num_flux->neigh_info[1].dnnf_ds = NULL;

	assert(num_flux_i->bv_l.compute_member[2] == false); // Add support for this in future.
}
