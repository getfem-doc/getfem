/*===========================================================================

 Copyright (C) 2006-2020 Yves Renard, Julien Pommier, Jeremie Lasry.

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

#include "crack_bilaplacian.h"
#include "getfem/getfem_regular_meshes.h"
#include "getfem/getfem_assembling.h" /* import assembly methods (and norms comp.) */
#include "getfem/getfem_fourth_order.h"
#include "getfem/getfem_model_solvers.h"
using std::endl; using std::cout; using std::cerr;
using std::ends; using std::cin;
template <typename T> std::ostream &operator <<
  (std::ostream &o, const std::vector<T>& m) { gmm::write(o,m); return o; }

size_type is_global_dof_type_bis(getfem::pdof_description dof) {
  size_type global_dof = 0;
  for (dim_type d = 0; d < 4; ++d) {
    if (dof == getfem::global_dof(d)) {
      global_dof = 1;
    }
  }
  return global_dof;
}

/******** Exact Solution *******************************/

scalar_type D  = 1.;
scalar_type nu = 0.3;
scalar_type AAA = 0.1;    // mode II
scalar_type BBB = AAA * (3. * nu + 5.)/ (3. * (nu - 1.));
scalar_type DD = 0.0;   // mode 1
scalar_type CC = DD * (nu + 7.)/ (3. * (nu - 1.));
scalar_type EE = 0.0;           // singul 61
scalar_type FF = 0.0;           // singul 62
scalar_type GG = 0.0;           // singul 63
scalar_type HH = 0.0; //3.0     // singul 6

scalar_type P0 = 0.0;
scalar_type P1 = 0.0;
scalar_type P2 = 0.0;

scalar_type sol_u(const base_node &x) {
  scalar_type r = sqrt( x[0] * x[0] + x[1] * x[1] );
  //scalar_type theta = 2. * atan( x[1] / ( x[0] + r ) );
  scalar_type theta = atan2(x[1], x[0]);
  return sqrt(r*r*r)*(AAA*sin(theta/2.0)+BBB*sin(3.0/2.0*theta)+CC*cos(3.0/2.0*theta)+DD*cos(theta/2.0)) + EE * x[1] * (10. * x[1] * x[1]* x[1] + 1.);
}

scalar_type sol_F(const base_node &) {
  return 1.; //EE * D *  240.; //256. * cos(2. * x[1]);
}

void exact_solution_bilap::init(getfem::level_set &ls) {
  std::vector<getfem::pglobal_function> cfun(11);
  for (unsigned j=0; j < 4; ++j)
    cfun[j] = bilaplacian_crack_singular(j, ls, nu, 0.);
  cfun[4] = bilaplacian_crack_singular(61, ls, nu, 0.);
  cfun[5] = bilaplacian_crack_singular(62, ls, nu, 0.);
  cfun[6] = bilaplacian_crack_singular(63, ls, nu, 0.);
  cfun[7] = bilaplacian_crack_singular(6, ls, nu, 0.);
  cfun[8] = bilaplacian_crack_singular(10, ls, nu, 0.);
  cfun[9] = bilaplacian_crack_singular(11, ls, nu, 0.);
  cfun[10] = bilaplacian_crack_singular(12, ls, nu, 0.);

  mf.set_functions(cfun);
  U.resize(11); assert(mf.nb_dof() == 11);
  U[0] = AAA;
  U[1] = BBB;
  U[2] = CC;
  U[3] = DD;
  U[4] = EE;
  U[5] = FF;
  U[6] = GG;
  U[7] = HH;
  U[8] = P0;
  U[9] = P1;
  U[10] = P2;
}




scalar_type eval_fem_gradient_with_finite_differences(getfem::pfem pf,
                                                      const base_vector &coeff,
                                                      size_type cv,
                                                      bgeot::pgeometric_trans pgt,
                                                      bgeot::geotrans_inv_convex &gic,
                                                      const base_matrix &G,
                                                      base_node X0,
                                                      scalar_type h, unsigned dg) {
  X0[dg] -= h/2;
  base_node X0ref; gic.invert(X0, X0ref);
  getfem::fem_interpolation_context c(pgt, pf, X0ref, G, cv);

  base_vector val0(1);
  pf->interpolation(c, coeff, val0, 1);

  base_node X1(X0), X1ref; X1[dg] += h;
  gic.invert(X1, X1ref);
  c.set_xref(X1ref);

  base_vector val1(1);
  pf->interpolation(c, coeff, val1, 1);

  return (val1[0] - val0[0])/h;
}

scalar_type eval_fem_hessian_with_finite_differences(getfem::pfem pf,
                                                     const base_vector &coeff,
                                                     size_type cv,
                                                     bgeot::pgeometric_trans pgt,
                                                     bgeot::geotrans_inv_convex &gic,
                                                     const base_matrix &G,
                                                     base_node X0,
                                                     scalar_type h,
                                                     unsigned dg, unsigned dh) {
  X0[dh] -= h/2;
  scalar_type Gr0 =
    eval_fem_gradient_with_finite_differences(pf, coeff, cv, pgt, gic, G, X0, h, dg);
  base_node X1(X0);
  X1[dh] += h;
  scalar_type Gr1 =
    eval_fem_gradient_with_finite_differences(pf, coeff, cv, pgt, gic, G, X1, h, dg);
  return (Gr1 - Gr0)/h;
}

void validate_fem_derivatives(getfem::pfem pf, unsigned cv,
                              bgeot::pgeometric_trans pgt, const base_matrix &G) {
  unsigned N = unsigned(gmm::mat_nrows(G));
  scalar_type h = 1e-5;

  std::vector<base_node> pts(gmm::mat_ncols(G));
  for (unsigned j=0; j < pts.size(); ++j) {
    pts[j].resize(N); gmm::copy(gmm::mat_col(G, j), pts[j]);
  }
  cout << "validate_fem_derivatives: pf = " << &(*pf) << ", nbdof = "<< pf->nb_dof(cv) << ", cv = " << cv << " (~ at " << gmm::mean_value(pts) << ")\n";
  bgeot::geotrans_inv_convex gic(pts, pgt);

  //cout << "pts = " << pts << "\n";

  for (unsigned idof = 0; idof < pf->nb_dof(cv); ++idof) {
    /* choose a random point in the convex */
    base_node X0(N), X0ref;
    base_node w(pgt->nb_points());
    do {
      for (unsigned i=0; i < w.size(); ++i) w[i] = 0.1 + 0.8*gmm::random();
      gmm::scale(w, 1/gmm::vect_norm1(w));
      gmm::mult(G, w, X0);

      //cout << "w = " << w << "\n";

      gic.invert(X0, X0ref);

      // avoid discontinuity lines in the HCT composite element..
      if (gmm::abs(X0ref[0] + X0ref[1] - 1) > 1e-2 &&
          gmm::abs(X0ref[0] - X0ref[1]) > 1e-2 &&
          gmm::abs(X0[0]) > 1e-3 && gmm::abs(X0[1])> 1e-3) break;
    } while (1);
    //cout << "testing X0 = " << X0 << " (X0ref=" << X0ref << ")\n";


    base_vector coeff(pf->nb_dof(cv)); coeff[idof] = 1;
    base_matrix grad(1,N), grad_fd(1,N);
    base_matrix hess(1,N*N), hess_fd(1,N*N);

    getfem::fem_interpolation_context c(pgt, pf, X0ref, G, cv);
    pf->interpolation_grad(c, coeff, grad, 1);
    pf->interpolation_hess(c, coeff, hess, 1);

    for (unsigned dg = 0; dg < N; ++dg) {
      grad_fd[dg] =
        eval_fem_gradient_with_finite_differences(pf, coeff, cv, pgt, gic, G, X0, h, dg);
      for (unsigned dh = 0; dh < N; ++dh) {
        hess_fd(0,dg*N+dh) =
          eval_fem_hessian_with_finite_differences(pf, coeff, cv, pgt, gic, G, X0, h, dg, dh);
      }
    }

    scalar_type err_grad =
      gmm::vect_dist2((base_vector&)grad, (base_vector&)grad_fd);
    scalar_type err_hess =
      gmm::vect_dist2((base_vector&)hess, (base_vector&)hess_fd);

    if (err_grad > 1e-4 ||
        err_hess > 1e-4) {
      cout << "validate_fem_derivatives dof=" << idof << "/" << pf->nb_dof(cv) << " -- X0ref = " << X0ref << "\n";

      if (gmm::vect_dist2((base_vector&)grad, (base_vector&)grad_fd) > 1e-4)
        cout << "grad = " << (base_vector&)grad << "\ngrad_fd = " << (base_vector&)grad_fd << "\n";
      cout << "hess = " << (base_vector&)hess << "\nhess_fd = " << (base_vector&)hess_fd << "\n";
      if (err_grad + err_hess > 1.0) { cout << "---------> COMPLETEMENT FAUX!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"; abort(); }
    }
  }
}


