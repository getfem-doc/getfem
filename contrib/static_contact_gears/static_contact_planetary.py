#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Python GetFEM++ interface
#
# Copyright (C) 2010-2020 Konstantinos Poulios.
#
# This file is a part of GetFEM++
#
# GetFEM++  is  free software;  you  can  redistribute  it  and/or modify it
# under  the  terms  of the  GNU  Lesser General Public License as published
# by  the  Free Software Foundation;  either version 2.1 of the License,  or
# (at your option) any later version.
# This program  is  distributed  in  the  hope  that it will be useful,  but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or  FITNESS  FOR  A PARTICULAR PURPOSE.  See the GNU Lesser General Public
# License for more details.
# You  should  have received a copy of the GNU Lesser General Public License
# along  with  this program;  if not, write to the Free Software Foundation,
# Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301, USA.
#
############################################################################
"""  This example computes a planetary gear model incorporating different
     contact mechanisms like contact with a rigid obstacle and contact
     between elastic bodies of non matching meshes.

     This program is used to check that python-getfem is working. This is
     also a good example of use of GetFEM++.
"""
from math import cos, pi, sin

from getfem import *

# mesh import
m_1 = Mesh('import', 'gmsh', './static_contact_planetary_1.msh')
m_2 = Mesh('import', 'gmsh', './static_contact_planetary_2.msh')
m_p1 = Mesh('import', 'gmsh', './static_contact_planetary_3.msh')
m_p2 = Mesh('import', 'gmsh', './static_contact_planetary_4.msh')
m_p3 = Mesh('import', 'gmsh', './static_contact_planetary_5.msh')

z_1 = 20
z_2 = -64
z_p = 22

a = 99.
R_i = 31.

#rot_angle = 2e-2
torsion = 1000.e3

Lambda = 1.18e5
Mu = 0.83e5

qdim = 2
degree = 1

contact_algo = 1

# displacement meshfems
mfu_1 = MeshFem(m_1, qdim)
mfu_2 = MeshFem(m_2, qdim)
mfu_p1 = MeshFem(m_p1, qdim)
mfu_p2 = MeshFem(m_p2, qdim)
mfu_p3 = MeshFem(m_p3, qdim)

mfu_1.set_fem(Fem('FEM_QK(2,%d)' % (degree,)))
mfu_2.set_fem(Fem('FEM_QK(2,%d)' % (degree,)))
mfu_p1.set_fem(Fem('FEM_QK(2,%d)' % (degree,)))
mfu_p2.set_fem(Fem('FEM_QK(2,%d)' % (degree,)))
mfu_p3.set_fem(Fem('FEM_QK(2,%d)' % (degree,)))

# rhs meshfems
mfrhs_1 = MeshFem(m_1, 1)
mfrhs_2 = MeshFem(m_2, 1)
mfrhs_p1 = MeshFem(m_p1, 1)
mfrhs_p2 = MeshFem(m_p2, 1)
mfrhs_p3 = MeshFem(m_p3, 1)

mfrhs_1.set_fem(Fem('FEM_QK(2,%d)' % (degree,)))
mfrhs_2.set_fem(Fem('FEM_QK(2,%d)' % (degree,)))
mfrhs_p1.set_fem(Fem('FEM_QK(2,%d)' % (degree,)))
mfrhs_p2.set_fem(Fem('FEM_QK(2,%d)' % (degree,)))
mfrhs_p3.set_fem(Fem('FEM_QK(2,%d)' % (degree,)))

# integration methods
mim_1 = MeshIm(m_1, Integ('IM_QUAD(2)'))
mim_2 = MeshIm(m_2, Integ('IM_QUAD(2)'))
mim_p1 = MeshIm(m_p1, Integ('IM_QUAD(2)'))
mim_p2 = MeshIm(m_p2, Integ('IM_QUAD(2)'))
mim_p3 = MeshIm(m_p3, Integ('IM_QUAD(2)'))

# regions definitions for boundary conditions
RG_NEUMANN_1 = 1
RG_NEUMANN_2 = 2
RG_NEUMANN_p1 = 3
RG_NEUMANN_p2 = 4
RG_NEUMANN_p3 = 5

