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

#include "boundary.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "macros.h"

#include "multiarray.h"

#include "element_solver.h"
#include "face.h"
#include "face_solver.h"
#include "volume.h"
#include "volume_solver.h"

#include "compute_face_rlhs.h"
#include "multiarray_operator.h"
#include "operator.h"
#include "simulation.h"
#include "solve_dg.h"

// Static function declarations ************************************************************************************* //

/** \brief Constructor for the solution interpolating from the neighbouring volume to the face cubature nodes.
 *  \return See brief. */
static const struct const_Multiarray_d* constructor_s_fc_interp
	(const struct Solver_Face* face, ///< Defined for \ref constructor_Boundary_Value_Input_face_s_fcl_interp.
	 const struct Simulation* sim,   ///< Defined for \ref constructor_Boundary_Value_Input_face_s_fcl_interp.
	 const int side_index            ///< The index of the side of the face under consideration.
	);

// Interface functions ********************************************************************************************** //

void constructor_Boundary_Value_Input_face_s_fcl_interp
	(struct Boundary_Value_Input* bv_i, const struct Solver_Face* face, const struct Simulation* sim)
{
	const int side_index = 0;
	bv_i->normals = face->normals_fc;
	bv_i->xyz     = face->xyz_fc;
	bv_i->s       = constructor_s_fc_interp(face,sim,side_index);
	bv_i->g       = NULL;
}

void destructor_Boundary_Value_Input (struct Boundary_Value_Input* bv_i)
{
	if (bv_i->s)
		destructor_const_Multiarray_d(bv_i->s);
	if (bv_i->g)
		destructor_const_Multiarray_d(bv_i->g);
}

void constructor_Boundary_Value_s_fcl_interp
	(struct Boundary_Value* bv, const struct Boundary_Value_Input* bv_i, const struct Solver_Face* face,
	 const struct Simulation* sim)
{
	UNUSED(bv_i);
	const int side_index = 1;

	struct Multiarray_d* sol_r_fcr = (struct Multiarray_d*) constructor_s_fc_interp(face,sim,side_index);
	permute_Multiarray_d_fc(sol_r_fcr,'R',side_index,face);

	bv->s     = (const struct const_Multiarray_d*)sol_r_fcr;
	bv->g     = NULL;
	bv->ds_ds = NULL;
}

void destructor_Boundary_Value (struct Boundary_Value* bv)
{
	if (bv->s)
		destructor_const_Multiarray_d(bv->s);
	if (bv->g)
		destructor_const_Multiarray_d(bv->g);
	if (bv->ds_ds)
		destructor_const_Multiarray_d(bv->ds_ds);
}

// Static functions ************************************************************************************************* //
// Level 0 ********************************************************************************************************** //

static const struct const_Multiarray_d* constructor_s_fc_interp
	(const struct Solver_Face* s_face, const struct Simulation* sim, const int side_index)
{
	const struct Operator* cv0_vs_fc = get_operator__cv0_vs_fc(side_index,s_face);

	// sim may be used to store a parameter establishing which type of operator to use for the computation.
	UNUSED(sim);
	const char op_format = 'd';

	struct Solver_Volume* s_volume = (struct Solver_Volume*) ((struct Face*)s_face)->neigh_info[side_index].volume;

	const struct const_Multiarray_d* s_coef = (const struct const_Multiarray_d*) s_volume->sol_coef;

	return constructor_mm_NN1_Operator_const_Multiarray_d(cv0_vs_fc,s_coef,'C',op_format,s_coef->order,NULL);
}
