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

#ifndef DPG__volume_solver_adaptive_h__INCLUDED
#define DPG__volume_solver_adaptive_h__INCLUDED
/** \file
 *  \brief Provides the interface for the \ref Adaptive_Solver_Volume container and associated functions.
 */

#include "volume_solver.h"

/// \brief Container for data relating to the adaptive solver volumes.
struct Adaptive_Solver_Volume {
	struct Solver_Volume volume; ///< The base \ref Solver_Volume_T.

	int adapt_type; ///< The type of adaptation to use. Options: see \ref definitions_adaptation.h.

	int p_ref_prev; ///< The previous value of \ref Solver_Volume_T::p_ref.
	int ind_h;      ///< The index of the h-refinement (if applicable).

	bool updated;         ///< Flag for whether the volume was updated.
	bool updated_geom_nc; ///< Flag for whether the volume geometry was updated due to non-conformity.

	/// Flag for whether the volume geometry was updated due to non-conformity from a single face.
	bool updated_geom_nc_face;

	struct Intrusive_Link* child_0; ///< Pointer to the first child (if applicable).
	struct Intrusive_Link* parent;  ///< Pointer to the parent (if applicable).
};

/// \brief Constructor for a derived \ref Adaptive_Solver_Volume.
void constructor_derived_Adaptive_Solver_Volume
	(struct Volume* volume_ptr,   ///< Pointer to the volume.
	 const struct Simulation* sim ///< \ref Simulation.
	);

/// \brief Destructor for a derived \ref Adaptive_Solver_Volume.
void destructor_derived_Adaptive_Solver_Volume
	(struct Volume* volume_ptr ///< Pointer to the volume.
	);

/// \brief Initialize adaptation related parameters for the current \ref Adaptive_Solver_Volume.
void initialize_Adaptive_Solver_Volume
	(struct Adaptive_Solver_Volume*const a_s_vol ///< \ref Adaptive_Solver_Volume.
	);

#endif // DPG__volume_solver_adaptive_h__INCLUDED
