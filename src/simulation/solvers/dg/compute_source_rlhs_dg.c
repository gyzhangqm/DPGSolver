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

#include "compute_volume_rlhs_dg.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "macros.h"
#include "definitions_intrusive.h"

#include "volume_solver_dg.h"

#include "intrusive.h"
#include "simulation.h"
#include "test_case.h"

// Static function declarations ************************************************************************************* //

#include "def_templates_type_d.h"
#include "def_templates_test_case.h"

// Interface functions ********************************************************************************************** //

void compute_source_rhs_dg (const struct Simulation* sim)
{
	assert(sim->volumes->name == IL_VOLUME_SOLVER_DG);

	struct Test_Case_T* test_case = (struct Test_Case_T*)sim->test_case_rc->tc;

	for (struct Intrusive_Link* curr = sim->volumes->first; curr; curr = curr->next) {
		const struct Solver_Volume* s_vol       = (struct Solver_Volume*) curr;
		struct DG_Solver_Volume* dg_s_vol = (struct DG_Solver_Volume*) curr;

		test_case->compute_source_rhs(sim,s_vol,dg_s_vol->rhs);
	}
}

// Static functions ************************************************************************************************* //
// Level 0 ********************************************************************************************************** //
