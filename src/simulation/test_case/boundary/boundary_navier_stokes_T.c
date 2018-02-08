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
 *  \brief Provides the templated Navier-Stokes boundary condition functions.
 *  \todo clean-up.
 */

#include <assert.h>
#include <math.h>
#include <float.h>

#include "macros.h"
#include "definitions_bc.h"
#include "definitions_core.h"
#include "definitions_physics.h"


#include "def_templates_boundary.h"

#include "def_templates_multiarray.h"
#include "def_templates_vector.h"

#include "def_templates_face_solver.h"

#include "def_templates_math_functions.h"
#include "def_templates_solution.h"

// Static function declarations ************************************************************************************* //

#define NEQ  NEQ_EULER  ///< Number of equations.
#define NVAR NVAR_EULER ///< Number of variables.

struct Exact_Boundary_Data;

/** \brief Pointer to functions computing the exact velocities on the boundary.
 *
 *  \param xyz     xyz-coordinates of the point.
 *  \param eb_data \ref Exact_Boundary_Data.
 */
typedef const Type* (*compute_uvw_ex_fptr)
	(const Real xyz[DIM],
	 const struct Exact_Boundary_Data*const eb_data
	);

/// \brief Container for data relating to the exact values on the domain boundary.
struct Exact_Boundary_Data {
	Real rho,  ///< Exact boundary density.
	     E,    ///< Exact boundary total 'e'nergy.
	     nfEv; ///< Exact boundary 'n'ormal 'v'iscous 'f'lux for the total 'e'nergy equation.

	compute_uvw_ex_fptr compute_uvw_ex; ///< Pointer to function computing the exact velocities on the boundary.
};

/** \brief Return the statically allocated \ref Exact_Boundary_Data container.
 *  \return See brief. */
static struct Exact_Boundary_Data get_Exact_Boundary_Data
	(const int viscous_bc_type,        ///< The type of viscous boundary condition.
	 const struct Simulation*const sim ///< \ref Simulation.
	);

/** \brief Version of \ref constructor_Boundary_Value_fptr_T computing members using the exact values for all variables
 *         and interpolated values for all gradients on a wall.
 *
 *  The boundary conditions are computed as:
 *  - s_b = 2*s_exact - s_i (such that 0.5*(s_b+s_i) == s_exact);
 *  - g_b = g_i_corrected;
 *  where 's' represents an arbitrary 's'olution variable, and 'g' represents an arbitrary 'g'radient variable. The
 *  corrected boundary gradients are specified such that the normal boundary viscous fluxes are identical when computed
 *  either as:
 *  1. n_dot_F = n (dot) f_viscous(s_l,g_l); or
 *  2. n_dof_F = n (dot) 0.5 * ( f_viscous(s_l,g_l) + f_viscous(s_b,g_b) ).
 *
 *  \warning This boundary condition **is not well-posed** as it has been proven that only NVAR-1 boundary conditions
 *           should be imposed for the Navier-Stokes equations on a no-slip wall (Table 2, Remark 11, \cite
 *           Nordstrom2005). Consequently, it should used only under special circumstances, such as for the
 *           Taylor-Couette case where an additional condition is required due to the domain being periodic.
 */
static void constructor_Boundary_Value_T_navier_stokes_no_slip_all_general
	(struct Boundary_Value_T*const bv,               ///< See brief.
	 const struct Boundary_Value_Input_T*const bv_i, ///< See brief.
	 const struct Solver_Face_T*const s_face,        ///< See brief.
	 const struct Simulation*const sim,              ///< See brief.
	 const struct Exact_Boundary_Data*const eb_data  ///< \ref Exact_Boundary_Data.
	);

// Interface functions ********************************************************************************************** //

void constructor_Boundary_Value_T_navier_stokes_no_slip_all_rotating
	(struct Boundary_Value_T* bv, const struct Boundary_Value_Input_T* bv_i, const struct Solver_Face_T* s_face,
	 const struct Simulation* sim)
{
// pointer to boundary velocity computing function.
// read/set data (V_tangential)
	struct Exact_Boundary_Data eb_data;
	EXIT_UNSUPPORTED;
	constructor_Boundary_Value_T_navier_stokes_no_slip_all_general(bv,bv_i,s_face,sim,&eb_data);
}

void constructor_Boundary_Value_T_navier_stokes_no_slip_flux_adiabatic
	(struct Boundary_Value_T* bv, const struct Boundary_Value_Input_T* bv_i, const struct Solver_Face_T* s_face,
	 const struct Simulation* sim)
{
	struct Exact_Boundary_Data eb_data = get_Exact_Boundary_Data(DIABATIC_FLUX_CONSTANT_ZERO,sim);
printf("%f\n",eb_data.nfEv);
UNUSED(eb_data);
	EXIT_ADD_SUPPORT; UNUSED(bv); UNUSED(bv_i); UNUSED(s_face); UNUSED(sim);
}

