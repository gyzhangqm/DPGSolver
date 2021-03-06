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

#include "face_solver_adaptive.h"

#include "macros.h"
#include "definitions_adaptation.h"

#include "simulation.h"

// Static function declarations ************************************************************************************* //

// Interface functions ********************************************************************************************** //

void constructor_derived_Adaptive_Solver_Face (struct Face* face_ptr, const struct Simulation* sim)
{
	struct Adaptive_Solver_Face* a_s_face = (struct Adaptive_Solver_Face*) face_ptr;
	struct Solver_Face* s_face            = (struct Solver_Face*) face_ptr;

	a_s_face->p_ref_prev = s_face->p_ref;
	initialize_Adaptive_Solver_Face(a_s_face);
	UNUSED(sim);
}

void destructor_derived_Adaptive_Solver_Face (struct Face* face_ptr)
{
	struct Adaptive_Solver_Face* a_s_face = (struct Adaptive_Solver_Face*) face_ptr;
	UNUSED(a_s_face);
}

void initialize_Adaptive_Solver_Face (struct Adaptive_Solver_Face*const a_s_face)
{
	a_s_face->adapt_type = ADAPT_NONE;
	a_s_face->ind_h      = -1;
	a_s_face->updated    = false;

	a_s_face->child_0 = NULL;
	a_s_face->parent  = NULL;
}

// Static functions ************************************************************************************************* //
// Level 0 ********************************************************************************************************** //
