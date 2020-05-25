/*===========================================================================

 Copyright (C) 2007-2020 Yves Renard, Julien Pommier.

 This file is a part of GetFEM

 GetFEM  is  free software;  you  can  redistribute  it  and/or modify it
 under  the  terms  of the  GNU  Lesser General Public License as published
 by  the  Free Software Foundation;  either version 3 of the License,  or
 (at your option) any later version along with the GCC Runtime Library
 Exception either version 3.1 or (at your option) any later version.
 This program  is  distributed  in  the  hope  that it will be useful,  but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or  FITNESS  FOR  A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 License and GCC Runtime Library Exception for more details.
 You  should  have received a copy of the GNU Lesser General Public License
 along  with  this program;  if not, write to the Free Software Foundation,
 Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301, USA.

===========================================================================*/
#include "getfem/getfem_mesh_im_level_set.h"
#include "getfem/getfem_mesh_im_level_set.h"
using std::endl; using std::cout; using std::cerr;
using std::ends; using std::cin;


/* some GetFEM types that we will be using */
using bgeot::base_small_vector; /* special class for small (dim<16) vectors */
using bgeot::base_node;  /* geometrical nodes(derived from base_small_vector)*/
using bgeot::scalar_type; /* = double */
using bgeot::size_type;   /* = unsigned long */
using bgeot::base_matrix; /* small dense matrix. */

void test_2d() {
  getfem::mesh m; m.read_from_file("meshes/disc_2D_degree3.mesh");
  getfem::mesh_fem mf(m);
  getfem::mesh_level_set mls(m);
  getfem::mesh_im_level_set mim(mls, getfem::mesh_im_level_set::INTEGRATE_ALL,
				getfem::int_method_descriptor("IM_TRIANGLE(6)"));
  getfem::level_set ls1(m, 2), ls2(m, 2), ls3(m, 2);
  const getfem::mesh_fem &ls1mf = ls1.get_mesh_fem();
  scalar_type R1=.4;
  for (unsigned i=0; i < ls1mf.nb_dof(); ++i) {
    ls1.values()[i] = gmm::vect_dist2_sqr(ls1mf.point_of_basic_dof(i), 
					  getfem::base_node(0,0)) -R1*R1;
  }
  const getfem::mesh_fem &ls2mf = ls2.get_mesh_fem();
  scalar_type R2=.1;
  for (unsigned i=0; i < ls2mf.nb_dof(); ++i) {
    ls2.values()[i] = gmm::vect_dist2_sqr(ls2mf.point_of_basic_dof(i), 
					  getfem::base_node(0,0.3)) -R2*R2;
  }
  const getfem::mesh_fem &ls3mf = ls3.get_mesh_fem();
  scalar_type R3=.08;
  for (unsigned i=0; i < ls3mf.nb_dof(); ++i) {
    ls3.values()[i] = -gmm::vect_dist2_sqr(ls3mf.point_of_basic_dof(i), 
					   getfem::base_node(0,0.48)) +R3*R3;
  }
    
  mim.set_integration_method(m.convex_index(),
			     getfem::int_method_descriptor("IM_TRIANGLE(6)"));

  scalar_type area(0);
  base_matrix G;
  for (dal::bv_visitor i(m.convex_index()); !i.finished(); ++i) {
    getfem::papprox_integration pai
      = mim.int_method_of_element(i)->approx_method();
    bgeot::vectors_to_base_matrix(G, m.points_of_convex(i));
    bgeot::geotrans_interpolation_context c(m.trans_of_convex(i),
					    pai->point(0), G);
    for (size_type j = 0; j < pai->nb_points_on_convex(); ++j) {
      c.set_xref(pai->point(j));
      if (gmm::vect_norm2(c.xreal()) <= R1) area += pai->coeff(j) * c.J(); 
    }
  }
  cout << "Area of largest circle : " << area
       << " compared to exact value : " << M_PI*R1*R1 << endl;
    
  mls.add_level_set(ls1);
  mls.add_level_set(ls2);
  mls.add_level_set(ls3);
  mls.adapt(); mim.adapt();
  // Test computing the area of largest circle
  area = 0.;
  for (dal::bv_visitor i(m.convex_index()); !i.finished(); ++i) {
    getfem::papprox_integration pai
      = mim.int_method_of_element(i)->approx_method();
    bgeot::vectors_to_base_matrix(G, m.points_of_convex(i));
    bgeot::geotrans_interpolation_context c(m.trans_of_convex(i),
					    pai->point(0), G);
    for (size_type j = 0; j < pai->nb_points_on_convex(); ++j) {
      c.set_xref(pai->point(j));
      if (gmm::vect_norm2(c.xreal()) <= R1) area += pai->coeff(j) * c.J(); 
    }
  }
  cout << "Area of largest circle : " << area
       << " compared to exact value : " << M_PI*R1*R1 << endl;
  if (gmm::abs(area - M_PI*R1*R1) > 1E-3)
    GMM_ASSERT1(false, "Cutting integration method has failed : " << area
		<< " instead of " << M_PI*R1*R1 << ".");
}


