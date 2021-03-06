// Copyright 2017 Philip Zwanenburg
// MIT License (https://github.com/PhilipZwanenburg/DPGSolver/blob/master/LICENSE)

#ifndef DPG__compute_errors_h__INCLUDED
#define DPG__compute_errors_h__INCLUDED

#include "S_VOLUME.h"


extern void compute_errors        (struct S_VOLUME *VOLUME, double *L2Error2, double *Vol, unsigned int *DOF, const unsigned int solved);
extern void compute_errors_global (void);

#endif // DPG__compute_errors_h__INCLUDED
