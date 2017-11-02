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

#include "complex_multiarray.h"

#include <assert.h>

#include "macros.h"

#include "multiarray.h"

// Static function declarations ************************************************************************************* //

// Interface functions ********************************************************************************************** //

void set_Multiarray_c_Multiarray_d (struct Multiarray_c* a, const struct const_Multiarray_d* b)
{
	const ptrdiff_t size = compute_size(a->order,a->extents);
	assert(size == compute_size(b->order,b->extents));

	for (int i = 0; i < size; ++i)
		a->data[i] = b->data[i];
}

// Static functions ************************************************************************************************* //
// Level 0 ********************************************************************************************************** //
