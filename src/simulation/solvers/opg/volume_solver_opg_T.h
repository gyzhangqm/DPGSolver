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
 *  \brief Provides the interface for the templated \ref OPG_Solver_Volume_T container and associated functions.
 *
 *  These volumes are needed by the 'O'ptimal 'P'etrov 'G'alerkin solver functions.
 */

#include "def_templates_matrix.h"
#include "def_templates_multiarray.h"
#include "def_templates_volume_solver.h"
#include "def_templates_volume_solver_opg.h"

/// \brief Container for data relating to the OPG solver volumes.
struct OPG_Solver_Volume_T {
	struct Solver_Volume_T volume; ///< The base \ref Solver_Volume_T.

/// \todo Consider preconstructing m_block diag as it seems that this is possibly the only usage of m.
	const struct const_Matrix_T* m; ///< The mass matrix.

/// \todo Delete m_inv if unused.
	const struct const_Matrix_T* m_inv; ///< The inverse mass matrix.

	/** Flag relating to what type of boundary condition is required for \ref Solver_Volume_T::test_s_coef for faces
	 *  adjacent to boundary volumes.
	 *
	 *  Generally, only one boundary constraint should be applied for each volume. For example, in the case of the
	 *  linear advection equation with a QUAD element having two outflow faces, constraining the value of the test
	 *  function on both outflow boundaries does not allow for the recovery of even the exact constant solution.
	 *  This is most clear in the case of a pt = 1 QUAD where this results in a single free parameter. Constraining
	 *  only one of the two faces allows for the computation of the test function to obtain the exact solution. Note
	 *  that the same problem occurs for the continuous test space used in the original article of Brunken et al.,
	 *  section 3.2 \cite Brunken2018.
	 */
	int bc_test_s;
};

/// \brief Constructor for a derived \ref OPG_Solver_Volume_T.
void constructor_derived_OPG_Solver_Volume_T
	(struct Volume* volume_ptr,   ///< Pointer to the volume.
	 const struct Simulation* sim ///< \ref Simulation.
	);

/// \brief Destructor for a derived \ref OPG_Solver_Volume_T.
void destructor_derived_OPG_Solver_Volume_T
	(struct Volume* volume_ptr ///< Pointer to the volume.
	);

/** \brief Get the pointer to the appropriate \ref OPG_Solver_Element::vc0_vs_vs operator.
 *  \return See brief. */
const struct Operator* get_operator__vc0_vs_vs_T
	 (const struct OPG_Solver_Volume_T*const opg_s_vol ///< Standard.
	  );

/** \brief Get the appropriate sub-range of the \ref OPG_Solver_Element::cv1_vt_vs operators.
 *  \return See brief. */
struct Multiarray_Operator get_operator__cv1_vt_vs_T
	(const struct OPG_Solver_Volume_T*const opg_s_vol ///< Standard.
	 );

/** \brief Get the pointer to the appropriate \ref OPG_Solver_Element::cv0_vg_vt operator.
 *  \return See brief. */
const struct Operator* get_operator__cv0_vg_vt_T
	(const struct OPG_Solver_Volume_T*const opg_s_vol ///< Standard.
	 );

#include "undef_templates_matrix.h"
#include "undef_templates_multiarray.h"
#include "undef_templates_volume_solver.h"
#include "undef_templates_volume_solver_opg.h"