void test_3d() {
  getfem::mesh m; m.read_from_file("meshes/ball_3D_P2_84_elements.mesh");
  getfem::mesh_fem mf(m);
  getfem::mesh_level_set mls(m);
  getfem::mesh_im_level_set mim(mls, getfem::mesh_im_level_set::INTEGRATE_ALL, 
				getfem::int_method_descriptor("IM_TETRAHEDRON(6)"));
  getfem::level_set ls1(m, 2), ls2(m, 2), ls3(m, 2);
  const getfem::mesh_fem &ls1mf = ls1.get_mesh_fem();
  scalar_type R1=.4;
  for (unsigned i=0; i < ls1mf.nb_dof(); ++i) {
    ls1.values()[i] = gmm::vect_dist2_sqr(ls1mf.point_of_basic_dof(i), 
					  getfem::base_node(0,0,0)) -R1*R1;
  }
  const getfem::mesh_fem &ls2mf = ls2.get_mesh_fem();
  scalar_type R2=.1;
  for (unsigned i=0; i < ls2mf.nb_dof(); ++i) {
    ls2.values()[i] = gmm::vect_dist2_sqr(ls2mf.point_of_basic_dof(i), 
					  getfem::base_node(0,0.3,0)) -R2*R2;
  }
  const getfem::mesh_fem &ls3mf = ls3.get_mesh_fem();
  scalar_type R3=.08;
  for (unsigned i=0; i < ls3mf.nb_dof(); ++i) {
    ls3.values()[i] = -gmm::vect_dist2_sqr(ls3mf.point_of_basic_dof(i), 
					   getfem::base_node(0,0.48,0)) +R3*R3;
  }
    
  mim.set_integration_method(m.convex_index(),
			     getfem::int_method_descriptor("IM_TETRAHEDRON(6)"));

  scalar_type area(0);
  base_matrix G;
  for (dal::bv_visitor i(m.convex_index()); !i.finished(); ++i) {
    getfem::papprox_integration pai
      = mim.int_method_of_element(i)->approx_method();
    bgeot::vectors_to_base_matrix(G, m.points_of_convex(i));
    bgeot::geotrans_interpolation_context c(m.trans_of_convex(i),
					    pai->point(0), G);
    for (size_type j = 0; j < pai->nb_points_on_convex(); ++j) {
      c.set_xref(pai->point(j));
      if (gmm::vect_norm2(c.xreal()) <= R1) area += pai->coeff(j) * c.J(); 
    }
  }
  cout << "Area of largest circle : " << area
       << " compared to exact value : " << 4/3.*M_PI*R1*R1*R1 << endl;
    
  mls.add_level_set(ls1);
  //mim.add_level_set(ls2);
  //mim.add_level_set(ls3);
  // mim.adapt();

  // Test computing the area of largest circle
  area = 0.;
  for (dal::bv_visitor i(m.convex_index()); !i.finished(); ++i) {
    getfem::papprox_integration pai
      = mim.int_method_of_element(i)->approx_method();
    bgeot::vectors_to_base_matrix(G, m.points_of_convex(i));
    bgeot::geotrans_interpolation_context c(m.trans_of_convex(i),
					    pai->point(0), G);
    for (size_type j = 0; j < pai->nb_points_on_convex(); ++j) {
      c.set_xref(pai->point(j));
      if (gmm::vect_norm2(c.xreal()) <= R1) area += pai->coeff(j) * c.J(); 
    }
  }
  cout << "Area of largest circle : " << area
       << " compared to exact value : " << 4/3.*M_PI*R1*R1*R1 << endl;
  if (gmm::abs(area - 4/3.*M_PI*R1*R1*R1) > 1E-3)
    GMM_ASSERT1(false, "Cutting integration method has failed");
}

int main(/* int argc, char **argv */) {

  GMM_SET_EXCEPTION_DEBUG; // Exceptions make a memory fault, to debug.
  FE_ENABLE_EXCEPT;        // Enable floating point exception for Nan.  

  try {
    // getfem::getfem_mesh_level_set_noisy();
    test_2d();
  }
  GMM_STANDARD_CATCH_ERROR;
  return 0;
}
