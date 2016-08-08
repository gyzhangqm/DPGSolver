// Copyright 2016 Philip Zwanenburg
// MIT License (https://github.com/PhilipZwanenburg/DPGSolver/master/LICENSE)

#include "compute_errors.h"

/*
 *	Purpose:
 *		Compute L2 errors for supported test cases.
 *
 *	Comments:
 *
 *	Notation:
 *
 *	References:
 */

static void output_errors (const double *L2Error, const unsigned int DOF, const double Vol);
static void collect_errors (void);
static char *set_fname(const unsigned int collect);

struct S_OPERATORS {
	unsigned int NvnS, NvnI;
	double       *I_vG_vI, *w_vI, *ChiS_vI;
};

static void init_ops(struct S_OPERATORS *OPS, const struct S_VOLUME *VOLUME, const unsigned int IndClass)
{
	// Standard datatypes
	unsigned int P, type, curved;
	struct S_ELEMENT *ELEMENT, *ELEMENT_OPS;

	P = VOLUME->P;
	type   = VOLUME->type;
	curved = VOLUME->curved;

	ELEMENT = get_ELEMENT_type(type);
	if (1 || type == TRI || type == TET || type == PYR)
		ELEMENT_OPS = ELEMENT;
	else if (type == LINE || type == QUAD || type == HEX || type == WEDGE)
		ELEMENT_OPS = ELEMENT->ELEMENTclass[IndClass];

	OPS->NvnS = ELEMENT_OPS->NvnS[P];
	if (!curved) {
		OPS->NvnI = ELEMENT_OPS->NvnIs[P];

		OPS->I_vG_vI = ELEMENT_OPS->I_vGs_vIs[1][P][0];
		OPS->w_vI    = ELEMENT_OPS->w_vIs[P];
		OPS->ChiS_vI = ELEMENT_OPS->ChiS_vIs[P][P][0];
	} else {
		OPS->NvnI = ELEMENT_OPS->NvnIc[P];

		OPS->I_vG_vI = ELEMENT_OPS->I_vGc_vIc[P][P][0];
		OPS->w_vI    = ELEMENT_OPS->w_vIc[P];
		OPS->ChiS_vI = ELEMENT_OPS->ChiS_vIc[P][P][0];
	}
}

