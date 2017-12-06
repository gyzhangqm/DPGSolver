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
 */

#include <assert.h>

#include "macros.h"

// Static function declarations ************************************************************************************* //

/// \brief Container for a vector with an associated index.
struct Vector_T_indexed {
	ptrdiff_t index;         ///< The index.
	struct Vector_T* vector; ///< The \ref Vector_T\*.
};

/** \brief Move constructor for a \ref Vector_T_indexed\*\* from a \ref Vector_T\*\*.
 *  \return Standard. */
static struct Vector_T_indexed** constructor_move_Vector_T_indexed
	(const ptrdiff_t size,  ///< The number of elements.
	 struct Vector_T** data ///< The data to be moved.
	);

/// \brief Destructor for a \ref Vector_T_indexed\*\*.
static void destructor_Vector_T_indexed
	(struct Vector_T_indexed** src, ///< Standard.
	 const ptrdiff_t size           ///< The number of elements.
	);

/** \brief Comparison function for std::qsort between `struct Vector_T_indexed**` `a` and `b`.
 *  \return The lexicographical comparison of `a` and `b`.
 *
 *  \note Input Vectors must be have sorted data.
 */
static int cmp_Vector_T_indexed
	(const void *a, ///< Variable 1.
	 const void *b  ///< Variable 2.
	);

/** \brief Reorder a \ref Multiarray_Vector_T based on the provided ordering.
 *  \warning This is not currently done in place.
 */
static void reorder_Multiarray_Vector_T
	(struct Multiarray_Vector_T*const a, ///< Standard.
	 const int*const ordering    ///< The ordering.
	);

/** \brief Compute the total number of entries in a \ref Multiarray_Vector_T\*.
 *  \return See brief. */
static ptrdiff_t compute_total_entries
	(const struct Multiarray_Vector_T*const src ///< Standard.
	);

// Interface functions ********************************************************************************************** //

Type* get_row_Multiarray_T (const ptrdiff_t row, const struct Multiarray_T* a)
{
	assert(a->order == 2);
	assert(a->layout == 'R');
	return &a->data[row*(a->extents[1])];
}

const Type* get_row_const_Multiarray_T (const ptrdiff_t row, const struct const_Multiarray_T* a)
{
	assert(a->order >  0);
	assert(a->order <= 2);
	assert(a->layout == 'R');

	const ptrdiff_t ext_1 = ( a->order == 1 ? 1 : a->extents[1] );
	return &a->data[row*ext_1];
}

Type* get_col_Multiarray_T (const ptrdiff_t col, struct Multiarray_T* a)
{
	assert(a->layout == 'C');

	const ptrdiff_t ext_0 = a->extents[0];
	return &a->data[col*ext_0];
}

const Type* get_col_const_Multiarray_T (const ptrdiff_t col, const struct const_Multiarray_T* a)
{
	return (const Type*) get_col_Multiarray_T(col,(struct Multiarray_T*)a);
}

void set_to_value_Multiarray_T (struct Multiarray_T*const a, const Type val)
{
	const ptrdiff_t size = compute_size(a->order,a->extents);
	for (ptrdiff_t i = 0; i < size; ++i)
		a->data[i] = val;
}

void set_Multiarray_Vector_T_T
	(struct Multiarray_Vector_T* a, const Type* data_V, const int*const ext_V)
{
	const ptrdiff_t size = compute_size(a->order,a->extents);
	for (ptrdiff_t i = 0; i < size; i++) {
		resize_Vector_T(a->data[i],ext_V[i]);
		for (ptrdiff_t j = 0; j < ext_V[i]; j++)
			a->data[i]->data[j] = *data_V++;
	}
}

void set_Multiarray_T (struct Multiarray_T* a_o, const struct const_Multiarray_T* a_i)
{
	assert(a_o->owns_data);

	resize_Multiarray_T(a_o,a_i->order,a_i->extents);
	a_o->layout = a_i->layout;

	const ptrdiff_t size = compute_size(a_i->order,a_i->extents);
	for (ptrdiff_t i = 0; i < size; ++i)
		a_o->data[i] = a_i->data[i];
}

void set_Multiarray_T_Multiarray_R (struct Multiarray_T* a, const struct const_Multiarray_R* b)
{
	const ptrdiff_t size = compute_size(a->order,a->extents);
	assert(size == compute_size(b->order,b->extents));

	for (int i = 0; i < size; ++i)
		a->data[i] = (Type)b->data[i];
}

struct Vector_i* sort_Multiarray_Vector_T (struct Multiarray_Vector_T* a, const bool return_indices)
{
	// sort the individual Vector entries
	const ptrdiff_t size = compute_size(a->order,a->extents);
	for (ptrdiff_t i = 0; i < size; ++i)
		sort_Vector_T(a->data[i]);

	// sort the Vectors
	int* ordering_i = NULL;
	if (!return_indices) {
		qsort(a->data,(size_t)size,sizeof(a->data[0]),cmp_Vector_T);
	} else {
		struct Vector_T_indexed** a_indexed = constructor_move_Vector_T_indexed(size,a->data); // destructed

		qsort(a_indexed,(size_t)size,sizeof(a_indexed[0]),cmp_Vector_T_indexed);

		ordering_i = malloc((size_t)size * sizeof *ordering_i); // keep
		for (ptrdiff_t i = 0; i < size; ++i)
			ordering_i[i] = (int)a_indexed[i]->index;

		reorder_Multiarray_Vector_T(a,ordering_i);

		destructor_Vector_T_indexed(a_indexed,size);
	}

