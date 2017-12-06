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
 *  \brief Provides the macro definitions used for c-style templating related to the face rlhs computing functions.
 */

#if TYPE_RC == TYPE_REAL

///\{ \name Data types
///\}

///\{ \name Function pointers
///\}

///\{ \name Function names
#define get_operator__tw0_vt_fc_T               get_operator__tw0_vt_fc
#define get_operator__cv0_vs_fc_T               get_operator__cv0_vs_fc
#define permute_Matrix_T_fc                     permute_Matrix_d_fc
#define get_operator__nc_fc_T                   get_operator__nc_fc
#define constructor_Numerical_Flux_Input_data_T constructor_Numerical_Flux_Input_data
#define destructor_Numerical_Flux_Input_data_T  destructor_Numerical_Flux_Input_data
#define constructor_lhs_f_1_T                   constructor_lhs_f_1
///\}

#elif TYPE_RC == TYPE_COMPLEX


#endif