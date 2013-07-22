// Gmsh project created on Thu Mar 20 14:44:49 2008
Point(1) = {0,0,0,14};
Point(2) = {0.5,0,0,14};
Point(3) = {-0.5,0,0,14};
Point(4) = {0,0.5,0,14};
Point(5) = {0,-0.5,0,14};
Point(6) = {0.5,0,3,14};
Point(7) = {-0.5,0,3,14};
Point(8) = {0,0,3,14};
Point(9) = {0,0.5,3,14};
Point(10) = {0,-0.5,3,14};

Point(11) = {-10,-10,0,4};
Point(12) = {-10,-10,3,4};
Point(13) = {-10,10,3,4};
Point(14) = {-10,10,0,4};
Point(15) = {20,10,0,4};
Point(16) = {20,10,3,4};
Point(17) = {20,-10,3,4};
Point(18) = {20,-10,0,4};


Line(1) = {14,15};
Line(2) = {15,18};
Line(3) = {18,11};
Line(4) = {11,14};
Line(5) = {13,16};
Line(6) = {16,17};
Line(7) = {17,12};
Line(8) = {12,13};
Line(9) = {14,13};
Line(10) = {15,16};
Line(11) = {18,17};
Line(12) = {11,12};
Line(13) = {3,7};
Line(14) = {2,6};
Line(15) = {4,9};
Line(16) = {5,10};

Circle(17) = {4,1,3};
Circle(18) = {3,1,5};
Circle(19) = {5,1,2};
Circle(20) = {2,1,4};
Circle(21) = {9,8,7};
Circle(22) = {7,8,10};
Circle(23) = {10,8,6};
Circle(24) = {6,8,9};

Line Loop(27) = {5,6,7,8};
Line Loop(28) = {21,22,23,24};
Plane Surface(29) = {27,28};
Line Loop(30) = {1,2,3,4};
Line Loop(31) = {17,18,19,20};
Plane Surface(32) = {30,31};
Line Loop(33) = {9,-8,-12,4};
Plane Surface(34) = {33};
Line Loop(35) = {2,11,-6,-10};
Plane Surface(36) = {35};
Line Loop(37) = {10,-5,-9,1};
Plane Surface(38) = {37};
Line Loop(39) = {3,12,-7,-11};
Plane Surface(40) = {39};
Line Loop(41) = {17,13,-21,-15};
Ruled Surface(42) = {41};
Line Loop(43) = {13,22,-16,-18};
Ruled Surface(44) = {43};
Line Loop(45) = {16,23,-14,-19};
Ruled Surface(46) = {45};
Line Loop(47) = {14,24,-15,-20};
Ruled Surface(48) = {47};
Physical Surface(49) = {29,32,38,36,40,34};
Physical Surface(50) = {42,44,46,48};
Surface Loop(51) = {29,38,36,32,40,34,48,46,44,42};
Volume(52) = {51};
Physical Volume(53) = {52};
Recombine Surface {29};
Characteristic Length {6} = 1;
Characteristic Length {9} = 1;
Recombine Surface {29};
Characteristic Length {6} = 12;
Characteristic Length {6} = 12;