	struct Vector_i* ordering = constructor_move_Vector_i_i(size,true,ordering_i); // returned

	return ordering;
}

struct Vector_T* collapse_Multiarray_Vector_T (const struct Multiarray_Vector_T*const src)
{
	const ptrdiff_t n_entries = compute_total_entries(src);

	Type*const data = malloc((size_t)n_entries * sizeof *data); // keep

	ptrdiff_t ind_d = 0;
	const ptrdiff_t size = compute_size(src->order,src->extents);
	for (ptrdiff_t i = 0; i < size; ++i) {
		struct Vector_T*const src_curr = src->data[i];
		const ptrdiff_t size_V = src_curr->ext_0;
		for (ptrdiff_t j = 0; j < size_V; ++j) {
			data[ind_d] = src_curr->data[j];
			++ind_d;
		}
	}

	struct Vector_T*const dest = calloc(1,sizeof *dest); // returned
	dest->ext_0     = n_entries;
	dest->owns_data = true;
	dest->data      = data;

	return dest;
}

void resize_Multiarray_T (struct Multiarray_T* a, const int order, const ptrdiff_t* extents)
{
#if 1
	assert(a->order == order);
#else
	a->extents = realloc(a->extents,order * sizeof *a->extents);
#endif
	a->order   = order;
	for (int i = 0; i < order; ++i)
		a->extents[i] = extents[i];

	a->data = realloc(a->data,(size_t)compute_size(order,extents) * sizeof *a->data);
}

const struct const_Vector_T* get_const_Multiarray_Vector_T
	(const struct const_Multiarray_Vector_T* src, const ptrdiff_t*const sub_indices)
{
	assert(src != NULL);
	return src->data[compute_index_sub_container(src->order,0,src->extents,sub_indices)];
}

struct const_Vector_T interpret_const_Multiarray_as_Vector_T (const struct const_Multiarray_T* a_Ma)
{
	assert(a_Ma->order == 1);
	struct const_Vector_T a =
		{ .ext_0     = a_Ma->extents[0],
		  .owns_data = false,
		  .data      = a_Ma->data, };
	return a;
}

struct Matrix_T interpret_Multiarray_as_Matrix_T (const struct Multiarray_T* a_Ma)
{
	assert(a_Ma->order == 2);
	struct Matrix_T a =
		{ .layout    = a_Ma->layout,
		  .ext_0     = a_Ma->extents[0],
		  .ext_1     = a_Ma->extents[1],
		  .owns_data = false,
		  .data      = a_Ma->data, };
	return a;
}

// Static functions ************************************************************************************************* //
// Level 0 ********************************************************************************************************** //

static struct Vector_T_indexed** constructor_move_Vector_T_indexed
	(const ptrdiff_t size, struct Vector_T** data)
{
	struct Vector_T_indexed** dest = malloc((size_t)size * sizeof *dest); // returned

	for (ptrdiff_t i = 0; i < size; i++) {
		dest[i] = calloc(1,sizeof *dest[i]); // keep
		dest[i]->index  = i;
		dest[i]->vector = data[i];
	}
	return dest;
}

static void destructor_Vector_T_indexed (struct Vector_T_indexed** src, const ptrdiff_t size)
{
	for (ptrdiff_t i = 0; i < size; ++i)
		free(src[i]);
	free(src);
}

static void reorder_Multiarray_Vector_T (struct Multiarray_Vector_T*const a, const int*const ordering)
{
	const ptrdiff_t size = compute_size(a->order,a->extents);

	struct Vector_T* b[size];
	for (ptrdiff_t i = 0; i < size; i++)
		b[i] = a->data[ordering[i]];

	for (ptrdiff_t i = 0; i < size; i++)
		a->data[i] = b[i];
}

static int cmp_Vector_T_indexed (const void *a, const void *b)
{
	const struct Vector_T_indexed*const*const ia = (const struct Vector_T_indexed*const*const) a,
	                             *const*const ib = (const struct Vector_T_indexed*const*const) b;

	const ptrdiff_t size_a = (*ia)->vector->ext_0,
	                size_b = (*ib)->vector->ext_0;

	if (size_a > size_b)
		return 1;
	else if (size_a < size_b)
		return -1;

	const Type*const data_a = (*ia)->vector->data,
	          *const data_b = (*ib)->vector->data;

	for (ptrdiff_t i = 0; i < size_a; ++i) {
#if TYPE_RC == TYPE_COMPLEX
		if (creal(data_a[i]) > creal(data_b[i]))
			return 1;
		else if (creal(data_a[i]) < creal(data_b[i]))
			return -1;
#else
		if (data_a[i] > data_b[i])
			return 1;
		else if (data_a[i] < data_b[i])
			return -1;
#endif
	}
	return 0;
}

static ptrdiff_t compute_total_entries (const struct Multiarray_Vector_T*const src)
{
	const ptrdiff_t size = compute_size(src->order,src->extents);

	ptrdiff_t count = 0;
	for (ptrdiff_t i = 0; i < size; ++i)
		count += src->data[i]->ext_0;

	return count;
}