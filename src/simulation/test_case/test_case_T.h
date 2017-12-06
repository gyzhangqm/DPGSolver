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
 *  \brief Provides templated container(s) and functions relating to the test cases.
 */

#include <stdbool.h>

struct Simulation;

/** \brief Container for test case specific information.
 *
 *  This container is used to hold test case specific variables (such as those related to the pde under consideration)
 *  as well as function pointers to various functions such that the control flow is not broken during run-time.
 *
 *  As this container is a general template for many supported test cases, it is frequently the case that some of the
 *  function pointers are not necessary. In this case, they are set to point to functions which simply return
 *  immediately.
 */
struct Test_Case_T {
	const int pde_index; ///< Index corresponding to \ref Simulation::pde_name.

	const bool has_1st_order, ///< Flag for whether the pde under consideration has 1st order terms.
	           has_2nd_order; ///< Flag for whether the pde under consideration has 2nd order terms.

	const int n_var, ///< Number of variables in the PDE under consideration.
	          n_eq;  ///< Number of equations in the PDE under consideration.

	set_sol_fptr_T set_sol;  ///< Function pointer to the function used to set data relating to the solution.
	set_sol_fptr_T set_grad; ///< Function pointer to the function used to set data relating to the solution gradients.

	/** Pointer to function constructing the physical xyz coordinates from either parametric space or from straight to
	 *  curved elements. */
	constructor_xyz_fptr constructor_xyz;

	constructor_sol_fptr_T constructor_sol; ///< Function pointer to the function used to construct the solution.

	// Solver related parameters
	char solver_method_curr; ///< The current solver method. Options: 'e'xplicit, 'i'mplicit.

	/// The type of solver procedure to be used for the simulation. Options: See definitions_test_case.h.
	const int solver_proc;

	// Parameters for explicit simulations.
	const int solver_type_e; ///< The explicit solver type. Options: See definitions_test_case.h.

	double time;             ///< The current time.
	const double time_final; ///< The final time.
	const double dt;         ///< The time increment at each stage of the explicit solve.

	// Parameters for implicit simulations.
	/** Flag for whether the Schur complement should be used for the global system solve. This option is available
	 *  whenever it is possible for certain degrees of freedom to be statically condensed out of the global
	 *  system. */
	const bool use_schur_complement;

	// Parameters for explicit/implicit simulations.
	const int solver_type_i; ///< The implicit solver type. Options: See definitions_test_case.h.

	const int ind_num_flux[2], ///< Integer indices of the type of 1st/2nd order numerical fluxes.
	          ind_test_norm;   ///< Integer indices of the type of test_norm.

	const double exit_tol_e,   ///< The exit tolerance for the residual during the explicit solver stage.
	             exit_ratio_e, ///< The exit ratio for the residual during the explicit solver stage.
	             exit_tol_i,   ///< The exit tolerance for the residual during the implicit solver stage.
	             exit_ratio_i; ///< The exit ratio for the residual during the implicit solver stage.

	const bool flux_comp_mem_e[MAX_FLUX_OUT], ///< \ref Flux_Input::compute_member (explicit).
	           flux_comp_mem_i[MAX_FLUX_OUT]; ///< \ref Flux_Input::compute_member (implicit).

	/// Function pointer to the function used to call the combination of 1st and 2nd order flux functions.
	compute_Flux_fptr_T compute_Flux;

/// \todo Merge the flux computation functions (without and with Jacobian) and remove one of these function pointers.
	/// Function pointers to the functions used to compute the 1st/2nd order fluxes for the explicit solver.
	compute_Flux_fptr_T compute_Flux_e[2];

	/** Function pointers to the functions used to compute the 1st/2nd order fluxes (and optionally flux Jacobians)
	 *  for the implicit solver. */
	compute_Flux_fptr_T compute_Flux_i[2];

/// \todo Delete if unused.
	const bool num_flux_comp_mem_e[MAX_NUM_FLUX_OUT], ///< \ref Boundary_Value_Input::compute_member (explicit).
	           num_flux_comp_mem_i[MAX_NUM_FLUX_OUT]; ///< \ref Boundary_Value_Input::compute_member (implicit).

	/// Function pointer to the function used to call the combination of 1st and 2nd order numerical flux functions.
	compute_Numerical_Flux_fptr_T compute_Numerical_Flux;

/// \todo Merge the numerical flux computation functions (without and with Jacobian) and remove one of these function pointers.
	/// Function pointers to the functions used to compute the 1st/2nd order numerical fluxes for the explicit solver.
	compute_Numerical_Flux_fptr_T compute_Numerical_Flux_e[2];

	/** Function pointers to the functions used to compute the 1st/2nd order numerical fluxes (and optionally
	 *  Jacobians) for the implicit solver. */
	compute_Numerical_Flux_fptr_T compute_Numerical_Flux_i[2];

	/** Constructor for solution and gradient from the left volume at face cubature nodes as seen from the left
	 *  volume. */
	constructor_Boundary_Value_Input_face_fptr_T constructor_Boundary_Value_Input_face_fcl;

	/** \note constructor_*_r_fcl are provided as part of the Solver_Face_* container as they are dependent upon
	 *        whether the face is on a boundary or not. */

	compute_source_rhs_fptr_T compute_source_rhs; ///< Function pointer to the rhs source computation function.

	constructor_Error_CE_fptr constructor_Error_CE; ///< Function pointer to the function computing the error.

	// Miscellaneous parameters
	const bool display_progress; ///< Flag for whether the solver progress should be displayed (in stdout).

	const double conv_order_discount; ///< Discount on the value of the convergence order required for optimality.
};

/** \brief Constructor for a \ref Test_Case_T.
 *  \return See brief. */
struct Test_Case_T* constructor_Test_Case_T
	(const struct Simulation* sim ///< \ref Simulation.
	);

/// \brief Destructor for a \ref Test_Case_T.
void destructor_Test_Case_T
	(const struct Test_Case_T* test_case ///< \ref Test_Case_T.
	);