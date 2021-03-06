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
 *  \brief Provides the templated euler numerical flux functions.
 *  \todo Clean-up.
 */

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include "gsl/gsl_math.h"

#include "macros.h"
#include "definitions_bc.h"
#include "definitions_core.h"
#include "definitions_physics.h"


#include "def_templates_numerical_flux.h"

#include "def_templates_multiarray.h"

#include "def_templates_boundary.h"
#include "def_templates_const_cast_d.h"
#include "def_templates_flux.h"
#include "def_templates_math_functions.h"

// Static function declarations ************************************************************************************* //

#define NEQ  NEQ_EULER  ///< Number of equations.
#define NVAR NVAR_EULER ///< Number of variables.
#define NVR  NVAR_EULER ///< Number of variables.

/** \brief Compute the Jacobian of the maximum velocity used in the Lax-Friedrichs scheme wrt the solution variables.
 *  \return See brief. */
static const Type* compute_dmaxV_ds
	(const Type rho_inv,  ///< Inverse density.
	 const Type p,        ///< Pressure.
	 const Type V2,       ///< Velocity magnitude squared.
	 const Type V,        ///< Velocity magnitude.
	 const Type c,        ///< Sound speed.
	 const Type*const uvw ///< Velocity components.
		);

// Interface functions ********************************************************************************************** //

void compute_Numerical_Flux_T_euler_lax_friedrichs
	(const struct Numerical_Flux_Input_T* num_flux_i, struct mutable_Numerical_Flux_T* num_flux)
{
	const ptrdiff_t NnTotal = num_flux_i->bv_l.s->extents[0];

	Type const *const nL = num_flux_i->bv_l.normals->data;

	Type const *const WL = num_flux_i->bv_l.s->data,
	           *const WR = num_flux_i->bv_r.s->data;

	Type       *const nFluxNum = num_flux->nnf->data;

	Type const *rhoL_ptr  = &WL[NnTotal*0],
	           *rhouL_ptr = &WL[NnTotal*1],
	           *EL_ptr    = &WL[NnTotal*(DIM+1)],

	           *rhoR_ptr  = &WR[NnTotal*0],
	           *rhouR_ptr = &WR[NnTotal*1],
	           *ER_ptr    = &WR[NnTotal*(DIM+1)];

	Type const *n_ptr = nL;

	struct Flux_Input_T* flux_i = malloc(sizeof *flux_i); // free
	flux_i->compute_member = num_flux_i->flux_i->compute_member;
	flux_i->s = constructor_move_const_Multiarray_T_T('C',2,(ptrdiff_t[]){1,NVR},false,NULL); // destructed

	struct mutable_Flux_T flux;
	flux.f = constructor_move_Multiarray_T_T('C',3,(ptrdiff_t[]){1,DIM,NEQ},false,NULL); // destructed

	Type *nF_ptr[NEQ];
	for (int eq = 0; eq < NEQ; eq++)
		nF_ptr[eq] = &nFluxNum[eq*NnTotal];

	if (DIM == 3) {
		Type const *rhovL_ptr = &WL[NnTotal*2],
		           *rhowL_ptr = &WL[NnTotal*3],

		           *rhovR_ptr = &WR[NnTotal*2],
		           *rhowR_ptr = &WR[NnTotal*3];

		for (ptrdiff_t n = 0; n < NnTotal; n++) {
			// Left VOLUME
			Type const rhoL  = *rhoL_ptr++,
			           rhouL = *rhouL_ptr++,
			           rhovL = *rhovL_ptr++,
			           rhowL = *rhowL_ptr++,
			           EL    = *EL_ptr++,

			           rhoL_inv = 1.0/rhoL,
			           uL = rhouL*rhoL_inv,
			           vL = rhovL*rhoL_inv,
			           wL = rhowL*rhoL_inv,

			           V2L = uL*uL+vL*vL+wL*wL,
			           VL  = sqrt_T(V2L),

			           pL  = GM1*(EL-0.5*rhoL*V2L),
			           cL  = sqrt_T(GAMMA*pL/rhoL);

			// Right VOLUME
			Type const rhoR  = *rhoR_ptr++,
			           rhouR = *rhouR_ptr++,
			           rhovR = *rhovR_ptr++,
			           rhowR = *rhowR_ptr++,
			           ER    = *ER_ptr++,

			           rhoR_inv = 1.0/rhoR,
			           uR = rhouR*rhoR_inv,
			           vR = rhovR*rhoR_inv,
			           wR = rhowR*rhoR_inv,

			           V2R = uR*uR+vR*vR+wR*wR,
			           VR  = sqrt_T(V2R),

			           pR  = GM1*(ER-0.5*rhoR*V2R),
			           cR  = sqrt_T(GAMMA*pR/rhoR);

			Type const maxlL = VL+cL,
			           maxlR = VR+cR;

			Type maxV;
			if (real_T(maxlL) > real_T(maxlR))
				maxV = maxlL;
			else
				maxV = maxlR;

			Type const n1 = *n_ptr++,
			           n2 = *n_ptr++,
			           n3 = *n_ptr++;

			Type WLn[] = {rhoL, rhouL, rhovL, rhowL, EL};
			Type FLn[NEQ*DIM] = { 0.0 };
			const_cast_T1(&flux_i->s->data,WLn);
			flux.f->data   = FLn;
			compute_Flux_T_euler(flux_i,&flux);

			Type const *FL1_ptr = FLn,
			           *FL2_ptr = FL1_ptr+1,
			           *FL3_ptr = FL2_ptr+1;

			Type WRn[] = {rhoR, rhouR, rhovR, rhowR, ER};
			Type FRn[NEQ*DIM] = { 0.0 };
			const_cast_T1(&flux_i->s->data,WRn);
			flux.f->data   = FRn;
			compute_Flux_T_euler(flux_i,&flux);

			Type const *FR1_ptr = FRn,
			           *FR2_ptr = FR1_ptr+1,
			           *FR3_ptr = FR2_ptr+1;

			for (int eq = 0; eq < NEQ; eq++) {
				*nF_ptr[eq]++ = 0.5*( n1*((*FL1_ptr)+(*FR1_ptr))+n2*((*FL2_ptr)+(*FR2_ptr))+n3*((*FL3_ptr)+(*FR3_ptr))
				                     + maxV*(WLn[eq]-WRn[eq]));

				FL1_ptr += DIM;
				FL2_ptr += DIM;
				FL3_ptr += DIM;
				FR1_ptr += DIM;
				FR2_ptr += DIM;
				FR3_ptr += DIM;
			}
		}
	} else if (DIM == 2) {
		Type const *rhovL_ptr = &WL[NnTotal*2],

		           *rhovR_ptr = &WR[NnTotal*2];

		for (ptrdiff_t n = 0; n < NnTotal; n++) {
			// Left VOLUME
			Type const rhoL  = *rhoL_ptr++,
			           rhouL = *rhouL_ptr++,
			           rhovL = *rhovL_ptr++,
			           EL    = *EL_ptr++,

			           rhoL_inv = 1.0/rhoL,
			           uL = rhouL*rhoL_inv,
			           vL = rhovL*rhoL_inv,

			           V2L = uL*uL+vL*vL,
			           VL  = sqrt_T(V2L),

			           pL  = GM1*(EL-0.5*rhoL*V2L),
			           cL  = sqrt_T(GAMMA*pL/rhoL);

			// Right VOLUME
			Type const rhoR  = *rhoR_ptr++,
			           rhouR = *rhouR_ptr++,
			           rhovR = *rhovR_ptr++,
			           ER    = *ER_ptr++,

			           rhoR_inv = 1.0/rhoR,
			           uR = rhouR*rhoR_inv,
			           vR = rhovR*rhoR_inv,

			           V2R = uR*uR+vR*vR,
			           VR  = sqrt_T(V2R),

			           pR  = GM1*(ER-0.5*rhoR*V2R),
			           cR  = sqrt_T(GAMMA*pR/rhoR);

			Type const maxlL = VL+cL,
			           maxlR = VR+cR;

			Type maxV;
			if (real_T(maxlL) > real_T(maxlR))
				maxV = maxlL;
			else
				maxV = maxlR;

			Type const n1 = *n_ptr++,
			           n2 = *n_ptr++;

			Type WLn[] = {rhoL, rhouL, rhovL, EL};
			Type FLn[NEQ*DIM] = { 0.0 };
			const_cast_T1(&flux_i->s->data,WLn);
			flux.f->data   = FLn;
			compute_Flux_T_euler(flux_i,&flux);

			Type const *FL1_ptr = FLn,
			           *FL2_ptr = FL1_ptr+1;

			Type WRn[] = {rhoR, rhouR, rhovR, ER};
			Type FRn[NEQ*DIM] = { 0.0 };
			const_cast_T1(&flux_i->s->data,WRn);
			flux.f->data   = FRn;
			compute_Flux_T_euler(flux_i,&flux);

			Type const *FR1_ptr = FRn,
			           *FR2_ptr = FR1_ptr+1;

			for (int eq = 0; eq < NEQ; eq++) {
				*nF_ptr[eq]++ = 0.5*( n1*((*FL1_ptr)+(*FR1_ptr))+n2*((*FL2_ptr)+(*FR2_ptr)) + maxV*(WLn[eq]-WRn[eq]));

				FL1_ptr += DIM;
				FL2_ptr += DIM;
				FR1_ptr += DIM;
				FR2_ptr += DIM;
			}
		}
	} else if (DIM == 1) {
		for (ptrdiff_t n = 0; n < NnTotal; n++) {
			// Left VOLUME
			Type const rhoL  = *rhoL_ptr++,
			           rhouL = *rhouL_ptr++,
			           EL    = *EL_ptr++,

			           rhoL_inv = 1.0/rhoL,
			           uL = rhouL*rhoL_inv,

			           V2L = uL*uL,
			           VL  = sqrt_T(V2L),

			           pL  = GM1*(EL-0.5*rhoL*V2L),
			           cL  = sqrt_T(GAMMA*pL/rhoL);

			// Right VOLUME
			Type const rhoR  = *rhoR_ptr++,
			           rhouR = *rhouR_ptr++,
			           ER    = *ER_ptr++,

			           rhoR_inv = 1.0/rhoR,
			           uR = rhouR*rhoR_inv,

			           V2R = uR*uR,
			           VR  = sqrt_T(V2R),

			           pR  = GM1*(ER-0.5*rhoR*V2R),
			           cR  = sqrt_T(GAMMA*pR/rhoR);

			Type const maxlL = VL+cL,
			           maxlR = VR+cR;

			Type maxV;
			if (real_T(maxlL) > real_T(maxlR))
				maxV = maxlL;
			else
				maxV = maxlR;

			Type const n1 = *n_ptr++;

			Type WLn[] = {rhoL, rhouL, EL};
			Type FLn[NEQ*DIM] = { 0.0 };
			const_cast_T1(&flux_i->s->data,WLn);
			flux.f->data   = FLn;
			compute_Flux_T_euler(flux_i,&flux);

			Type const *FL1_ptr = FLn;

			Type WRn[] = {rhoR, rhouR, ER};
			Type FRn[NEQ*DIM] = { 0.0 };
			const_cast_T1(&flux_i->s->data,WRn);
			flux.f->data   = FRn;
			compute_Flux_T_euler(flux_i,&flux);

			Type const *FR1_ptr = FRn;

			for (int eq = 0; eq < NEQ; eq++) {
				*nF_ptr[eq]++ = 0.5*(n1*((*FL1_ptr)+(*FR1_ptr)) + maxV*(WLn[eq]-WRn[eq]));

				FL1_ptr += DIM;
				FR1_ptr += DIM;
			}
		}
	}
	destructor_const_Multiarray_T(flux_i->s);
	destructor_Multiarray_T(flux.f);
	free(flux_i);
}

