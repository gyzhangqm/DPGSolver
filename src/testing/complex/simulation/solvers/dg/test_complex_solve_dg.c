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

#include "test_complex_solve_dg.h"

#include <assert.h>

#include "macros.h"
#include "definitions_elements.h"
#include "definitions_intrusive.h"
#include "definitions_tol.h"
#include "definitions_test_integration.h"

#include "complex_multiarray.h"

#include "test_complex_face_solver_dg.h"
#include "test_complex_volume_solver_dg.h"

#include "test_complex_boundary.h"
#include "test_complex_boundary_advection.h"
#include "test_complex_boundary_euler.h"
#include "test_complex_compute_face_rhs.h"
#include "test_complex_compute_grad_coef_dg.h"
#include "test_complex_compute_face_rhs_dg.h"
#include "test_complex_compute_volume_rhs_dg.h"
#include "test_support_computational_elements.h"
#include "test_support_multiarray.h"
#include "test_complex_test_case.h"

#include "multiarray.h"
#include "vector.h"

#include "volume_solver_dg.h"

#include "compute_face_rlhs.h"
#include "const_cast.h"
#include "intrusive.h"
#include "simulation.h"
#include "solve.h"
#include "solve_dg.h"
#include "test_case.h"

// Static function declarations ************************************************************************************* //

/** \brief Constructor for the list of \ref Volume\*s adjacent to (and including) the current volume.
 *  \return Standard. */
static struct Intrusive_List* constructor_Volumes_local
	(const struct Volume* vol,    ///< The centre \ref Volume.
	 const struct Simulation* sim ///< \ref Simulation.
	);

/** \brief Constructor for the list of \ref Face\*s adjacent to the current volume.
 *  \return Standard. */
static struct Intrusive_List* constructor_Faces_local
	(const struct Volume* vol,    ///< The centre \ref Volume.
	 const struct Simulation* sim ///< \ref Simulation.
	);

/// \brief Compute the complex rhs terms based on the value of \ref CHECK_LIN.
static void compute_rhs_cmplx_step_dg
	(struct Intrusive_List* volumes_local, ///< Return of \ref constructor_Volumes_local.
	 struct Intrusive_List* faces_local,   ///< Return of \ref constructor_Faces_local.
	 const struct Simulation* sim          ///< \ref Simulation.
	);

/// \brief Set a column of the lhs matrix using the values of the complex rhs for the dg scheme.
static void set_col_lhs_cmplx_step_dg
	(const int col_l,                       ///< The local (to the volume solution dof) column index.
	 const struct Solver_Volume_c* s_vol_c, /**< The \ref Solver_Volume_T associated with the current column of the
	                                         *   matrix. */
	 struct Intrusive_List* volumes_local,  ///< Return of \ref constructor_Volumes_local.
	 struct Solver_Storage_Implicit* ssi    ///< \ref Solver_Storage_Implicit.
	);

// Interface functions ********************************************************************************************** //

#include "def_templates_type_dc.h"
#include "solve_dg_T.c"

void perturb_solution_dg (const struct Simulation* sim)
{
/// \todo template this but as part of test_complex_solve_dg_T such that functions are not accessible to the main code.
	if (sim->test_case_rc->is_real) {
		for (struct Intrusive_Link* curr = sim->volumes->first; curr; curr = curr->next)
			perturb_Multiarray_d(((struct Solver_Volume*)curr)->sol_coef,MAX_PERTURB);
	} else {
		for (struct Intrusive_Link* curr = sim->volumes->first; curr; curr = curr->next)
			perturb_Multiarray_c(((struct Solver_Volume_c*)curr)->sol_coef,MAX_PERTURB);
	}
}

void compute_lhs_cmplx_step_dg (const struct Simulation* sim, struct Solver_Storage_Implicit* ssi)
{
	assert(!sim->test_case_rc->is_real);

	struct Test_Case_c* test_case = (struct Test_Case_c*) sim->test_case_rc->tc;
	assert(test_case->solver_method_curr == 'e'); // Should not be computing Jacobian terms.

	for (struct Intrusive_Link* curr_c = sim->volumes->first; curr_c; curr_c = curr_c->next) {
		struct Volume* vol = (struct Volume*) curr_c;
/// \todo Add a "volume_central" for volume/face terms, use volumes_local (including all neigh) for grad_coef.
		struct Intrusive_List* volumes_local = constructor_Volumes_local(vol,sim);
		struct Intrusive_List* faces_local   = constructor_Faces_local(vol,sim);

		struct Solver_Volume_c* s_vol = (struct Solver_Volume_c*) curr_c;
		struct Multiarray_c* sol_coef_c = s_vol->sol_coef;
		const ptrdiff_t n_col_l = compute_size(sol_coef_c->order,sol_coef_c->extents);
		for (int col_l = 0; col_l < n_col_l; ++col_l) {
			sol_coef_c->data[col_l] += CX_STEP*I;
			compute_rhs_cmplx_step_dg(volumes_local,faces_local,sim);
			sol_coef_c->data[col_l] -= CX_STEP*I;

			set_col_lhs_cmplx_step_dg(col_l,(struct Solver_Volume_c*)curr_c,volumes_local,ssi);
		}
		destructor_IL(volumes_local,true);
		destructor_IL(faces_local,true);
	}
	petsc_mat_vec_assemble(ssi);
}

