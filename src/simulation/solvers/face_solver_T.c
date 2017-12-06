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
#include <string.h>

#include "macros.h"
#include "definitions_test_case.h"
#include "definitions_bc.h"

// Static function declarations ************************************************************************************* //

/** \brief Set the function pointers to the appropriate functions to compute values needed for the numerical flux
 *         computation. */
static void set_function_pointers_num_flux_T
	(struct Solver_Face_T* s_face,  ///< Pointer to the \ref Solver_Face.
	 const struct Simulation* sim ///< \ref Simulation.
	);

// Interface functions ********************************************************************************************** //

void constructor_derived_Solver_Face_T (struct Face* face_ptr, const struct Simulation* sim)
{
	struct Solver_Face_T* s_face = (struct Solver_Face_T*) face_ptr;

	const_cast_ptrdiff(&s_face->ind_dof,-1);
	const_cast_i(&s_face->p_ref,sim->p_ref[0]);
	const_cast_i(&s_face->ml,0);
	const_cast_c(&s_face->cub_type,(check_for_curved_neigh((struct Face*)s_face) ? 'c' : 's'));

	s_face->nf_coef = constructor_empty_Multiarray_T('C',2,(ptrdiff_t[]){0,0});   // destructed

	const_constructor_move_Multiarray_T(
		&s_face->xyz_fc,constructor_empty_Multiarray_T('C',2,(ptrdiff_t[]){0,0}));        // destructed
	const_constructor_move_Multiarray_T(
		&s_face->normals_fc,constructor_empty_Multiarray_T('R',2,(ptrdiff_t[]){0,0}));    // destructed
	const_constructor_move_Multiarray_T(
		&s_face->jacobian_det_fc,constructor_empty_Multiarray_T('C',1,(ptrdiff_t[]){0})); // destructed

	set_function_pointers_num_flux_T(s_face,sim);
}

void destructor_derived_Solver_Face_T (struct Face* face_ptr)
{
	struct Solver_Face_T* face = (struct Solver_Face_T*) face_ptr;

	destructor_Multiarray_T(face->nf_coef);

	destructor_const_Multiarray_T(face->xyz_fc);
	destructor_const_Multiarray_T(face->normals_fc);
	destructor_const_Multiarray_T(face->jacobian_det_fc);
}

// Static functions ************************************************************************************************* //
// Level 0 ********************************************************************************************************** //

/** \brief Set the function pointers to the appropriate functions to compute boundary values needed for the numerical
 *         flux computation. */
static void set_function_pointers_num_flux_bc_T
	(struct Solver_Face_T* s_face,  ///< Defined for \ref set_function_pointers_num_flux.
	 const struct Simulation* sim ///< Defined for \ref set_function_pointers_num_flux.
	);

static void set_function_pointers_num_flux_T (struct Solver_Face_T* s_face, const struct Simulation* sim)
{
	const struct Face* face = (struct Face*) s_face;
	if (!face->boundary) {
		struct Test_Case* test_case = sim->test_case;
		switch (test_case->pde_index) {
		case PDE_ADVECTION:
		case PDE_EULER:
			s_face->constructor_Boundary_Value_fcl = constructor_Boundary_Value_s_fcl_interp;
			break;
		case PDE_POISSON:
			EXIT_UNSUPPORTED;
			break;
		case PDE_NAVIER_STOKES:
			EXIT_UNSUPPORTED;
			break;
		default:
			EXIT_ERROR("Unsupported: %d\n",test_case->pde_index);
			break;
		}
	} else {
		set_function_pointers_num_flux_bc_T(s_face,sim);
	}
}

// Level 1 ********************************************************************************************************** //

/// \brief Version of \ref set_function_pointers_num_flux_bc for the linear advection equation.
static void set_function_pointers_num_flux_bc_advection_T
	(struct Solver_Face_T* s_face ///< See brief.
	);

/// \brief Version of \ref set_function_pointers_num_flux_bc for the Euler equations.
static void set_function_pointers_num_flux_bc_euler_T
	(struct Solver_Face_T* s_face ///< See brief.
	);

static void set_function_pointers_num_flux_bc_T (struct Solver_Face_T* s_face, const struct Simulation* sim)
{
	switch (sim->test_case->pde_index) {
	case PDE_ADVECTION: set_function_pointers_num_flux_bc_advection_T(s_face); break;
	case PDE_EULER:     set_function_pointers_num_flux_bc_euler_T(s_face);     break;
	default:
		EXIT_ERROR("Unsupported: %d\n",sim->test_case->pde_index);
		break;
	}
}

// Level 2 ********************************************************************************************************** //

#include "boundary_advection.h"
#include "boundary_euler.h"

static void set_function_pointers_num_flux_bc_advection_T (struct Solver_Face_T* s_face)
{
	const struct Face* face = (struct Face*) s_face;

	const int bc = face->bc % BC_STEP_SC;
	switch (bc) {
	case BC_INFLOW:
		s_face->constructor_Boundary_Value_fcl = constructor_Boundary_Value_advection_inflow;
		break;
	case BC_OUTFLOW:
		s_face->constructor_Boundary_Value_fcl = constructor_Boundary_Value_advection_outflow;
		break;
	default:
		EXIT_ERROR("Unsupported: %d\n",face->bc);
		break;
	}
}

static void set_function_pointers_num_flux_bc_euler_T (struct Solver_Face_T* s_face)
{
	const struct Face* face = (struct Face*) s_face;

	const int bc = face->bc % BC_STEP_SC;
	switch (bc) {
	case BC_RIEMANN:
		s_face->constructor_Boundary_Value_fcl = constructor_Boundary_Value_euler_riemann;
		break;
	case BC_SLIPWALL:
		s_face->constructor_Boundary_Value_fcl = constructor_Boundary_Value_euler_slipwall;
		break;
	default:
		EXIT_ERROR("Unsupported: %d\n",face->bc);
		break;
	}
}