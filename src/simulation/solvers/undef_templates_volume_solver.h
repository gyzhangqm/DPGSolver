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
 *  \brief Undefine macro definitions for c-style templating relating to \ref Solver_Volume_T containers/functions.
 */

///\{ \name Data types
#undef Solver_Volume_T
///\}

///\{ \name Function names
#undef constructor_derived_Solver_Volume_T
#undef destructor_derived_Solver_Volume_T
#undef get_operator__w_vc__s_e_T
#undef constructor_mass_T
#undef constructor_inverse_mass_T
#undef constructor_l2_proj_operator_s_T
#undef get_operator__cv0_vs_vs_T
#undef get_operator__cv0_vr_vs_T
#undef get_operator__cv0_vg_vs_T
#undef get_operator__cv0_vs_vc_T
#undef get_operator__cv0_vt_vc_T
#undef get_operator__cv0_vr_vc_T
#undef get_operator__tw1_vt_vc_T
#undef get_operator__cv1_vt_vc_T
///\}

#undef set_function_pointers_constructor_xyz_surface
