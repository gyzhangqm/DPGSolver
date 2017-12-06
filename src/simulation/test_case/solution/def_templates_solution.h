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
 *  \brief Provides the macro definitions used for c-style templating related to the solution functions.
 */

#if TYPE_RC == TYPE_REAL

///\{ \name Data types
#define Solution_Container_T Solution_Container
///\}

///\{ \name Function pointers
#define constructor_sol_fptr_T         constructor_sol_fptr
#define mutable_constructor_sol_fptr_T mutable_constructor_sol_fptr
#define set_sol_fptr_T                 set_sol_fptr
#define compute_source_rhs_fptr_T      compute_source_rhs_fptr
///\}

///\{ \name Function names
#define constructor_const_sol_invalid_T constructor_const_sol_invalid
#define set_initial_solution_T          set_initial_solution
#define set_sg_do_nothing_T             set_sg_do_nothing
#define constructor_xyz_sol_T           constructor_xyz_sol
#define compute_coef_from_val_vs_T      compute_coef_from_val_vs
#define constructor_sol_v_T             constructor_sol_v
#define compute_source_rhs_do_nothing_T compute_source_rhs_do_nothing
#define update_Solution_Container_sol_T update_Solution_Container_sol
#define constructor_xyz_vc_interp_T     constructor_xyz_vc_interp
#define get_operator__tw0_vt_vc_T       get_operator__tw0_vt_vc

#define set_initial_v_sg_coef_T    set_initial_v_sg_coef
#define set_initial_f_nf_coef_T    set_initial_f_nf_coef
#define get_operator__cv0_vg_vc_T  get_operator__cv0_vg_vc
#define constructor_xyz_v_T        constructor_xyz_v
#define constructor_xyz_f_T        constructor_xyz_f
#define constructor_normals_ff_T   constructor_normals_ff
#define constructor_nf_T           constructor_nf
#define compute_coef_from_val_ff_T compute_coef_from_val_ff
#define constructor_metrics_ff_T   constructor_metrics_ff
///\}

///\{ \name Function names (pde specific)
#define convert_variables_T convert_variables
///\}

#elif TYPE_RC == TYPE_COMPLEX

#endif