void constructor_Boundary_Value_T_navier_stokes_no_slip_flux_diabatic
	(struct Boundary_Value_T* bv, const struct Boundary_Value_Input_T* bv_i, const struct Solver_Face_T* s_face,
	 const struct Simulation* sim)
{
	struct Exact_Boundary_Data eb_data = get_Exact_Boundary_Data(DIABATIC_FLUX_CONSTANT,sim);
UNUSED(eb_data);
	EXIT_ADD_SUPPORT; UNUSED(bv); UNUSED(bv_i); UNUSED(s_face); UNUSED(sim);
}

// Static functions ************************************************************************************************* //
// Level 0 ********************************************************************************************************** //

/** \brief Read/set the required solution data of the \ref Exact_Boundary_Data container (assumes: zero velocity,
 *         diabatic boundary). */
static void read_and_set_data_diabatic_flux
	(const char*const input_path,              ///< Defined in \ref fopen_input.
	 struct Exact_Boundary_Data*const eb_data, ///< \ref Exact_Boundary_Data.
	 const bool is_adiabatic                   ///< Flag for whether the boundary is adiabatic.
	);

static struct Exact_Boundary_Data get_Exact_Boundary_Data (const int viscous_bc_type, const struct Simulation*const sim)
{
	static bool need_input = true;

	static struct Exact_Boundary_Data eb_data;
	if (need_input) {
		need_input = false;
		switch (viscous_bc_type) {
		case DIABATIC_FLUX_CONSTANT_ZERO:
			read_and_set_data_diabatic_flux(sim->input_path,&eb_data,true);
			break;
		case DIABATIC_FLUX_CONSTANT:
			read_and_set_data_diabatic_flux(sim->input_path,&eb_data,false);
			break;
		default:
			EXIT_ERROR("Unsupported: %d\n",viscous_bc_type);
			break;
		}
	}
	return eb_data;
}

static void constructor_Boundary_Value_T_navier_stokes_no_slip_all_general
	(struct Boundary_Value_T*const bv, const struct Boundary_Value_Input_T*const bv_i,
	 const struct Solver_Face_T*const s_face, const struct Simulation*const sim,
	 const struct Exact_Boundary_Data*const eb_data)
{
	EXIT_ADD_SUPPORT; UNUSED(bv); UNUSED(bv_i); UNUSED(s_face); UNUSED(sim);
	const bool*const c_m = bv_i->compute_member;
	assert(c_m[0] == true);

	const struct const_Multiarray_T* sol_l = bv_i->s;

	const Type*const rho_l  = get_col_const_Multiarray_T(0,sol_l),
	          *const E_l    = get_col_const_Multiarray_T(NVAR-1,sol_l),
	          *const rhou_l = get_col_const_Multiarray_T(1,sol_l);
	IF_DIM_GE_2( const Type*const rhov_l = get_col_const_Multiarray_T(2,sol_l) );
	IF_DIM_GE_3( const Type*const rhow_l = get_col_const_Multiarray_T(3,sol_l) );

	const ptrdiff_t n_n = sol_l->extents[0];

	const struct const_Multiarray_R* xyz = xyz = bv_i->xyz;

	assert(c_m[0] == true);
	struct Multiarray_T* sol = constructor_empty_Multiarray_T('C',2,(ptrdiff_t[]){n_n,NVAR}); // keep

	Type*const rho  = get_col_Multiarray_T(0,sol),
	    *const E    = get_col_Multiarray_T(NVAR-1,sol),
	    *const rhou = get_col_Multiarray_T(1,sol);
	IF_DIM_GE_2( Type*const rhov = get_col_Multiarray_T(2,sol) );
	IF_DIM_GE_3( Type*const rhow = get_col_Multiarray_T(3,sol) );

	const Type rho_ex = eb_data->rho,
	           E_ex   = eb_data->E;

	for (int n = 0; n < n_n; n++) {
		const Real xyz_n[] = ARRAY_DIM( get_col_const_Multiarray_R(0,xyz)[n],
		                                get_col_const_Multiarray_R(1,xyz)[n],
		                                get_col_const_Multiarray_R(2,xyz)[n] );
		const Type*const uvw_ex = eb_data->compute_uvw_ex(xyz_n,eb_data);

		rho[n]  = -rho_l[n]  + 2.0*rho_ex;
		IF_DIM_GE_1( rhou[n] = -rhou_l[n] + 2.0*rho[n]*uvw_ex[0] );
		IF_DIM_GE_2( rhov[n] = -rhov_l[n] + 2.0*rho[n]*uvw_ex[1] );
		IF_DIM_GE_3( rhow[n] = -rhow_l[n] + 2.0*rho[n]*uvw_ex[2] );
		E[n]    = -E_l[n]    + 2.0*E_ex;
	}
	bv->s = (struct const_Multiarray_T*)sol;

	if (c_m[1] == true) {
		struct Multiarray_T*const ds_ds = constructor_zero_Multiarray_T('C',3,(ptrdiff_t[]){n_n,NVAR,NVAR}); // keep

		for (int vr_b = 0; vr_b < NVAR; ++vr_b) {
		for (int vr_i = 0; vr_i < NVAR; ++vr_i) {
			if (!(vr_b == vr_i))
				continue;
			struct Vector_T ds_V = interpret_Multiarray_slice_as_Vector_T(ds_ds,(ptrdiff_t[]){vr_b,vr_i});
			set_to_value_Vector_T(&ds_V,-1.0);
		}}
		bv->ds_ds = (const struct const_Multiarray_T*) ds_ds;
	}

	assert(c_m[2] == true);
	if (bv_i->g)
		bv->g = constructor_copy_const_Multiarray_T(bv_i->g); // keep

	if (c_m[3] == true && bv->g) {
		struct Multiarray_T*const dg_dg =
			constructor_zero_Multiarray_T('C',5,(ptrdiff_t[]){n_n,NVAR,DIM,NVAR,DIM}); // keep

		for (int vr_b = 0; vr_b < NVAR; ++vr_b) {
		for (int d_b  = 0; d_b  < DIM;  ++d_b)  {
		for (int vr_i = 0; vr_i < NVAR; ++vr_i) {
		for (int d_i  = 0; d_i  < DIM;  ++d_i)  {
			if (!(vr_b == vr_i && d_b == d_i))
				continue;
			struct Vector_T dg_V = interpret_Multiarray_slice_as_Vector_T(dg_dg,(ptrdiff_t[]){vr_b,d_b,vr_i,d_i});
			set_to_value_Vector_T(&dg_V,1.0);
		}}}}
		bv->dg_dg = (const struct const_Multiarray_T*) dg_dg;
	}

	assert(c_m[3] == false);
	assert(c_m[4] == false);
	assert(c_m[5] == false);
}

