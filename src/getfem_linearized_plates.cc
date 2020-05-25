/*===========================================================================

 Copyright (C) 2004-2020 Yves Renard, Jeremie Lasry, Mathieu Fabre

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


#include "getfem/getfem_linearized_plates.h"


namespace getfem {

  size_type add_Mindlin_Reissner_plate_brick(model &md,
                                             const mesh_im & mim,
                                             const mesh_im & mim_red,
                                             const std::string &U,
                                             const std::string &Theta,
                                             const std::string &param_E,
                                             const std::string &param_nu,
                                             const std::string &param_epsilon,
                                             const std::string &param_kappa,
                                             size_type variant,
                                             size_type region) {
    
    
    std::string test_U = "Test_" + sup_previous_and_dot_to_varname(U);
    std::string test_Theta = "Test_" + sup_previous_and_dot_to_varname(Theta);
    std::string proj_Theta = (variant == 2) ?
      ("Elementary_transformation("+Theta+",_2D_rotated_RT0_projection__434)")
      : Theta;
    std::string proj_test_Theta = (variant == 2) ?
      ("Elementary_transformation("+test_Theta
       +",_2D_rotated_RT0_projection__434)") : test_Theta;
    
    std::string D = "(("+param_E+")*pow("+param_epsilon+
      ",3))/(12*(1-sqr("+param_nu+")))";
    std::string G = "(("+param_E+")*("+param_epsilon+"))*("+param_kappa+
      ")/(2*(1+("+param_nu+")))";
    std::string E_theta = "(Grad_" + Theta + "+(Grad_" + Theta + ")')/2";
    std::string E_test_Theta="(Grad_"+test_Theta+"+(Grad_"+test_Theta+")')/2";
    
    std::string expr_left =
      D+"*(( 1-("+param_nu+"))*("+E_theta+"):("+E_test_Theta+")+("+param_nu+
      ")*Trace("+E_theta+")*Trace("+E_test_Theta+"))";
    
    std::string expr_right = 
      "("+G+")*(Grad_"+U+"-"+proj_Theta+").Grad_"+test_U+
      "-("+G+")*(Grad_"+U+"-"+proj_Theta+")."+proj_test_Theta;
    
    switch(variant) {
    case 0: // Without reduction
      return add_linear_generic_assembly_brick
        (md, mim, expr_left+"+"+expr_right, region, false, false,
         "Reissner-Mindlin plate model brick");
    case 1: // With reduced integration
      add_linear_generic_assembly_brick
        (md, mim, expr_left, region, false, false,
         "Reissner-Mindlin plate model brick, rotation term");
      return add_linear_generic_assembly_brick
        (md, mim_red, expr_right, region, false, false,
         "Reissner-Mindlin plate model brick, transverse shear term");
    case 2: // Variant with projection on rotated RT0
      add_2D_rotated_RT0_projection(md, "_2D_rotated_RT0_projection__434");
      return add_linear_generic_assembly_brick
        (md, mim, expr_left+"+"+expr_right, region, false, false,
         "Reissner-Mindlin plate model brick");
      break;
    default: GMM_ASSERT1(false, "Invalid variant for Reissner-Mindlin brick.");
    }
    return size_type(-1);
  }
  
  
  size_type add_enriched_Mindlin_Reissner_plate_brick(model &md,
                                             const mesh_im & mim,
                                             const mesh_im & mim_red1,
                                             const mesh_im & mim_red2,
                                             const std::string &Ua,
                                             const std::string &Theta,
                                             const std::string &U3,
                                             const std::string &Theta3,                                             
                                             const std::string &param_E,
                                             const std::string &param_nu,
                                             const std::string &param_epsilon,
                                             size_type variant,
                                             size_type region) {
    
    std::string test_Ua = "Test_" + sup_previous_and_dot_to_varname(Ua);
    std::string test_U3 = "Test_" + sup_previous_and_dot_to_varname(U3);    
    std::string test_Theta = "Test_" + sup_previous_and_dot_to_varname(Theta);
    std::string proj_Theta = (variant >= 2) ?
      ("Elementary_transformation("+Theta+",_2D_rotated_RT0_projection__434)")
      : Theta;
    std::string proj_test_Theta = (variant >= 2) ?
      ("Elementary_transformation("+test_Theta+",_2D_rotated_RT0_projection__434)") 
      : test_Theta;
    std::string test_Theta3 = "Test_" + sup_previous_and_dot_to_varname(Theta3);
    std::string proj_Theta3 = (variant == 3) ?
      ("Elementary_transformation("+Theta3+",_P0_projection__434)")
      : Theta3;
    std::string proj_test_Theta3 = (variant == 3) ?
      ("Elementary_transformation("+test_Theta3+",_P0_projection__434)") 
      : test_Theta3;
    std::string D1 = "(("+param_E+")*pow("+param_epsilon+",3))/(12*(1+("+param_nu+")))";
    std::string D2 = D1+"*("+param_nu+")/(1-2*("+param_nu+"))";    
    std::string D3 = D1+"/2";
    std::string G1 = "(("+param_E+")*("+param_epsilon+"))/(1+("+param_nu+"))";
    std::string G2 = G1+"*("+param_nu+")/(1-2*("+param_nu+"))";    
    std::string G3 = G1+"/2";
    
    std::string E_Ua = "(Grad_" + Ua + "+(Grad_" + Ua + ")')/2";
    std::string E_test_Ua = "(Grad_" + test_Ua + "+(Grad_" + test_Ua + ")')/2";
    std::string E_Theta = "(Grad_" + Theta + "+(Grad_" + Theta + ")')/2";
    std::string E_test_Theta="(Grad_"+test_Theta+"+(Grad_"+test_Theta+")')/2";
    
    std::string expr_no_coupled_1 = G1+"*("+E_Ua+"):("+E_test_Ua+") + "+G1+"*("+Theta3+")*("+test_Theta3+")";
    std::string expr_no_coupled_2 = D1+"*("+E_Theta+"):("+E_test_Theta+") + "+D2+"*Trace(Grad_"+Theta+")*Trace(Grad_"+test_Theta+") + "+D3+"*(Grad_"+Theta3+").(Grad_"+test_Theta3+")";

    std::string expr_coupled_1 = G3+"*(Grad_"+U3+" + "+proj_Theta+").(Grad_"+test_U3+" + "+proj_test_Theta+")";
    std::string expr_coupled_2 = G2+"*(Trace("+E_Ua+") + "+proj_Theta3+")*(Trace("+E_test_Ua+") + "+proj_test_Theta3+")";
        
    switch(variant) {
    case 0: // Without reduction
        add_nonlinear_generic_assembly_brick
        (md, mim, expr_no_coupled_1+"+"+expr_no_coupled_2, region, false, false,
         "enriched Reissner-Mindlin plate model brick, no coupled");
        return add_nonlinear_generic_assembly_brick
        (md, mim, expr_coupled_1+"+"+expr_coupled_2, region, false, false,
         "enriched Reissner-Mindlin plate model brick, coupled");
    case 1: // With reduced integration
        add_nonlinear_generic_assembly_brick
        (md, mim, expr_no_coupled_1+"+"+expr_no_coupled_2, region, false, false,
         "enriched Reissner-Mindlin plate model brick, no coupled");
        add_nonlinear_generic_assembly_brick
        (md, mim_red1, expr_coupled_1, region, false, false,
         "enriched Reissner-Mindlin plate model brick, coupled MR");
        return add_nonlinear_generic_assembly_brick
        (md, mim_red2, expr_coupled_2, region, false, false,
         "enriched Reissner-Mindlin plate model brick, coupled eMR");
    case 2: // Variant with projection on rotated RT0 and reduction
        add_2D_rotated_RT0_projection(md, "_2D_rotated_RT0_projection__434");
        add_nonlinear_generic_assembly_brick
        (md, mim, expr_no_coupled_1+"+"+expr_no_coupled_2, region, false, false,
         "enriched Reissner-Mindlin plate model brick, no coupled");
        add_nonlinear_generic_assembly_brick
        (md, mim, expr_coupled_1, region, false, false,
         "enriched Reissner-Mindlin plate model brick, coupled MR");
        return add_nonlinear_generic_assembly_brick
        (md, mim_red2, expr_coupled_2, region, false, false,
         "enriched Reissner-Mindlin plate model brick, coupled eMR");  
    case 3: // Variant with projection on rotated RT0 and projection P0
        add_2D_rotated_RT0_projection(md, "_2D_rotated_RT0_projection__434");
        add_P0_projection(md, "_P0_projection__434");
        add_nonlinear_generic_assembly_brick
        (md, mim, expr_no_coupled_1+"+"+expr_no_coupled_2, region, false, false,
         "enriched Reissner-Mindlin plate model brick, no coupled");
        add_nonlinear_generic_assembly_brick
        (md, mim, expr_coupled_1, region, false, false,
         "enriched Reissner-Mindlin plate model brick, coupled MR");
        return add_nonlinear_generic_assembly_brick
        (md, mim, expr_coupled_2, region, false, false,
         "enriched Reissner-Mindlin plate model brick, coupled eMR");  
      break;
    default: GMM_ASSERT1(false, " testInvalid variant for enriched Reissner-Mindlin brick.");
    }
    return size_type(-1);
  }








  // For the moment, only projection onto rotated RT0 element in dimension 2

  class _2D_Rotated_RT0_projection_transformation
    : public virtual_elementary_transformation {

  public:

    virtual void give_transformation(const mesh_fem &mf, const mesh_fem &mf2,
                                     size_type cv, base_matrix &M) const{

      THREAD_SAFE_STATIC base_matrix M_old;
      THREAD_SAFE_STATIC pfem pf_old = nullptr;

      GMM_ASSERT1(&mf == &mf2,
                  "This transformation works on identical fems only");
      
      // Obtaining the fem descriptors
      pfem pf1 = mf.fem_of_element(cv);
      size_type N = 2;
      GMM_ASSERT1(pf1->dim() == 2, "This projection is only defined "
                  "for two-dimensional elements");
      size_type qmult =  N / pf1->target_dim();
      
      bool simplex = false;
      if (pf1->ref_convex(cv) == bgeot::simplex_of_reference(dim_type(N))) {
        simplex = true;
      } else if (pf1->ref_convex(cv)
                 == bgeot::parallelepiped_of_reference(dim_type(N))) {
        simplex = false;
      } else {
        GMM_ASSERT1(false, "Cannot adapt the method for such an element.");
      }

      if (pf1 == pf_old && pf1->is_equivalent() && M.size() == M_old.size()) {
        gmm::copy(M_old, M);
        return;
      }

      std::stringstream fem_desc;
      fem_desc << "FEM_RT0" << (simplex ? "":"Q") << "(" << N << ")";
      pfem pf2 = fem_descriptor(fem_desc.str());

      // Obtaining a convenient integration method
      size_type degree = pf1->estimated_degree() + pf2->estimated_degree();
      bgeot::pgeometric_trans pgt = mf.linked_mesh().trans_of_convex(cv);
      papprox_integration pim
        = classical_approx_im(pgt, dim_type(degree))->approx_method();

      // Computation of mass matrices
      size_type ndof1 = pf1->nb_dof(cv) * qmult;
      size_type ndof2 = pf2->nb_dof(0);
      base_matrix M1(ndof1, ndof1), M2(ndof2, ndof2), B(ndof1, ndof2);
      base_matrix aux0(ndof1, ndof1), aux1(ndof1, ndof2), aux2(ndof1, ndof2);
      base_matrix aux3(ndof2, ndof2);

      
      base_matrix G;
      bgeot::vectors_to_base_matrix(G, mf.linked_mesh().points_of_convex(cv));
      fem_interpolation_context ctx1(pgt, pf1, base_node(N), G, cv);
      fem_interpolation_context ctx2(pgt, pf2, base_node(N), G, cv);

      base_tensor t1, t2;
      base_matrix tv1, tv2;
        
      for (size_type i = 0; i < pim->nb_points_on_convex(); ++i) {

        scalar_type coeff = pim->coeff(i); // Mult by ctx.J() not useful here
        ctx1.set_xref(pim->point(i));
        ctx2.set_xref(pim->point(i));    
        pf1->real_base_value(ctx1, t1);
        vectorize_base_tensor(t1, tv1, ndof1, pf1->target_dim(), N);
        pf2->real_base_value(ctx2, t2);
        vectorize_base_tensor(t2, tv2, ndof2, pf2->target_dim(), N);
        for (size_type j = 0; j < ndof2; ++j) std::swap(tv2(j,0), tv2(j,1));
       
        gmm::mult(tv1, gmm::transposed(tv1), aux0);
        gmm::add(gmm::scaled(aux0, coeff), M1);
        gmm::mult(tv2, gmm::transposed(tv2), aux3);
        gmm::add(gmm::scaled(aux3, coeff), M2);
        gmm::mult(tv1, gmm::transposed(tv2), aux1);
        gmm::add(gmm::scaled(aux1, coeff), B);
      }
      
      
      // Computation of M
      gmm::lu_inverse(M1);
      gmm::lu_inverse(M2);
      gmm::mult(M1, B, aux1);
      gmm::mult(aux1, M2, aux2);
      GMM_ASSERT1(gmm::mat_nrows(M) == ndof1,
                  "Element not convenient for projection");
      gmm::mult(aux2, gmm::transposed(B), M);
      gmm::clean(M, 1E-15);
      M_old = M; pf_old = pf1;
    }
  };

  void add_2D_rotated_RT0_projection(model &md, std::string name) {
    pelementary_transformation
      p = std::make_shared<_2D_Rotated_RT0_projection_transformation>();
    md.add_elementary_transformation(name, p);
  }



  // Can be simplified ...
  class _P0_projection_transformation
    : public virtual_elementary_transformation {

  public:

    virtual void give_transformation(const mesh_fem &mf, const mesh_fem &mf2,
                                     size_type cv, base_matrix &M) const{

      THREAD_SAFE_STATIC base_matrix M_old;
      THREAD_SAFE_STATIC pfem pf_old = nullptr;

      GMM_ASSERT1(&mf == &mf2,
                  "This transformation works on identical fems only");
        
      // Obtaining the fem descriptors
      pfem pf1 = mf.fem_of_element(cv);
      size_type N = mf.get_qdim(), d = pf1->dim();
      // GMM_ASSERT1(pf1->dim() == 2, "This projection is only defined "
      //             "for two-dimensional elements");
      size_type qmult =  N / pf1->target_dim();
      
      bool simplex = false;
      if (pf1->ref_convex(cv) == bgeot::simplex_of_reference(dim_type(d))) {
        simplex = true;
      } else if (pf1->ref_convex(cv)
                 == bgeot::parallelepiped_of_reference(dim_type(d))) {
        simplex = false;
      } else {
        GMM_ASSERT1(false, "Cannot adapt the method for such an element.");
      }

      if (pf1 == pf_old && pf1->is_equivalent() && M.size() == M_old.size()) {
        gmm::copy(M_old, M);
        return;
      }

      std::stringstream fem_desc;
      fem_desc << "FEM_" << (simplex ? "PK":"QK") << "(" << d << "," << 0 << ")";
      pfem pf2 = fem_descriptor(fem_desc.str());

      // Obtaining a convenient integration method
      size_type degree = pf1->estimated_degree() + pf2->estimated_degree();
      bgeot::pgeometric_trans pgt = mf.linked_mesh().trans_of_convex(cv);
      papprox_integration pim
        = classical_approx_im(pgt, dim_type(degree))->approx_method();

      // Computation of mass matrices
      size_type ndof1 = pf1->nb_dof(cv) * qmult;
      size_type ndof2 = pf2->nb_dof(0) * qmult;
      base_matrix M1(ndof1, ndof1), M2(ndof2, ndof2), B(ndof1, ndof2);
      base_matrix aux0(ndof1, ndof1), aux1(ndof1, ndof2), aux2(ndof1, ndof2);
      base_matrix aux3(ndof2, ndof2);

      
      base_matrix G;
      bgeot::vectors_to_base_matrix(G, mf.linked_mesh().points_of_convex(cv));
      fem_interpolation_context ctx1(pgt, pf1, base_node(d), G, cv);
      fem_interpolation_context ctx2(pgt, pf2, base_node(d), G, cv);

      base_tensor t1, t2;
      base_matrix tv1, tv2;
        
      for (size_type i = 0; i < pim->nb_points_on_convex(); ++i) {

        scalar_type coeff = pim->coeff(i); // Mult by ctx.J() not useful here
        ctx1.set_xref(pim->point(i));
        ctx2.set_xref(pim->point(i));    
        pf1->real_base_value(ctx1, t1);
        vectorize_base_tensor(t1, tv1, ndof1, pf1->target_dim(), N);
        pf2->real_base_value(ctx2, t2);
        vectorize_base_tensor(t2, tv2, ndof2, pf2->target_dim(), N);
        // for (size_type j = 0; j < ndof2; ++j) std::swap(tv2(j,0), tv2(j,1));
       
        gmm::mult(tv1, gmm::transposed(tv1), aux0);
        gmm::add(gmm::scaled(aux0, coeff), M1);
        gmm::mult(tv2, gmm::transposed(tv2), aux3);
        gmm::add(gmm::scaled(aux3, coeff), M2);
        gmm::mult(tv1, gmm::transposed(tv2), aux1);
        gmm::add(gmm::scaled(aux1, coeff), B);
      }
      
      
      // Computation of M
      gmm::lu_inverse(M1);
      gmm::lu_inverse(M2);
      gmm::mult(M1, B, aux1);
      gmm::mult(aux1, M2, aux2);
      GMM_ASSERT1(gmm::mat_nrows(M) == ndof1,
                  "Element not convenient for projection");
      gmm::mult(aux2, gmm::transposed(B), M);
      gmm::clean(M, 1E-15);
      M_old = M; pf_old = pf1;
    }
  };




  void add_P0_projection(model &md, std::string name) {
    pelementary_transformation
      p = std::make_shared<_P0_projection_transformation>();
    md.add_elementary_transformation(name, p);
  }




  // RT0 projection in any dimension. Unused for the moment.


//   class RT0_projection_transformation
//     : public virtual_elementary_transformation {

//   public:

//     virtual void give_transformation(const mesh_fem &mf, size_type cv,
//                                      base_matrix &M) const{


      
//       // Obtaining the fem descriptors
//       pfem pf1 = mf.fem_of_element(cv);
//       size_type N = pf1->dim();
//       size_type qmult =  N / pf1->target_dim();

//       bool simplex = false;
//       if (pf1->ref_convex(cv) == bgeot::simplex_of_reference(dim_type(N))) {
//         simplex = true;
//       } else if (pf1->ref_convex(cv)
//                  == bgeot::parallelepiped_of_reference(dim_type(N))) {
//         simplex = false;
//       } else {
//         GMM_ASSERT1(false, "Cannot adapt the method for such an element.");
//       }

//       GMM_ASSERT1(pf1->is_equivalent(), "For tau-equivalent fem only."); // Indeed no, for the moment ...

//       std::stringstream fem_desc;
//       fem_desc << "FEM_RT0" << (simplex ? "":"Q") << "(" << N << ")";
//       pfem pf2 = fem_descriptor(fem_desc.str());

//       // Obtaining a convenient integration method
//       size_type degree = pf1->estimated_degree() + pf2->estimated_degree();
//       bgeot::pgeometric_trans pgt = mf.linked_mesh().trans_of_convex(cv);
//       papprox_integration pim
//         = classical_approx_im(pgt, dim_type(degree))->approx_method();

//       // Computation of mass matrices
//       size_type ndof1 = pf1->nb_dof(cv) * qmult;
//       size_type ndof2 = pf2->nb_dof(0);
//       base_matrix M1(ndof1, ndof1), M2(ndof2, ndof2), B(ndof1, ndof2);
//       base_matrix aux0(ndof1, ndof1), aux1(ndof1, ndof2), aux2(ndof1, ndof2);
//       base_matrix aux3(ndof2, ndof2);

      
//       base_matrix G;
//       bgeot::vectors_to_base_matrix(G, mf.linked_mesh().points_of_convex(cv));
//       fem_interpolation_context ctx1(pgt, pf1, base_node(N), G, cv);
//       fem_interpolation_context ctx2(pgt, pf2, base_node(N), G, cv);

//       base_tensor t1, t2;
//       base_matrix tv1, tv2;
        
//       for (size_type i = 0; i < pim->nb_points_on_convex(); ++i) {

//         scalar_type coeff = pim->coeff(i); // Mult by ctx.J() not useful here
//         ctx1.set_xref(pim->point(i));
//         ctx2.set_xref(pim->point(i));    
//         pf1->real_base_value(ctx1, t1);
//         vectorize_base_tensor(t1, tv1, ndof1, pf1->target_dim(), N);
//         pf2->real_base_value(ctx2, t2);
//         vectorize_base_tensor(t2, tv2, ndof2, pf2->target_dim(), N);


//         // for (size_type j = 0; j < 4; ++j)
//         //  std::swap(tv2(j,0), tv2(j,1));

       
//         gmm::mult(tv1, gmm::transposed(tv1), aux0);
//         gmm::add(gmm::scaled(aux0, coeff), M1);
//         gmm::mult(tv2, gmm::transposed(tv2), aux3);
//         gmm::add(gmm::scaled(aux3, coeff), M2);
//         gmm::mult(tv1, gmm::transposed(tv2), aux1);
//         gmm::add(gmm::scaled(aux1, coeff), B);
//       }
      
      
//       // Computation of M
//       gmm::lu_inverse(M1);
//       gmm::lu_inverse(M2);
//       gmm::mult(M1, B, aux1);
//       gmm::mult(aux1, M2, aux2);
//       GMM_ASSERT1(gmm::mat_nrows(M) == ndof1,
//                   "Element not convenient for projection");
//       gmm::mult(aux2, gmm::transposed(B), M);
//       gmm::clean(M, 1E-15);
//       // cout << "M = " << M << endl;
//     }
//   };








}  /* end of namespace getfem.                                             */

