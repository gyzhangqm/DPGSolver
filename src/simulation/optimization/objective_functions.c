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


// Start with this file for the optimization objective function to test
// functionals out. This must be eventually templated

#include "objective_functions.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "face_solver.h"
#include "volume.h"
#include "volume_solver.h"

#include "macros.h"

#include "multiarray.h"
#include "vector.h"

#include "intrusive.h"
#include "simulation.h"
#include "file_processing.h"
#include "operator.h"
#include "compute_face_rlhs.h"
#include "solution_euler.h"

#include "definitions_physics.h"
#include "definitions_math.h"

// Static function declarations ************************************************************************************* //

/// \brief Container for solution data relating to 'c'oefficients of 'd'rag and 'l'ift.
struct Sol_Data__c_dl {
	// Read parameters
	double rho,      ///< The free stream density.
	       p,        ///< The free stream pressure.
	       mach,     ///< The free stream 'mach' number.
	       theta,    ///< The free stream flow angle in the xy-plane (in radians).
	       area_ref; ///< The reference area used to scale the coefficients.
};

/** \brief Return the statically allocated \ref Sol_Data__c_dl container.
 *  \return See brief. */
static struct Sol_Data__c_dl get_sol_data__c_dl
	( );

static void read_data_c_dl (struct Sol_Data__c_dl*const sol_data);

/** \brief Constructor for the solution interpolating from the neighbouring volume to the face cubature nodes.
 *  \return See brief. */
static const struct const_Multiarray_d* constructor_s_fc_interp
	(const struct Solver_Face* face, ///< Defined for \ref constructor_Boundary_Value_Input_face_s_fcl_interp_T.
	 const struct Simulation* sim,     ///< Defined for \ref constructor_Boundary_Value_Input_face_s_fcl_interp_T.
	 const int side_index              ///< The index of the side of the face under consideration.
	);


// Interface functions ********************************************************************************************** //



const struct const_Multiarray_d* compute_Cl_Cd(const struct Simulation* sim){

	/*
	Compute the lift and drag coefficient of the airfoil (2D)

	Inputs:
		sim = The simulation data structure

	Return:
		Multiarray with the values of the lift coefficients. Returned
		multiarray is in column major form and stores the data in the
		form [Cl, Cd].
	*/

	int i, num_fc;

	// Get the freestream flow data and reference area
	const struct Sol_Data__c_dl sol_data = get_sol_data__c_dl();

	const double rho_fs   = sol_data.rho,
	             p_fs     = sol_data.p,
	             c_fs     = sqrt(GAMMA*p_fs/rho_fs),
	             V_fs     = sol_data.mach*c_fs,
	             denom    = 0.5*rho_fs*V_fs*V_fs*sol_data.area_ref,
	             theta_fs = sol_data.theta;

	// The force components along the Normal and Axial directions
	double	Force_N = 0.0, 
			Force_A = 0.0;

	for (struct Intrusive_Link* curr = sim->faces->first; curr; curr = curr->next) {
		// Loop through the faces that are on the wall boundary
		const struct Face*const face = (struct Face*) curr;
		const struct Solver_Face*const s_face = (struct Solver_Face*) curr;
		if (!is_face_wall_boundary(face))
			continue;

		// Get the solution (s) at the face cubature (fc) nodes and convert it to 
		// primitive variables (pv)
		const struct const_Multiarray_d* pv_fc = constructor_s_fc_interp(s_face,sim,0);
		convert_variables((struct Multiarray_d*)pv_fc,'c','p');
		
		// Get the normals (n) at the face cubature (fc) nodes
		const struct const_Multiarray_d* n_fc = s_face->normals_fc;

		num_fc = (int) pv_fc->extents[0]; // number of face cubature (fc) nodes

		// Pressure (p) at face cubature (fc) nodes as an array
		const double *p_fc_i = get_col_const_Multiarray_d(pv_fc->extents[1]-1,pv_fc);

		// Integrate the pressure multiplied by the negative of the normal component.
		// Compute the integral using quadrature
		const struct const_Vector_d*const w_fc = get_operator__w_fc__s_e(s_face);

		struct Multiarray_d *aero_coeffs_alt = constructor_empty_Multiarray_d('C',2,(ptrdiff_t[]){num_fc, 2});
	
		compute_cd_cl_values(aero_coeffs_alt, pv_fc, 'p', n_fc);


		for (i = 0; i < num_fc; i++){
			const double*const n = get_row_const_Multiarray_d(i,n_fc);

			Force_A += -1.0*p_fc_i[i]*n[0] *  w_fc->data[i];
			Force_N += -1.0*p_fc_i[i]*n[1] *  w_fc->data[i];

		}

		// Destroy the allocated vectors
		destructor_const_Multiarray_d(pv_fc);

	}

	// Get the lift and drag by taking into account the angle of attack 
	double 	L = Force_N*cos(theta_fs) - Force_A*sin(theta_fs),
			D = Force_N*sin(theta_fs) + Force_A*cos(theta_fs);

	double 	Cl = L/denom,
			Cd = D/denom;

 	struct Multiarray_d *aero_coeffs = constructor_empty_Multiarray_d('C',2,(ptrdiff_t[]){2, 1}); // destructed
	get_col_Multiarray_d(0, aero_coeffs)[0] = Cl;
	get_col_Multiarray_d(0, aero_coeffs)[1] = Cd;

	return (const struct const_Multiarray_d*) aero_coeffs;
	
}

// Static functions ************************************************************************************************* //

static struct Sol_Data__c_dl get_sol_data__c_dl ( )
{
	static bool need_input = true;

	static struct Sol_Data__c_dl sol_data;
	if (need_input) {
		need_input = false;
		read_data_c_dl(&sol_data);
	}
	return sol_data;
}

static void read_data_c_dl (struct Sol_Data__c_dl*const sol_data)
{
	const int count_to_find = 5;
	int count_found = 0;
	char line[STRLEN_MAX];
	FILE* input_file = NULL;

	input_file = fopen_input('s',NULL,NULL); // closed
	while (fgets(line,sizeof(line),input_file)) {
		read_skip_string_count_d("density",  &count_found,line,&sol_data->rho);
		read_skip_string_count_d("pressure", &count_found,line,&sol_data->p);
		read_skip_string_count_d("mach",     &count_found,line,&sol_data->mach);
		read_skip_string_count_d("theta_deg",&count_found,line,&sol_data->theta);
	}
	fclose(input_file);

	sol_data->theta *= PI/180.0;

	input_file = fopen_input('g',NULL,NULL); // closed
	while (fgets(line,sizeof(line),input_file)) {
		read_skip_string_count_c_style_d("area_ref",&count_found,line,&sol_data->area_ref);
	}
	fclose(input_file);

	if (count_found != count_to_find)
		EXIT_ERROR("Did not find the required number of variables");
}

static const struct const_Multiarray_d* constructor_s_fc_interp
	(const struct Solver_Face* s_face, const struct Simulation* sim, const int side_index)
{
	const struct Operator* cv0_vs_fc = get_operator__cv0_vs_fc(side_index,s_face);

	// sim may be used to store a parameter establishing which type of operator to use for the computation.
	UNUSED(sim);
	const char op_format = 'd';

	struct Solver_Volume* s_volume = (struct Solver_Volume*) ((struct Face*)s_face)->neigh_info[side_index].volume;

	const struct const_Multiarray_d* s_coef = (const struct const_Multiarray_d*) s_volume->sol_coef;

	return constructor_mm_NN1_Operator_const_Multiarray_d(cv0_vs_fc,s_coef,'C',op_format,s_coef->order,NULL);
}

