// Copyright 2017 Philip Zwanenburg
// MIT License (https://github.com/PhilipZwanenburg/DPGSolver/blob/master/LICENSE)

#ifndef DPG__solver_functions_h__INCLUDED
#define DPG__solver_functions_h__INCLUDED

extern void get_face_ordering (const unsigned int d, const unsigned int IndOrd, const unsigned int FType,
                                const unsigned int Ns, const unsigned int Nn, const unsigned int *symms,
                                const double *rst, unsigned int *nOrd);

#endif // DPG__solver_functions_h__INCLUDED