void compute_errors(const struct S_VOLUME *VOLUME, double *L2Error2, double *Vol, unsigned int *DOF,
                    const unsigned int solved)
{
	// Initialize DB Parameters
	unsigned int d    = DB.d,
	             Nvar = DB.Nvar;

	// Standard datatypes
	unsigned int i, iMax, j, NvnS, NvnI, IndU;
	double       *XYZ_vI, 
	             *rho, *p, *s, *sEx, *U, *UEx, *What, *W,
	             *detJV_vI, *w_vI, *ChiS_vI, *wdetJV_vI, err;

	struct S_OPERATORS *OPS;

	OPS = malloc(sizeof *OPS); // free

	for (i = 0, iMax = NVAR3D+1; i < iMax; i++)
		L2Error2[i] = 0.0;

	init_ops(OPS,VOLUME,0);

	NvnS    = OPS->NvnS;
	NvnI    = OPS->NvnI;
	w_vI    = OPS->w_vI;
	ChiS_vI = OPS->ChiS_vI;

	detJV_vI = VOLUME->detJV_vI;

	wdetJV_vI = malloc(NvnI * sizeof *wdetJV_vI); // free
	for (i = 0; i < NvnI; i++)
		wdetJV_vI[i] = w_vI[i]*detJV_vI[i];

	W = malloc(NvnI*Nvar   * sizeof *W); // free
	U = malloc(NvnI*NVAR3D * sizeof *U); // free
	s = malloc(NvnI        * sizeof *s); // free

	What = VOLUME->What;

	mm_CTN_d(NvnI,Nvar,NvnS,ChiS_vI,What,W);

	convert_variables(W,U,d,3,NvnI,1,'c','p');

	rho = &U[NvnI*0];
	p   = &U[NvnI*(NVAR3D-1)];
	for (i = 0; i < NvnI; i++)
		s[i] = p[i]/pow(rho[i],GAMMA);

	XYZ_vI = malloc(NvnI*d * sizeof *XYZ_vI); // free
	mm_CTN_d(NvnI,d,VOLUME->NvnG,OPS->I_vG_vI,VOLUME->XYZ,XYZ_vI);

	UEx = malloc(NvnI*NVAR3D * sizeof *UEx); // free
	sEx = malloc(NvnI        * sizeof *sEx); // free

	compute_exact_solution(NvnI,XYZ_vI,UEx,sEx,solved);
/*
printf("%d\n",VOLUME->indexg);
array_print_d(NvnI,Nvar,U,'C');
array_print_d(NvnI,Nvar,UEx,'C');
array_print_d(NvnI,1,wdetJV_vI,'R');
*/

	for (i = 0; i <= NVAR3D; i++) {
		IndU = i*NvnI;
		if (i == 0 || i == NVAR3D-1) { // rho, p
			for (j = 0; j < NvnI; j++) {
				err = (U[IndU+j]-UEx[IndU+j])/UEx[IndU+j];
				L2Error2[i] += err*err*wdetJV_vI[j];
//if (i == NVAR3D-1)
//	printf("% .3e % .3e % .3e\n",err,err*err*wdetJV_vI[j],L2Error2[i]);
			}
		} else if (i > 0 && i < NVAR3D-1) { // u, v, w (Not normalized as variables may be negative)
			for (j = 0; j < NvnI; j++) {
				err = U[IndU+j]-UEx[IndU+j];
				L2Error2[i] += err*err*wdetJV_vI[j];
			}
		} else if (i == NVAR3D) { // s
			for (j = 0; j < NvnI; j++) {
				err = (s[j]-sEx[j])/sEx[j];
				L2Error2[i] += err*err*wdetJV_vI[j];
			}
		}
	}

	*DOF = NvnS;
	*Vol = 0.0;
	for (i = 0; i < NvnI; i++)
		*Vol += wdetJV_vI[i];

	free(wdetJV_vI);
	free(W);
	free(U);
	free(s);
	free(UEx);
	free(sEx);
	free(XYZ_vI);

	free(OPS);
}

void compute_errors_global(void)
{
	/*
	 *	Purpose:
	 *		Compute and output global L2 solution errors.
	 *
	 *	Comments:
	 *		The VOLUME loop is placed outside of the compute_errors function as the same function is used on a VOLUME
	 *		basis in adapt_initial.
	 */

	// Initialize DB Parameters
	int MPIrank = DB.MPIrank;

	// Standard datatypes
	unsigned int i, DOF, DOF_l;
	double       Vol, Vol_l, *L2Error2, *L2Error2_l;

	struct S_VOLUME *VOLUME;

	// silence
	DOF = 0; Vol = 0.0;

	L2Error2   = calloc(NVAR3D+1   , sizeof *L2Error2);   // free
	L2Error2_l = malloc((NVAR3D+1) * sizeof *L2Error2_l); // free

	for (VOLUME = DB.VOLUME; VOLUME; VOLUME = VOLUME->next) {
		compute_errors(VOLUME,L2Error2_l,&Vol_l,&DOF_l,1);

		Vol += Vol_l;
		DOF += DOF_l;

		for (i = 0; i <= NVAR3D; i++)
			L2Error2[i] += L2Error2_l[i];
	}
	free(L2Error2_l);

	// Write to files and collect
	output_errors(L2Error2,DOF,Vol);
	free(L2Error2);

	MPI_Barrier(MPI_COMM_WORLD);
	if (!MPIrank)
		collect_errors();
}

