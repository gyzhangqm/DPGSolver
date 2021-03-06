// Modifiable Parameters

Refine = 0;

lc = 0.4/2^Refine;

rIn = 0.5;
rOut = 1.0;



// Geometry Specification

Point(1) = {rIn,0,0,lc};
Point(2) = {rOut,0,0,lc};
Point(3) = {0,rIn,0,lc};
Point(4) = {rIn,rIn,0,lc};
Point(5) = {0,rOut,0,lc};
Point(6) = {rOut,rOut,0,lc};

Line(1001) = {1,2};
Line(1002) = {3,5};
Line(1003) = {4,3};
Line(1004) = {4,1};
Line(1005) = {6,5};
Line(1006) = {6,2};
Line(1007) = {4,6};

Transfinite Line {1001:1004} = 1*2^(Refine)+1 Using Progression 1;
Transfinite Line {1005:1006} = 1*2^(Refine)+1 Using Progression 1;
Transfinite Line {1007}      = 1*2^(Refine)+1 Using Progression 1;

Line Loop (4001) = {1007,1005,-1002,-1003};
Line Loop (4002) = {-1007,1004,1001,-1006};

Plane Surface(4001) = {4001};
Plane Surface(4002) = {4002};

Transfinite Surface{4001};
Transfinite Surface{4002};

//Recombine Surface{4002};



// Physical Parameters for '.msh' file

Physical Line(10011) = {1002}; // Straight Dirichlet
Physical Line(10012) = {1001}; // Straight Neumann
Physical Line(20011) = {1003:1004}; // Curved Dirichlet
Physical Line(20012) = {1005:1006}; // Curved Neumann

Physical Surface(9401) = {4001};
Physical Surface(9402) = {4002};



// Visualization in gmsh

Color Black{ Surface{4001:4002}; }
Geometry.Color.Points = Black;
