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

#include "volume.h"
#include "definitions_elements.h"
#include "intrusive.h"

#include <stdlib.h>
#include <stdbool.h>

#include "macros.h"
#include "definitions_bc.h"
#include "definitions_core.h"
#include "definitions_intrusive.h"
#include "definitions_mesh.h"

#include "multiarray.h"
#include "matrix.h"
#include "vector.h"

#include "simulation.h"
#include "mesh.h"
#include "mesh_readers.h"
#include "mesh_connectivity.h"
#include "mesh_vertices.h"
#include "const_cast.h"
#include "face.h"
#include "element.h"
#include "geometry.h"

// Static function declarations ************************************************************************************* //

/// \brief Container for \ref Volume related mesh information.
struct Volume_mesh_info {
	int elem_type;                        ///< The type of \ref Element associated with the volume.
	const struct const_Vector_i* ve_inds; ///< The indices of the vertices of the volume.

	const struct const_Vector_i* to_lf;   ///< The relevant row of \ref Mesh_Connectivity::v_to_lf.
};

/** \brief Constructor for an individual \ref Volume.
 *  \return Standard. */
static struct Volume* constructor_Volume
	(const struct Simulation*const sim,          ///< \ref Simulation.
	 const struct Mesh*const mesh,               ///< \ref Mesh.
	 const struct Volume_mesh_info*const vol_mi, ///< \ref Volume_mesh_info.
	 const int index                             ///< The volume index.
	);

/// \brief Destructor for a \ref Volume in a \ref Intrusive_List, excluding the memory for the link itself.
static void destructor_Volume_link
	(struct Volume* volume ///< Standard.
	);

/** \brief Check for a match of boundaries relating to the two input vertex bc vectors.
 *
 *  The two input vectors of vertex boundary conditions are searched for a matching boundary condition possibly subject
 *  to the constraint of looking only at curved boundary conditions.
 *
 *  \return If a matching boundary condition is found for the two vertices, its value is returned; 0 otherwise. */
static int find_bc_match
	(const struct const_Vector_i*const ve_bc_0, ///< Vector of boundary conditions associated with the first vertex.
	 const struct const_Vector_i*const ve_bc_1, ///< Vector of boundary conditions associated with the second vertex.
	 const bool curved_only                     /**< Flag indicating whether only curved boundary conditions should
	                                             *   be considered. */
	);

// Interface functions ********************************************************************************************** //

struct Intrusive_List* constructor_Volumes (struct Simulation*const sim, const struct Mesh*const mesh)
{
	struct Intrusive_List* volumes = constructor_empty_IL(IL_VOLUME,NULL); // returned

	const struct const_Vector_i*const            elem_types = mesh->mesh_data->elem_types;
	const struct const_Multiarray_Vector_i*const node_nums  = mesh->mesh_data->node_nums;

	const struct const_Multiarray_Vector_i*const v_to_lf = mesh->mesh_conn->v_to_lf;

	const ptrdiff_t n_v = compute_size(v_to_lf->order,v_to_lf->extents);
	for (int v = 0; v < n_v; ++v) {
		const ptrdiff_t ind_v = v + mesh->mesh_data->ind_v;
		struct Volume_mesh_info vol_mi =
			{ .elem_type = elem_types->data[ind_v],
			  .ve_inds   = node_nums->data[ind_v],
			  .to_lf     = v_to_lf->data[v],
			};

		push_back_IL(volumes,(struct Intrusive_Link*) constructor_Volume(sim,mesh,&vol_mi,v));
	}
	sim->n_v = n_v;

	return volumes;
}

void destructor_Volumes (struct Intrusive_List* volumes)
{
	for (const struct Intrusive_Link* curr = volumes->first; curr; ) {
		struct Intrusive_Link* next = curr->next;
		destructor_Volume_link((struct Volume*) curr);
		curr = next;
	}
	destructor_IL(volumes,true);
}

