#include <stdlib.h>

#include "database.h"
#include "functions.h"

/*
 *	Purpose:
 *		Free memory of various structures.
 *
 *	Comments:
 *
 *	Notation:
 *
 *	Memory freed:
 *
 *	References:
 *
 */

void memory_destructor_E(struct S_ELEMENT *ELEMENT)
{
	int NP = DB.NP;

	// Mesh
	free(ELEMENT->Nfve);
	free(ELEMENT->VeC);
	free(ELEMENT->VeE);
	free(ELEMENT->VeF);

	// Operators

	// Normals
	free(ELEMENT->nr);

	// VOLUME Nodes
	array_free2_d(1 ,ELEMENT->rst_vGs);
	array_free2_d(NP,ELEMENT->rst_vGc);
	array_free2_d(NP,ELEMENT->rst_vCs);
	array_free2_d(NP,ELEMENT->rst_vCc);
	array_free2_d(NP,ELEMENT->rst_vJs);
	array_free2_d(NP,ELEMENT->rst_vJc);
	array_free2_d(NP,ELEMENT->rst_vS);
	array_free2_d(NP,ELEMENT->rst_vF);
	array_free2_d(NP,ELEMENT->rst_vFrs);
	array_free2_d(NP,ELEMENT->rst_vFrc);
	array_free2_d(NP,ELEMENT->rst_vIs);
	array_free2_d(NP,ELEMENT->rst_vIc);
	array_free2_d(1 ,ELEMENT->rst_vP);

	array_free2_d(NP,ELEMENT->wvIs);
	array_free2_d(NP,ELEMENT->wvIc);

	array_free2_ui(1 ,ELEMENT->Con_rst_vP);

	free(ELEMENT->NvnGs);
	free(ELEMENT->NvnGc);
	free(ELEMENT->NvnCs);
	free(ELEMENT->NvnCc);
	free(ELEMENT->NvnJs);
	free(ELEMENT->NvnJc);
	free(ELEMENT->NvnS);
	free(ELEMENT->NvnF);
	free(ELEMENT->NvnFrs);
	free(ELEMENT->NvnFrc);
	free(ELEMENT->NvnIs);
	free(ELEMENT->NvnIc);
	free(ELEMENT->NvnP);

	// FACET Nodes
	array_free3_d(NP,ELEMENT->Nf,ELEMENT->rst_fGc);
	array_free3_d(NP,ELEMENT->Nf,ELEMENT->rst_fIs);
	array_free3_d(NP,ELEMENT->Nf,ELEMENT->rst_fIc);

	array_free2_d(NP,ELEMENT->wfIs);
	array_free2_d(NP,ELEMENT->wfIc);

	free(ELEMENT->NfnGc);
	free(ELEMENT->NfnIs);
	free(ELEMENT->NfnIc);

	// Operators
	array_free2_d(NP,ELEMENT->I_vGs_vGc);

	free(ELEMENT);
}

void memory_destructor_V(struct S_VOLUME *VOLUME)
{
//	int NP = DB.NP;

	// Structures

	// Geometry
	free(VOLUME->XYZc);
	free(VOLUME->XYZs);

	free(VOLUME);
}