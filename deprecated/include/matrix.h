// Copyright 2017 Philip Zwanenburg
// MIT License (https://github.com/PhilipZwanenburg/DPGSolver/blob/master/LICENSE)

#ifndef DPG__Matrix_h__INCLUDED
#define DPG__Matrix_h__INCLUDED
/**	\file
 *	\brief Provides Matrix_\* containers and related functions.
 *
 *	Potentially relevant comments may be found in containers.h.
 *
 *	Matrices are 2D Multiarrays.
 */

#include <stddef.h>
#include <stdbool.h>

/// \brief Matrix (`double`).
struct Matrix_d {
	char layout;

	ptrdiff_t extents[2];

	bool    owns_data;
	double* data;
};

/// \brief Matrix (`const double`).
struct const_Matrix_d {
	const char layout;

	const ptrdiff_t extents[2];

	const bool         owns_data;
	const double*const data;
};

/// \brief Matrix (`int`).
struct Matrix_i {
	char layout;

	ptrdiff_t extents[2];

	bool    owns_data;
	int* data;
};

/// \brief Matrix (`const int`).
struct const_Matrix_i {
	const char layout;

	const ptrdiff_t extents[2];

	const bool    owns_data;
	const int* data;
};

// Constructor/Destructor functions ********************************************************************************* //

/// \brief Constructs an empty \ref Matrix_d\*.
struct Matrix_d* constructor_empty_Matrix_d
	(const char layout,  ///< Standard.
	 const ptrdiff_t ext_0, ///< Standard.
	 const ptrdiff_t ext_1  ///< Standard.
	);

/// \brief Move Constructor for a \ref const_Matrix_d\*.
struct const_Matrix_d* constructor_move_const_Matrix_d_Matrix_d
	(struct Matrix_d*const src ///< Source Matrix.
	);

/// \brief Move Constructor for a `const` \ref const_Matrix_d `*const`.
void const_constructor_move_Matrix_d
	(const struct const_Matrix_d*const* dest, ///< Destination.
	 struct Matrix_d* src                     ///< Source.
	);

/// \brief Destructs a \ref Matrix_d\*.
void destructor_Matrix_d
	(struct Matrix_d* a ///< Standard.
	);

/// \brief Constructs an empty \ref Matrix_i\*.
struct Matrix_i* constructor_empty_Matrix_i
	(const char layout,  ///< Standard.
	 const ptrdiff_t ext_0, ///< Standard.
	 const ptrdiff_t ext_1  ///< Standard.
	);

/// \brief Move Constructor for a \ref const_Matrix_i\*.
struct const_Matrix_i* constructor_move_const_Matrix_i_Matrix_i
	(struct Matrix_i*const src ///< Source Matrix.
	);

/// \brief Move Constructor for a `const` \ref const_Matrix_i `*const`.
void const_constructor_move_Matrix_i
	(const struct const_Matrix_i*const* dest, ///< Destination.
	 struct Matrix_i* src                     ///< Source.
	);

/// \brief Destructs a \ref Matrix_i\*.
void destructor_Matrix_i
	(struct Matrix_i* a ///< Standard.
	);

// Helper functions ************************************************************************************************* //

/** \brief Get pointer to row of row-major \ref Matrix_d\*.
 *	\return Pointer to the first entry of the row.
 */
double* get_row_Matrix_d
	(const ptrdiff_t row,     ///< Desired row.
	 const struct Matrix_d* a ///< Matrix.
	);

/** \brief Get pointer to row of row-major \ref const_Matrix_d\*.
 *	\return Pointer to the first entry of the row.
 */
const double* get_row_const_Matrix_d
	(const ptrdiff_t row,           ///< Desired row.
	 const struct const_Matrix_d* a ///< Matrix.
	);

/** \brief Get pointer to row of row-major \ref Matrix_i\*.
 *	\return Pointer to the first entry of the row.
 */
int* get_row_Matrix_i
	(const ptrdiff_t row,     ///< Desired row.
	 const struct Matrix_i* a ///< Matrix.
	);

/** \brief Compute the norm of the specified row of the input \ref Matrix_d.
 *	\return See brief. */
double compute_norm_Matrix_d_row
	(const ptrdiff_t row,           ///< The row.
	 const struct Matrix_d*const a, ///< The input matrix.
	 const char*const norm_type     ///< The norm type.
	);

/** \brief Get value of the (row,col) entry of a \ref Matrix_i\*.
 *	\return See brief. */
int get_val_Matrix_i
	(const ptrdiff_t row,          ///< The row.
	 const ptrdiff_t col,          ///< The column.
	 const struct Matrix_i*const a ///< Standard.
	);

/** \brief Get value of the (row,col) entry of a \ref const_Matrix_i\*.
 *	\return See brief. */
int get_val_const_Matrix_i
	(const ptrdiff_t row,                ///< The row.
	 const ptrdiff_t col,                ///< The column.
	 const struct const_Matrix_i*const a ///< Standard.
	);

/// \brief Set the values of the destination row to that of the source data.
void set_row_Matrix_d
	(const ptrdiff_t row,         ///< The destination row.
	 const struct Matrix_d* dest, ///< The destination Matrix.
	 const double*const data_src  ///< The source data.
	);

// Printing functions *********************************************************************************************** //

/// \brief Print a \ref Matrix_d\* to the terminal.
void print_Matrix_d
	(const struct Matrix_d*const a ///< Standard.
	);

/// \brief Print a \ref const_Matrix_d\* to the terminal.
void print_const_Matrix_d
	(const struct const_Matrix_d*const a ///< Standard.
	);

/// \brief Print a \ref Matrix_i\* to the terminal.
void print_Matrix_i
	(const struct Matrix_i*const a ///< Standard.
	);

/// \brief Print a \ref const_Matrix_i\* to the terminal.
void print_const_Matrix_i
	(const struct const_Matrix_i*const a ///< Standard.
	);


#endif // DPG__Matrix_h__INCLUDED
