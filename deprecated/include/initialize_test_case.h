// Copyright 2017 Philip Zwanenburg
// MIT License (https://github.com/PhilipZwanenburg/DPGSolver/blob/master/LICENSE)

#ifndef DPG__initialize_test_case_h__INCLUDED
#define DPG__initialize_test_case_h__INCLUDED

extern void initialize_test_case            (const unsigned int adapt_update_MAX);
extern void initialize_test_case_parameters (void);
extern void compute_solution                (const unsigned int Nn, double *XYZ, double *UEx, const unsigned int solved);

#endif // DPG__initialize_test_case_h__INCLUDED
