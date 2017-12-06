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

#include <string.h>

#include "macros.h"
#include "definitions_test_case.h"

// Static function declarations ************************************************************************************* //

/** \brief Constructor for the inverse mass matrix of the input volume.
 *  \return See brief. */
static const struct const_Matrix_T* constructor_inverse_mass
	(const struct DG_Solver_Volume_T* volume ///< \ref DG_Solver_Volume.
	);

// Interface functions ********************************************************************************************** //

void constructor_derived_DG_Solver_Volume_T (struct Volume* volume_ptr, const struct Simulation* sim)
{
	struct Needed_Members needed_members = set_needed_members(sim);

	struct Solver_Volume_T* s_volume  = (struct Solver_Volume_T*) volume_ptr;
	struct DG_Solver_Volume_T* volume = (struct DG_Solver_Volume_T*) volume_ptr;

	const int order = s_volume->sol_coef->order;
	ptrdiff_t* extents = s_volume->sol_coef->extents;

	volume->rhs        = constructor_empty_Multiarray_T('C',order,extents); // destructed
	volume->sol_coef_p =
		( needed_members.sol_coef_p ? constructor_empty_Multiarray_T('C',order,extents) : NULL ); // destructed

	volume->m_inv = ( needed_members.m_inv ? constructor_inverse_mass(volume) : NULL ); // destructed
}

void destructor_derived_DG_Solver_Volume_T (struct Volume* volume_ptr)
{
	struct DG_Solver_Volume_T* volume = (struct DG_Solver_Volume_T*) volume_ptr;

	destructor_Multiarray_T(volume->rhs);
	if (volume->sol_coef_p)
		destructor_Multiarray_T(volume->sol_coef_p);
	if (volume->m_inv)
		destructor_const_Matrix_R(volume->m_inv);
}

// Static functions ************************************************************************************************* //
// Level 0 ********************************************************************************************************** //

/** \brief Constructor for the mass matrix of the input volume.
 *  \return See brief. */
static const struct const_Matrix_R* constructor_mass
	(const struct Solver_Volume_T* s_vol ///< \ref Solver_Volume.
	);

static const struct const_Matrix_R* constructor_inverse_mass (const struct DG_Solver_Volume_T* volume)
{
	const struct const_Matrix_R* m     = constructor_mass((struct Solver_Volume_T*)volume); // destructed
	const struct const_Matrix_R* m_inv = constructor_inverse_const_Matrix_R(m);             // returned
	destructor_const_Matrix_R(m);

	return m_inv;
}

// Level 1 ********************************************************************************************************** //

static const struct const_Matrix_R* constructor_mass (const struct Solver_Volume_T* s_vol)
{
	struct Volume* vol               = (struct Volume*) s_vol;
	const struct Solver_Element* s_e = (struct Solver_Element*) vol->element;

	const int p      = s_vol->p_ref,
	          curved = vol->curved;
	const struct Operator* cv0_vs_vc = get_Multiarray_Operator(s_e->cv0_vs_vc[curved],(ptrdiff_t[]){0,0,p,p});
	const struct const_Vector_R* w_vc = get_operator__w_vc__s_e(s_vol);

	const struct const_Vector_R jacobian_det_vc = interpret_const_Multiarray_as_Vector_R(s_vol->jacobian_det_vc);
	const struct const_Vector_R* wJ_vc = constructor_dot_mult_const_Vector_R(w_vc,&jacobian_det_vc,1); // destructed

	const struct const_Matrix_R* m_l = cv0_vs_vc->op_std;
	const struct const_Matrix_R* m_r = constructor_mm_diag_const_Matrix_R(1.0,m_l,wJ_vc,'L',false); // destructed
	destructor_const_Vector_R(wJ_vc);

	const struct const_Matrix_R* mass = constructor_mm_const_Matrix_R('T','N',1.0,m_l,m_r,'R'); // returned
	destructor_const_Matrix_R(m_r);

	return mass;
}
