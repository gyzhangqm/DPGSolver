Include "../Parameters.geo";


// Geometry Specification
L = 1;
H = 1;

Point(1) = {-L,-H,-0,lc};
Point(2) = {+L,-H,-0,lc};
Point(3) = {-L,+H,-0,lc};
Point(4) = {+L,+H,-0,lc};

Line(1001) = {1,2};
Line(1002) = {3,4};
Line(2001) = {1,3};
Line(2002) = {2,4};

Transfinite Line{1001:1002} = 2^(MeshLevel)+1 Using Progression 1;
Transfinite Line{2001:2002} = 2^(MeshLevel)+1 Using Progression 1;

Line Loop (4001) = {1001,2002,-1002,-2001};

Plane Surface(4001) = {4001};

Transfinite Surface {4001};
If (MeshType == QUAD)
	Recombine Surface{4001};
EndIf



// Physical parameters for '.msh' file
BC_Straight =   BC_STEP_SC;
BC_Curved   = 2*BC_STEP_SC;
If (MeshCurving == STRAIGHT)
	BC_Base = BC_Straight;
Else
	BC_Base = BC_Curved;
EndIf

If (PDEName == POISSON)
	Physical Line(BC_Base+BC_DIRICHLET) = {1001:1002,2001:2002};
ElseIf (PDEName == EULER)
	Physical Point(BC_Base+PERIODIC_XL) = {1,3};
	Physical Point(BC_Base+PERIODIC_XR) = {2,4};
	Physical Point(BC_Base+PERIODIC_YL) = {1,2};
	Physical Point(BC_Base+PERIODIC_YR) = {3,4};

	Physical Line(BC_Base+PERIODIC_XL) = {2001};
	Physical Line(BC_Base+PERIODIC_XR) = {2002};
	Physical Line(BC_Base+PERIODIC_YL) = {1001};
	Physical Line(BC_Base+PERIODIC_YR) = {1002};

	// Periodic Indicator (Slave = Master)

	Periodic Line {2002} = {2001}; // Periodic (x)
	Periodic Line {1002} = {1001}; // Periodic (y)
EndIf

Physical Surface(9401) = 4001;



// Visualization in gmsh

Color Black{ Surface{4001}; }
Geometry.Color.Points = Black;