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

#ifndef DPG__boundary_templates_c_h__INCLUDED
#define DPG__boundary_templates_c_h__INCLUDED
/** \file
 *  \brief Provides the macro definitions used for c-style templating related to the complex boundary functions.
 *
 *  See \ref boundary_templates.h for description of parameters.
 */

#define Boundary_Value_T Boundary_Value_c             ///< See brief.
#define Boundary_Value_Input_T Boundary_Value_Input_c ///< See brief.
#define Boundary_Value_Input_R Boundary_Value_Input   ///< See brief.

#define constructor_Boundary_Value_advection_inflow_T constructor_Boundary_Value_c_advection_inflow   ///< See brief.
#define constructor_Boundary_Value_advection_outflow_T constructor_Boundary_Value_c_advection_outflow ///< See brief.

#define constructor_Boundary_Value_euler_riemann_T constructor_Boundary_Value_c_euler_riemann   ///< See brief.
#define constructor_Boundary_Value_euler_slipwall_T constructor_Boundary_Value_c_euler_slipwall ///< See brief.

#endif // DPG__boundary_templates_c_h__INCLUDED