void validate_fem_derivatives(const getfem::mesh_fem &mf) {
  bgeot::base_matrix G;
  for (dal::bv_visitor cv(mf.convex_index()); !cv.finished(); ++cv) {
    //if (mf.nb_dof_of_element(cv) > 12) {
      vectors_to_base_matrix(G, mf.linked_mesh().points_of_convex(cv));
      validate_fem_derivatives(mf.fem_of_element(cv), unsigned(cv), mf.linked_mesh().trans_of_convex(cv), G);
      //}
  }
}






/*                                                          */
/*****  Methods for class bilaplacian_crack_problem  ********/
/*                                                          */





void bilaplacian_crack_problem::init(void) {
  std::string MESH_FILE = PARAM.string_value("MESH_FILE");
  std::string MESH_TYPE = PARAM.string_value("MESH_TYPE","Mesh type ");
  std::string FEM_TYPE  = PARAM.string_value("FEM_TYPE","FEM name");
  std::string INTEGRATION = PARAM.string_value("INTEGRATION",
                                               "Name of integration method");
  cout << "MESH_TYPE=" << MESH_TYPE << "\n";
  cout << "FEM_TYPE="  << FEM_TYPE << "\n";
  cout << "INTEGRATION=" << INTEGRATION << "\n";

  std::string SIMPLEX_INTEGRATION = PARAM.string_value("SIMPLEX_INTEGRATION",
                                         "Name of simplex integration method");
  std::string SINGULAR_INTEGRATION = PARAM.string_value("SINGULAR_INTEGRATION");
  enrichment_option = unsigned(PARAM.int_value("ENRICHMENT_OPTION",
                                               "Enrichment option"));
  enr_area_radius = PARAM.real_value("RADIUS_ENR_AREA",
                                     "radius of the enrichment area");

  sol_ref = PARAM.int_value("SOL_REF");
  scalar_type angle_rot = PARAM.real_value("ANGLE_ROT");
  size_type N = 2;
  /* First step : build the mesh */
  if (!MESH_FILE.empty()) {
    mesh.read_from_file(MESH_FILE);
    // printing number of elements
    cout << "Number of element of the mesh: " << mesh.convex_index().card() << "\n";
    base_small_vector tt(N);
    tt[0] = PARAM.real_value("TRANSLAT_X");
    tt[1] = PARAM.real_value("TRANSLAT_Y");
    if (sol_ref == 1 || sol_ref == 2) {
      tt[0] -= PARAM.real_value("CRACK_SEMI_LENGTH");
    }
    cout << "TRANSLAT_X = " << tt[0] << " ; TRANSLAT_Y = " << tt[1] << "\n";
    mesh.translation(tt);
    MESH_TYPE = bgeot::name_of_geometric_trans
      (mesh.trans_of_convex(mesh.convex_index().first_true()));
    bgeot::pgeometric_trans pgt =
      bgeot::geometric_trans_descriptor(MESH_TYPE);
    cout << "MESH_TYPE=" << MESH_TYPE << "\n";
    N = mesh.dim();
  }
  else {
    bgeot::pgeometric_trans pgt =
      bgeot::geometric_trans_descriptor(MESH_TYPE);
    N = pgt->dim();
    GMM_ASSERT1(N == 2, "For a plate problem, N should be 2");
    std::vector<size_type> nsubdiv(N);
    NX = PARAM.int_value("NX", "Number of space steps ");
    std::fill(nsubdiv.begin(),nsubdiv.end(), NX);
    if (sol_ref == 1)  nsubdiv[0] = NX / 2;
    if (sol_ref == 2) {
      size_type NY = PARAM.int_value("NY");
      nsubdiv[1] = NY;
    }
    getfem::regular_unit_mesh(mesh, nsubdiv, pgt, PARAM.int_value("MESH_NOISED") != 0);

    /* scale the unit mesh to [LX,LY,..] and incline it */
    base_small_vector tt(N);
    switch (sol_ref) {
    case 0 : {
      tt[0] = - 0.5 + PARAM.real_value("TRANSLAT_X");
      tt[1] = - 0.5 + PARAM.real_value("TRANSLAT_Y");
    } break;
    case 1 : {
      tt[0] = - PARAM.real_value("CRACK_SEMI_LENGTH");
      tt[1] = - 0.5 + PARAM.real_value("TRANSLAT_Y");
    } break;
    case 2 : {
      tt[0] = - PARAM.real_value("CRACK_SEMI_LENGTH");
      tt[1] = - 0.5 + PARAM.real_value("TRANSLAT_Y");
    } break;
    case 3 : {
      tt[0] = - 0.5 + PARAM.real_value("TRANSLAT_X");
      tt[1] = - 0.5 + PARAM.real_value("TRANSLAT_Y");
    } break;
    default :
      GMM_ASSERT1(false, "SOL_REF parameter is undefined");
      break;
    }
    mesh.translation(tt);
    bgeot::base_matrix M(N,N);
    for (size_type i=0; i < N; ++i) {
      static const char *t[] = {"LX","LY","LZ"};
      M(i,i) = (i<3) ? PARAM.real_value(t[i],t[i]) : 1.0;
    }
    if (sol_ref == 2) {
      M(0, 0) = 1.5;
      M(1, 1) = 1.0;
    }
    if (sol_ref == 0 || sol_ref == 3) {
      M(0, 0) = cos(angle_rot);
      M(1, 1) = M(0, 0);
      M(1, 0) = sin(angle_rot);
      M(0, 1) = - M(1, 0);
    }
    mesh.transformation(M);
  }

  scalar_type quality = 1.0, avg_area = 0. , min_area = 1. , max_area = 0., area;
  scalar_type radius, avg_radius = 0., min_radius = 1., max_radius = 0.;
  size_type cpt = 0;
  for (dal::bv_visitor i(mesh.convex_index()); !i.finished(); ++i) {
    quality = std::min(quality, mesh.convex_quality_estimate(i));
    area = mesh.convex_area_estimate(i);
    radius = mesh.convex_radius_estimate(i);
    avg_area += area;
    avg_radius += radius;
    min_radius = std::min(radius, min_radius);
    max_radius = std::max(radius, max_radius);
    min_area = std::min(min_area, area);
    max_area = std::max(max_area, area);
    cpt++;
  }
  avg_area /= scalar_type(cpt);
  avg_radius /= scalar_type(cpt);
/*  cout << "quality of mesh : " << quality << endl;
    cout << "average radius : " << avg_radius << endl;
    cout << "radius min : " << min_radius << " ; radius max : " << max_radius << endl;
    cout << "average area : " << avg_area << endl;
    cout << "area min : " << min_area << " ; area max : " << max_area << endl;  */

   /* read the parameters   */
  epsilon = PARAM.real_value("EPSILON", "thickness");
  nu = PARAM.real_value("NU", "nu");
  D = PARAM.real_value("D", "Flexure modulus");
  int mv = int(PARAM.int_value("MORTAR_VERSION", "Mortar version") );
  int cv = int(PARAM.int_value("CLOSING_VERSION") );
  mortar_version = size_type(mv);
  closing_version = size_type(cv);
  datafilename=PARAM.string_value("ROOTFILENAME","Base name of data files.");
  residual=PARAM.real_value("RESIDUAL"); if (residual == 0.) residual = 1e-10;
  KL = PARAM.int_value("KL", "Kirchhoff-Love model or not") != 0;
  D = PARAM.real_value("D", "Flexion modulus");
  if (KL) nu = PARAM.real_value("NU", "Poisson ratio");

  cutoff.fun_num = PARAM.int_value("CUTOFF_FUNC", "cutoff function");
  cutoff.radius = PARAM.real_value("CUTOFF", "Cutoff");
  cutoff.radius1 = PARAM.real_value("CUTOFF1", "Cutoff1");
  cutoff.radius0 = PARAM.real_value("CUTOFF0", "Cutoff0");

 // Setting the integration methods

  getfem::pfem pf_u = getfem::fem_descriptor(FEM_TYPE);
  getfem::pintegration_method ppi =
    getfem::int_method_descriptor(INTEGRATION);
  getfem::pintegration_method sppi =
    getfem::int_method_descriptor(SIMPLEX_INTEGRATION);
  getfem::pintegration_method sing_ppi =
    (SINGULAR_INTEGRATION.size() ? getfem::int_method_descriptor(SINGULAR_INTEGRATION)
                                 : 0);

  mim.set_integration_method(mesh.convex_index(), ppi);
  mls.add_level_set(ls);
  mim.set_simplex_im(sppi, sing_ppi);

  /* Setting the finite element on the mf_u */
  mf_pre_u.set_finite_element(mesh.convex_index(), pf_u);
  getfem::pfem pf_partition_of_unity =
    getfem::fem_descriptor(PARAM.string_value("PARTITION_OF_UNITY_FEM_TYPE"));
  mf_partition_of_unity.set_finite_element(mesh.convex_index(), pf_partition_of_unity);

  // set the mesh_fem of the multipliers (for the dirichlet condition)
  std::string dirichlet_fem_name = PARAM.string_value("DIRICHLET_FEM_TYPE");
  if (dirichlet_fem_name.size() == 0)
    mf_mult.set_finite_element(mesh.convex_index(), pf_u);
  else {
    cout << "DIRICHLET_FEM_TYPE="  << dirichlet_fem_name << "\n";
    mf_mult.set_finite_element(mesh.convex_index(),
                               getfem::fem_descriptor(dirichlet_fem_name));
  }
  std::string dirichlet_der_fem_name
    = PARAM.string_value("DIRICHLET_DER_FEM_TYPE", "");  // for the dirichlet condition on the derivative
  if (dirichlet_der_fem_name.size() == 0)
    mf_mult_d.set_finite_element(mesh.convex_index(), pf_u);
  else {
    cout << "DIRICHLET_DER_FEM_TYPE="  << dirichlet_der_fem_name << "\n";
    mf_mult_d.set_finite_element(mesh.convex_index(),
                                 getfem::fem_descriptor(dirichlet_der_fem_name));
  }

  /* set the finite element on mf_rhs (same as mf_u if DATA_FEM_TYPE is
     not used in the .param file */
  std::string data_fem_name = PARAM.string_value("DATA_FEM_TYPE");
  if (data_fem_name.size() == 0) {
    GMM_ASSERT1(pf_u->is_lagrange(), "You are using a non-lagrange FEM. "
                << "In that case you need to set "
                << "DATA_FEM_TYPE in the .param file");
    mf_rhs.set_finite_element(mesh.convex_index(), pf_u);
  } else {
    mf_rhs.set_finite_element(mesh.convex_index(),
                              getfem::fem_descriptor(data_fem_name));
  }

  // set the mesh_fem for the multipliers for the case the Integral Matching
  mf_pre_mortar.set_finite_element(mesh.convex_index(),
             getfem::fem_descriptor(PARAM.string_value("MORTAR_FEM_TYPE")));
  mf_pre_mortar_deriv.set_finite_element(mesh.convex_index(),
             getfem::fem_descriptor(PARAM.string_value("MORTAR_DERIV_FEM_TYPE")));

  /* set boundary conditions: non-homogeneous Dirichlet */
  cout << "Selecting Neumann and Dirichlet boundaries\n";
  getfem::mesh_region border_faces;
  getfem::outer_faces_of_mesh(mesh, border_faces);
  if (sol_ref == 0 && angle_rot == 0.) {
    for (getfem::mr_visitor i(border_faces); !i.finished(); ++i) {
      base_node un = mesh.normal_of_face_of_convex(i.cv(), i.f());
      un /= gmm::vect_norm2(un);
      if (gmm::abs(un[0]) >= 0.9999) { // vertical edges
        mesh.region(SIMPLE_SUPPORT_BOUNDARY_NUM).add(i.cv(), i.f());
        mesh.region(CLAMPED_BOUNDARY_NUM).add(i.cv(), i.f());
      }
      else { // horizontal edges
        unsigned id_point_1_of_face, id_point_2_of_face;
        scalar_type x1, x2;
        id_point_1_of_face = mesh.structure_of_convex(i.cv())->ind_points_of_face(i.f())[0];
        id_point_2_of_face = mesh.structure_of_convex(i.cv())->ind_points_of_face(i.f())[1];
        x1 = mesh.points_of_convex(i.cv())[id_point_1_of_face][1];
        x2 = mesh.points_of_convex(i.cv())[id_point_2_of_face][1];
        if (gmm::abs(x1) > 0.4999 && gmm::abs(x2) > 0.4999) { // on the boundary => clamped
          mesh.region(SIMPLE_SUPPORT_BOUNDARY_NUM).add(i.cv(), i.f());
          mesh.region(CLAMPED_BOUNDARY_NUM).add(i.cv(), i.f());
        }
        else { // on the crack => free boundary condition
          mesh.region(MOMENTUM_BOUNDARY_NUM).add(i.cv(), i.f());
          mesh.region(FORCE_BOUNDARY_NUM).add(i.cv(), i.f());
        }
      }
    }
  }


  if (sol_ref == 1 ) {
    for (getfem::mr_visitor i(border_faces); !i.finished(); ++i) {
      base_node un = mesh.normal_of_face_of_convex(i.cv(), i.f());
      un /= gmm::vect_norm2(un);
      if (un[0] <= -0.9999 || un[0] >= 0.9999) {
      //if  (-un[0] >= 0.999) {
        mesh.region(CLAMPED_BOUNDARY_NUM).add(i.cv(), i.f());
      }
      if (gmm::abs(un[1]) >= 0.999) {
        mesh.region(SIMPLE_SUPPORT_BOUNDARY_NUM).add(i.cv(), i.f());
        mesh.region(MOMENTUM_BOUNDARY_NUM).add(i.cv(), i.f());
      }
    }
  }

  if (sol_ref == 2 ) {
    for (getfem::mr_visitor i(border_faces); !i.finished(); ++i) {
      base_node un = mesh.normal_of_face_of_convex(i.cv(), i.f());
      un /= gmm::vect_norm2(un);
      if (un[0] < - 0.9999) { // symetry condition
        mesh.region(CLAMPED_BOUNDARY_NUM).add(i.cv(), i.f());
      }
      else {
        mesh.region(SIMPLE_SUPPORT_BOUNDARY_NUM).add(i.cv(), i.f());
        mesh.region(MOMENTUM_BOUNDARY_NUM).add(i.cv(), i.f());
      }
    }
  }

  if (sol_ref == 0 && angle_rot != 0. ) {
    // does not support the case of a conformal mesh around the crack
    // (all the boundary, including the crack faces, will be clamped).
    for (getfem::mr_visitor i(border_faces); !i.finished(); ++i) {
      mesh.region(CLAMPED_BOUNDARY_NUM).add(i.cv(), i.f());
      mesh.region(SIMPLE_SUPPORT_BOUNDARY_NUM).add(i.cv(), i.f());
    }
  }

  exact_sol.init(ls);

  cout << "initialisation de la level-set : \n";

  // Setting the level-set
  ls.reinit();
  // scalar_type a = PARAM.real_value("CRACK_SEMI_LENGTH");
  for (size_type d = 0; d < ls.get_mesh_fem().nb_dof(); ++d) {
    scalar_type x = ls.get_mesh_fem().point_of_basic_dof(d)[0];
    scalar_type y = ls.get_mesh_fem().point_of_basic_dof(d)[1];
    if (sol_ref == 0) {
      ls.values(0)[d] = y; // + 1/4.*(x + .25);
      ls.values(1)[d] = x;
    }
    if (sol_ref == 1) {
      ls.values(0)[d] = y;
      ls.values(1)[d] = x; //x * x - a * a;
    }
    if (sol_ref == 2) { // to modify if rotation is supported
      ls.values(0)[d] = y;
      ls.values(1)[d] = x; //x * x - a * a;
    }
  }
  //ls.simplify(0.5);
  ls.touch();
  mls.adapt();
  mim.adapt();
  mfls_u.adapt();
//   mfls_mult.adapt();
//   mfls_mult_d.adapt();
  mfls_mortar.adapt();
  mfls_mortar_deriv.adapt();
  cout << "mfls_u.nb_dof()=" << mfls_u.nb_dof() << "\n";
}