void compute_Numerical_Flux_T_euler_lax_friedrichs_jacobian
	(const struct Numerical_Flux_Input_T* num_flux_i, struct mutable_Numerical_Flux_T* num_flux)
{
	const struct const_Multiarray_T*const sL = num_flux_i->bv_l.s;
	const Type*const rhoL_p      = get_col_const_Multiarray_T(0,sL),
	          *const rhouvwL_p[] = ARRAY_DIM(get_col_const_Multiarray_T(1,sL),
	                                         get_col_const_Multiarray_T(2,sL),
	                                         get_col_const_Multiarray_T(3,sL)),
	          *const EL_p        = get_col_const_Multiarray_T(NVAR-1,sL);

	const struct const_Multiarray_T*const sR = num_flux_i->bv_r.s;
	const Type*const rhoR_p      = get_col_const_Multiarray_T(0,sR),
	          *const rhouvwR_p[] = ARRAY_DIM(get_col_const_Multiarray_T(1,sR),
	                                         get_col_const_Multiarray_T(2,sR),
	                                         get_col_const_Multiarray_T(3,sR)),
	          *const ER_p        = get_col_const_Multiarray_T(NVAR-1,sR);

	const bool*const c_m = num_flux_i->flux_i->compute_member;

	assert(c_m[0]);
	Type* nnf[NEQ] = { NULL };
	for (int eq = 0; eq < NEQ; eq++)
		nnf[eq] = get_col_Multiarray_T(eq,num_flux->nnf);

	assert(c_m[1]);
	Type* dnnf_dsL[NEQ*NVR] = { NULL };
	Type* dnnf_dsR[NEQ*NVR] = { NULL };
	for (int vr = 0; vr < NVR; ++vr) {
	for (int eq = 0; eq < NEQ; ++eq) {
		const int ind = eq+NEQ*(vr);
		dnnf_dsL[ind] = get_col_Multiarray_T(ind,num_flux->neigh_info[0].dnnf_ds);
		dnnf_dsR[ind] = get_col_Multiarray_T(ind,num_flux->neigh_info[1].dnnf_ds);
	}}

	assert(!c_m[3]); // Add support for Hessian terms if desired.

	struct Flux_Input_T flux_i = { .compute_member = c_m, };
	flux_i.s = constructor_move_const_Multiarray_T_T('C',2,(ptrdiff_t[]){1,NVR},false,NULL); // destructed

	struct mutable_Flux_T flux;
	flux.f     = constructor_move_Multiarray_T_T('C',3,(ptrdiff_t[]){1,DIM,NEQ},false,NULL);      // destructed
	flux.df_ds = constructor_move_Multiarray_T_T('C',4,(ptrdiff_t[]){1,DIM,NEQ,NVAR},false,NULL); // destructed

	const struct const_Multiarray_T*const nL_p = num_flux_i->bv_l.normals;

	const ptrdiff_t n_n = sL->extents[0];
	for (ptrdiff_t n = 0; n < n_n; ++n) {
		const Type rhoL      = rhoL_p[n],
		           rhouvwL[] = ARRAY_DIM(rhouvwL_p[0][n],rhouvwL_p[1][n],rhouvwL_p[2][n]),
		           EL        = EL_p[n],
		           rhoL_inv  = 1.0/rhoL,
		           uvwL[]    = ARRAY_DIM(rhoL_inv*rhouvwL[0],rhoL_inv*rhouvwL[1],rhoL_inv*rhouvwL[2]),
		           V2L       = compute_V2_from_uvw_T(uvwL),
		           VL        = sqrt_T(V2L),
		           pL        = GM1*(EL-0.5*rhoL*V2L),
		           cL        = sqrt_T(GAMMA*pL/rhoL);

		const Type rhoR      = rhoR_p[n],
		           rhouvwR[] = ARRAY_DIM(rhouvwR_p[0][n],rhouvwR_p[1][n],rhouvwR_p[2][n]),
		           ER        = ER_p[n],
		           rhoR_inv  = 1.0/rhoR,
		           uvwR[]    = ARRAY_DIM(rhoR_inv*rhouvwR[0],rhoR_inv*rhouvwR[1],rhoR_inv*rhouvwR[2]),
		           V2R       = compute_V2_from_uvw_T(uvwR),
		           VR        = sqrt_T(V2R),
		           pR        = GM1*(ER-0.5*rhoR*V2R),
		           cR        = sqrt_T(GAMMA*pR/rhoR);

		const Type sL_n[] = ARRAY_VAR(rhoL, rhouvwL[0], rhouvwL[1], rhouvwL[2], EL);
		const_cast_T1(&flux_i.s->data,sL_n);

		Type fL_n[DIM*NEQ]          = {0.0};
		Type dfL_dsL_n[DIM*NEQ*NVR] = {0.0};
		flux.f->data     = fL_n;
		flux.df_ds->data = dfL_dsL_n;
		compute_Flux_T_euler(&flux_i,&flux);

		const Type sR_n[] = ARRAY_VAR(rhoR, rhouvwR[0], rhouvwR[1], rhouvwR[2], ER);
		const_cast_T1(&flux_i.s->data,sR_n);

		Type fR_n[DIM*NEQ]          = {0.0};
		Type dfR_dsR_n[DIM*NEQ*NVR] = {0.0};
		flux.f->data     = fR_n;
		flux.df_ds->data = dfR_dsR_n;
		compute_Flux_T_euler(&flux_i,&flux);


		Type const maxlL = VL+cL,
		           maxlR = VR+cR;
		Type maxV = 0.0;
		bool use_left = true;
		if (real_T(maxlL) > real_T(maxlR)) {
			maxV = maxlL;
			use_left = true;
		} else {
			maxV = maxlR;
			use_left = false;
		}

		const Type*const nL = get_row_const_Multiarray_T(n,nL_p);

		for (int eq = 0; eq < NEQ; ++eq) {
			for (int d = 0; d < DIM; ++d) {
				const int ind_f = d+DIM*eq;
				nnf[eq][n] += nL[d]*(fL_n[ind_f]+fR_n[ind_f]);
			}
			nnf[eq][n] += maxV*(sL_n[eq]-sR_n[eq]);
			nnf[eq][n] *= 0.5;
		}

		const Type*const dmaxV_ds = ( use_left ?
		                              compute_dmaxV_ds(rhoL_inv,pL,V2L,VL,cL,uvwL) :
		                              compute_dmaxV_ds(rhoR_inv,pR,V2R,VR,cR,uvwR) );

		for (int vr = 0; vr < NVR; ++vr) {
		for (int eq = 0; eq < NEQ; ++eq) {
			const int ind_dnnf = eq+NEQ*(vr);
			for (int d = 0; d < DIM; ++d) {
				const int ind_f = d+DIM*ind_dnnf;
				dnnf_dsL[ind_dnnf][n] += nL[d]*(dfL_dsL_n[ind_f]);
				dnnf_dsR[ind_dnnf][n] += nL[d]*(dfR_dsR_n[ind_f]);
			}

			if (use_left)
				dnnf_dsL[ind_dnnf][n] += dmaxV_ds[vr]*(sL_n[eq]-sR_n[eq]);
			else
				dnnf_dsR[ind_dnnf][n] += dmaxV_ds[vr]*(sL_n[eq]-sR_n[eq]);
			if (vr == eq) {
				dnnf_dsL[ind_dnnf][n] += maxV*(1.0);
				dnnf_dsR[ind_dnnf][n] += maxV*(-1.0);
			}

			dnnf_dsL[ind_dnnf][n] *= 0.5;
			dnnf_dsR[ind_dnnf][n] *= 0.5;
		}}
	}

	destructor_const_Multiarray_T(flux_i.s);
	destructor_Multiarray_T(flux.f);
	destructor_Multiarray_T(flux.df_ds);
}

