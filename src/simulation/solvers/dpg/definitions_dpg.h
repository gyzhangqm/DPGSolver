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

#ifndef DPG__definitions_dpg_h__INCLUDED
#define DPG__definitions_dpg_h__INCLUDED
/** \file
 *  \brief Provides the definitions related to the dpg solver.
 */

///\{ \name Available test norms.
#define TEST_NORM_INVALID 1

// See Chan's thesis p. 118 for the robust norms (advection-diffusion, NS).
#define TEST_NORM_H0         11 ///< H0 norm (L2 norm)
#define TEST_NORM_H1         21 ///< H1 norm
#define TEST_NORM_H1_UPWIND  22 ///< H1 norm using differentiation operators in the streamline direction.
#define TEST_NORM_ADJOINT    31 ///< Norm based on the formal adjoint of the bilinear form.
///\}

///\{ \name Available conservation enforcement procedures.
#define CONSERVATION_NOT_ENFORCED  110 ///< Not enforced. (Value must be positive and less than all other options)
#define CONSERVATION_LAGRANGE_MULT 111 ///< Enforced using Lagrange multiplier method.
///\}

#endif // DPG__definitions_dpg_h__INCLUDED