bool check_ve_condition
	(const struct const_Multiarray_Vector_i*const f_ve, const struct const_Vector_i*const ve_inds,
	 const struct const_Vector_i*const ve_condition, const struct const_Multiarray_Vector_i*const ve_bc,
	 const bool curved_only)
{
	const int n_lf = (int)compute_size(f_ve->order,f_ve->extents);
	for (int lf = 0; lf < n_lf; ++lf) {
		const struct const_Vector_i*const f_ve_f = f_ve->data[lf];

		int count_ve_condition = 0;

		const struct const_Vector_i* ve_bc_V = NULL;

		const int n_ve_f = (int)f_ve_f->ext_0;
		for (int ve = 0; ve < n_ve_f; ++ve) {
			const int ind_ve = ve_inds->data[f_ve_f->data[ve]];
			if (ve_condition->data[ind_ve]) {
				if (count_ve_condition == 0)
					ve_bc_V = ve_bc->data[ind_ve];

				// Ensure that the the vertices are on the same curved surface
				if (find_bc_match(ve_bc_V,ve_bc->data[ind_ve],curved_only)) {
					if (++count_ve_condition == 2)
						return true;
				}
			}
		}
	}
	return false;
}

void update_volumes_element
	(struct Intrusive_List* volumes,             ///< The list of \ref Volume\*s to be updated.
	 const struct const_Intrusive_List* elements ///< The list of elements from which to replace the current ones.
	)
{
	for (struct Intrusive_Link* curr = volumes->first; curr; curr = curr->next) {
		struct Volume* volume = (struct Volume*) curr;

		const_cast_const_Element(&volume->element,get_element_by_type(elements,volume->element->type));
	}
}

struct Volume* constructor_copy_Volume
	(const struct Volume*const vol_i, const struct Simulation*const sim, const bool independent_elements)
{
	struct Volume* volume = calloc(1,sizeof *volume); // returned

	const_cast_i(&volume->index,vol_i->index);

	const_constructor_move_const_Multiarray_d
		(&volume->xyz_ve,constructor_copy_const_Multiarray_d(vol_i->xyz_ve)); // destructed

	for (int i = 0; i < NFMAX;    ++i) {
	for (int j = 0; j < NSUBFMAX; ++j) {
		const_cast_Face(&volume->faces[i][j],NULL);
	}}

	if (independent_elements)
		const_cast_const_Element(&volume->element,get_element_by_type(sim->elements,vol_i->element->type));
	else
		const_cast_const_Element(&volume->element,vol_i->element);

	volume->bc_faces = constructor_copy_const_Vector_i(vol_i->bc_faces); // destructed
	volume->bc_edges = constructor_copy_const_Vector_i(vol_i->bc_edges); // destructed

	const_cast_b(&volume->boundary,vol_i->boundary);
	const_cast_b(&volume->curved,vol_i->curved);

	return volume;
}

// Static functions ************************************************************************************************* //
// Level 0 ********************************************************************************************************** //

/** \brief Constructor for \ref Volume::bc_faces.
 *  \return See brief. */
static const struct const_Vector_i* constructor_bc_faces
	(const struct const_Element*const element, ///< The element associated with the volume.
	 const struct const_Vector_i*const to_lf   ///< Current volume component of \ref Mesh_Connectivity::v_to_lf.
	);

/** \brief Constructor for \ref Volume::bc_edges.
 *  \return See brief. */
static const struct const_Vector_i* constructor_bc_edges
	(const struct const_Element*const element,  ///< The element associated with the volume.
	 const struct const_Vector_i*const ve_inds, ///< The vertex indices for the volume.
	 const struct Mesh_Vertices*const mesh_vert ///< \ref Mesh_Vertices.
	);

/** \brief Check if the current volume is on a boundary.
 *
 *  This function works similarly to \ref check_if_curved_v.
 *
 *  \return `true` if on a boundary. */
static bool check_if_boundary_v
	(const struct const_Vector_i*const to_lf,           ///< Current volume component of \ref Mesh_Connectivity::v_to_lf.
	 const struct const_Multiarray_Vector_i*const f_ve, ///< Defined in \ref Element.
	 const struct const_Vector_i*const ve_inds,         ///< The vertex indices for the volume.
	 const struct Mesh_Vertices*const mesh_vert         ///< \ref Mesh_Vertices.
	);