// Static functions ************************************************************************************************* //
// Level 0 ********************************************************************************************************** //

/** \brief Check whether the \ref Volume is a neighbour (inclusive) of the current \ref Volume.
 *  \return `true` if it is a neighbour (or is the current volume); `false` otherwise. */
static bool is_volume_neighbour
	(const struct Volume* vol,     ///< The volume under investigation.
	 const struct Volume* vol_curr ///< The current volume.
	);

/** \brief Check whether the \ref Face neighbours the current \ref Volume.
 *  \return `true` if neighbouring; `false` otherwise. */
static bool is_face_neighbour
	(const struct Face* face,      ///< The face under investigation.
	 const struct Volume* vol_curr ///< The current volume.
	);

static struct Intrusive_List* constructor_Volumes_local (const struct Volume* vol, const struct Simulation* sim)
{
	struct Intrusive_List* volumes = constructor_empty_IL(IL_VOLUME_SOLVER_DG,NULL);

	const size_t sizeof_base = sizeof(struct DG_Solver_Volume_c);
	for (struct Intrusive_Link* curr = sim->volumes->first; curr; curr = curr->next) {
		if (!is_volume_neighbour((struct Volume*)curr,vol))
			continue;

		// A copy is required such that the link in the global list is not modified.
		push_back_IL(volumes,constructor_copied_Intrusive_Link(curr,sizeof_base,sizeof_base));
	}

	return volumes;
}

static struct Intrusive_List* constructor_Faces_local (const struct Volume* vol, const struct Simulation* sim)
{
	struct Intrusive_List* faces = constructor_empty_IL(IL_FACE_SOLVER_DG,NULL);

	const size_t sizeof_base = sizeof(struct DG_Solver_Face_c);
	for (struct Intrusive_Link* curr = sim->faces->first; curr; curr = curr->next) {
		if (!is_face_neighbour((struct Face*)curr,vol))
			continue;

		// A copy is required such that the link in the global list is not modified.
		push_back_IL(faces,constructor_copied_Intrusive_Link(curr,sizeof_base,sizeof_base));
	}

	return faces;
}

static void compute_rhs_cmplx_step_dg
	(struct Intrusive_List* volumes_local, struct Intrusive_List* faces_local, const struct Simulation* sim)
{
	initialize_zero_memory_volumes(volumes_local);
	compute_grad_coef_dg_c(sim,volumes_local,faces_local);
	switch (CHECK_LIN) {
	case CHECK_LIN_VOLUME:
		compute_volume_rlhs_dg_c(sim,NULL,volumes_local);
		break;
	case CHECK_LIN_FACE:
		compute_face_rlhs_dg_c(sim,NULL,faces_local);
		break;
	case CHECK_LIN_ALL:
		compute_volume_rlhs_dg_c(sim,NULL,volumes_local);
		compute_face_rlhs_dg_c(sim,NULL,faces_local);
		break;
	default:
		EXIT_ERROR("Unsupported: %d.\n",CHECK_LIN);
		break;
	}
}

static void set_col_lhs_cmplx_step_dg
	(const int col_l, const struct Solver_Volume_c* s_vol_c, struct Intrusive_List* volumes_local,
	 struct Solver_Storage_Implicit* ssi)
{
	ssi->col = (int)s_vol_c->ind_dof+col_l;

	for (struct Intrusive_Link* curr_r = volumes_local->first; curr_r; curr_r = curr_r->next) {
		struct Solver_Volume_c* s_vol_r = (struct Solver_Volume_c*) curr_r;
		ssi->row = (int)s_vol_r->ind_dof+0;

		struct Multiarray_c* sol_coef_r = s_vol_r->sol_coef;

		const ptrdiff_t ext_0 = compute_size(sol_coef_r->order,sol_coef_r->extents);
		PetscInt idxm[ext_0];
		for (int i = 0; i < ext_0; ++i)
			idxm[i] = ssi->row+i;

		PetscInt idxn[1] = { ssi->col, };

		struct Multiarray_c* rhs_r = s_vol_r->rhs;
		PetscScalar vv[ext_0];
		for (int i = 0; i < ext_0; ++i)
			vv[i] = cimag(rhs_r->data[i])/CX_STEP;

		MatSetValues(ssi->A,(PetscInt)ext_0,idxm,1,idxn,vv,ADD_VALUES);
	}
}

// Level 1 ********************************************************************************************************** //

static bool is_volume_neighbour (const struct Volume* vol, const struct Volume* vol_curr)
{
	for (int f = 0; f < NFMAX; ++f) {
	for (int sf = 0; sf < NSUBFMAX; ++sf) {
		const struct Face* face = vol_curr->faces[f][sf];

		if (!face)
			continue;

		const int n_side = ( face->boundary ? 1 : 2 );
		for (int i = 0; i < n_side; ++i) {
			if (vol->index == face->neigh_info[i].volume->index)
				return true;
		}
	}}
	return false;
}

static bool is_face_neighbour (const struct Face* face, const struct Volume* vol_curr)
{
	for (int f = 0; f < NFMAX; ++f) {
	for (int sf = 0; sf < NSUBFMAX; ++sf) {
		const struct Face* face_n = vol_curr->faces[f][sf];

		if (!face_n)
			continue;

		if (face->index == face_n->index)
			return true;
	}}
	return false;
}