// /* compute the relative error with respect to the exact solution */
// void bilaplacian_crack_problem::compute_error(plain_vector &U) {
//   GMM_ASSERT1(!mf_rhs.is_reduced(), "To be adapted");
//   std::vector<scalar_type> V(mf_rhs.nb_dof());
//   getfem::interpolation(mf_u(), mf_rhs, U, V);
//   for (size_type i = 0; i < mf_rhs.nb_dof(); ++i)
//     V[i] -= sol_u(mf_rhs.point_of_basic_dof(i));
//   cout.precision(16);
//   cout  << "L2 error = " << getfem::asm_L2_norm(mim, mf_rhs, V)  << endl
//         << "H1 error = " << getfem::asm_H1_norm(mim, mf_rhs, V)  << endl
//         << "H2 error = " << getfem::asm_H2_norm(mim, mf_rhs, V)  << endl
//         /*<< "Linfty error = " << gmm::vect_norminf(V)  << endl*/;
//   cout  << "semi-norme H1 = " << getfem::asm_H1_semi_norm(mim, mf_rhs, V)  << endl
//         << "semi-norme H2 = " << getfem::asm_H2_semi_norm(mim, mf_rhs, V)  << endl;
//
// }

/* compute the error with respect to the exact solution */
void bilaplacian_crack_problem::compute_error(plain_vector &U) {

  if (PARAM.real_value("RADIUS_SPLIT_DOMAIN") == 0) {
    plain_vector V(gmm::vect_size(U));
    gmm::clear(V);

    cout << "L2 ERROR:"
       << getfem::asm_L2_dist(mim, mf_u(), U,
                              exact_sol.mf, exact_sol.U) << "\n";
    cout << "H1 ERROR:"
         << getfem::asm_H1_dist(mim, mf_u(), U,
                                exact_sol.mf, exact_sol.U) << "\n";
    cout << "H2 ERROR:"
         << getfem::asm_H2_dist(mim, mf_u(), U,
                              exact_sol.mf, exact_sol.U) << "\n";
//     cout << "SEMI H1 ERROR:"
//          << getfem::asm_H1_semi_dist(mim, mf_u(), U,
//                               exact_sol.mf, exact_sol.U) << "\n";
//     cout << "SEMI H2 ERROR:"
//          << getfem::asm_H2_semi_dist(mim, mf_u(), U,
//                               exact_sol.mf, exact_sol.U) << "\n";
    if (PARAM.int_value("NORM_EXACT")) {
    cout << "L2 exact:"
         << getfem::asm_L2_dist(mim, mf_u(), V,
                                exact_sol.mf, exact_sol.U) << "\n";
    cout << "H1 exact:"
         << getfem::asm_H1_dist(mim, mf_u(), V,
                                exact_sol.mf, exact_sol.U) << "\n";
    cout << "H2 exact:"
         << getfem::asm_H2_dist(mim, mf_u(), V,
                              exact_sol.mf, exact_sol.U) << "\n";
    }
  }
  else {
    getfem::mesh_region r_center, r_ext;
    scalar_type radius_split_domain = PARAM.real_value("RADIUS_SPLIT_DOMAIN");
    bool in_area;
    for (dal::bv_visitor cv(mesh.convex_index()); !cv.finished(); ++cv) {
      in_area = true;
      /* For each element, we test all of its nodes.
         If all the nodes are inside the enrichment area,
         then the element is completly inside the area too */
      for (unsigned j=0; j < mesh.nb_points_of_convex(cv); ++j) {
        if (gmm::sqr(mesh.points_of_convex(cv)[j][0] ) +
            gmm::sqr(mesh.points_of_convex(cv)[j][1] ) >
            gmm::sqr(radius_split_domain))
          in_area = false;
        break;
      }
      if (in_area)
        r_center.add(cv);
      else
        r_ext.add(cv);
    }
    scalar_type L2_center, H1_center, H2_center;
    cout << "ERROR SPLITTED - RADIUS =  " << radius_split_domain << "\n";
    cout << "Error on the crack tip zone:\n";
    L2_center = getfem::asm_L2_dist(mim, mf_u(), U, exact_sol.mf, exact_sol.U, r_center);
    cout << "  L2 error:" << L2_center << "\n";
    H1_center = getfem::asm_H1_dist(mim, mf_u(), U, exact_sol.mf, exact_sol.U, r_center);
    cout << "  H1 error:" << H1_center << "\n";
    H2_center = getfem::asm_H2_dist(mim, mf_u(), U, exact_sol.mf, exact_sol.U, r_center);
    cout << "  H2 error:" << H2_center << "\n";

    cout << "Error on the remaining part of the domain:\n";
    scalar_type L2_ext, H1_ext, H2_ext;
    L2_ext = getfem::asm_L2_dist(mim, mf_u(), U, exact_sol.mf, exact_sol.U, r_ext);
    cout << "  L2 error:" << L2_ext << "\n";
    H1_ext = getfem::asm_H1_dist(mim, mf_u(), U, exact_sol.mf, exact_sol.U, r_ext);
    cout << "  H1 error:" << H1_ext << "\n";
    H2_ext = getfem::asm_H2_dist(mim, mf_u(), U, exact_sol.mf, exact_sol.U, r_ext);
    cout << "  H2 error:" << H2_ext << "\n";

    cout << "Error on the hole domain:\n";
    cout << "L2 ERROR:"
         << gmm::sqrt( gmm::sqr(L2_center) + gmm::sqr(L2_ext) ) << "\n";

    cout << "H1 ERROR:"
         << gmm::sqrt( gmm::sqr(H1_center) + gmm::sqr(H1_ext) ) << "\n";
    cout << "H2 ERROR:"
         << gmm::sqrt( gmm::sqr(H2_center) + gmm::sqr(H2_ext) ) << "\n";
  }
}