/** \brief Check if the current volume is curved.
 *
 *  This function returns `true` if:
 *	1. The domain is mapped (and all elements are curved); or
 *	2. The volume contains an edge which has at least two vertices which were marked as curved.
 *
 *  It is not enough to simply check if any of the adjacent faces lie on a curved boundary as this does not account for
 *  3D volumes which only have a curved edge.
 *
 *  \return `true` if curved. */
static bool check_if_curved_v
	(const int domain_type,                             ///< Defined in \ref Simulation.
	 const struct const_Multiarray_Vector_i*const f_ve, ///< Defined in \ref Element.
	 const struct const_Vector_i*const ve_inds,         ///< The vertex indices for the volume.
	 const struct Mesh_Vertices*const mesh_vert         ///< \ref Mesh_Vertices.
	);

/** \brief Constructor for the xyz coordinates of the volume vertices.
 *  \return See brief. */
static struct Multiarray_d* constructor_volume_vertices
	(const struct const_Vector_i*const ve_inds, ///< The vertex indices.
	 const struct const_Matrix_d*const nodes    ///< \ref Mesh_Data::nodes.
	);

static struct Volume* constructor_Volume
	(const struct Simulation*const sim, const struct Mesh*const mesh,
	 const struct Volume_mesh_info*const vol_mi, const int index)
{
	const struct const_Matrix_d*const nodes = mesh->mesh_data->nodes;
	const struct Mesh_Vertices*const mesh_vert = mesh->mesh_vert;

	struct Volume* volume = calloc(1,sizeof *volume); // returned
	const_cast_i(&volume->index,index);

	const_constructor_move_Multiarray_d(&volume->xyz_ve,constructor_volume_vertices(vol_mi->ve_inds,nodes)); // dest.

	for (int i = 0; i < NFMAX;    ++i) {
	for (int j = 0; j < NSUBFMAX; ++j) {
		const_cast_Face(&volume->faces[i][j],NULL);
	}}

	const_cast_const_Element(&volume->element,get_element_by_type(sim->elements,vol_mi->elem_type));

	volume->bc_faces = constructor_bc_faces(volume->element,vol_mi->to_lf);
	volume->bc_edges = constructor_bc_edges(volume->element,vol_mi->ve_inds,mesh_vert);

/// \todo Change the check_if_boundary/check_if_curved functions to use bc_faces/bc_edges.
	const_cast_b(&volume->boundary,
	             check_if_boundary_v(vol_mi->to_lf,volume->element->f_ve,vol_mi->ve_inds,mesh_vert));
	const_cast_b(&volume->curved,
	             check_if_curved_v(sim->domain_type,volume->element->f_ve,vol_mi->ve_inds,mesh_vert));

	return volume;
}

static void destructor_Volume_link (struct Volume* volume)
{
	destructor_const_Multiarray_d(volume->xyz_ve);
	destructor_const_Vector_i(volume->bc_faces);
	destructor_const_Vector_i(volume->bc_edges);
}

static int find_bc_match
	(const struct const_Vector_i*const ve_bc_0, const struct const_Vector_i*const ve_bc_1, const bool curved_only)
{
	const ptrdiff_t i_max = ve_bc_0->ext_0;
	for (ptrdiff_t i = 0; i < i_max; ++i) {
		const int bc_0 = ve_bc_0->data[i];

		if (curved_only && bc_0 < 2*BC_STEP_SC)
			continue;

		if (find_val_Vector_i(ve_bc_1,bc_0,false))
			return bc_0;
	}
	return 0;
}

// Level 1 ********************************************************************************************************** //

