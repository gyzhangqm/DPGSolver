// Copyright 2016 Philip Zwanenburg
// MIT License (https://github.com/PhilipZwanenburg/DPGSolver/master/LICENSE)

#ifndef DPG__fluxes_inviscid_c_h__INCLUDED
#define DPG__fluxes_inviscid_c_h__INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>

#include "Parameters.h"


extern void flux_inviscid_c (const unsigned int Nn, const unsigned int Nel, complex double *W, complex double *F,
                             const unsigned int d, const unsigned int Neq);

#endif // DPG__fluxes_inviscid_c_h__INCLUDED