// Level 1 ********************************************************************************************************** //

/// \brief Subset of read_and_set_data_diabatic_flux reading values to set \ref Exact_Boundary_Data::nfEv.
static void set_data_nfEv
	(const char*const input_path,              ///< See brief.
	 struct Exact_Boundary_Data*const eb_data, ///< See brief.
	 const bool is_adiabatic                   ///< See brief.
	);

/** \brief Version of \ref compute_uvw_ex_fptr imposing zero velocity.
 *  \return See brief. */
static const Type* compute_uvw_ex_zero
	(const Real xyz[DIM],
	 const struct Exact_Boundary_Data*const eb_data
	);

static void read_and_set_data_diabatic_flux
	(const char*const input_path, struct Exact_Boundary_Data*const eb_data, const bool is_adiabatic)
{
	const int count_to_find = 1;
	int count_found = 0;

	FILE* input_file = NULL;
	char line[STRLEN_MAX];

	int diabatic_flux_type = VISCOUS_BC_INVALID;

	input_file = fopen_input(input_path,'s',NULL); // closed
	while (fgets(line,sizeof(line),input_file)) {
		read_skip_convert_const_i(line,"diabatic_flux_type",&diabatic_flux_type,&count_found);
	}
	fclose(input_file);

	if (count_found != count_to_find)
		EXIT_ERROR("Did not find the required number of variables");

	switch (diabatic_flux_type) {
	case DIABATIC_FLUX_CONSTANT:
		set_data_nfEv(input_path,eb_data,is_adiabatic);
		break;
	default:
		EXIT_ERROR("Unsupported: %d\n",diabatic_flux_type);
		break;
	}

	eb_data->compute_uvw_ex = compute_uvw_ex_zero;
}

// Level 2 ********************************************************************************************************** //

static void set_data_nfEv
	(const char*const input_path, struct Exact_Boundary_Data*const eb_data, const bool is_adiabatic)
{
	const int count_to_find = 5;
	int count_found = 0;

	char line[STRLEN_MAX];

	int viscosity_type = VISCOSITY_INVALID;
	Real Pr   = DBL_MAX,
	     r_s  = DBL_MAX,
	     mu   = DBL_MAX,
	     dTdn = DBL_MAX;

	FILE* input_file = fopen_input(input_path,'s',NULL); // closed
	while (fgets(line,sizeof(line),input_file)) {
		read_skip_convert_i(line,"viscosity_type",&viscosity_type,&count_found);

		read_skip_string_count_d("Pr",  &count_found,line,&Pr);
		read_skip_string_count_d("r_s", &count_found,line,&r_s);
		read_skip_string_count_d("mu",  &count_found,line,&mu);
		read_skip_string_count_d("dTdn",&count_found,line,&dTdn);
	}
	fclose(input_file);

	if (count_found != count_to_find)
		EXIT_ERROR("Did not find the required number of variables");

	assert(viscosity_type == VISCOSITY_CONSTANT); // Otherwise constant dTdn still results in varying nfEv.

	const Real Cp = compute_cp_ideal_gas(r_s);
	eb_data->nfEv = -mu*Cp/Pr*dTdn;

	assert(!is_adiabatic || eb_data->nfEv == 0.0);
}

static const Type* compute_uvw_ex_zero (const Real xyz[DIM], const struct Exact_Boundary_Data*const eb_data)
{
	UNUSED(xyz); UNUSED(eb_data);
	static const Type uvw[DIM] = {0,};
	return uvw;
}