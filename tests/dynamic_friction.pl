$bin_dir = "$ENV{srcdir}/../bin";
$tmp = `$bin_dir/createmp elas.param`;

sub catch { `rm -f $tmp`; exit(1); }
$SIG{INT} = 'catch';

open(TMPF, ">$tmp") or die "Open file impossible : $!\n";
print TMPF <<
LX = 1.0;		% size in X.
LY = 1.0;	        % size in Y.
LZ = 1.0;		% size in Z.
MU = 30;
LAMBDA = 55;
FRICTION_COEF = 0.0;    % Friction coefficient.
PG = 9810; 		% constante de gravitation (sur terre) (en mm/s^2).
RHO = 6e-6;     	% densite massique "realiste" pour l'acier
T = 0.05;
DT = 0.001;             % Time step
MESH_TYPE = 'GT_PK(2,1)';         % linear triangles
NX = 10;            	          % space step.
MESH_NOISE = 0;         % Set to one if you want to "shake" the mesh
RESIDUAL = 1E-9;     	% residu for Newton.
NOISY = 0;
SCHEME = 3; % 0 = theta-method, 1 = Newmark, 2 = middle point
            % 3 = middle point with modified contact forces
THETA=1.0;
BETA=1.0;
GAMMA=0.5;
RESTIT = 1.0;           % Restitution coefficient for Paoli scheme
NOCONTACT_MASS = 0;     % Suppress or not the mass of contact nodes
PERIODICITY=0;          % Periodic condition
DT_ADAPT = 0;           % Time step adaptation regarding the energy
R = 100.0;              % Augmentation parameter
DIRICHLET = 0;
DIRICHLET_RATIO = -0.2	   % parametre pour la condition de Dirichlet
INIT_VERT_SPEED = -100.0;  % Initial vertical velocity
INIT_VERT_POS = 1.0;       % Initial vertical position
FOUNDATION_HSPEED = 0.;    % Horizontal velocity of the rigid foundation
STATIONARY = 0;            % Initial condition is the stationary solution ?
PERT_STATIONARY = 0.0;     % Perturbation on the initial velocity
FEM_TYPE = 'FEM_PK(2, 1)';     % Main FEM
DATA_FEM_TYPE = 'FEM_PK(2,1)'; % must be defined for non-Lagrangian main FEM
INTEGRATION = 'IM_TRIANGLE(6)'; % Quadrature rule
% MESHNAME='splx:';
% MESHNAME='donut_regulier_8_elements_288ddl.mesh';
% MESHNAME='donut_regulier_64_elements_1920ddl.mesh';
% MESHNAME='donut_regulier_512_elements_13824ddl.mesh';
% MESHNAME='donut_regulier_32_elements.mesh';
% MESHNAME='donut_regulier_72_elements.mesh';
% MESHNAME='donut_regulier_128_elements.mesh';
% MESHNAME='donut_regulier_200_elements.mesh';
% MESHNAME='donut_regulier_288_elements.mesh';
% MESHNAME='donut_regulier_392_elements.mesh';
% MESHNAME='donut_regulier_512_elements.mesh';
% MESHNAME='donut_regulier_648_elements.mesh';
% MESHNAME='donut_regulier_800_elements.mesh';
% MESHNAME='disc_P2_h11.mesh';
% MESHNAME='disc_P2_h8.mesh';
% MESHNAME='disc_P2_h6.mesh';
MESHNAME='meshes/disc_P2_h4.mesh';
% MESHNAME='disc_P2_h2.mesh'; 
% MESHNAME='disc_P2_h1.mesh';
% MESHNAME='disc_P2_h0.5.mesh';
% MESHNAME='disc_P2_h0.3.mesh';
ROOTFILENAME = 'dynamic_friction';     % Root of data files.
DX_EXPORT = 0 % export solution to an OpenDX file ?
DT_EXPORT = 0.001; % Time step for the export

;
close(TMPF);



$er = 0;
open F, "./dynamic_friction $tmp 2>&1 |" or die;
while (<F>) {
#  print $_;
  if ($_ =~ /error has been detected/)
  {
    $er = 1;
    print "============================================\n";
    print $_, <F>;
  }
}
close(F); if ($?) { `rm -f $tmp`; exit(1); }
if ($er == 1) { `rm -f $tmp`; exit(1); }
`rm -f $tmp`;