static const struct const_Vector_i* constructor_bc_faces
	(const struct const_Element*const element, const struct const_Vector_i*const to_lf)
{
	struct Vector_i* bc_faces = constructor_empty_Vector_i(element->n_f); // returned
	set_to_value_Vector_i(bc_faces,BC_INVALID);

	// Check for faces on domain boundaries
	const ptrdiff_t i_max = to_lf->ext_0;
	for (ptrdiff_t i = 0; i < i_max; ++i) {
		if (to_lf->data[i] > BC_STEP_SC)
			bc_faces->data[i] = to_lf->data[i];
	}

	return (const struct const_Vector_i*) bc_faces;
}

static const struct const_Vector_i* constructor_bc_edges
	(const struct const_Element*const element, const struct const_Vector_i*const ve_inds,
	 const struct Mesh_Vertices*const mesh_vert)
{
	struct Vector_i* bc_edges = constructor_empty_Vector_i(element->n_e); // returned
	set_to_value_Vector_i(bc_edges,BC_INVALID);

	if (DIM != DMAX)
		return (const struct const_Vector_i*) bc_edges;

	const struct const_Vector_i*const ve_condition     = mesh_vert->ve_boundary;
	const struct const_Multiarray_Vector_i*const ve_bc = mesh_vert->ve_bc;

	const int n_le = element->n_e;
	for (int le = 0; le < n_le; ++le) {
		const struct const_Vector_i*const e_ve_e = element->e_ve->data[le];

		int count_ve_condition = 0;

		const struct const_Vector_i* ve_bc_V = NULL;

		const int n_ve_e = (int)e_ve_e->ext_0;
		for (int ve = 0; ve < n_ve_e; ++ve) {
			const int ind_ve = ve_inds->data[e_ve_e->data[ve]];
			if (ve_condition->data[ind_ve]) {
				if (count_ve_condition++ == 0) {
					ve_bc_V = ve_bc->data[ind_ve];
					continue;
				}

				// Ensure that the the vertices are on the same boundary
				const int bc = find_bc_match(ve_bc_V,ve_bc->data[ind_ve],false);
				if (bc)
					bc_edges->data[le] = bc;
			}
		}
	}
EXIT_ERROR("Not yet tested. Ensure that all is working as expected.\n");
	return (const struct const_Vector_i*) bc_edges;
}

static bool check_if_boundary_v
	(const struct const_Vector_i*const to_lf, const struct const_Multiarray_Vector_i*const f_ve,
	 const struct const_Vector_i*const ve_inds, const struct Mesh_Vertices*const mesh_vert)
{
	// If the volume has a face on a domain boundary
	const ptrdiff_t i_max = to_lf->ext_0;
	for (ptrdiff_t i = 0; i < i_max; ++i) {
		if (to_lf->data[i] > BC_STEP_SC)
			return true;
	}

	// If the volume has 2 vertices on a domain boundary (i.e. a boundary edge)
	return check_ve_condition(f_ve,ve_inds,mesh_vert->ve_boundary,mesh_vert->ve_bc,false);
}

static bool check_if_curved_v
	(const int domain_type, const struct const_Multiarray_Vector_i*const f_ve,
	 const struct const_Vector_i*const ve_inds, const struct Mesh_Vertices*const mesh_vert)
{
	if (domain_type == DOM_PARAMETRIC)
		return true;

	// If the volume has 2 vertices on a curved domain boundary (i.e. a curved boundary edge)
	return check_ve_condition(f_ve,ve_inds,mesh_vert->ve_curved,mesh_vert->ve_bc,true);
}

static struct Multiarray_d* constructor_volume_vertices
	(const struct const_Vector_i*const ve_inds, const struct const_Matrix_d*const nodes)
{
	struct Matrix_d* dest_M = constructor_empty_Matrix_d('R',ve_inds->ext_0,nodes->ext_1); // destructed

	const ptrdiff_t i_max = dest_M->ext_0;
	for (ptrdiff_t i = 0; i < i_max; ++i)
		set_row_Matrix_d(i,dest_M,get_row_const_Matrix_d(ve_inds->data[i],nodes));

	struct Multiarray_d* dest = constructor_move_Multiarray_d_Matrix_d(dest_M);
	destructor_Matrix_d(dest_M);

	return dest;
}
