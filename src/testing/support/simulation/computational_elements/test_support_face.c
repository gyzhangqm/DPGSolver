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
/**	\file
 */

#include "test_support_face.h"

#include "test_support_multiarray.h"
#include "test_support_volume.h"

#include <stdlib.h>
#include <stdio.h>

#include "macros.h"

#include "multiarray.h"

#include "face.h"
#include "face_solver.h"
#include "element.h"

#include "file_processing.h"
#include "intrusive.h"

// Static function declarations ************************************************************************************* //

// Interface functions ********************************************************************************************** //

struct Face* constructor_Face_file
	(FILE* file, char* line, const struct const_Intrusive_List*const elements,
	 const struct Intrusive_List*const volumes)
{
	struct Face* face = calloc(1,sizeof *face); // returned

	if (volumes != NULL) {
		read_skip_const_i_1(line,1,&face->index,1);
		read_skip_file_const_b("boundary",file,&face->boundary);
		read_skip_file_const_b("curved",file,&face->curved);
		read_skip_file_const_i("bc",file,&face->bc);

		int elem_type;
		read_skip_file_i("elem_type",file,&elem_type);
		const_cast_const_Element(&face->element,get_element_by_type(elements,elem_type));

		for (int n = 0; n < 2; ++n) {
			skip_lines(file,1);

			read_skip_file_i("ind_lf",file,&face->neigh_info[n].ind_lf);
			read_skip_file_i("ind_href",file,&face->neigh_info[n].ind_href);
			read_skip_file_i("ind_sref",file,&face->neigh_info[n].ind_sref);
			read_skip_file_i("ind_ord",file,&face->neigh_info[n].ind_ord);

			int ind_v = -1;
			read_skip_file_i("ind_v",file,&ind_v);
			if (ind_v >= 0)
				face->neigh_info[n].volume = get_volume_by_index(volumes,ind_v);
			else
				face->neigh_info[n].volume = NULL;
		}
	}

	return face;
}

void constructor_file_Solver_Face (FILE* file, char* line, struct Face* face_ptr)
{
	UNUSED(line);

	struct Solver_Face* face = (struct Solver_Face*) face_ptr;

	skip_lines(file,1);
	struct Multiarray_d* normals = constructor_file_Multiarray_d(file,true); // keep

	destructor_const_Multiarray_d(face->normals_fc);
	const_constructor_move_Multiarray_d(&face->normals_fc,normals);
}

// Static functions ************************************************************************************************* //
// Level 0 ********************************************************************************************************** //

