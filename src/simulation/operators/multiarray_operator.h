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

#ifndef DPG__multiarray_operator_h__INCLUDED
#define DPG__multiarray_operator_h__INCLUDED
/** \file
 *  \brief Provides Multiarray_Operator\* containers and related functions.
 */

#include <stdbool.h>
#include <stddef.h>

struct const_Vector_i;

/// Container holding the multiarray of operators in various formats.
struct Multiarray_Operator {
	const int order;               ///< Defined in \ref Multiarray_T.
	const ptrdiff_t*const extents; ///< Defined in \ref Multiarray_T.

	const bool owns_data;                   ///< Defined in \ref Multiarray_T.
	const struct Operator*const*const data; ///< The array of pointers to \ref Operator containers.
};

// Constructor functions ******************************************************************************************** //
// Default constructors ********************************************************************************************* //

/** \brief Constructor for a default \ref Multiarray_Operator\*.
 *  \return Standard. */
const struct Multiarray_Operator* constructor_default_Multiarray_Operator ();

// Empty constructors *********************************************************************************************** //

/** \brief Constructor for an empty \ref Multiarray_Operator\*.
 *  \return Standard. */
const struct Multiarray_Operator* constructor_empty_Multiarray_Operator
	(const int order,                ///< Defined in \ref Multiarray_Operator.
	 const ptrdiff_t*const extents_i ///< The input extents.
	);

/** \brief Constructor for an empty \ref Multiarray_Operator\* with extents input as a \ref const_Vector_T\*.
 *  \return Standard. */
const struct Multiarray_Operator* constructor_empty_Multiarray_Operator_V
	(const struct const_Vector_i*const extents_i_V ///< The input extents in vector format.
	);

// Destructors ****************************************************************************************************** //

/// \brief Destructs a \ref Multiarray_Operator\*.
void destructor_Multiarray_Operator
	(const struct Multiarray_Operator* a ///< Standard.
	);

/// \brief Destructor for a \ref Multiarray_Operator\*[2].
void destructor_Multiarray2_Operator
	(const struct Multiarray_Operator* a[2] ///< Standard.
	);

/// \brief Version of \ref destructor_Multiarray_Operator which does not call the destructor for a `NULL` input.
void destructor_Multiarray_Operator_conditional
	(const struct Multiarray_Operator* a ///< Standard.
	);

/// \brief Version of \ref destructor_Multiarray2_Operator which does not call the destructor for `NULL` inputs.
void destructor_Multiarray2_Operator_conditional
	(const struct Multiarray_Operator* a[2] ///< See brief.
	);

// Setter functions ************************************************************************************************* //

/** \brief Set and return a statically allocated \ref Multiarray_Operator\* from a sub range of a
 *         \ref Multiarray_Operator\*.
 *  \return See brief. */
struct Multiarray_Operator set_MO_from_MO
	(const struct Multiarray_Operator* src, ///< The source.
	 const int order_o,                     ///< The order of the output (destination).
	 const ptrdiff_t*const sub_indices      ///< The sub-indices specifying which part of the source to extract.
	);

/** \brief `const` version of \ref alloc_mutable_MO_from_MO.
 *  \return See brief. */
const struct Multiarray_Operator* alloc_MO_from_MO
	(const struct Multiarray_Operator* src, ///< See brief.
	 const int order_o,                     ///< See brief.
	 const ptrdiff_t*const sub_indices      ///< See brief.
	);

/// \brief "Destructor" for \ref alloc_MO_from_MO.
void free_MO_from_MO
	(const struct Multiarray_Operator* src ///< Standard.
	);

/** \brief Get a pointer to a \ref Operator\* from a sub range of a \ref Multiarray_Operator\*.
 *  \return See brief. */
const struct Operator* get_Multiarray_Operator
	(const struct Multiarray_Operator* src, ///< The source.
	 const ptrdiff_t*const sub_indices      ///< The sub-indices specifying which part of the source to extract.
	);

// Printing functions *********************************************************************************************** //

/// \brief Print a \ref Multiarray_Operator\* to the terminal displaying entries below the default tolerance as 0.0.
void print_Multiarray_Operator
	(const struct Multiarray_Operator*const a ///< Standard.
	);

/// \brief Print a \ref Multiarray_Operator\* to the terminal displaying entries below the tolerance as 0.0.
void print_Multiarray_Operator_tol
	(const struct Multiarray_Operator*const a, ///< Standard.
	 const double tol                          ///< The tolerance.
	);

/// \brief Increment the counter for the indicies by 1.
void increment_counter_MaO
	(const int order,               ///< Defined in \ref Multiarray_T.
	 const ptrdiff_t*const extents, ///< Defined in \ref Multiarray_T.
	 ptrdiff_t*const counter        ///< The counter for the indices.
	);

#endif // DPG__multiarray_operator_h__INCLUDED
