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

#include "solve.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "macros.h"
#include "definitions_test_case.h"
#include "definitions_intrusive.h"

#include "computational_elements.h"

#include "intrusive.h"
#include "simulation.h"
#include "solve_explicit.h"
#include "solve_implicit.h"
#include "solution.h"
#include "test_case.h"

#include "compute_grad_coef_dg.h"
#include "compute_volume_rlhs_dg.h"
#include "compute_face_rlhs_dg.h"

// Static function declarations ************************************************************************************* //

/// \brief Set the memory of the rhs and lhs (if applicable) terms to zero for the volumes.
static void zero_memory_volumes
	(const struct Simulation* sim ///< \ref Simulation.
	);

// Interface functions ********************************************************************************************** //

void solve_for_solution (struct Simulation* sim)
{
	assert(sim->volumes->name == IL_SOLVER_VOLUME);
	assert(sim->faces->name   == IL_SOLVER_FACE);

	constructor_derived_Elements(sim,IL_SOLUTION_ELEMENT);
	set_initial_solution(sim);
	destructor_derived_Elements(sim,IL_ELEMENT);

	const struct Test_Case* test_case = sim->test_case;
	switch (test_case->solver_proc) {
	case SOLVER_E:
		solve_explicit(sim);
		break;
	case SOLVER_I:
		solve_implicit(sim);
		break;
	case SOLVER_EI:
		solve_explicit(sim);
		solve_implicit(sim);
		break;
	default:
		EXIT_ERROR("Unsupported: %d\n",sim->test_case->solver_proc);
		break;
	}
EXIT_ADD_SUPPORT;
}

double compute_rhs (const struct Simulation* sim)
{
	/// \todo Add assertions relevant to rhs then call 'compute_rlhs'.
	double max_rhs = 0.0;

	switch (sim->method) {
	case METHOD_DG:
		zero_memory_volumes(sim); // Possibly move this to a dg specific file.
		compute_grad_coef_dg(sim);
		compute_volume_rlhs_dg(sim);
		compute_face_rlhs_dg(sim);
EXIT_UNSUPPORTED;
		break;
	default:
		EXIT_ERROR("Unsupported: %d\n",sim->method);
		break;
	}

	return max_rhs;
}

// Static functions ************************************************************************************************* //
// Level 0 ********************************************************************************************************** //

static void zero_memory_volumes (const struct Simulation* sim)
{
	for (struct Intrusive_Link* curr = sim->volumes->first; curr; curr = curr->next) {
		set_to_value_Multiarray_d(((struct DG_Solver_Volume*)curr)->rhs,0.0);
	}
}