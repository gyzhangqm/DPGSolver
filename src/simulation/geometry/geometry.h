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

#ifndef DPG__geometry_h__INCLUDED
#define DPG__geometry_h__INCLUDED
/** \file
 *  \brief Provides the interface to functions used for geometry processing.
 */

struct Simulation;
struct Intrusive_List;
struct Solver_Volume;
struct Solver_Face;
struct Multiarray_d;
struct const_Multiarray_d;

/** \brief Set up the solver geometry:
 *  - \ref Solver_Volume::metrics_vm;
 *  - \ref Solver_Volume::metrics_vc;
 *  - \ref Solver_Volume::jacobian_det_vc;
 *  - \todo [ref here] Solver_Face::normals_fc;
 *  - \todo [ref here] Solver_Face::jacobian_det_fc;
 *
 *  Requires that:
 *  - \ref Simulation::volumes points to a list of \ref Solver_Volume\*s;
 *  - \ref Simulation::faces   points to a list of \todo [ref here] Solver_Face\*s.
 */
void set_up_solver_geometry
	(struct Simulation* sim ///< \ref Simulation.
	);

/// \brief Compute the face unit normal vectors at the nodes corresponding to the given face metrics.
void compute_unit_normals
	(const int ind_lf,                             ///< Defined for \ref compute_unit_normals_and_det.
	 const struct const_Multiarray_d* normals_ref, ///< Defined for \ref compute_unit_normals_and_det.
	 const struct const_Multiarray_d* metrics_f,   ///< Defined for \ref compute_unit_normals_and_det.
	 struct Multiarray_d* normals_f                ///< Defined for \ref compute_unit_normals_and_det.
	);

/** \brief Compute the geometry of the \ref Solver_Volume.
 *
 *  The following members are set:
 *  - Solver_Volume::metrics_vm;
 *  - Solver_Volume::metrics_vc;
 *  - Solver_Volume::jacobian_det_vc.
 *
 *  Following the analysis of Kopriva \cite Kopriva2006, the metric terms are computed using the curl-form such that
 *  the free-stream preservation property may be recovered. The consistent symmetric-conservative (CSC) metric of Abe
 *  and Haga (section 5.3) is used for the implementation \cite Abe2015. The steps are repeated below to clarify the
 *  equivalence of the prodecure adopted here with their procedure:
 *  - (step 0-1) As the metric contributions computed in step 0 are computed in a basis of sufficient order to
 *    represent them exactly and are subsequently interpolated to the consistent grid points (CGPs), the metric
 *    contributions are here computed directly at the CGPs.
 *  	- Our terminology for the GPs is R_vg ((R)eference coordinates of the (v)olume (g)eometry nodes).
 *  	- Our terminology for the CGPs is R_vm ((R)eference coordinates of the (v)olume (m)etric nodes).
 *  	- We allow for flexibility in the order of the R_vm nodes such that superparametric geometry can be used on
 *  	  curved domain boundaries; Abe and Haga use an isoparametric partial metric representation **before** the
 *  	  differentiation is applied, resulting in a subparametric metric representation (see eq. (43) \cite Abe2015).
 *  - (step 2) The computed metric terms are interpolated to the solution points (SPs).
 *  	- As the flux reconstruction scheme is collocated (solution interpolation and cubature nodes are coincident),
 *  	  the interpolation to the SPs is equivalent to the interpolation to the cubature nodes. Thus, interpolation
 *  	  to the R_vc ((R)eference coordinates of the (v)olume (c)ubature) is then performed in the implementation
 *  	  here.
 *
 *  \todo Investigate requirement of superparametric geometry on curved surfaces and add comments. Potentially ok by
 *        using over-integration in curved elements.
 *
 *  Given the 3D geometry Jacobian ordering of
 *
 *  \f{eqnarray*}{
 *  	J  = \{ &\{x_r,x_s,x_t\}, &\\
 *  	        &\{y_r,y_s,y_t\}, &\\
 *  	        &\{z_r,z_s,z_t\}  &\},
 *  \f}
 *
 *  using the nonconservative metric (NC) for clarity of exposition (section 5.1 \cite Abe2015), the ordering of the
 *  metric terms is:
 *
 *  \f{eqnarray*}{
 *  	m  = \{ &\{ +(y_s z_t - y_t z_s), -(y_r z_t - y_t z_r), +(y_r z_s - y_s z_r) \}, &\\
 *  	        &\{ -(x_s z_t - x_t z_s), +(x_r z_t - x_t z_r), -(x_r z_s - x_s z_r) \}, &\\
 *  	        &\{ +(x_s y_t - x_t y_s), -(x_r y_t - x_t y_r), +(x_r y_s - x_s y_r) \}, &\}.
 *  \f}
 */
void compute_geometry_volume
	(struct Solver_Volume* s_vol, ///< \ref Solver_Volume.
	 const struct Simulation* sim ///< \ref Simulation.
	);

/** \brief Compute the geometry of the \ref Solver_Face.
 *
 *  The following members are set:
 *  - Solver_Face::xyz_fc;
 *  - Solver_Face::n_fc;
 *  - Solver_Face::jacobian_det_fc.
 */
void compute_geometry_face
	(struct Solver_Face* face, ///< \ref Solver_Face.
	 struct Simulation* sim    ///< \ref Simulation.
	);

#endif // DPG__geometry_h__INCLUDED