RG_DIRICHLET_1 = 10
RG_DIRICHLET_2 = 20
RG_CONTACT_p1 = 30
RG_CONTACT_p2 = 40
RG_CONTACT_p3 = 50

RG_CONTACT_1_p1 = 13
RG_CONTACT_1_p2 = 14
RG_CONTACT_1_p3 = 15

RG_CONTACT_2_p1 = 23
RG_CONTACT_2_p2 = 24
RG_CONTACT_2_p3 = 25

RG_CONTACT_p1_1 = 31
RG_CONTACT_p1_2 = 32

RG_CONTACT_p2_1 = 41
RG_CONTACT_p2_2 = 42

RG_CONTACT_p3_1 = 51
RG_CONTACT_p3_2 = 52

for i in range(1, z_1 + 1):
   m_1.set_region(RG_NEUMANN_1, m_1.region(100043+100*i))
   m_1.set_region(RG_NEUMANN_1, m_1.region(100083+100*i))

for i in range(1, abs(z_2) + 1):
   m_2.set_region(RG_DIRICHLET_2, m_2.region(200043+100*i))
   m_2.set_region(RG_DIRICHLET_2, m_2.region(200083+100*i))

for i in range(1, z_p + 1):
   m_p1.set_region(RG_CONTACT_p1, m_p1.region(300043+100*i))
   m_p1.set_region(RG_CONTACT_p1, m_p1.region(300083+100*i))

   m_p2.set_region(RG_CONTACT_p2, m_p2.region(400043+100*i))
   m_p2.set_region(RG_CONTACT_p2, m_p2.region(400083+100*i))

   m_p3.set_region(RG_CONTACT_p3, m_p3.region(500043+100*i))
   m_p3.set_region(RG_CONTACT_p3, m_p3.region(500083+100*i))

m_1.set_region(RG_CONTACT_1_p1, m_1.region(100053+100*1))
m_1.set_region(RG_CONTACT_1_p1, m_1.region(100053+100*z_1))

m_1.set_region(RG_CONTACT_1_p2, m_1.region(100053+100*7))
m_1.set_region(RG_CONTACT_1_p2, m_1.region(100053+100*8))

m_1.set_region(RG_CONTACT_1_p3, m_1.region(100053+100*13))
m_1.set_region(RG_CONTACT_1_p3, m_1.region(100053+100*14))
m_1.set_region(RG_CONTACT_1_p3, m_1.region(100053+100*15))

m_2.set_region(RG_CONTACT_2_p1, m_2.region(200053+100*1))
m_2.set_region(RG_CONTACT_2_p1, m_2.region(200053+100*2))
m_2.set_region(RG_CONTACT_2_p1, m_2.region(200053+100*abs(z_2)))

m_2.set_region(RG_CONTACT_2_p2, m_2.region(200053+100*21))
m_2.set_region(RG_CONTACT_2_p2, m_2.region(200053+100*22))
m_2.set_region(RG_CONTACT_2_p1, m_2.region(200053+100*23))

m_2.set_region(RG_CONTACT_2_p3, m_2.region(200053+100*42))
m_2.set_region(RG_CONTACT_2_p3, m_2.region(200053+100*43))
m_2.set_region(RG_CONTACT_2_p3, m_2.region(200053+100*44))

m_p1.set_region(RG_CONTACT_p1_1, m_p1.region(300053+100*12))
m_p1.set_region(RG_CONTACT_p1_1, m_p1.region(300053+100*13))

m_p1.set_region(RG_CONTACT_p1_2, m_p1.region(300013+100*1))
m_p1.set_region(RG_CONTACT_p1_2, m_p1.region(300013+100*2))
m_p1.set_region(RG_CONTACT_p1_2, m_p1.region(300013+100*3))

m_p2.set_region(RG_CONTACT_p2_1, m_p2.region(400053+100*12))
m_p2.set_region(RG_CONTACT_p2_1, m_p2.region(400053+100*13))

m_p2.set_region(RG_CONTACT_p2_2, m_p2.region(400013+100*1))
m_p2.set_region(RG_CONTACT_p2_2, m_p2.region(400013+100*2))
m_p2.set_region(RG_CONTACT_p2_2, m_p2.region(400013+100*3))

