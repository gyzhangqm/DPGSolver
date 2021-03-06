// Copyright 2017 Philip Zwanenburg
// MIT License (https://github.com/PhilipZwanenburg/DPGSolver/blob/master/LICENSE)
/**	\file
 *	\brief Reads a mesh file and returns relevant data as part of

 \todo Complete this.
 */

#include "mesh.h"
#include "intrusive.h"
#include "mesh_readers.h"
#include "mesh_connectivity.h"

#include <limits.h>
#include <string.h>

#include "Macros.h"
#include "element.h"

#include "file_processing.h"

// Static function declarations ************************************************************************************* //

/** \brief Constructor for a \ref Mesh.
 *	\return Standard. */
struct Mesh* constructor_Mesh ();

// Interface functions ********************************************************************************************** //

struct Mesh* set_up_mesh (const struct Mesh_Input* mesh_input, const struct const_Intrusive_List* elements)
{
	struct Mesh* mesh = constructor_Mesh();

	*(struct Mesh_Data**)&         mesh->mesh_data = mesh_reader(mesh_input->mesh_name_full,mesh_input->d);
	*(struct Mesh_Connectivity**)& mesh->mesh_conn = mesh_connect(mesh->mesh_data,elements);
	*(struct Mesh_Vertices**)&     mesh->mesh_vert = mesh_process_vertices(mesh,elements,mesh_input);

	return mesh;
}

void destructor_Mesh (struct Mesh* mesh)
{
	destructor_Mesh_Data((struct Mesh_Data*)mesh->mesh_data);
	destructor_Mesh_Connectivity((struct Mesh_Connectivity*)mesh->mesh_conn);
	destructor_Mesh_Vertices((struct Mesh_Vertices*)mesh->mesh_vert);
	free(mesh);
}

ptrdiff_t get_first_volume_index (const struct const_Vector_i*const elem_per_dim, const int d)
{
	ptrdiff_t ind = 0;
	for (int dim = 0; dim < d; dim++)
		ind += elem_per_dim->data[dim];
	return ind;
}

// Static functions ************************************************************************************************* //
// Level 0 ********************************************************************************************************** //

struct Mesh* constructor_Mesh ()
{
	struct Mesh* mesh = malloc(1 * sizeof *mesh); // returned
	return mesh;
}