static void output_errors(const double *L2Error2, const unsigned int DOF, const double Vol)
{
	/*
	 * Comments:
	 *		Squared L2 errors are output here.
	 */

	// standard datatypes
	unsigned int i;
	char         *f_name;

	FILE *fID;

	f_name = set_fname(0); // free
	if ((fID = fopen(f_name,"w")) == NULL)
		printf("Error: File: %s, did not open.\n",f_name), exit(1);

	fprintf(fID,"DOF         Vol         L2rho2      L2u2        L2v2        L2w2        L2p2        L2s2\n");
	fprintf(fID,"%-10d  %.4e  ",DOF,Vol);
	for (i = 0; i <= NVAR3D; i++)
		fprintf(fID,"%.4e  ",L2Error2[i]);

	fclose(fID);
	free(f_name);
}

static void collect_errors(void)
{
	// Initialize DB Parameters
	int MPIsize = DB.MPIsize;

	// Standard datatypes
	char         *f_name, StringRead[STRLEN_MAX], *data;
	int          rank, offset;
	unsigned int i, DOF;
	double       tmp_d, *L2Error2, Vol, *L2Error;

	FILE *fID;

	L2Error2 = calloc(NVAR3D+1 , sizeof *L2Error2); // free
	Vol = 0;
	for (rank = 0; rank < MPIsize; rank++) {
		f_name = set_fname(0); // free
		if ((fID = fopen(f_name,"r")) == NULL)
			printf("Error: File: %s, did not open.\n",f_name), exit(1);

		if (fscanf(fID,"%[^\n]\n",StringRead) == 1) { ; }
		if (fscanf(fID,"%[^\n]\n",StringRead) == 1) {
			i = 0;
			data = StringRead;
			if (sscanf(data," %d%n",&DOF,&offset) == 1)
				data += offset;
			if (sscanf(data," %lf%n",&tmp_d,&offset) == 1) {
				Vol += tmp_d;
				data += offset;
			}
			while (sscanf(data," %lf%n",&tmp_d,&offset) == 1) {
				L2Error2[i++] += tmp_d;
				data += offset;
			}
		}

		fclose(fID);
		free(f_name);
	}

	L2Error = malloc((NVAR3D+1) * sizeof *L2Error); // free
	for (i = 0; i <= NVAR3D; i++)
		L2Error[i] = sqrt(L2Error2[i]/Vol);
	free(L2Error2);

	f_name = set_fname(1); // free
	if ((fID = fopen(f_name,"w")) == NULL)
		printf("Error: File: %s, did not open.\n",f_name), exit(1);

	fprintf(fID,"DOF         L2rho       L2u         L2v         L2w         L2p         L2s\n");
	fprintf(fID,"%-10d  ",DOF);
	for (i = 0; i <= NVAR3D; i++)
		fprintf(fID,"%.4e  ",L2Error[i]);
	free(L2Error);

	fclose(fID);
	free(f_name);
}

static char *set_fname(const unsigned int collect)
{
	// Initialize DB Parameters
	char *TestCase = DB.TestCase,
	     *MeshType = DB.MeshType;
	int  MPIrank = DB.MPIrank;

	// standard datatypes
	char *f_name, string[STRLEN_MAX];

	f_name = malloc(STRLEN_MAX * sizeof *f_name); // keep (requires external free)

	if (!collect)
		strcpy(f_name,"errors/");
	else
		strcpy(f_name,"results/");

	strcat(f_name,TestCase); strcat(f_name,"/");
	strcat(f_name,MeshType); strcat(f_name,"/");
	if (!collect)
		strcat(f_name,"L2errors2_");
	else
		strcat(f_name,"L2errors_");
	sprintf(string,"%dD_",DB.d);   strcat(f_name,string);
	                               strcat(f_name,MeshType);
	sprintf(string,"_ML%d",DB.ML); strcat(f_name,string);
	if (DB.Adapt == ADAPT_0)
		sprintf(string,"P%d",DB.PGlobal), strcat(f_name,string);
	if (!collect)
		sprintf(string,"_%d",MPIrank), strcat(f_name,string);
	strcat(f_name,".txt");

	return f_name;
}