m_p3.set_region(RG_CONTACT_p3_1, m_p3.region(500053+100*12))
m_p3.set_region(RG_CONTACT_p3_1, m_p3.region(500053+100*13))

m_p3.set_region(RG_CONTACT_p3_2, m_p3.region(500013+100*1))
m_p3.set_region(RG_CONTACT_p3_2, m_p3.region(500013+100*2))
m_p3.set_region(RG_CONTACT_p3_2, m_p3.region(500013+100*3))

if contact_algo != 0:
   RG_CONTACT_TOTAL_1 = 16
   RG_CONTACT_TOTAL_2 = 26
   RG_CONTACT_TOTAL_p1 = 33
   RG_CONTACT_TOTAL_p2 = 43
   RG_CONTACT_TOTAL_p3 = 53
   m_1.set_region(RG_CONTACT_TOTAL_1, m_1.region(RG_CONTACT_1_p1))
   m_1.set_region(RG_CONTACT_TOTAL_1, m_1.region(RG_CONTACT_1_p2))
   m_1.set_region(RG_CONTACT_TOTAL_1, m_1.region(RG_CONTACT_1_p3))
   m_2.set_region(RG_CONTACT_TOTAL_2, m_2.region(RG_CONTACT_2_p1))
   m_2.set_region(RG_CONTACT_TOTAL_2, m_2.region(RG_CONTACT_2_p2))
   m_2.set_region(RG_CONTACT_TOTAL_2, m_2.region(RG_CONTACT_2_p3))
   m_p1.set_region(RG_CONTACT_TOTAL_p1, m_p1.region(RG_CONTACT_p1))
   m_p1.set_region(RG_CONTACT_TOTAL_p1, m_p1.region(RG_CONTACT_p1_1))
   m_p1.set_region(RG_CONTACT_TOTAL_p1, m_p1.region(RG_CONTACT_p1_2))
   m_p2.set_region(RG_CONTACT_TOTAL_p2, m_p2.region(RG_CONTACT_p2))
   m_p2.set_region(RG_CONTACT_TOTAL_p2, m_p2.region(RG_CONTACT_p2_1))
   m_p2.set_region(RG_CONTACT_TOTAL_p2, m_p2.region(RG_CONTACT_p2_2))
   m_p3.set_region(RG_CONTACT_TOTAL_p3, m_p3.region(RG_CONTACT_p3))
   m_p3.set_region(RG_CONTACT_TOTAL_p3, m_p3.region(RG_CONTACT_p3_1))
   m_p3.set_region(RG_CONTACT_TOTAL_p3, m_p3.region(RG_CONTACT_p3_2))

# model definition
model=Model('real')
model.add_fem_variable('u_1', mfu_1)
model.add_fem_variable('u_2', mfu_2)
model.add_fem_variable('u_p1', mfu_p1)
model.add_fem_variable('u_p2', mfu_p2)
model.add_fem_variable('u_p3', mfu_p3)

if contact_algo == 0:
   model.add_initialized_data('lambda', Lambda)
   model.add_initialized_data('mu', Mu)
   model.add_isotropic_linearized_elasticity_brick(mim_1, 'u_1', 'lambda', 'mu')
   model.add_isotropic_linearized_elasticity_brick(mim_2, 'u_2', 'lambda', 'mu')
   model.add_isotropic_linearized_elasticity_brick(mim_p1, 'u_p1', 'lambda', 'mu')
   model.add_isotropic_linearized_elasticity_brick(mim_p2, 'u_p2', 'lambda', 'mu')
   model.add_isotropic_linearized_elasticity_brick(mim_p3, 'u_p3', 'lambda', 'mu')
else:
   elast_law = 'SaintVenant Kirchhoff'
   model.add_initialized_data('elast_params', [Lambda, Mu])
   model.add_nonlinear_elasticity_brick(mim_1, 'u_1', elast_law, 'elast_params')
   model.add_nonlinear_elasticity_brick(mim_2, 'u_2', elast_law, 'elast_params')
   model.add_nonlinear_elasticity_brick(mim_p1, 'u_p1', elast_law, 'elast_params')
   model.add_nonlinear_elasticity_brick(mim_p2, 'u_p2', elast_law, 'elast_params')
   model.add_nonlinear_elasticity_brick(mim_p3, 'u_p3', elast_law, 'elast_params')