void compute_Numerical_Flux_T_euler_roe_pike
	(const struct Numerical_Flux_Input_T* num_flux_i, struct mutable_Numerical_Flux_T* num_flux)
{
	/// The simple entropy fix is taken from (eq. (35), \cite Qu2015).

	const ptrdiff_t NnTotal = num_flux_i->bv_l.s->extents[0];

	Type const *const nL = num_flux_i->bv_l.normals->data;

	Type const *const WL = num_flux_i->bv_l.s->data,
	           *const WR = num_flux_i->bv_r.s->data;

	Type       *const nFluxNum = num_flux->nnf->data;

	Type const *rhoL_ptr  = &WL[NnTotal*0],
	           *rhouL_ptr = &WL[NnTotal*1],
	           *EL_ptr    = &WL[NnTotal*(DIM+1)],

	           *rhoR_ptr  = &WR[NnTotal*0],
	           *rhouR_ptr = &WR[NnTotal*1],
	           *ER_ptr    = &WR[NnTotal*(DIM+1)];

	Type const *n_ptr = nL;

	Type *nF_ptr[NEQ];
	for (int eq = 0; eq < NEQ; eq++)
		nF_ptr[eq] = &nFluxNum[eq*NnTotal];

	if (DIM == 3) {
		Type const *rhovL_ptr = &WL[NnTotal*2],
		           *rhowL_ptr = &WL[NnTotal*3],

		           *rhovR_ptr = &WR[NnTotal*2],
		           *rhowR_ptr = &WR[NnTotal*3];

		for (ptrdiff_t n = 0; n < NnTotal; n++) {
			Type const n1 = *n_ptr++,
			           n2 = *n_ptr++,
			           n3 = *n_ptr++;

			// Left VOLUME
			Type const rhoL  = *rhoL_ptr++,
			           rhouL = *rhouL_ptr++,
			           rhovL = *rhovL_ptr++,
			           rhowL = *rhowL_ptr++,
			           EL    = *EL_ptr++,

			           rhoL_inv  = 1.0/rhoL,
			           uL = rhouL*rhoL_inv,
			           vL = rhovL*rhoL_inv,
			           wL = rhowL*rhoL_inv,

			           V2L = uL*uL+vL*vL+wL*wL,
			           VnL = n1*uL+n2*vL+n3*wL,

			           pL  = GM1*(EL-0.5*rhoL*V2L),
			           HL  = (EL+pL)*rhoL_inv;

			// Right VOLUME
			Type const rhoR  = *rhoR_ptr++,
			           rhouR = *rhouR_ptr++,
			           rhovR = *rhovR_ptr++,
			           rhowR = *rhowR_ptr++,
			           ER    = *ER_ptr++,

			           rhoR_inv  = 1.0/rhoR,
			           uR = rhouR*rhoR_inv,
			           vR = rhovR*rhoR_inv,
			           wR = rhowR*rhoR_inv,

			           V2R = uR*uR+vR*vR+wR*wR,
			           VnR = n1*uR+n2*vR+n3*wR,

			           pR  = GM1*(ER-0.5*rhoR*V2R),
			           HR  = (ER+pR)*rhoR_inv;

			// Roe-averaged states
			Type const r   = sqrt_T(rhoR/rhoL),
			           rP1 = r+1.0,

			           rho = r*rhoL,
			           u   = (r*uR+uL)/rP1,
			           v   = (r*vR+vL)/rP1,
			           w   = (r*wR+wL)/rP1,
			           H   = (r*HR+HL)/rP1,
			           Vn  = n1*u+n2*v+n3*w,
			           V2  = u*u+v*v+w*w,
			           c2  = GM1*(H-0.5*V2),
			           c   = sqrt_T(c2);

			// Compute eigenvalues (with entropy fix)
			const Type l1   = min_abs_real_T(VnL-c,Vn-c),
			           l5   = max_abs_real_T(VnR+c,Vn+c),
			           l234 = ( real_T(Vn) > 0.0 ? Vn : -Vn );

			Type const drho  = rhoR-rhoL,
			           drhou = rhoR*uR-rhoL*uL,
			           drhov = rhoR*vR-rhoL*vL,
			           drhow = rhoR*wR-rhoL*wL,
			           dE    = ER-EL,
			           dp    = pR-pL,
			           dVn   = VnR-VnL,

			           lc1 = 0.5*(l5+l1) - l234,
			           lc2 = 0.5*(l5-l1),

			           disInter1 = lc1*dp/(c*c) + lc2*rho*dVn/c,
			           disInter2 = lc1*rho*dVn  + lc2*dp/c;

			// Compute contribution of normal flux components (multiplied by 0.5 below)
			Type const rhoVnL = rhoL*VnL,
			           rhoVnR = rhoR*VnR,
			           pLR    = pL + pR;

			Type const nF1 = rhoVnL      + rhoVnR,
			           nF2 = rhoVnL*uL   + rhoVnR*uR  + n1*pLR,
			           nF3 = rhoVnL*vL   + rhoVnR*vR  + n2*pLR,
			           nF4 = rhoVnL*wL   + rhoVnR*wR  + n3*pLR,
			           nF5 = VnL*(EL+pL) + VnR*(ER+pR),

			           dis1 = l234*drho  + disInter1,
			           dis2 = l234*drhou + disInter1*u + disInter2*n1,
			           dis3 = l234*drhov + disInter1*v + disInter2*n2,
			           dis4 = l234*drhow + disInter1*w + disInter2*n3,
			           dis5 = l234*dE    + disInter1*H + disInter2*(Vn);

			// Assemble components
			int IndnF = 0;
			*nF_ptr[IndnF++]++ = 0.5*(nF1 - dis1);
			*nF_ptr[IndnF++]++ = 0.5*(nF2 - dis2);
			*nF_ptr[IndnF++]++ = 0.5*(nF3 - dis3);
			*nF_ptr[IndnF++]++ = 0.5*(nF4 - dis4);
			*nF_ptr[IndnF++]++ = 0.5*(nF5 - dis5);
		}
	} else if (DIM == 2) {
		Type const *rhovL_ptr = &WL[NnTotal*2],

		           *rhovR_ptr = &WR[NnTotal*2];

		for (ptrdiff_t n = 0; n < NnTotal; n++) {
			Type const n1 = *n_ptr++,
			           n2 = *n_ptr++;

			// Left VOLUME
			Type const rhoL  = *rhoL_ptr++,
			           rhouL = *rhouL_ptr++,
			           rhovL = *rhovL_ptr++,
			           EL    = *EL_ptr++,

			           rhoL_inv  = 1.0/rhoL,
			           uL = rhouL*rhoL_inv,
			           vL = rhovL*rhoL_inv,

			           V2L = uL*uL+vL*vL,
			           VnL = n1*uL+n2*vL,

			           pL  = GM1*(EL-0.5*rhoL*V2L),
			           HL  = (EL+pL)*rhoL_inv;

			// Right VOLUME
			Type const rhoR  = *rhoR_ptr++,
			           rhouR = *rhouR_ptr++,
			           rhovR = *rhovR_ptr++,
			           ER    = *ER_ptr++,

			           rhoR_inv  = 1.0/rhoR,
			           uR = rhouR*rhoR_inv,
			           vR = rhovR*rhoR_inv,

			           V2R = uR*uR+vR*vR,
			           VnR = n1*uR+n2*vR,

			           pR  = GM1*(ER-0.5*rhoR*V2R),
			           HR  = (ER+pR)*rhoR_inv;
			// Roe-averaged states
			Type const r   = sqrt_T(rhoR/rhoL),
			           rP1 = r+1.0,

			           rho = r*rhoL,
			           u   = (r*uR+uL)/rP1,
			           v   = (r*vR+vL)/rP1,
			           H   = (r*HR+HL)/rP1,
			           Vn  = n1*u+n2*v,
			           V2  = u*u+v*v,
			           c2  = GM1*(H-0.5*V2),
			           c   = sqrt_T(c2);

			// Compute eigenvalues (with entropy fix)
			const Type l1   = min_abs_real_T(VnL-c,Vn-c),
			           l5   = max_abs_real_T(VnR+c,Vn+c),
			           l234 = ( real_T(Vn) > 0.0 ? Vn : -Vn );

			Type const drho  = rhoR-rhoL,
			           drhou = rhoR*uR-rhoL*uL,
			           drhov = rhoR*vR-rhoL*vL,
			           dE    = ER-EL,
			           dp    = pR-pL,
			           dVn   = VnR-VnL,

			           lc1 = 0.5*(l5+l1) - l234,
			           lc2 = 0.5*(l5-l1),

			           disInter1 = lc1*dp/(c*c) + lc2*rho*dVn/c,
			           disInter2 = lc1*rho*dVn  + lc2*dp/c;

			// Compute contribution of normal flux components (multiplied by 0.5 below)
			Type const rhoVnL = rhoL*VnL,
			           rhoVnR = rhoR*VnR,
			           pLR    = pL + pR;

			Type const nF1 = rhoVnL      + rhoVnR,
			           nF2 = rhoVnL*uL   + rhoVnR*uR  + n1*pLR,
			           nF3 = rhoVnL*vL   + rhoVnR*vR  + n2*pLR,
			           nF5 = VnL*(EL+pL) + VnR*(ER+pR),

			           dis1 = l234*drho  + disInter1,
			           dis2 = l234*drhou + disInter1*u + disInter2*n1,
			           dis3 = l234*drhov + disInter1*v + disInter2*n2,
			           dis5 = l234*dE    + disInter1*H + disInter2*(Vn);

			// Assemble components
			int IndnF = 0;
			*nF_ptr[IndnF++]++ = 0.5*(nF1 - dis1);
			*nF_ptr[IndnF++]++ = 0.5*(nF2 - dis2);
			*nF_ptr[IndnF++]++ = 0.5*(nF3 - dis3);
			*nF_ptr[IndnF++]++ = 0.5*(nF5 - dis5);
		}
	} else if (DIM == 1) {
		for (ptrdiff_t n = 0; n < NnTotal; n++) {
			Type const n1 = *n_ptr++;

			// Left VOLUME
			Type const rhoL  = *rhoL_ptr++,
			           rhouL = *rhouL_ptr++,
			           EL    = *EL_ptr++,

			           rhoL_inv  = 1.0/rhoL,
			           uL = rhouL*rhoL_inv,

			           V2L = uL*uL,
			           VnL = n1*uL,

			           pL  = GM1*(EL-0.5*rhoL*V2L),
			           HL  = (EL+pL)*rhoL_inv;

			// Right VOLUME
			Type const rhoR  = *rhoR_ptr++,
			           rhouR = *rhouR_ptr++,
			           ER    = *ER_ptr++,

			           rhoR_inv  = 1.0/rhoR,
			           uR = rhouR*rhoR_inv,

			           V2R = uR*uR,
			           VnR = n1*uR,

			           pR  = GM1*(ER-0.5*rhoR*V2R),
			           HR  = (ER+pR)*rhoR_inv;

			// Roe-averaged states
			Type const r   = sqrt_T(rhoR/rhoL),
			           rP1 = r+1.0,

			           rho = r*rhoL,
			           u   = (r*uR+uL)/rP1,
			           H   = (r*HR+HL)/rP1,
			           Vn  = n1*u,
			           V2  = u*u,
			           c2  = GM1*(H-0.5*V2),
			           c   = sqrt_T(c2);

			// Compute eigenvalues (with entropy fix)
			const Type l1   = min_abs_real_T(VnL-c,Vn-c),
			           l5   = max_abs_real_T(VnR+c,Vn+c),
			           l234 = ( real_T(Vn) > 0.0 ? Vn : -Vn );

			Type const drho  = rhoR-rhoL,
			           drhou = rhoR*uR-rhoL*uL,
			           dE    = ER-EL,
			           dp    = pR-pL,
			           dVn   = VnR-VnL,

			           lc1 = 0.5*(l5+l1) - l234,
			           lc2 = 0.5*(l5-l1),

			           disInter1 = lc1*dp/(c*c) + lc2*rho*dVn/c,
			           disInter2 = lc1*rho*dVn  + lc2*dp/c;

			// Compute contribution of normal flux components (multiplied by 0.5 below)
			Type const rhoVnL = rhoL*VnL,
			           rhoVnR = rhoR*VnR,
			           pLR    = pL + pR;

			Type const nF1 = rhoVnL      + rhoVnR,
			           nF2 = rhoVnL*uL   + rhoVnR*uR  + n1*pLR,
			           nF5 = VnL*(EL+pL) + VnR*(ER+pR),

			           dis1 = l234*drho  + disInter1,
			           dis2 = l234*drhou + disInter1*u + disInter2*n1,
			           dis5 = l234*dE    + disInter1*H + disInter2*(Vn);

			// Assemble components
			int IndnF = 0;
			*nF_ptr[IndnF++]++ = 0.5*(nF1 - dis1);
			*nF_ptr[IndnF++]++ = 0.5*(nF2 - dis2);
			*nF_ptr[IndnF++]++ = 0.5*(nF5 - dis5);
		}
	}
}

