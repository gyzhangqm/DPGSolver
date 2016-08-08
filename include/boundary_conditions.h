// Copyright 2016 Philip Zwanenburg
// MIT License (https://github.com/PhilipZwanenburg/DPGSolver/master/LICENSE)

#ifndef DPG__boundary_conditions_h__INCLUDED
#define DPG__boundary_conditions_h__INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "Database.h"
#include "Parameters.h"

#include "variable_functions.h"


extern void boundary_Riemann  (const unsigned int Nn, const unsigned int Nel, double *XYZ, double *WL, double *WOut,
                               double *WB, double *nL);
extern void boundary_SlipWall (const unsigned int Nn, const unsigned int Nel, double *WL, double *WB, double *nL);

#endif // DPG__boundary_conditions_h__INCLUDED