#F = mfrhs_1.eval('-y*%e,x*%e' % (rot_angle,rot_angle) )
#model.add_initialized_fem_data('dirichlet_1', mfrhs_1, F)
model.add_initialized_data('dirichlet_2', [0.,0.])
#model.add_Dirichlet_condition_with_multipliers(mim_1, 'u_1', mfu_1, RG_DIRICHLET_1, 'dirichlet_1')
model.add_Dirichlet_condition_with_multipliers(mim_2, 'u_2', mfu_2, RG_DIRICHLET_2, 'dirichlet_2')

M = torsion / size(mfrhs_1.basic_dof_on_region(RG_NEUMANN_1))
F = mfrhs_1.eval('-y*%e/(x**2+y**2),x*%e/(x**2+y**2)' % (M, M) )
model.add_initialized_fem_data('neumann_1', mfrhs_1, F)
model.add_source_term_brick(mim_1, 'u_1', 'neumann_1', RG_NEUMANN_1)

model.add_initialized_data('penalty_param', 1e0)
model.add_mass_brick(mim_1, 'u_1', 'penalty_param')
model.add_mass_brick(mim_p1, 'u_p1', 'penalty_param')
model.add_mass_brick(mim_p2, 'u_p2', 'penalty_param')
model.add_mass_brick(mim_p3, 'u_p3', 'penalty_param')

bearing_p1 = 'sqrt((x-(%e))^2+(y-(%e))^2)-(%e)' % (0., a, R_i)
bearing_p2 = 'sqrt((x-(%e))^2+(y-(%e))^2)-(%e)' % (a*cos(7*pi/6), a*sin(7*pi/6), R_i)
bearing_p3 = 'sqrt((x-(%e))^2+(y-(%e))^2)-(%e)' % (a*cos(11*pi/6), a*sin(11*pi/6), R_i)

if contact_algo == 0:
   model.add_initialized_data( 'r', Mu * (3*Lambda + 2*Mu) / (Lambda + Mu) )
   model.add_nodal_contact_between_nonmatching_meshes_brick(mim_1, mim_p1, 'u_1', 'u_p1', 'lambda_1_p1_n', 'r', RG_CONTACT_1_p1, RG_CONTACT_p1_1)
   model.add_nodal_contact_between_nonmatching_meshes_brick(mim_p1, mim_2, 'u_p1', 'u_2', 'lambda_p1_2_n', 'r', RG_CONTACT_p1_2, RG_CONTACT_2_p1)
   model.add_nodal_contact_between_nonmatching_meshes_brick(mim_1, mim_p2, 'u_1', 'u_p2', 'lambda_1_p2_n', 'r', RG_CONTACT_1_p2, RG_CONTACT_p2_1)
   model.add_nodal_contact_between_nonmatching_meshes_brick(mim_p2, mim_2, 'u_p2', 'u_2', 'lambda_p2_2_n', 'r', RG_CONTACT_p2_2, RG_CONTACT_2_p2)
   model.add_nodal_contact_between_nonmatching_meshes_brick(mim_1, mim_p3, 'u_1', 'u_p3', 'lambda_1_p3_n', 'r', RG_CONTACT_1_p3, RG_CONTACT_p3_1)
   model.add_nodal_contact_between_nonmatching_meshes_brick(mim_p3, mim_2, 'u_p3', 'u_2', 'lambda_p3_2_n', 'r', RG_CONTACT_p3_2, RG_CONTACT_2_p3)

   nbc = size(mfu_p1.basic_dof_on_region(RG_CONTACT_p1)) / qdim
   model.add_variable('lambda_p1', nbc)
   model.add_nodal_contact_with_rigid_obstacle_brick \
     (mim_p1, 'u_p1', 'lambda_p1', 'r', RG_CONTACT_p1, bearing_p1, 1)

   nbc = size(mfu_p2.basic_dof_on_region(RG_CONTACT_p2)) / qdim
   model.add_variable('lambda_p2', nbc)
   model.add_nodal_contact_with_rigid_obstacle_brick \
     (mim_p2, 'u_p2', 'lambda_p2', 'r', RG_CONTACT_p2, bearing_p2, 1)

   nbc = size(mfu_p3.basic_dof_on_region(RG_CONTACT_p3)) / qdim
   model.add_variable('lambda_p3', nbc)
   model.add_nodal_contact_with_rigid_obstacle_brick \
     (mim_p3, 'u_p3', 'lambda_p3', 'r', RG_CONTACT_p3, bearing_p3, 1)