/**************************************************************************/
/*  Model.                                                                */
/**************************************************************************/

bool bilaplacian_crack_problem::solve(plain_vector &U) {

  if (enrichment_option == 2 || enrichment_option == 3) {
    cout << "setting singularities \n";
    if (PARAM.int_value("SING_BASE_TYPE") == 0) {
      std::vector<getfem::pglobal_function> ufunc(4);
      for (size_type i = 0; i < ufunc.size(); ++i) {
        ufunc[i] = bilaplacian_crack_singular(i, ls, nu, 0.);
      }
      mf_sing_u.set_functions(ufunc);
    }
    if (PARAM.int_value("SING_BASE_TYPE") == 1) {
      std::vector<getfem::pglobal_function> ufunc(2);
      for (size_type i = 0; i < ufunc.size(); ++i) {
        ufunc[i] = bilaplacian_crack_singular(i + 4, ls, nu, 0.);
      }
      mf_sing_u.set_functions(ufunc);
    }
  }
  else if (enrichment_option == 4) {
    cout << "Setting up the singular functions for the cutoff enrichment\n";
    if (PARAM.int_value("SING_BASE_TYPE") == 0) {
      std::vector<getfem::pglobal_function> vfunc(4);
      for (size_type i = 0; i < vfunc.size(); ++i) {
        /* use the singularity */
        getfem::pxy_function
          s = std::make_shared<crack_singular_bilaplacian_xy_function>
              (i, ls, nu, 0.);
        /* use the product of the singularity function
           with a cutoff */
        getfem::pxy_function
          cc =  std::make_shared<getfem::cutoff_xy_function>
          (int(cutoff.fun_num), cutoff.radius, cutoff.radius1, cutoff.radius0);
        s = std::make_shared<getfem::product_of_xy_functions>(s, cc);
        vfunc[i] = getfem::global_function_on_level_set(ls, s);
      }
      mf_sing_u.set_functions(vfunc);
    }
    if (PARAM.int_value("SING_BASE_TYPE") == 1) {
      std::vector<getfem::pglobal_function> vfunc(2);
      for (size_type i = 0; i < vfunc.size(); ++i) {
        /* use the singularity */
        getfem::pxy_function
          s = std::make_shared<crack_singular_bilaplacian_xy_function>
              (i+4, ls, nu, 0.);
        /* use the product of the singularity function
           with a cutoff */
        getfem::pxy_function
          cc =  std::make_shared<getfem::cutoff_xy_function>
          (int(cutoff.fun_num), cutoff.radius, cutoff.radius1, cutoff.radius0);
        s = std::make_shared<getfem::product_of_xy_functions>(s, cc);
        vfunc[i] = getfem::global_function_on_level_set(ls, s);
      }
      mf_sing_u.set_functions(vfunc);
    }
  }
  // Setting the enrichment --------------------------------------------/

  switch(enrichment_option) {
  case -1: // classical FEM
    {
      mf_u_sum.set_mesh_fems(mf_pre_u);
    }
    break;
  case 0 :  // No enrichment
    {
      mf_u_sum.set_mesh_fems(mfls_u);
      // an optional treatment : exporting a representation of the mesh
      if (!PARAM.int_value("MIXED_ELEMENTS")) {
        getfem::mesh_fem mf_enrich(mesh);
        getfem::pfem pf_mef = getfem::classical_fem(mesh.trans_of_convex(mesh.convex_index().first_true()), 1 );
        mf_enrich.set_finite_element(mesh.convex_index(), pf_mef);
        std::vector<scalar_type> UU(mf_enrich.nb_dof());
        std::fill(UU.begin(), UU.end() ,0.);
        cout << "exporting mesh to " << "mesh_representation.vtk" << "..\n";
        getfem::vtk_export exp("mesh_representation.vtk", false);
        exp.exporting(mf_enrich);
        exp.write_point_data(mf_enrich, UU, "mesh");
        cout << "export done, you can view the data file with (for example)\n"
          "mayavi -d mesh_representation.vtk -f "
          "WarpScalar -m BandedSurfaceMap -m Outline\n";
      }
    }
    break;
  case 1 :
    {
      cout << "\npointwise matching\n";
      /* first : selecting the convexes that are completly included in the enrichment area */
      for (dal::bv_visitor i(mesh.convex_index()); !i.finished(); ++i) {
        pm_convexes.add(i);
        /* For each element, we test all of its nodes.
           If all the nodes are inside the enrichment area,
           then the element is completly inside the area too */
        for (unsigned j=0; j < mesh.nb_points_of_convex(i); ++j) {
          if (gmm::sqr(mesh.points_of_convex(i)[j][0]) +
              gmm::sqr(mesh.points_of_convex(i)[j][1]) >
              gmm::sqr(enr_area_radius))
            pm_convexes.sup(i);
          break;
        }
      }

      for (dal::bv_visitor cv(mf_sing_u.convex_index()); !cv.finished(); ++cv) {
        if (!pm_convexes.is_in(cv))
          mf_sing_u.set_finite_element(cv, 0);
      }
      cout << "mf_sing_u: convex_index() = " << mf_sing_u.convex_index().card() << " convexes\n";

      //mf_u_sum.set_mesh_fems(mfls_u_ext, mf_pre_u); //_ext, mf_sing_u);
      mf_u_sum.set_smart_global_dof_linking(true);
      mf_u_sum.set_mesh_fems(mf_pre_u, mf_sing_u);

      cout << "mf_u_sum.nb_dof = " << mf_u_sum.nb_dof() << "\n";
      cout << "mfls_u.convex_index = " << mfls_u.convex_index() << "\nmf_sing_u: " << mf_sing_u.convex_index() << "\n";

    } break;
  case 2 :  // standard XFEM on a fixed zone
    {
      dal::bit_vector enriched_dofs;
      plain_vector X(mf_partition_of_unity.nb_dof());
      plain_vector Y(mf_partition_of_unity.nb_dof());
      getfem::interpolation(ls.get_mesh_fem(), mf_partition_of_unity,
                            ls.values(1), X);
      getfem::interpolation(ls.get_mesh_fem(), mf_partition_of_unity,
                            ls.values(0), Y);
      for (size_type j = 0; j < mf_partition_of_unity.nb_dof(); ++j) {
        if (gmm::sqr(X[j]) + gmm::sqr(Y[j]) <= gmm::sqr(enr_area_radius))
          enriched_dofs.add(j);
      }
      //cout << "enriched_dofs: " << enriched_dofs << "\n";
      if (enriched_dofs.card() < 3)
        GMM_WARNING0("There is " << enriched_dofs.card() <<
                     " enriched dofs for the crack tip");
      mf_u_product.set_enrichment(enriched_dofs);
      mf_u_sum.set_mesh_fems(mf_u_product, mfls_u);
      cout << "enrichment done \n";
    } break;
  case 4 :
    {
      if (cutoff.fun_num == getfem::cutoff_xy_function::EXPONENTIAL_CUTOFF)
        cout<<"Using exponential Cutoff..."<<endl;
      else
        cout<<"Using Polynomial Cutoff..."<<endl;
//        dal::bit_vector enriched_dofs;
//        enriched_dofs.clear();
//        cout << "mf_cutoff.nb_dof() = " << mf_cutoff.nb_dof() << "\n";
//
//        for (size_type j = 0; j < mf_cutoff.nb_dof(); ++j) {
//          enriched_dofs.add(j);
//        }
//        for (dal::bv_visitor j(enriched_dofs); !j.finished(); ++j) {
//          cout << j << "; ";
//        }
//        cout << "\n";
//        cout << "mf_prod_cutoff.nb_dof() = " << mf_prod_cutoff.nb_dof() << "\n";
//        cout << "mf_sing_u.nb_dof() = " << mf_sing_u.nb_dof() << "\n";
//        mf_prod_cutoff.set_enrichment(enriched_dofs);
//
//        cout << "mf_prod_cutoff.nb_dof() = " << mf_prod_cutoff.nb_dof() << "\n";

      //mf_u_sum.set_mesh_fems(mf_prod_cutoff, mfls_u);
      mf_u_sum.set_mesh_fems(mf_sing_u, mfls_u);
    } break;
  case 3 : // Integral matching (mortar)
    {
      cout << "\nIntegral Matching (Mortar)\n";
      dal::bit_vector cvlist_in_area, cvlist_out_area;
      bool in_area = true;
      for (dal::bv_visitor cv(mesh.convex_index());
           !cv.finished(); ++cv) {
        in_area = true;
        /* For each element, we test all of its nodes.
           If all the nodes are inside the enrichment area,
           then the element is completly inside the area too */
        for (unsigned j=0; j < mesh.nb_points_of_convex(cv); ++j) {
//          if (gmm::sqr(mesh.points_of_convex(cv)[j][0] ) +
//              gmm::sqr(mesh.points_of_convex(cv)[j][1] ) >
//              gmm::sqr(enr_area_radius)) {
          if ( (gmm::abs(mesh.points_of_convex(cv)[j][0] ) > enr_area_radius) ||
               (gmm::abs(mesh.points_of_convex(cv)[j][1] ) > enr_area_radius)) {
            in_area = false;
            break;
          }
        }

        /* "remove" the global function on convexes outside the enrichment
           area */
        if (!in_area) {
          cvlist_out_area.add(cv);
          mf_sing_u.set_finite_element(cv, 0);
          mf_u().set_dof_partition(cv, 1);
        } else
          cvlist_in_area.add(cv);
      }

      /* extract the boundary of the enrichment area, from the
         "inside" point-of-view, and from the "outside"
         point-of-view */
      getfem::mesh_region r_border, r_enr_out;
      getfem::outer_faces_of_mesh(mesh, r_border);

      getfem::outer_faces_of_mesh(mesh, cvlist_in_area,
                                  mesh.region(MORTAR_BOUNDARY_IN));
      getfem::outer_faces_of_mesh(mesh, cvlist_out_area,
                                  mesh.region(MORTAR_BOUNDARY_OUT));
      for (getfem::mr_visitor v(r_border); !v.finished(); ++v) {
        mesh.region(MORTAR_BOUNDARY_OUT).sup(v.cv(), v.f());
      }
      if (PARAM.int_value("MORTAR_WITHOUT_SINGUL"))
         mf_u_sum.set_mesh_fems(mfls_u);
      else
         mf_u_sum.set_mesh_fems(mf_sing_u, mfls_u);

      //cout << "cvlist_in_area: " << cvlist_in_area << "\n";
      cout << "mfls_u.nb_dof: " << mfls_u.nb_dof() << "\n";
      cout << "mf_u_sum.nb_dof: " << mf_u_sum.nb_dof() << "\n";
      //cout << "MORTAR_BOUNDARY_IN: " << mesh.region(MORTAR_BOUNDARY_IN) << "\n";
      //cout << "MORTAR_BOUNDARY_OUT: " << mesh.region(MORTAR_BOUNDARY_OUT) << "\n";

      // an optional treatment : creating a representation of the enrichment area
      getfem::mesh_fem mf_enrich(mesh);
      for (dal::bv_visitor i(mesh.convex_index()); !i.finished(); ++i) {
        getfem::pfem pf_mef = getfem::classical_fem(mesh.trans_of_convex(i), 1);
        mf_enrich.set_finite_element(i, pf_mef);
      }
      std::vector<scalar_type> UU(mf_enrich.nb_dof());
      std::fill(UU.begin(), UU.end() ,0.);
      cout << "exporting the enrichment zone: \n";
      GMM_ASSERT1(!mf_enrich.is_reduced(), "To be adapted");
      for (dal::bv_visitor i(cvlist_in_area); !i.finished(); ++i) {
        for (unsigned int j = 0;
             j < mf_enrich.ind_basic_dof_of_element(i).size(); ++j)
          UU[mf_enrich.ind_basic_dof_of_element(i)[j]] = 1.;
      }

      cout << "exporting enrichment to " << "enrichment_zone.vtk" << "..\n";
      getfem::vtk_export exp("enrichment_zone.vtk", false);
      exp.exporting(mf_enrich);
      exp.write_point_data(mf_enrich, UU, "enrichment");
      cout << "export done, you can view the data file with (for example)\n"
              "mayavi -d enrichment_zone.vtk -f "
              "WarpScalar -m BandedSurfaceMap -m Outline\n";

//      // Another optional treatment :
//      // Searching the elements that are both crossed by the crack
//      // and with one of their faces which constitutes a part of the
//      // boundary between the enriched zone and the rest of the domain.
//      getfem::mesh_region &boundary = mesh.region(MORTAR_BOUNDARY_IN);
//      unsigned int cpt = 0;
//      for (dal::bv_visitor i(cvlist_in_area); !i.finished(); ++i) {
//        if (mls.is_convex_cut(i)) {
//          // Among the faces of the convex, we search if some are
//          // part of the boundary
//          cpt = 0;
//          for (unsigned j=0; j < mesh.structure_of_convex(i) ->nb_faces(); ++j) {
//            if (boundary.is_in(i,j))
//              cpt += 1;
//          }
//          if (cpt) {
//            cout << "\n The convex number " << i << " is crossed by the crack :\n";
//            cout << "  it has : " << cpt << " face(s) among the boundary.\n \n ";
//          }
//        }
//      }

    }  // end of "enrichment_option = 3"
    break;
  default :
    GMM_ASSERT1(false, "Enrichment_option parameter is undefined");
    break;
  }  // end of switch

  mesh.write_to_file("toto.mesh");

  if (PARAM.int_value("SHOW_NAME_OF_DOF")==1) {  // printing the type of each dof
    unsigned Q = mf_u().get_qdim();
    GMM_ASSERT1(!mf_u().is_reduced(), "To be adapted");
    for (unsigned d=0; d < mf_u().nb_dof(); d += Q) {
      printf("dof %4d @ %+6.2f:%+6.2f: ", d,
             mf_u().point_of_basic_dof(d)[0], mf_u().point_of_basic_dof(d)[1]);

      const getfem::mesh::ind_cv_ct cvs = mf_u().convex_to_basic_dof(d);
      for (size_type i=0; i < cvs.size(); ++i) {
        size_type cv = cvs[i];
        //if (pm_cvlist.is_in(cv)) flag1 = true; else flag2 = true;

        getfem::pfem pf = mf_u().fem_of_element(cv);
        unsigned ld = unsigned(-1);
        for (unsigned dd = 0; dd < mf_u().nb_basic_dof_of_element(cv); dd += Q) {
          if (mf_u().ind_basic_dof_of_element(cv)[dd] == d) {
            ld = dd/Q; break;
          }
        }
        if (ld == unsigned(-1)) {
          cout << "DOF " << d << "NOT FOUND in " << cv << " BUG BUG\n";
        } else {
          printf(" %3d:%.16s", unsigned(cv), name_of_dof_2(pf->dof_types().at(ld)).c_str());
        }
      }
      printf("\n");
    }
  }

  //cout << "validate mf_sing_u():\n"; validate_fem_derivatives(mf_sing_u);

  //cout << "validate mf_u():\n"; validate_fem_derivatives(mf_u());

  cout << "Number of dof for u --> : " << mf_u().nb_dof() << endl;
  scalar_type pressure;
  pressure = PARAM.real_value("PRESSURE");
  // Bilaplacian brick.

  getfem::model model;

  // Main unknown of the problem.
  model.add_fem_variable("u", mf_u());

  model.add_initialized_scalar_data("D", D);
  if (KL) {
    model.add_initialized_scalar_data("nu", nu);
    getfem::add_bilaplacian_brick_KL(model, mim, "u", "D", "nu");
  } else {
    getfem::add_bilaplacian_brick(model, mim, "u", "D");
  }

  // Defining the volumic source term.
  size_type nb_dof_rhs = mf_rhs.nb_dof();
  plain_vector F(nb_dof_rhs);
  gmm::clear(F);
  if (sol_ref == 2) {
    getfem::interpolation_function(mf_rhs, F, sol_F);
    gmm::scale(F, pressure);
  }
  //Volumic source term brick.
  model.add_initialized_fem_data("VolumicData", mf_rhs, F);
  getfem::add_source_term_brick(model, mim, "u", "VolumicData");

  // Normal derivative Dirichlet condition brick
  model.add_initialized_fem_data("DData", exact_sol.mf, exact_sol.U);
  if (sol_ref == 2)
    add_normal_derivative_Dirichlet_condition_with_multipliers
      (model, mim, "u", mf_mult_d, CLAMPED_BOUNDARY_NUM, "DData", true);
  else
    add_normal_derivative_Dirichlet_condition_with_multipliers
      (model, mim, "u", mf_mult_d, CLAMPED_BOUNDARY_NUM);

  if (sol_ref == 2)
    add_Dirichlet_condition_with_multipliers
      (model, mim, "u", mf_mult, SIMPLE_SUPPORT_BOUNDARY_NUM);
  else
    add_Dirichlet_condition_with_multipliers
      (model, mim, "u", mf_mult, SIMPLE_SUPPORT_BOUNDARY_NUM, "DData");


  sparse_matrix H(1, mf_u().nb_dof());
  if (enrichment_option == 3 ) {
    /* add a constraint brick for the mortar junction between
       the enriched area and the rest of the mesh */

    // calcul des matrices de contraintes
    plain_vector R(1);
//    sparse_matrix H(1, mf_u().nb_dof());
    this->set_matrix_mortar(H);

    /* because of the discontinuous partition of mf_u(), some levelset
       enriched functions do not contribute any more to the
       mass-matrix (the ones which are null on one side of the
       levelset, when split in two by the mortar partition, may create
       a "null" dof whose base function is all zero.. */
    sparse_matrix M2(mf_u().nb_dof(), mf_u().nb_dof());
    getfem::asm_mass_matrix(M2, mim, mf_u(), mf_u());
    //gmm::HarwellBoeing_IO::write("M2.hb", M2);
    cout << "PARAM.real_value(\"SEUIL\") : " << PARAM.real_value("SEUIL") << "\n";
    for (size_type d = 0; d < mf_u().nb_dof(); ++d) {
      // if (M2(d,d) < 1e-7) cout << "  weak mf_u() dof " << d << " @ " <<
      //                     mf_u().point_of_dof(d) << " M2(d,d) = " << M2(d,d) << "\n";
      if (M2(d,d) < PARAM.real_value("SEUIL")) {
        cout << "removed : " << mf_u().point_of_basic_dof(d) << "\n";
        size_type n = gmm::mat_nrows(H);
        gmm::resize(H, n+1, gmm::mat_ncols(H));
        H(n, d) = 1;
      }
    }
    gmm::resize(R, gmm::mat_nrows(H));
    model.add_fixed_size_variable("mult_mo", gmm::mat_nrows(H));
    getfem::add_constraint_with_multipliers(model, "u", "mult_mo", H, R);
    gmm::Harwell_Boeing_save("H.hb", H);
  }

  if ( PARAM.real_value("SEUIL_FINAL")!=0 ) {
  // suppression of nodes with a very small term on the stiffness matrix diag

    sparse_matrix M2(mf_u().nb_dof(), mf_u().nb_dof());
    sparse_matrix H1(0, mf_u().nb_dof());
    //getfem::asm_mass_matrix(M2, mim, mf_u(), mf_u());
    base_vector RR(mf_rhs.nb_dof(), 1.0);
    getfem::asm_stiffness_matrix_for_bilaplacian(M2, mim, mf_u(),
                                                 mf_rhs, RR);

    //cout << "stiffness_matrix_for_bilaplacian : " << M2 << "\n";
    cout << "termes diagonaux, de la matrice de rigidite, inferieurs a 1e-10 : ";
    for (size_type d = 0; d < mf_u().nb_dof(); ++d) {
        if (M2(d,d) < 1e-10) cout << M2(d,d) << " ; ";
    }
    cout << "\n";
    cout << "SEUIL_FINAL = " << PARAM.real_value("SEUIL_FINAL") << "\n";
    for (size_type d = 0; d < mf_u().nb_dof(); ++d) {
      if (M2(d,d) < PARAM.real_value("SEUIL_FINAL")) {
        cout << "OULALA " << d << " @ " << mf_u().point_of_basic_dof(d) << " : " << M2(d,d) << "\n";
        size_type n = gmm::mat_nrows(H);
        gmm::resize(H1, n+1, gmm::mat_ncols(H));
        H1(n, d) = 1;
      }
    }
    base_vector R(gmm::mat_nrows(H1));
    model.add_fixed_size_variable("mult_fi", gmm::mat_nrows(H1));
    getfem::add_constraint_with_multipliers(model, "u", "mult_fi", H1, R);
    gmm::Harwell_Boeing_save("M2.hb", M2);
  }

  cout << "Total number of variables : " << model.nb_dof() << endl;
  gmm::iteration iter(residual, 1, 40000);
  getfem::standard_solve(model, iter);

  // Solution extraction
  gmm::resize(U, mf_u().nb_dof());
  gmm::copy(model.real_variable("u"), U);

  /****************************/
  if (PARAM.int_value("FIC_ORTHO") ) {
    sparse_matrix A = model.real_tangent_matrix();
    plain_vector b = model.real_rhs();
    gmm::scale(b, -1.);
    plain_vector X(b);
    scalar_type condest;
#if defined(GMM_USES_MUMPS)
    gmm::MUMPS_solve(A, X, gmm::scaled(b, scalar_type(-1)));
#else
    gmm::SuperLU_solve(A, X, gmm::scaled(b, scalar_type(-1)), condest, 1);
    cout << "cond super LU = " << 1./condest << "\n";
#endif
    cout << "X = " << gmm::sub_vector(X, gmm::sub_interval(0, 10)) << "\n";
    cout << "U = " << gmm::sub_vector(U, gmm::sub_interval(0, 10)) << "\n";

    unsigned q = mf_u().get_qdim();

    base_small_vector tab_fic(4);
    std::vector<size_type> ind_sing(2);
    unsigned cpt = 0;
    if (PARAM.int_value("ENRICHMENT_OPTION") == 3) {
    // affichage des coeffs devant les singularites, avec le raccord integral
      GMM_ASSERT1(!mf_u().is_reduced(), "To be adapted");

      for (unsigned d=0; d < mf_u().nb_dof(); d += q) {
        size_type cv = mf_u().first_convex_of_basic_dof(d);
        getfem::pfem pf = mf_u().fem_of_element(cv);
        unsigned ld = unsigned(-1);
        for (unsigned dd = 0; dd < mf_u().nb_basic_dof_of_element(cv); dd += q) {
          if (mf_u().ind_basic_dof_of_element(cv)[dd] == d) {
            ld = dd/q; break;
          }
        }
        if (ld == unsigned(-1)) {
          cout << "DOF " << d << "NOT FOUND in " << cv << " BUG BUG\n";
        }
        else {
          if (is_global_dof_type_bis(pf->dof_types().at(ld))) {
            cout << "coeff:" << U[d] << "\n";
            cout << "dof index:" << d << "\n";
            tab_fic[cpt] = U[d];
            ind_sing[cpt] = d;
            cpt +=1;
          }
        }
      }
    }


    plain_vector b1(gmm::mat_nrows(A)), b2(b1), X1(b1), X2(b1);

    scalar_type as1, as2, as1s2, bs1, bs2;
    as1 = A(ind_sing[0], ind_sing[0]);
    scalar_type max = 0.;
    size_type imax = 0;
    for (unsigned i = 0;  i < gmm::mat_nrows(A); i++) {
      if (gmm::abs(A(ind_sing[0],i)) > max && i!= ind_sing[0] && i!= ind_sing[1]) {
        max = gmm::abs(A(ind_sing[0],i));
        imax = i;
      }
    }
    cout << "imax = " << imax << endl;
    cout << "max = " << max << endl;
    if (imax < mf_u().nb_dof())
      cout << "position de imax = [" << mf_u().point_of_basic_dof(imax)[0] << "; " << mf_u().point_of_basic_dof(imax)[1] << "\n";
    as2 = A(ind_sing[1], ind_sing[1]);
    as1s2 = A(ind_sing[0], ind_sing[1]);
    bs1 = b[ind_sing[0]];
    bs2 = b[ind_sing[1]];
    gmm::copy(gmm::mat_col(A, ind_sing[0]), b1);
    gmm::copy(gmm::mat_col(A, ind_sing[1]), b2);
    //cout << "gmm::mat_col(A, ind_sing[0]) = " << b1 << "\n";
    //cout << "gmm::mat_col(A, ind_sing[1]) = " << b2 << "\n";

    for (int i=0; i < 2; i++) {
      for (unsigned j=0; j < gmm::mat_nrows(A); j++) {
        A(ind_sing[i],j) = 0.;
        A(j,ind_sing[i]) = 0.;
      }
      A(ind_sing[i], ind_sing[i]) = 1.;
      b[ind_sing[i]] = 0.;
      b1[ind_sing[i]] = 0.;
      b2[ind_sing[i]] = 0.;
    }

#if defined(GMM_USES_MUMPS)
    gmm::MUMPS_solve(A, X1, b1);
    gmm::MUMPS_solve(A, X2, b2);
#else
    gmm::SuperLU_solve(A, X1, b1, condest, 1);
    cout << "solving for s1 OK, cond = " << 1./condest << "\n";
    gmm::SuperLU_solve(A, X2, b2, condest, 1);
    cout << "solving for s2 OK, cond = " << 1./condest << "\n";
#endif
    cout << "X1[ind_sing[0]] = " << X1[ind_sing[0]] << "\n";
    cout << "X1 = " << gmm::sub_vector(X1, gmm::sub_interval(0, 10)) << "\n";
    scalar_type max1 = 0., max2 = 0.;
    size_type imax1 = 0 , imax2 = 0;
    for (unsigned i = 0;  i < X1.size(); i++) {
      if (gmm::abs(X1[i]) > max1) {
        max1 = gmm::abs(X1[i]);
        imax1 = i;
      }
      if (gmm::abs(X2[i]) > max2) {
        max2 = gmm::abs(X2[i]);
        imax2 = i;
      }
    }
    cout << "imax1 = " << imax1 << endl;
    cout << "max1 = " << max1 << endl;
    if (imax1 < mf_u().nb_dof())
      cout << "position de imax1 = [" << mf_u().point_of_basic_dof(imax1)[0] << " ; " << mf_u().point_of_basic_dof(imax1)[1] << "\n";
    cout << "imax2 = " << imax2 << endl;
    cout << "max2 = " << max2 << endl;
    if (imax2 < mf_u().nb_dof())
      cout << "position de imax2 = [" << mf_u().point_of_basic_dof(imax2)[0] << " ; " << mf_u().point_of_basic_dof(imax2)[1] << "\n";
    //cout << "X1 = " << gmm::sub_vector(X1, gmm::sub_interval(0, 100)) << "\n";
    //cout << "X2 = " << gmm::sub_vector(X2, gmm::sub_interval(0, 100)) << "\n";
    base_matrix M(2,2);
    plain_vector AX1(gmm::mat_nrows(A)), AX2(AX1);
    gmm::mult(A, X1, AX1);
    gmm::mult(A, X2, AX2);
    M(0,0) = as1 - 2. * gmm::vect_sp(b1, X1) + gmm::vect_sp(X1, AX1);
    M(1,1) = as2 - 2. * gmm::vect_sp(b2, X2) + gmm::vect_sp(X2, AX2);
    M(0,1) = as1s2 - gmm::vect_sp(b1, X2) - gmm::vect_sp(b2, X1) + gmm::vect_sp(X1, AX2);
    M(1,0) = M(0,1);
    plain_vector Z(2), FIC_ORTHO(2);
    Z[0] = bs1 - gmm::vect_sp(X1, b);
    Z[1] = bs2 - gmm::vect_sp(X2, b);
    gmm::lu_solve(M, FIC_ORTHO, Z);

    cout << "[as1 as2 as1s2] = " << as1 << " ; " << as2 << " ; " << as1s2 << "\n";
    cout << "[bs1 bs2] = " << bs1 << " ; " << bs2 << "\n";
    cout << "M = " << M << "\n";
    cout << "Z = " << Z << "\n";

    cout << "FIC1 ORTHO = " << FIC_ORTHO[0] << "\n";
    cout << "FIC2 ORTHO = " << FIC_ORTHO[1] << "\n";
    cout << "-----------------------\n";
  }
  /************************************************/
  return true;
}

