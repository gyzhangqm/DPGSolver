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
 *  \brief Undefine macro definitions for c-style templating relating to geometry functions.
 */

#include "undef_templates_geometry_blended.h"
#include "undef_templates_geometry_normals.h"
#include "undef_templates_geometry_parametric.h"
#include "undef_templates_geometry_surface.h"

#undef constructor_xyz_fptr_T
#undef compute_geom_coef_fptr_T

#undef set_up_solver_geometry_T
#undef set_up_solver_geometry_p1_T
#undef compute_unit_normals_T
#undef compute_geometry_volume_T
#undef compute_geometry_face_T
#undef constructor_xyz_s_ho_T
#undef constructor_geom_coef_ho_T
#undef correct_for_exact_normals_T

#undef compute_geom_coef_fptr_T
#undef compute_normals_T
#undef set_fptr_geom_coef_T
#undef compute_geom_coef_p1
#undef set_jacobian_permutation
#undef compute_detJV_T
#undef compute_cofactors_T
#undef constructor_xyz_fc
#undef constructor_xyz_fc_on_exact_boundary
#undef compute_unit_normals_and_det_T
#undef compute_vol_jacobian_det_fc_T
#undef compute_geometry_volume_p1_T
#undef compute_geometry_face_p1_T
#undef compute_geom_coef_straight_T
#undef compute_geom_coef_blended_T
#undef compute_geom_coef_parametric_T
#undef correct_face_xyz_straight_T
#undef constructor_f_geom_coef_p1
#undef constructor_cc0_vgc_fgc_indices
#undef get_operator__cv0_vgc_fis
#undef get_operator__vc0_fis_fgc
#undef get_operator__cc0_vgc_fgc
#undef constructor_volume_metric_terms_T