else:
   aug_factor = 0.1;
   model.add_initialized_data( 'r', aug_factor * Mu * (3*Lambda + 2*Mu) / (Lambda + Mu) )
   model.add_initialized_data( 'f_coeff', 0.)

#   pre_mflambda_1 = MeshFem(m_1, qdim)
#   pre_mflambda_1.set_classical_fem(1)
#   dol_1 = pre_mflambda_1.basic_dof_on_region(RG_CONTACT_TOTAL_1)
#   mflambda_1 = MeshFem('partial', pre_mflambda_1, dol_1)

#   pre_mflambda_2 = MeshFem(m_2, qdim)
#   pre_mflambda_2.set_classical_fem(1)
#   dol_2 = pre_mflambda_2.basic_dof_on_region(RG_CONTACT_TOTAL_2)
#   mflambda_2 = MeshFem('partial', pre_mflambda_2, dol_2)

   pre_mflambda_p1 = MeshFem(m_p1, qdim)
   pre_mflambda_p1.set_classical_fem(1)
   dol_p1 = pre_mflambda_p1.basic_dof_on_region(RG_CONTACT_TOTAL_p1)
   mflambda_p1 = MeshFem('partial', pre_mflambda_p1, dol_p1)

   pre_mflambda_p2 = MeshFem(m_p2, qdim)
   pre_mflambda_p2.set_classical_fem(1)
   dol_p2 = pre_mflambda_p2.basic_dof_on_region(RG_CONTACT_TOTAL_p2)
   mflambda_p2 = MeshFem('partial', pre_mflambda_p2, dol_p2)

   pre_mflambda_p3 = MeshFem(m_p3, qdim)
   pre_mflambda_p3.set_classical_fem(1)
   dol_p3 = pre_mflambda_p3.basic_dof_on_region(RG_CONTACT_TOTAL_p3)
   mflambda_p3 = MeshFem('partial', pre_mflambda_p3, dol_p3)

#   model.add_fem_variable('lambda_1', mflambda_1)
#   model.add_fem_variable('lambda_2', mflambda_2)
   model.add_fem_variable('lambda_p1', mflambda_p1)
   model.add_fem_variable('lambda_p2', mflambda_p2)
   model.add_fem_variable('lambda_p3', mflambda_p3)

#   ib_lsc = model.add_integral_large_sliding_contact_brick_with_field_extension \
#      (mim_1, 'u_1', 'lambda_1', 'r', 'f_coeff', RG_CONTACT_TOTAL_1)
#   model.add_boundary_to_large_sliding_contact_brick(ib_lsc, mim_2, 'u_2', 'lambda_2', RG_CONTACT_TOTAL_2)
#   model.add_boundary_to_large_sliding_contact_brick(ib_lsc, mim_p1, 'u_p1', 'lambda_p1', RG_CONTACT_TOTAL_p1)
#   model.add_boundary_to_large_sliding_contact_brick(ib_lsc, mim_p2, 'u_p2', 'lambda_p2', RG_CONTACT_TOTAL_p2)
#   model.add_boundary_to_large_sliding_contact_brick(ib_lsc, mim_p3, 'u_p3', 'lambda_p3', RG_CONTACT_TOTAL_p3)
#   model.add_rigid_obstacle_to_large_sliding_contact_brick(ib_lsc, bearing_p1)
#   model.add_rigid_obstacle_to_large_sliding_contact_brick(ib_lsc, bearing_p2)
#   model.add_rigid_obstacle_to_large_sliding_contact_brick(ib_lsc, bearing_p3)
   release_dist = 5.
   delaunay = False
   self_contact = False
   cut_angle = 0.2
   use_raytrace = True
   nodes_mode = 0
   ref_conf = False
   mcff = MultiContactFrame(model, 2, release_dist, delaunay, self_contact,
                            cut_angle, use_raytrace, nodes_mode, ref_conf)
   mcff.add_slave_boundary(mim_p1, RG_CONTACT_TOTAL_p1, 'u_p1', 'lambda_p1')
   mcff.add_slave_boundary(mim_p2, RG_CONTACT_TOTAL_p2, 'u_p2', 'lambda_p2')
   mcff.add_slave_boundary(mim_p3, RG_CONTACT_TOTAL_p3, 'u_p3', 'lambda_p3')
   mcff.add_master_boundary(mim_1, RG_CONTACT_TOTAL_1, 'u_1')
   mcff.add_master_boundary(mim_2, RG_CONTACT_TOTAL_2, 'u_2')
   mcff.add_obstacle(bearing_p1)
   mcff.add_obstacle(bearing_p2)
   mcff.add_obstacle(bearing_p3)

   alpha = 1
   model.add_initialized_data('alpha', alpha)
   model.add_integral_large_sliding_contact_brick_raytrace(mcff, 'r', 'f_coeff', 'alpha')