void compute_Numerical_Flux_T_euler_roe_pike_jacobian
	(const struct Numerical_Flux_Input_T* num_flux_i, struct mutable_Numerical_Flux_T* num_flux)
{
	/// The simple entropy fix is taken from (eq. (35), \cite Qu2015).

	const ptrdiff_t NnTotal = num_flux_i->bv_l.s->extents[0];

	Type const *const nL = num_flux_i->bv_l.normals->data;

	Type const *const WL = num_flux_i->bv_l.s->data,
	           *const WR = num_flux_i->bv_r.s->data;

	Type *const nFluxNum = (num_flux->nnf ? num_flux->nnf->data : NULL ),
	     *const dnFdWL   = num_flux->neigh_info[0].dnnf_ds->data,
	     *const dnFdWR   = num_flux->neigh_info[1].dnnf_ds->data;

	Type const *rhoL_ptr  = &WL[NnTotal*0],
	           *rhouL_ptr = &WL[NnTotal*1],
	           *EL_ptr    = &WL[NnTotal*(DIM+1)],

	           *rhoR_ptr  = &WR[NnTotal*0],
	           *rhouR_ptr = &WR[NnTotal*1],
	           *ER_ptr    = &WR[NnTotal*(DIM+1)];

	Type const *n_ptr = nL;

	Type *nF_ptr[NEQ];
	if (nFluxNum != NULL) {
		for (int eq = 0; eq < NEQ; eq++)
			nF_ptr[eq] = &nFluxNum[eq*NnTotal];
	}

	// Note: dnFdW*_ptr are ordered by [node, eq, var] but are set by [node,var,eq] below.
	Type *dnFdWL_ptr[NEQ*NEQ];
	for (int eq = 0; eq < NEQ; eq++) {
	for (int var = 0; var < NVAR; var++) {
		dnFdWL_ptr[eq*NVAR+var] = &dnFdWL[(eq+var*NVAR)*NnTotal];
	}}

	Type *dnFdWR_ptr[NEQ*NEQ];
	if (dnFdWR != NULL) {
		for (int eq = 0; eq < NEQ; eq++) {
		for (int var = 0; var < NVAR; var++) {
			dnFdWR_ptr[eq*NVAR+var] = &dnFdWR[(eq+var*NVAR)*NnTotal];
		}}
	}

	if (DIM == 3) {
		Type const *rhovL_ptr = &WL[NnTotal*2],
		           *rhowL_ptr = &WL[NnTotal*3],

		           *rhovR_ptr = &WR[NnTotal*2],
		           *rhowR_ptr = &WR[NnTotal*3];

		for (ptrdiff_t n = 0; n < NnTotal; n++) {
			Type const n1 = *n_ptr++,
			           n2 = *n_ptr++,
			           n3 = *n_ptr++;

			// Left VOLUME
			Type const rhoL  = *rhoL_ptr++,
			           rhouL = *rhouL_ptr++,
			           rhovL = *rhovL_ptr++,
			           rhowL = *rhowL_ptr++,
			           EL    = *EL_ptr++,

			           rhoL_inv  = 1.0/rhoL,
			           rhoLr_inv = sqrt_T(rhoL_inv),
			           uL = rhouL*rhoL_inv,
			           vL = rhovL*rhoL_inv,
			           wL = rhowL*rhoL_inv,

			           V2L = uL*uL+vL*vL+wL*wL,
			           VnL = n1*uL+n2*vL+n3*wL,

			           pL  = GM1*(EL-0.5*rhoL*V2L),
			           HL  = (EL+pL)*rhoL_inv;

			// Right VOLUME
			Type const rhoR  = *rhoR_ptr++,
			           rhouR = *rhouR_ptr++,
			           rhovR = *rhovR_ptr++,
			           rhowR = *rhowR_ptr++,
			           ER    = *ER_ptr++,

			           rhoR_inv  = 1.0/rhoR,
			           rhoRr_inv = sqrt_T(rhoR_inv),
			           uR = rhouR*rhoR_inv,
			           vR = rhovR*rhoR_inv,
			           wR = rhowR*rhoR_inv,

			           V2R = uR*uR+vR*vR+wR*wR,
			           VnR = n1*uR+n2*vR+n3*wR,

			           pR  = GM1*(ER-0.5*rhoR*V2R),
			           HR  = (ER+pR)*rhoR_inv;

			// Roe-averaged states
			Type const r   = sqrt_T(rhoR/rhoL),
			           rP1 = r+1.0,

			           rho = r*rhoL,
			           u   = (r*uR+uL)/rP1,
			           v   = (r*vR+vL)/rP1,
			           w   = (r*wR+wL)/rP1,
			           H   = (r*HR+HL)/rP1,
			           Vn  = n1*u+n2*v+n3*w,
			           V2  = u*u+v*v+w*w,
			           c2  = GM1*(H-0.5*V2),
			           c   = sqrt_T(c2),

			           Den  = sqrt_T(rhoL) + sqrt_T(rhoR);

			// Compute eigenvalues (with entropy fix)
			unsigned int case_l1;
			Type const l1L     = VnL-c;
			Type       l1      = Vn-c,
			           sign_l1 = 1.0;
			if (abs_T(l1L) < abs_T(l1)) {
				case_l1 = 0;
				if (real_T(l1L) < 0.0)
					sign_l1 = -1.0;
				l1 = sign_l1*l1L;
			} else {
				case_l1 = 1;
				if (real_T(l1) < 0.0)
					sign_l1 = -1.0;
				l1 = sign_l1*l1;
			}

			unsigned int case_l5;
			Type const l5R     = VnR+c;
			Type       l5      = Vn+c,
			           sign_l5 = 1.0;
			if (abs_T(l5R) > abs_T(l5)) {
				case_l5 = 0;
				if (real_T(l5R) < 0.0)
					sign_l5 = -1.0;
				l5 = sign_l5*l5R;
			} else {
				case_l5 = 1;
				if (real_T(l5) < 0.0)
					sign_l5 = -1.0;
				l5 = sign_l5*l5;
			}

			Type sign_l234 = 1.0;
			if (real_T(Vn) < 0.0)
				sign_l234 = -1.0;
			Type const l234 = sign_l234*(Vn);


			Type const drho  = rhoR-rhoL,
			           drhou = rhoR*uR-rhoL*uL,
			           drhov = rhoR*vR-rhoL*vL,
			           drhow = rhoR*wR-rhoL*wL,
			           dE    = ER-EL,
			           dp    = pR-pL,
			           dVn   = VnR-VnL,

			           lc1 = 0.5*(l5+l1) - l234,
			           lc2 = 0.5*(l5-l1),

			           disInter1 = lc1*dp/(c*c) + lc2*rho*dVn/c,
			           disInter2 = lc1*rho*dVn  + lc2*dp/c;

			if (nFluxNum != NULL) {
				Type const rhoVnL = rhoL*VnL,
				           rhoVnR = rhoR*VnR,
				           pLR    = pL + pR;

				Type const nF1 = rhoVnL      + rhoVnR,
				           nF2 = rhoVnL*uL   + rhoVnR*uR  + n1*pLR,
				           nF3 = rhoVnL*vL   + rhoVnR*vR  + n2*pLR,
				           nF4 = rhoVnL*wL   + rhoVnR*wR  + n3*pLR,
				           nF5 = VnL*(EL+pL) + VnR*(ER+pR),

				           dis1 = l234*drho  + disInter1,
				           dis2 = l234*drhou + disInter1*u + disInter2*n1,
				           dis3 = l234*drhov + disInter1*v + disInter2*n2,
				           dis4 = l234*drhow + disInter1*w + disInter2*n3,
				           dis5 = l234*dE    + disInter1*H + disInter2*(Vn);

				// Assemble components
				int IndnF = 0;
				*nF_ptr[IndnF++]++ = 0.5*(nF1 - dis1);
				*nF_ptr[IndnF++]++ = 0.5*(nF2 - dis2);
				*nF_ptr[IndnF++]++ = 0.5*(nF3 - dis3);
				*nF_ptr[IndnF++]++ = 0.5*(nF4 - dis4);
				*nF_ptr[IndnF++]++ = 0.5*(nF5 - dis5);
			}

			Type dnF1dW[NEQ],  dnF2dW[NEQ],  dnF3dW[NEQ],  dnF4dW[NEQ],  dnF5dW[NEQ],
			       ddis1dW[NEQ], ddis2dW[NEQ], ddis3dW[NEQ], ddis4dW[NEQ], ddis5dW[NEQ];

			// Flux term
			Type const duLdW[]   = {-uL*rhoL_inv,  rhoL_inv,  0.0,       0.0,      0.0},
			           dvLdW[]   = {-vL*rhoL_inv,  0.0,       rhoL_inv,  0.0,      0.0},
			           dwLdW[]   = {-wL*rhoL_inv,  0.0,       0.0,       rhoL_inv, 0.0},
			           drhoLdW[] = { 1.0,          0.0,       0.0,       0.0,      0.0},
			           dELdW[]   = { 0.0,          0.0,       0.0,       0.0,      1.0},
			           dpLdW[]   = { GM1*0.5*V2L, -GM1*uL,   -GM1*vL,   -GM1*wL,   GM1};

			Type const rhoVn = rhoL*VnL;
			Type dVnLdW[NVAR];
			for (int var = 0; var < NVAR; var++) {
				dVnLdW[var] = n1*duLdW[var]+n2*dvLdW[var]+n3*dwLdW[var];
				Type const drhoVndW = drhoLdW[var]*VnL + rhoL*dVnLdW[var];

				dnF1dW[var] = drhoVndW;
				dnF2dW[var] = drhoVndW*uL + rhoVn*duLdW[var] + n1*dpLdW[var];
				dnF3dW[var] = drhoVndW*vL + rhoVn*dvLdW[var] + n2*dpLdW[var];
				dnF4dW[var] = drhoVndW*wL + rhoVn*dwLdW[var] + n3*dpLdW[var];
				dnF5dW[var] = dVnLdW[var]*(EL+pL) + VnL*(dELdW[var]+dpLdW[var]);
			}

			// Dissipation term
			Type const mult = rhoLr_inv/Den;
			Type const drhodW[] = { 0.5*rhoR/rho,             0.0,          0.0,          0.0,         0.0},
			           dudW[]   = {-0.5*(uL+u)*mult,          mult,         0.0,          0.0,         0.0},
			           dvdW[]   = {-0.5*(vL+v)*mult,          0.0,          mult,         0.0,         0.0},
			           dwdW[]   = {-0.5*(wL+w)*mult,          0.0,          0.0,          mult,        0.0},
			           dHdW[]   = {-0.5*(HL+H-GM1*V2L)*mult, -GM1*uL*mult, -GM1*vL*mult, -GM1*wL*mult, GAMMA*mult};

			for (int var = 0; var < NVAR; var++) {
				Type const dcdW  = 0.5*GM1/c*(dHdW[var]-(u*dudW[var]+v*dvdW[var]+w*dwdW[var])),
				           dVndW = n1*dudW[var]+n2*dvdW[var]+n3*dwdW[var];

				Type dl1dW;
				if (case_l1)
					dl1dW = sign_l1*(dVndW-dcdW);
				else
					dl1dW = sign_l1*(dVnLdW[var]-dcdW);

				Type dl5dW;
				if (case_l5)
					dl5dW = sign_l5*(dVndW+dcdW);
				else
					dl5dW = sign_l5*(dcdW);

				Type const dl234dW = sign_l234*dVndW,
				           dlc1dW = 0.5*(dl5dW+dl1dW) - dl234dW,
				           dlc2dW = 0.5*(dl5dW-dl1dW),

				           ddisInter1dW = (dlc1dW*dp-lc1*dpLdW[var])/c2-(2.0*lc1*dp*dcdW)/(c*c2) +
				                          (dlc2dW*rho*dVn+lc2*drhodW[var]*dVn-lc2*rho*dVnLdW[var])/c -
				                          (lc2*rho*dVn*dcdW)/c2,
				           ddisInter2dW = dlc1dW*rho*dVn+lc1*drhodW[var]*dVn-lc1*rho*dVnLdW[var] +
				                          (dlc2dW*dp-lc2*dpLdW[var])/c-(lc2*dp*dcdW)/c2,

				           drhoudW = drhoLdW[var]*uL+rhoL*duLdW[var],
				           drhovdW = drhoLdW[var]*vL+rhoL*dvLdW[var],
				           drhowdW = drhoLdW[var]*wL+rhoL*dwLdW[var];

				ddis1dW[var] = dl234dW*drho - l234*drhoLdW[var] + ddisInter1dW;
				ddis2dW[var] = dl234dW*drhou - l234*drhoudW + ddisInter1dW*u + disInter1*dudW[var] + ddisInter2dW*n1;
				ddis3dW[var] = dl234dW*drhov - l234*drhovdW + ddisInter1dW*v + disInter1*dvdW[var] + ddisInter2dW*n2;
				ddis4dW[var] = dl234dW*drhow - l234*drhowdW + ddisInter1dW*w + disInter1*dwdW[var] + ddisInter2dW*n3;
				ddis5dW[var] = dl234dW*dE - l234*dELdW[var] + ddisInter1dW*H  + disInter1*dHdW[var]
				                                            + ddisInter2dW*Vn + disInter2*dVndW;
			}

			int InddnFdW = 0;
			for (int var = 0; var < NVAR; var++) *dnFdWL_ptr[InddnFdW++]++ = 0.5*(dnF1dW[var]-ddis1dW[var]);
			for (int var = 0; var < NVAR; var++) *dnFdWL_ptr[InddnFdW++]++ = 0.5*(dnF2dW[var]-ddis2dW[var]);
			for (int var = 0; var < NVAR; var++) *dnFdWL_ptr[InddnFdW++]++ = 0.5*(dnF3dW[var]-ddis3dW[var]);
			for (int var = 0; var < NVAR; var++) *dnFdWL_ptr[InddnFdW++]++ = 0.5*(dnF4dW[var]-ddis4dW[var]);
			for (int var = 0; var < NVAR; var++) *dnFdWL_ptr[InddnFdW++]++ = 0.5*(dnF5dW[var]-ddis5dW[var]);

			if (dnFdWR != NULL) {
				// Flux term
				Type const duRdW[]   = {-uR*rhoR_inv,  rhoR_inv,  0.0,       0.0,      0.0},
				           dvRdW[]   = {-vR*rhoR_inv,  0.0,       rhoR_inv,  0.0,      0.0},
				           dwRdW[]   = {-wR*rhoR_inv,  0.0,       0.0,       rhoR_inv, 0.0},
				           drhoRdW[] = { 1.0,          0.0,       0.0,       0.0,      0.0},
				           dERdW[]   = { 0.0,          0.0,       0.0,       0.0,      1.0},
				           dpRdW[]   = { GM1*0.5*V2R, -GM1*uR,   -GM1*vR,   -GM1*wR,   GM1};

				Type const rhoVn = rhoR*VnR;
				Type dVnRdW[NVAR];
				for (int var = 0; var < NVAR; var++) {
					dVnRdW[var] = n1*duRdW[var]+n2*dvRdW[var]+n3*dwRdW[var];
					Type const drhoVndW = drhoRdW[var]*VnR + rhoR*dVnRdW[var];

					dnF1dW[var] = drhoVndW;
					dnF2dW[var] = drhoVndW*uR + rhoVn*duRdW[var] + n1*dpRdW[var];
					dnF3dW[var] = drhoVndW*vR + rhoVn*dvRdW[var] + n2*dpRdW[var];
					dnF4dW[var] = drhoVndW*wR + rhoVn*dwRdW[var] + n3*dpRdW[var];
					dnF5dW[var] = dVnRdW[var]*(ER+pR) + VnR*(dERdW[var]+dpRdW[var]);
				}

				// Dissipation term
				Type const mult = rhoRr_inv/Den;
				Type const drhodW[] = { 0.5*rhoL/rho,             0.0,          0.0,          0.0,         0.0},
				           dudW[]   = {-0.5*(uR+u)*mult,          mult,         0.0,          0.0,         0.0},
				           dvdW[]   = {-0.5*(vR+v)*mult,          0.0,          mult,         0.0,         0.0},
				           dwdW[]   = {-0.5*(wR+w)*mult,          0.0,          0.0,          mult,        0.0},
				           dHdW[]   = {-0.5*(HR+H-GM1*V2R)*mult, -GM1*uR*mult, -GM1*vR*mult, -GM1*wR*mult, GAMMA*mult};

				for (int var = 0; var < NVAR; var++) {
					Type const dcdW  = 0.5*GM1/c*(dHdW[var]-(u*dudW[var]+v*dvdW[var]+w*dwdW[var])),
					           dVndW = n1*dudW[var]+n2*dvdW[var]+n3*dwdW[var];

					Type dl1dW;
					if (case_l1)
						dl1dW = sign_l1*(dVndW-dcdW);
					else
						dl1dW = sign_l1*(-dcdW);

					Type dl5dW;
					if (case_l5)
						dl5dW = sign_l5*(dVndW+dcdW);
					else
						dl5dW = sign_l5*(dVnRdW[var]+dcdW);

					Type const dl234dW = sign_l234*dVndW,
					           dlc1dW = 0.5*(dl5dW+dl1dW) - dl234dW,
					           dlc2dW = 0.5*(dl5dW-dl1dW),

					           ddisInter1dW = (dlc1dW*dp+lc1*dpRdW[var])/c2-(2.0*lc1*dp*dcdW)/(c*c2) +
					                          (dlc2dW*rho*dVn+lc2*drhodW[var]*dVn+lc2*rho*dVnRdW[var])/c -
					                          (lc2*rho*dVn*dcdW)/c2,
					           ddisInter2dW = dlc1dW*rho*dVn+lc1*drhodW[var]*dVn+lc1*rho*dVnRdW[var] +
					                          (dlc2dW*dp+lc2*dpRdW[var])/c-(lc2*dp*dcdW)/c2,

					           drhoudW = drhoRdW[var]*uR+rhoR*duRdW[var],
					           drhovdW = drhoRdW[var]*vR+rhoR*dvRdW[var],
					           drhowdW = drhoRdW[var]*wR+rhoR*dwRdW[var];

					ddis1dW[var] = dl234dW*drho + l234*drhoRdW[var] + ddisInter1dW;
					ddis2dW[var] = dl234dW*drhou + l234*drhoudW + ddisInter1dW*u + disInter1*dudW[var] + ddisInter2dW*n1;
					ddis3dW[var] = dl234dW*drhov + l234*drhovdW + ddisInter1dW*v + disInter1*dvdW[var] + ddisInter2dW*n2;
					ddis4dW[var] = dl234dW*drhow + l234*drhowdW + ddisInter1dW*w + disInter1*dwdW[var] + ddisInter2dW*n3;
					ddis5dW[var] = dl234dW*dE + l234*dERdW[var] + ddisInter1dW*H  + disInter1*dHdW[var]
					                                            + ddisInter2dW*Vn + disInter2*dVndW;
				}

				int InddnFdW = 0;
				for (int var = 0; var < NVAR; var++) *dnFdWR_ptr[InddnFdW++]++ = 0.5*(dnF1dW[var]-ddis1dW[var]);
				for (int var = 0; var < NVAR; var++) *dnFdWR_ptr[InddnFdW++]++ = 0.5*(dnF2dW[var]-ddis2dW[var]);
				for (int var = 0; var < NVAR; var++) *dnFdWR_ptr[InddnFdW++]++ = 0.5*(dnF3dW[var]-ddis3dW[var]);
				for (int var = 0; var < NVAR; var++) *dnFdWR_ptr[InddnFdW++]++ = 0.5*(dnF4dW[var]-ddis4dW[var]);
				for (int var = 0; var < NVAR; var++) *dnFdWR_ptr[InddnFdW++]++ = 0.5*(dnF5dW[var]-ddis5dW[var]);
			}
		}
	} else if (DIM == 2) {
		Type const *rhovL_ptr = &WL[NnTotal*2],

		             *rhovR_ptr = &WR[NnTotal*2];

		for (ptrdiff_t n = 0; n < NnTotal; n++) {
			Type const n1 = *n_ptr++,
			           n2 = *n_ptr++;

			// Left VOLUME
			Type const rhoL  = *rhoL_ptr++,
			           rhouL = *rhouL_ptr++,
			           rhovL = *rhovL_ptr++,
			           EL    = *EL_ptr++,

			           rhoL_inv  = 1.0/rhoL,
			           rhoLr_inv = sqrt_T(rhoL_inv),
			           uL = rhouL*rhoL_inv,
			           vL = rhovL*rhoL_inv,

			           V2L = uL*uL+vL*vL,
			           VnL = n1*uL+n2*vL,

			           pL  = GM1*(EL-0.5*rhoL*V2L),
			           HL  = (EL+pL)*rhoL_inv;

			// Right VOLUME
			Type const rhoR  = *rhoR_ptr++,
			           rhouR = *rhouR_ptr++,
			           rhovR = *rhovR_ptr++,
			           ER    = *ER_ptr++,

			           rhoR_inv  = 1.0/rhoR,
			           rhoRr_inv = sqrt_T(rhoR_inv),
			           uR = rhouR*rhoR_inv,
			           vR = rhovR*rhoR_inv,

			           V2R = uR*uR+vR*vR,
			           VnR = n1*uR+n2*vR,

			           pR  = GM1*(ER-0.5*rhoR*V2R),
			           HR  = (ER+pR)*rhoR_inv;

			// Roe-averaged states
			Type const r   = sqrt_T(rhoR/rhoL),
			           rP1 = r+1.0,

			           rho = r*rhoL,
			           u   = (r*uR+uL)/rP1,
			           v   = (r*vR+vL)/rP1,
			           H   = (r*HR+HL)/rP1,
			           Vn  = n1*u+n2*v,
			           V2  = u*u+v*v,
			           c2  = GM1*(H-0.5*V2),
			           c   = sqrt_T(c2),

			           Den  = sqrt_T(rhoL) + sqrt_T(rhoR);

			// Compute eigenvalues (with entropy fix)
			unsigned int case_l1;
			Type const l1L     = VnL-c;
			Type       l1      = Vn-c,
			           sign_l1 = 1.0;
			if (abs_T(l1L) < abs_T(l1)) {
				case_l1 = 0;
				if (real_T(l1L) < 0.0)
					sign_l1 = -1.0;
				l1 = sign_l1*l1L;
			} else {
				case_l1 = 1;
				if (real_T(l1) < 0.0)
					sign_l1 = -1.0;
				l1 = sign_l1*l1;
			}

			unsigned int case_l5;
			Type const l5R     = VnR+c;
			Type       l5      = Vn+c,
			           sign_l5 = 1.0;
			if (abs_T(l5R) > abs_T(l5)) {
				case_l5 = 0;
				if (real_T(l5R) < 0.0)
					sign_l5 = -1.0;
				l5 = sign_l5*l5R;
			} else {
				case_l5 = 1;
				if (real_T(l5) < 0.0)
					sign_l5 = -1.0;
				l5 = sign_l5*l5;
			}

			Type sign_l234 = 1.0;
			if (real_T(Vn) < 0.0)
				sign_l234 = -1.0;
			Type const l234 = sign_l234*(Vn);


			Type const drho  = rhoR-rhoL,
			           drhou = rhoR*uR-rhoL*uL,
			           drhov = rhoR*vR-rhoL*vL,
			           dE    = ER-EL,
			           dp    = pR-pL,
			           dVn   = VnR-VnL,

			           lc1 = 0.5*(l5+l1) - l234,
			           lc2 = 0.5*(l5-l1),

			           disInter1 = lc1*dp/(c*c) + lc2*rho*dVn/c,
			           disInter2 = lc1*rho*dVn  + lc2*dp/c;

			if (nFluxNum != NULL) {
				Type const rhoVnL = rhoL*VnL,
				           rhoVnR = rhoR*VnR,
				           pLR    = pL + pR;

				Type const nF1 = rhoVnL      + rhoVnR,
				           nF2 = rhoVnL*uL   + rhoVnR*uR  + n1*pLR,
				           nF3 = rhoVnL*vL   + rhoVnR*vR  + n2*pLR,
				           nF5 = VnL*(EL+pL) + VnR*(ER+pR),

				           dis1 = l234*drho  + disInter1,
				           dis2 = l234*drhou + disInter1*u + disInter2*n1,
				           dis3 = l234*drhov + disInter1*v + disInter2*n2,
				           dis5 = l234*dE    + disInter1*H + disInter2*(Vn);

				// Assemble components
				int IndnF = 0;
				*nF_ptr[IndnF++]++ = 0.5*(nF1 - dis1);
				*nF_ptr[IndnF++]++ = 0.5*(nF2 - dis2);
				*nF_ptr[IndnF++]++ = 0.5*(nF3 - dis3);
				*nF_ptr[IndnF++]++ = 0.5*(nF5 - dis5);
			}

			Type dnF1dW[NEQ],  dnF2dW[NEQ],  dnF3dW[NEQ],  dnF5dW[NEQ],
			       ddis1dW[NEQ], ddis2dW[NEQ], ddis3dW[NEQ], ddis5dW[NEQ];

			// Flux term
			Type const duLdW[]   = {-uL*rhoL_inv,  rhoL_inv,  0.0,      0.0},
			           dvLdW[]   = {-vL*rhoL_inv,  0.0,       rhoL_inv, 0.0},
			           drhoLdW[] = { 1.0,          0.0,       0.0,      0.0},
			           dELdW[]   = { 0.0,          0.0,       0.0,      1.0},
			           dpLdW[]   = { GM1*0.5*V2L, -GM1*uL,   -GM1*vL,   GM1};

			Type const rhoVn = rhoL*VnL;
			Type dVnLdW[NVAR];
			for (int var = 0; var < NVAR; var++) {
				dVnLdW[var] = n1*duLdW[var]+n2*dvLdW[var];
				Type const drhoVndW = drhoLdW[var]*VnL + rhoL*dVnLdW[var];

				dnF1dW[var] = drhoVndW;
				dnF2dW[var] = drhoVndW*uL + rhoVn*duLdW[var] + n1*dpLdW[var];
				dnF3dW[var] = drhoVndW*vL + rhoVn*dvLdW[var] + n2*dpLdW[var];
				dnF5dW[var] = dVnLdW[var]*(EL+pL) + VnL*(dELdW[var]+dpLdW[var]);
			}

			// Dissipation term
			Type const mult = rhoLr_inv/Den;
			Type const drhodW[] = { 0.5*rhoR/rho,             0.0,          0.0,         0.0},
			           dudW[]   = {-0.5*(uL+u)*mult,          mult,         0.0,         0.0},
			           dvdW[]   = {-0.5*(vL+v)*mult,          0.0,          mult,        0.0},
			           dHdW[]   = {-0.5*(HL+H-GM1*V2L)*mult, -GM1*uL*mult, -GM1*vL*mult, GAMMA*mult};

			for (int var = 0; var < NVAR; var++) {
				Type const dcdW  = 0.5*GM1/c*(dHdW[var]-(u*dudW[var]+v*dvdW[var])),
				           dVndW = n1*dudW[var]+n2*dvdW[var];

				Type dl1dW;
				if (case_l1)
					dl1dW = sign_l1*(dVndW-dcdW);
				else
					dl1dW = sign_l1*(dVnLdW[var]-dcdW);

				Type dl5dW;
				if (case_l5)
					dl5dW = sign_l5*(dVndW+dcdW);
				else
					dl5dW = sign_l5*(dcdW);

				Type const dl234dW = sign_l234*dVndW,
				           dlc1dW = 0.5*(dl5dW+dl1dW) - dl234dW,
				           dlc2dW = 0.5*(dl5dW-dl1dW),

				           ddisInter1dW = (dlc1dW*dp-lc1*dpLdW[var])/c2-(2.0*lc1*dp*dcdW)/(c*c2) +
				                          (dlc2dW*rho*dVn+lc2*drhodW[var]*dVn-lc2*rho*dVnLdW[var])/c -
				                          (lc2*rho*dVn*dcdW)/c2,
				           ddisInter2dW = dlc1dW*rho*dVn+lc1*drhodW[var]*dVn-lc1*rho*dVnLdW[var] +
				                          (dlc2dW*dp-lc2*dpLdW[var])/c-(lc2*dp*dcdW)/c2,

				           drhoudW = drhoLdW[var]*uL+rhoL*duLdW[var],
				           drhovdW = drhoLdW[var]*vL+rhoL*dvLdW[var];

				ddis1dW[var] = dl234dW*drho - l234*drhoLdW[var] + ddisInter1dW;
				ddis2dW[var] = dl234dW*drhou - l234*drhoudW + ddisInter1dW*u + disInter1*dudW[var] + ddisInter2dW*n1;
				ddis3dW[var] = dl234dW*drhov - l234*drhovdW + ddisInter1dW*v + disInter1*dvdW[var] + ddisInter2dW*n2;
				ddis5dW[var] = dl234dW*dE - l234*dELdW[var] + ddisInter1dW*H  + disInter1*dHdW[var]
				                                            + ddisInter2dW*Vn + disInter2*dVndW;
			}

			int InddnFdW = 0;
			for (int var = 0; var < NVAR; var++) *dnFdWL_ptr[InddnFdW++]++ = 0.5*(dnF1dW[var]-ddis1dW[var]);
			for (int var = 0; var < NVAR; var++) *dnFdWL_ptr[InddnFdW++]++ = 0.5*(dnF2dW[var]-ddis2dW[var]);
			for (int var = 0; var < NVAR; var++) *dnFdWL_ptr[InddnFdW++]++ = 0.5*(dnF3dW[var]-ddis3dW[var]);
			for (int var = 0; var < NVAR; var++) *dnFdWL_ptr[InddnFdW++]++ = 0.5*(dnF5dW[var]-ddis5dW[var]);

			if (dnFdWR != NULL) {
				// Flux term
				Type const duRdW[]   = {-uR*rhoR_inv,  rhoR_inv,  0.0,      0.0},
				           dvRdW[]   = {-vR*rhoR_inv,  0.0,       rhoR_inv, 0.0},
				           drhoRdW[] = { 1.0,          0.0,       0.0,      0.0},
				           dERdW[]   = { 0.0,          0.0,       0.0,      1.0},
				           dpRdW[]   = { GM1*0.5*V2R, -GM1*uR,   -GM1*vR,   GM1};

				Type const rhoVn = rhoR*VnR;
				Type dVnRdW[NVAR];
				for (int var = 0; var < NVAR; var++) {
					dVnRdW[var] = n1*duRdW[var]+n2*dvRdW[var];
					Type const drhoVndW = drhoRdW[var]*VnR + rhoR*dVnRdW[var];

					dnF1dW[var] = drhoVndW;
					dnF2dW[var] = drhoVndW*uR + rhoVn*duRdW[var] + n1*dpRdW[var];
					dnF3dW[var] = drhoVndW*vR + rhoVn*dvRdW[var] + n2*dpRdW[var];
					dnF5dW[var] = dVnRdW[var]*(ER+pR) + VnR*(dERdW[var]+dpRdW[var]);
				}

				// Dissipation term
				Type const mult = rhoRr_inv/Den;
				Type const drhodW[] = { 0.5*rhoL/rho,             0.0,          0.0,         0.0},
				           dudW[]   = {-0.5*(uR+u)*mult,          mult,         0.0,         0.0},
				           dvdW[]   = {-0.5*(vR+v)*mult,          0.0,          mult,        0.0},
				           dHdW[]   = {-0.5*(HR+H-GM1*V2R)*mult, -GM1*uR*mult, -GM1*vR*mult, GAMMA*mult};

				for (int var = 0; var < NVAR; var++) {
					Type const dcdW  = 0.5*GM1/c*(dHdW[var]-(u*dudW[var]+v*dvdW[var])),
					           dVndW = n1*dudW[var]+n2*dvdW[var];

					Type dl1dW;
					if (case_l1)
						dl1dW = sign_l1*(dVndW-dcdW);
					else
						dl1dW = sign_l1*(-dcdW);

					Type dl5dW;
					if (case_l5)
						dl5dW = sign_l5*(dVndW+dcdW);
					else
						dl5dW = sign_l5*(dVnRdW[var]+dcdW);

					Type const dl234dW = sign_l234*dVndW,
					           dlc1dW = 0.5*(dl5dW+dl1dW) - dl234dW,
					           dlc2dW = 0.5*(dl5dW-dl1dW),

					           ddisInter1dW = (dlc1dW*dp+lc1*dpRdW[var])/c2-(2.0*lc1*dp*dcdW)/(c*c2) +
					                          (dlc2dW*rho*dVn+lc2*drhodW[var]*dVn+lc2*rho*dVnRdW[var])/c -
					                          (lc2*rho*dVn*dcdW)/c2,
					           ddisInter2dW = dlc1dW*rho*dVn+lc1*drhodW[var]*dVn+lc1*rho*dVnRdW[var] +
					                          (dlc2dW*dp+lc2*dpRdW[var])/c-(lc2*dp*dcdW)/c2,

					           drhoudW = drhoRdW[var]*uR+rhoR*duRdW[var],
					           drhovdW = drhoRdW[var]*vR+rhoR*dvRdW[var];

					ddis1dW[var] = dl234dW*drho + l234*drhoRdW[var] + ddisInter1dW;
					ddis2dW[var] = dl234dW*drhou + l234*drhoudW + ddisInter1dW*u + disInter1*dudW[var] + ddisInter2dW*n1;
					ddis3dW[var] = dl234dW*drhov + l234*drhovdW + ddisInter1dW*v + disInter1*dvdW[var] + ddisInter2dW*n2;
					ddis5dW[var] = dl234dW*dE + l234*dERdW[var] + ddisInter1dW*H  + disInter1*dHdW[var]
					                                            + ddisInter2dW*Vn + disInter2*dVndW;
				}

				int InddnFdW = 0;
				for (int var = 0; var < NVAR; var++) *dnFdWR_ptr[InddnFdW++]++ = 0.5*(dnF1dW[var]-ddis1dW[var]);
				for (int var = 0; var < NVAR; var++) *dnFdWR_ptr[InddnFdW++]++ = 0.5*(dnF2dW[var]-ddis2dW[var]);
				for (int var = 0; var < NVAR; var++) *dnFdWR_ptr[InddnFdW++]++ = 0.5*(dnF3dW[var]-ddis3dW[var]);
				for (int var = 0; var < NVAR; var++) *dnFdWR_ptr[InddnFdW++]++ = 0.5*(dnF5dW[var]-ddis5dW[var]);
			}
		}
	} else if (DIM == 1) {
		for (ptrdiff_t n = 0; n < NnTotal; n++) {
			Type const n1 = *n_ptr++;

			// Left VOLUME
			Type const rhoL  = *rhoL_ptr++,
			           rhouL = *rhouL_ptr++,
			           EL    = *EL_ptr++,

			           rhoL_inv  = 1.0/rhoL,
			           rhoLr_inv = sqrt_T(rhoL_inv),
			           uL = rhouL*rhoL_inv,

			           V2L = uL*uL,
			           VnL = n1*uL,

			           pL  = GM1*(EL-0.5*rhoL*V2L),
			           HL  = (EL+pL)*rhoL_inv;

			// Right VOLUME
			Type const rhoR  = *rhoR_ptr++,
			           rhouR = *rhouR_ptr++,
			           ER    = *ER_ptr++,

			           rhoR_inv  = 1.0/rhoR,
			           rhoRr_inv = sqrt_T(rhoR_inv),
			           uR = rhouR*rhoR_inv,

			           V2R = uR*uR,
			           VnR = n1*uR,

			           pR  = GM1*(ER-0.5*rhoR*V2R),
			           HR  = (ER+pR)*rhoR_inv;

			// Roe-averaged states
			Type const r   = sqrt_T(rhoR/rhoL),
			           rP1 = r+1.0,

			           rho = r*rhoL,
			           u   = (r*uR+uL)/rP1,
			           H   = (r*HR+HL)/rP1,
			           Vn  = n1*u,
			           V2  = u*u,
			           c2  = GM1*(H-0.5*V2),
			           c   = sqrt_T(c2),

			           Den  = sqrt_T(rhoL) + sqrt_T(rhoR);

			// Compute eigenvalues (with entropy fix)
			unsigned int case_l1;
			Type const l1L     = VnL-c;
			Type       l1      = Vn-c,
			           sign_l1 = 1.0;
			if (abs_T(l1L) < abs_T(l1)) {
				case_l1 = 0;
				if (real_T(l1L) < 0.0)
					sign_l1 = -1.0;
				l1 = sign_l1*l1L;
			} else {
				case_l1 = 1;
				if (real_T(l1) < 0.0)
					sign_l1 = -1.0;
				l1 = sign_l1*l1;
			}

			unsigned int case_l5;
			Type const l5R     = VnR+c;
			Type       l5      = Vn+c,
			           sign_l5 = 1.0;
			if (abs_T(l5R) > abs_T(l5)) {
				case_l5 = 0;
				if (real_T(l5R) < 0.0)
					sign_l5 = -1.0;
				l5 = sign_l5*l5R;
			} else {
				case_l5 = 1;
				if (real_T(l5) < 0.0)
					sign_l5 = -1.0;
				l5 = sign_l5*l5;
			}

			Type sign_l234 = 1.0;
			if (real_T(Vn) < 0.0)
				sign_l234 = -1.0;
			Type const l234 = sign_l234*(Vn);


			Type const drho  = rhoR-rhoL,
			           drhou = rhoR*uR-rhoL*uL,
			           dE    = ER-EL,
			           dp    = pR-pL,
			           dVn   = VnR-VnL,

			           lc1 = 0.5*(l5+l1) - l234,
			           lc2 = 0.5*(l5-l1),

			           disInter1 = lc1*dp/(c*c) + lc2*rho*dVn/c,
			           disInter2 = lc1*rho*dVn  + lc2*dp/c;

			if (nFluxNum != NULL) {
				Type const rhoVnL = rhoL*VnL,
				           rhoVnR = rhoR*VnR,
				           pLR    = pL + pR;

				Type const nF1 = rhoVnL      + rhoVnR,
				           nF2 = rhoVnL*uL   + rhoVnR*uR  + n1*pLR,
				           nF5 = VnL*(EL+pL) + VnR*(ER+pR),

				           dis1 = l234*drho  + disInter1,
				           dis2 = l234*drhou + disInter1*u + disInter2*n1,
				           dis5 = l234*dE    + disInter1*H + disInter2*(Vn);

				// Assemble components
				int IndnF = 0;
				*nF_ptr[IndnF++]++ = 0.5*(nF1 - dis1);
				*nF_ptr[IndnF++]++ = 0.5*(nF2 - dis2);
				*nF_ptr[IndnF++]++ = 0.5*(nF5 - dis5);
			}

			Type dnF1dW[NEQ],  dnF2dW[NEQ],  dnF5dW[NEQ],
			       ddis1dW[NEQ], ddis2dW[NEQ], ddis5dW[NEQ];

			// Flux term
			Type const duLdW[]   = {-uL*rhoL_inv,  rhoL_inv, 0.0},
			           drhoLdW[] = { 1.0,          0.0,      0.0},
			           dELdW[]   = { 0.0,          0.0,      1.0},
			           dpLdW[]   = { GM1*0.5*V2L, -GM1*uL,   GM1};

			Type const rhoVn = rhoL*VnL;
			Type dVnLdW[NVAR];
			for (int var = 0; var < NVAR; var++) {
				dVnLdW[var] = n1*duLdW[var];
				Type const drhoVndW = drhoLdW[var]*VnL + rhoL*dVnLdW[var];

				dnF1dW[var] = drhoVndW;
				dnF2dW[var] = drhoVndW*uL + rhoVn*duLdW[var] + n1*dpLdW[var];
				dnF5dW[var] = dVnLdW[var]*(EL+pL) + VnL*(dELdW[var]+dpLdW[var]);
			}

			// Dissipation term
			Type const mult = rhoLr_inv/Den;
			Type const drhodW[] = { 0.5*rhoR/rho,             0.0,         0.0},
			           dudW[]   = {-0.5*(uL+u)*mult,          mult,        0.0},
			           dHdW[]   = {-0.5*(HL+H-GM1*V2L)*mult, -GM1*uL*mult, GAMMA*mult};

			for (int var = 0; var < NVAR; var++) {
				Type const dcdW  = 0.5*GM1/c*(dHdW[var]-(u*dudW[var])),
				           dVndW = n1*dudW[var];

				Type dl1dW;
				if (case_l1)
					dl1dW = sign_l1*(dVndW-dcdW);
				else
					dl1dW = sign_l1*(dVnLdW[var]-dcdW);

				Type dl5dW;
				if (case_l5)
					dl5dW = sign_l5*(dVndW+dcdW);
				else
					dl5dW = sign_l5*(dcdW);

				Type const dl234dW = sign_l234*dVndW,
				           dlc1dW = 0.5*(dl5dW+dl1dW) - dl234dW,
				           dlc2dW = 0.5*(dl5dW-dl1dW),

				           ddisInter1dW = (dlc1dW*dp-lc1*dpLdW[var])/c2-(2.0*lc1*dp*dcdW)/(c*c2) +
				                          (dlc2dW*rho*dVn+lc2*drhodW[var]*dVn-lc2*rho*dVnLdW[var])/c -
				                          (lc2*rho*dVn*dcdW)/c2,
				           ddisInter2dW = dlc1dW*rho*dVn+lc1*drhodW[var]*dVn-lc1*rho*dVnLdW[var] +
				                          (dlc2dW*dp-lc2*dpLdW[var])/c-(lc2*dp*dcdW)/c2,

				           drhoudW = drhoLdW[var]*uL+rhoL*duLdW[var];

				ddis1dW[var] = dl234dW*drho - l234*drhoLdW[var] + ddisInter1dW;
				ddis2dW[var] = dl234dW*drhou - l234*drhoudW + ddisInter1dW*u + disInter1*dudW[var] + ddisInter2dW*n1;
				ddis5dW[var] = dl234dW*dE - l234*dELdW[var] + ddisInter1dW*H  + disInter1*dHdW[var]
				                                            + ddisInter2dW*Vn + disInter2*dVndW;
			}

			int InddnFdW = 0;
			for (int var = 0; var < NVAR; var++) *dnFdWL_ptr[InddnFdW++]++ = 0.5*(dnF1dW[var]-ddis1dW[var]);
			for (int var = 0; var < NVAR; var++) *dnFdWL_ptr[InddnFdW++]++ = 0.5*(dnF2dW[var]-ddis2dW[var]);
			for (int var = 0; var < NVAR; var++) *dnFdWL_ptr[InddnFdW++]++ = 0.5*(dnF5dW[var]-ddis5dW[var]);

			if (dnFdWR != NULL) {
				// Flux term
				Type const duRdW[]   = {-uR*rhoR_inv,  rhoR_inv, 0.0},
				           drhoRdW[] = { 1.0,          0.0,      0.0},
				           dERdW[]   = { 0.0,          0.0,      1.0},
				           dpRdW[]   = { GM1*0.5*V2R, -GM1*uR,   GM1};

				Type const rhoVn = rhoR*VnR;
				Type dVnRdW[NVAR];
				for (int var = 0; var < NVAR; var++) {
					dVnRdW[var] = n1*duRdW[var];
					Type const drhoVndW = drhoRdW[var]*VnR + rhoR*dVnRdW[var];

					dnF1dW[var] = drhoVndW;
					dnF2dW[var] = drhoVndW*uR + rhoVn*duRdW[var] + n1*dpRdW[var];
					dnF5dW[var] = dVnRdW[var]*(ER+pR) + VnR*(dERdW[var]+dpRdW[var]);
				}

				// Dissipation term
				Type const mult = rhoRr_inv/Den;
				Type const drhodW[] = { 0.5*rhoL/rho,             0.0,         0.0},
				           dudW[]   = {-0.5*(uR+u)*mult,          mult,        0.0},
				           dHdW[]   = {-0.5*(HR+H-GM1*V2R)*mult, -GM1*uR*mult, GAMMA*mult};

				for (int var = 0; var < NVAR; var++) {
					Type const dcdW  = 0.5*GM1/c*(dHdW[var]-(u*dudW[var])),
					           dVndW = n1*dudW[var];

					Type dl1dW;
					if (case_l1)
						dl1dW = sign_l1*(dVndW-dcdW);
					else
						dl1dW = sign_l1*(-dcdW);

					Type dl5dW;
					if (case_l5)
						dl5dW = sign_l5*(dVndW+dcdW);
					else
						dl5dW = sign_l5*(dVnRdW[var]+dcdW);

					Type const dl234dW = sign_l234*dVndW,
					           dlc1dW = 0.5*(dl5dW+dl1dW) - dl234dW,
					           dlc2dW = 0.5*(dl5dW-dl1dW),

					           ddisInter1dW = (dlc1dW*dp+lc1*dpRdW[var])/c2-(2.0*lc1*dp*dcdW)/(c*c2) +
					                          (dlc2dW*rho*dVn+lc2*drhodW[var]*dVn+lc2*rho*dVnRdW[var])/c -
					                          (lc2*rho*dVn*dcdW)/c2,
					           ddisInter2dW = dlc1dW*rho*dVn+lc1*drhodW[var]*dVn+lc1*rho*dVnRdW[var] +
					                          (dlc2dW*dp+lc2*dpRdW[var])/c-(lc2*dp*dcdW)/c2,

					           drhoudW = drhoRdW[var]*uR+rhoR*duRdW[var];

					ddis1dW[var] = dl234dW*drho + l234*drhoRdW[var] + ddisInter1dW;
					ddis2dW[var] = dl234dW*drhou + l234*drhoudW + ddisInter1dW*u + disInter1*dudW[var] + ddisInter2dW*n1;
					ddis5dW[var] = dl234dW*dE + l234*dERdW[var] + ddisInter1dW*H  + disInter1*dHdW[var]
					                                            + ddisInter2dW*Vn + disInter2*dVndW;
				}

				int InddnFdW = 0;
				for (int var = 0; var < NVAR; var++) *dnFdWR_ptr[InddnFdW++]++ = 0.5*(dnF1dW[var]-ddis1dW[var]);
				for (int var = 0; var < NVAR; var++) *dnFdWR_ptr[InddnFdW++]++ = 0.5*(dnF2dW[var]-ddis2dW[var]);
				for (int var = 0; var < NVAR; var++) *dnFdWR_ptr[InddnFdW++]++ = 0.5*(dnF5dW[var]-ddis5dW[var]);
			}
		}
	}
}

