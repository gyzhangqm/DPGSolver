Include "../Parameters.geo";


// Geometry Specification
rIn = 0.5;
rOut = 1.0;

If (MeshCurving == CURVED)
	Point(1) = {rIn,0,0,lc};
	Point(2) = {rOut,0,0,lc};
	Point(3) = {0,rIn,0,lc};
	Point(4) = {Sqrt(0.5)*rIn,Sqrt(0.5)*rIn,0,lc};
	Point(5) = {0,rOut,0,lc};
	Point(6) = {Sqrt(0.5)*rOut,Sqrt(0.5)*rOut,0,lc};
	Point(7) = {0,0,0,lc};

	Line(1001) = {1,2};
	Line(1002) = {3,5};
	Circle(1003) = {4,7,3};
	Circle(1004) = {4,7,1};
	Circle(1005) = {6,7,5};
	Circle(1006) = {6,7,2};
	Line(1007) = {4,6};
EndIf

Transfinite Line {1001:1004} = 1*2^(MeshLevel)+1 Using Progression 1;
Transfinite Line {1005:1006} = 1*2^(MeshLevel)+1 Using Progression 1;
Transfinite Line {1007}      = 1*2^(MeshLevel)+1 Using Progression 1;


Line Loop (4001) = {1007,1005,-1002,-1003};
Line Loop (4002) = {-1007,1004,1001,-1006};

Plane Surface(4001) = {4001};
Plane Surface(4002) = {4002};

Transfinite Surface{4001};
Transfinite Surface{4002};

If (MeshType == MIXED2D)
	Recombine Surface{4002};
ElseIf (MeshType == QUAD)
	Recombine Surface{4001,4002};
EndIf



// Physical Parameters for '.msh' file
BC_Straight =   BC_STEP_SC;
BC_Curved   = 2*BC_STEP_SC;

If (PDEName == POISSON)
	Physical Line(BC_Straight+BC_DIRICHLET) = {1002};
	Physical Line(BC_Straight+BC_NEUMANN)   = {1001};
	Physical Line(BC_Curved  +BC_DIRICHLET) = {1003:1004};
	Physical Line(BC_Curved  +BC_NEUMANN)   = {1005:1006};
EndIf

Physical Surface(9401) = {4001};
Physical Surface(9402) = {4002};



// Visualization in gmsh

Color Black{ Surface{4001:4002}; }
Geometry.Color.Points = Black;