template<typename VEC1, typename VEC2>
void asm_H2_semi_dist_map(const getfem::mesh_im &mim,
                          const getfem::mesh_fem &mf1, const VEC1 &U1,
                          const getfem::mesh_fem &mf2, const VEC2 &U2,
                          const getfem::mesh_fem &mf_P0, VEC1 &V,
                          getfem::mesh_region rg = getfem::mesh_region::all_convexes()) {
  mim.linked_mesh().intersect_with_mpi_region(rg);
  getfem::generic_assembly assem;
  assem.set("u1=data$1(#1); u2=data$2(#2); "
            "V(#3)+=u1(i).u1(j).comp(Hess(#1).Hess(#1).Base(#3))(i,d,e,j,d,e,:)"
            "+ u2(i).u2(j).comp(Hess(#2).Hess(#2).Base(#3))(i,d,e,j,d,e,:)"
            "- 2*u1(i).u2(j).comp(Hess(#1).Hess(#2).Base(#3))(i,d,e,j,d,e,:)");

  assem.push_mi(mim);
  assem.push_mf(mf1);
  assem.push_mf(mf2);
  assem.push_mf(mf_P0);
  assem.push_data(U1);
  assem.push_data(U2);
  assem.push_vec(V);
  assem.assembly(rg);
}

void bilaplacian_crack_problem::compute_H2_error_field(const plain_vector &U) {

    getfem::mesh_fem mf_P0(mesh);
    mf_P0.set_finite_element(mesh.convex_index(), getfem::classical_fem(mesh.trans_of_convex(0), 0));
    plain_vector V(mf_P0.nb_dof());
    asm_H2_semi_dist_map(mim, mf_u(), U, exact_sol.mf, exact_sol.U, mf_P0, V);
    cout << "exporting H2 error map\n";
    getfem::vtk_export exp2(datafilename + "_H2.vtk");
    exp2.exporting(mf_P0);
    exp2.write_point_data(mf_P0, V, "H2 error map");

    mf_P0.write_to_file(datafilename + "_H2.meshfem", true);
    gmm::vecsave(datafilename + "_H2.V", V);
}

