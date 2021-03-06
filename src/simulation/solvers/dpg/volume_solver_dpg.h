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

#ifndef DPG__volume_solver_dpg_h__INCLUDED
#define DPG__volume_solver_dpg_h__INCLUDED
/** \file
 *  \brief Provides the interface for the real \ref DPG_Solver_Volume_T container and associated functions.
 */

#include "volume_solver.h"

#include "def_templates_type_d.h"
#include "volume_solver_dpg_T.h"
#include "undef_templates_type.h"

#include "def_templates_type_dc.h"
#include "volume_solver_dpg_T.h"
#include "undef_templates_type.h"

struct DPG_Solver_Volume_c;

/// \brief Copy members from a real to a complex \ref DPG_Solver_Volume_T.
void copy_members_r_to_c_DPG_Solver_Volume
	(struct DPG_Solver_Volume_c*const dpg_s_vol,       ///< The complex \ref DPG_Solver_Volume_T.
	 const struct DPG_Solver_Volume*const dpg_s_vol_r, ///< The real \ref DPG_Solver_Volume_T.
	 const struct Simulation*const sim                 ///< \ref Simulation.
	);

#endif // DPG__volume_solver_dpg_h__INCLUDED
