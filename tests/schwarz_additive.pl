# Copyright (C) 2001-2020 Yves Renard
#
# This file is a part of GetFEM
#
# GetFEM  is  free software;  you  can  redistribute  it  and/or modify it
# under  the  terms  of the  GNU  Lesser General Public License as published
# by  the  Free Software Foundation;  either version 3 of the License,  or
# (at your option) any later version along with the GCC Runtime Library
# Exception either version 3.1 or (at your option) any later version.
# This program  is  distributed  in  the  hope  that it will be useful,  but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or  FITNESS  FOR  A PARTICULAR PURPOSE.  See the GNU Lesser General Public
# License and GCC Runtime Library Exception for more details.
# You  should  have received a copy of the GNU Lesser General Public License
# along  with  this program;  if not, write to the Free Software Foundation,
# Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301, USA.

$bin_dir = "$ENV{srcdir}/../bin";
$tmp = `$bin_dir/createmp schw.param`;

sub catch { `rm -f $tmp`; exit(1); }
$SIG{INT} = 'catch';

open(TMPF, ">$tmp") or die "Open file impossible : $!\n";
print TMPF <<""
N = 3;                  % dimension.
PG = 9.81;		% gravity constant.
RHO = 0.1;     	        % mass density
MU = 1.0;	        % shear elastic stiffness.
LAMBDA = 1.0;   	% elastic stiffness.
LX = 1.0;		% size in X.
LY = 1.0;	        % size in Y.
LZ = 1.0;		% size in Z.
D = 0.00;		% Dirichlet condition.
%%%%%   discretisation parameters :        			      %%%%%
K = 1;         % Degree of the finite element method.
NX = 10;       % space step.
NXCOARSE = 4;  % space step for the coarse mesh.
USECOARSE = 1; % use a coarse mesh or not.
RESIDUAL = 1E-7;  %
SOLVER = 3;     % 0 = C.G.
		% 1 = additive Schwarz with global and local CG
		% 2 = additive Schwarz with global et local Gmres
SUBDOMSIZE = 0.2;
OVERLAP = 0.0; % overlap between sub-domains in %
MESHNAME = '';

;
close(TMPF);


$er = 0;
open F, "./schwarz_additive $tmp 2>&1 |" or die;
while (<F>) {
  # print $_;
  if ($_ =~ /error has been detected/)
  {
    $er = 1;
    print " =============================================================\n";
    print $_, <F>;
  }
}
close(F); if ($?) { exit(1); }
if ($er == 1) { exit(1); }
`rm -f $tmp`;