print('nbdof_1', mfu_1.nbdof())
print('nbdof_2', mfu_2.nbdof())
print('nbdof_p1', mfu_p1.nbdof())
model.solve('noisy', 'lsolver','mumps','max_res',1e-6)

U_1 = model.variable('u_1')
U_2 = model.variable('u_2')
U_p1 = model.variable('u_p1')
U_p2 = model.variable('u_p2')
U_p3 = model.variable('u_p3')
if contact_algo == 0:
   VM_1 = model.compute_isotropic_linearized_Von_Mises_or_Tresca('u_1', 'lambda', 'mu', mfrhs_1)
   VM_2 = model.compute_isotropic_linearized_Von_Mises_or_Tresca('u_2', 'lambda', 'mu', mfrhs_2)
   VM_p1 = model.compute_isotropic_linearized_Von_Mises_or_Tresca('u_p1', 'lambda', 'mu', mfrhs_p1)
   VM_p2 = model.compute_isotropic_linearized_Von_Mises_or_Tresca('u_p2', 'lambda', 'mu', mfrhs_p2)
   VM_p3 = model.compute_isotropic_linearized_Von_Mises_or_Tresca('u_p3', 'lambda', 'mu', mfrhs_p3)
else:
   VM_1 = model.compute_Von_Mises_or_Tresca('u_1', elast_law, 'elast_params', mfrhs_1)
   VM_2 = model.compute_Von_Mises_or_Tresca('u_2', elast_law, 'elast_params', mfrhs_2)
   VM_p1 = model.compute_Von_Mises_or_Tresca('u_p1', elast_law, 'elast_params', mfrhs_p1)
   VM_p2 = model.compute_Von_Mises_or_Tresca('u_p2', elast_law, 'elast_params', mfrhs_p2)
   VM_p3 = model.compute_Von_Mises_or_Tresca('u_p3', elast_law, 'elast_params', mfrhs_p3)

mfu_1.export_to_vtk('static_contact_planetary_1.vtk', 'ascii',
                    mfrhs_1,  VM_1, 'Von Mises Stress', mfu_1, U_1, 'Displacement')

mfu_2.export_to_vtk('static_contact_planetary_2.vtk', 'ascii',
                    mfrhs_2,  VM_2, 'Von Mises Stress', mfu_2, U_2, 'Displacement')

mfu_p1.export_to_vtk('static_contact_planetary_p1.vtk', 'ascii',
                     mfrhs_p1,  VM_p1, 'Von Mises Stress', mfu_p1, U_p1, 'Displacement')

mfu_p2.export_to_vtk('static_contact_planetary_p2.vtk', 'ascii',
                     mfrhs_p2,  VM_p2, 'Von Mises Stress', mfu_p2, U_p2, 'Displacement')

mfu_p3.export_to_vtk('static_contact_planetary_p3.vtk', 'ascii',
                     mfrhs_p3,  VM_p3, 'Von Mises Stress', mfu_p3, U_p3, 'Displacement')