// Static functions ************************************************************************************************* //
// Level 0 ********************************************************************************************************** //

static const Type* compute_dmaxV_ds
	(const Type rho_inv, const Type p, const Type V2, const Type V, const Type c, const Type*const uvw)
{
	static Type dmaxV_ds[NVR];

	const Type drho_inv_ds[NVR] = ARRAY_VAR(-rho_inv*rho_inv, 0.0, 0.0, 0.0, 0.0 );
	Type dp_ds[NVR]             = ARRAY_VAR(0.5*V2, -uvw[0], -uvw[1], -uvw[2], 1.0 );
	for (int vr = 0; vr < NVR; ++vr)
		dp_ds[vr] *= GM1;

	const Type duvw_ds[NVR][DIM] = TENSOR_VAR_DIM( -uvw[0]*rho_inv, -uvw[1]*rho_inv, -uvw[2]*rho_inv,
	                                               +rho_inv,         0.0,             0.0,
	                                               +0.0,             rho_inv,         0.0,
	                                               +0.0,             0.0,             rho_inv,
	                                               +0.0,             0.0,             0.0 );
	for (int vr = 0; vr < NVR; ++vr) {
		dmaxV_ds[vr] = 0.0;
		for (int d = 0; d < DIM; ++d)
			dmaxV_ds[vr] += uvw[d]*duvw_ds[vr][d];
		dmaxV_ds[vr] /= V;
		dmaxV_ds[vr] += 0.5*GAMMA/c*(dp_ds[vr]*rho_inv+p*drho_inv_ds[vr]);
	}
	return dmaxV_ds;
}

#include "undef_templates_numerical_flux.h"

#include "undef_templates_multiarray.h"

#include "undef_templates_boundary.h"
#include "undef_templates_const_cast.h"
#include "undef_templates_flux.h"
#include "undef_templates_math_functions.h"
