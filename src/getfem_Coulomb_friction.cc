// -*- c++ -*- (enables emacs c++ mode)
//===========================================================================
//
// Copyright (C) 2004-2010 Yves Renard
//
// This file is a part of GETFEM++
//
// Getfem++  is  free software;  you  can  redistribute  it  and/or modify it
// under  the  terms  of the  GNU  Lesser General Public License as published
// by  the  Free Software Foundation;  either version 2.1 of the License,  or
// (at your option) any later version.
// This program  is  distributed  in  the  hope  that it will be useful,  but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or  FITNESS  FOR  A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// You  should  have received a copy of the GNU Lesser General Public License
// along  with  this program;  if not, write to the Free Software Foundation,
// Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301, USA.
//
//===========================================================================


#include "getfem/getfem_Coulomb_friction.h"

#include <getfem/getfem_arch_config.h>
#if GETFEM_HAVE_MUPARSER_MUPARSER_H
#include <muParser/muParser.h>
#endif

namespace getfem {

  //=========================================================================
  //
  //  Projection on a ball and gradient of the projection.
  //
  //=========================================================================

  template<typename VEC> static void ball_projection(const VEC &x,
						     scalar_type radius) {
    scalar_type a = gmm::vect_norm2(x);
    if (radius <= 0) gmm::clear(const_cast<VEC&>(x));
    else if (a > radius) gmm::scale(const_cast<VEC&>(x), radius/a); 
  }
  
  template<class VEC, class VECR>
  static void ball_projection_grad_r(const VEC &x, scalar_type radius,
				     VECR &g) {
    scalar_type a = gmm::vect_norm2(x);
    if (radius > 0 && a >= radius)
      gmm::copy(gmm::scaled(x, scalar_type(1)/a), g);
    else gmm::clear(g);
  }
  
  template <class VEC, class MAT>
  static void ball_projection_grad(const VEC &x, double radius, MAT &g) {
    if (radius <= scalar_type(0)) { gmm::clear(g); return; }
    gmm::copy(gmm::identity_matrix(), g);
    scalar_type a = gmm::vect_norm2(x);
    if (a >= radius) { 
      gmm::scale(g, radius/a);
      // gmm::rank_one_update(g, gmm::scaled(x, -radius/(a*a*a)), x);
      for (size_type i = 0; i < x.size(); ++i)
	for (size_type j = 0; j < x.size(); ++j)
	  g(i,j) -= radius*x[i]*x[j] / (a*a*a);
    }
  }

  //=========================================================================
  //
  //  Basic Brick (with given BN, BT, gap) and possibly two bodies
  //
  //=========================================================================

  struct Coulomb_friction_brick : public virtual_brick {

    mutable CONTACT_B_MATRIX BN1, BT1;
    mutable CONTACT_B_MATRIX BN2, BT2;
    mutable CONTACT_B_MATRIX BBN1, BBT1;
    mutable CONTACT_B_MATRIX BBN2, BBT2;
    mutable model_real_plain_vector gap, threshold, friction_coeff, alpha;
    mutable model_real_plain_vector RLN, RLT; 
    mutable scalar_type r, gamma;
    mutable bool is_init;
    bool Tresca_version, symmetrized, contact_only;
    bool really_stationary, friction_dynamic_term;
    bool two_variables;

    void init_BBN_BBT(void) const {
      gmm::resize(BBN1, gmm::mat_nrows(BN1), gmm::mat_ncols(BN1));
      gmm::copy(BN1, BBN1);
      if (two_variables) {
	gmm::resize(BBN2, gmm::mat_nrows(BN2), gmm::mat_ncols(BN2));
	gmm::copy(BN2, BBN2);
      }
      if (!contact_only) {
	gmm::resize(BBT1, gmm::mat_nrows(BT1), gmm::mat_ncols(BT1));
	gmm::copy(BT1, BBT1);
	if (two_variables) {
	  gmm::resize(BBT2, gmm::mat_nrows(BT2), gmm::mat_ncols(BT2));
	  gmm::copy(BT2, BBT2);
	}
      }
      size_type nbc = gmm::mat_nrows(BN1);
      size_type d = gmm::mat_nrows(BT1)/nbc;
      for (size_type i = 0; i < nbc; ++i) {
	gmm::scale(gmm::mat_row(BBN1, i), alpha[i]);
	if (two_variables)
	  gmm::scale(gmm::mat_row(BBN2, i), alpha[i]);
	if (!contact_only)
	  for (size_type k = 0; k < d; ++k) {
	    gmm::scale(gmm::mat_row(BBT1, d*i+k), alpha[i]);
	    if (two_variables)
	      gmm::scale(gmm::mat_row(BBT2, d*i+k), alpha[i]);
	  }
      }
      is_init = true;
    }

    void precomp(const model_real_plain_vector &u1,
		 const model_real_plain_vector &u2,
		 const model_real_plain_vector &lambda_n,
		 const model_real_plain_vector &lambda_t,
		 const model_real_plain_vector &wt1,
		 const model_real_plain_vector &wt2) const {
      gmm::resize(RLN, gmm::mat_nrows(BN1));
      if (!contact_only) gmm::resize(RLT, gmm::mat_nrows(BT1));

      gmm::copy(gmm::scaled(gap, r), RLN);
      for (size_type i = 0; i < gmm::mat_nrows(BN1); ++i) RLN[i] *= alpha[i];
      gmm::add(lambda_n, RLN);
      gmm::mult_add(BBN1, gmm::scaled(u1, -r), RLN);
      if (two_variables) gmm::mult_add(BBN2, gmm::scaled(u2, -r), RLN);
      if (!contact_only) {
	gmm::copy(lambda_t, RLT);
	if (friction_dynamic_term) {
	  gmm::mult_add(BBT1, gmm::scaled(wt1, -r*gamma), RLT);
	  if (two_variables)
	    gmm::mult_add(BBT2, gmm::scaled(wt2, -r*gamma), RLT);
	}
	if (!really_stationary) {
	  gmm::mult_add(BBT1, gmm::scaled(u1, -r), RLT);
	  if (two_variables) gmm::mult_add(BBT2, gmm::scaled(u2, -r), RLT);
	}
      }
    }

    // Common part for all contact with friction bricks
    void basic_asm_real_tangent_terms(const model_real_plain_vector &u1,
				      const model_real_plain_vector &u2,
				      const model_real_plain_vector &lambda_n,
				      const model_real_plain_vector &lambda_t,
				      const model_real_plain_vector &wt1,
				      const model_real_plain_vector &wt2,
				      model::real_matlist &matl,
				      model::real_veclist &vecl,
				      build_version version) const {
      size_type nbt = 4 + (contact_only ? 0 : 4) + (two_variables ? 3 : 0)
	+ (two_variables && !contact_only ? 2 : 0);
      GMM_ASSERT1(matl.size() == nbt,
		  "Wrong number of terms for the contact brick");
 
      const scalar_type vt1(1);
      size_type nbc = gmm::mat_nrows(BN1);
      size_type d = gmm::mat_nrows(BT1)/nbc;

      // Matrices to be filled
      size_type nt = 0;
      model_real_sparse_matrix &T_u1_u1 = matl[nt++], &T_u2_u2 = matl[nt++];
      if (!two_variables) nt--;
      model_real_sparse_matrix &T_u1_n = matl[nt++], &T_n_u1 = matl[nt++];
      if (!two_variables) nt -= 2;
      model_real_sparse_matrix &T_u2_n = matl[nt++], &T_n_u2 = matl[nt++];
      size_type nvec_lambda_n = nt;
      model_real_sparse_matrix &T_n_n = matl[nt++];
      if (contact_only) nt -= 2;
      model_real_sparse_matrix &T_u1_t = matl[nt++], &T_t_u1 = matl[nt++];
      if (contact_only || !two_variables) nt -= 2;
      model_real_sparse_matrix &T_u2_t = matl[nt++], &T_t_u2 = matl[nt++];
      if (contact_only) nt -= 2;
      size_type nvec_lambda_t = nt;
      model_real_sparse_matrix &T_t_t = matl[nt++], &T_t_n = matl[nt++];

      // Rhs to be filled
      model_real_plain_vector &ru1 = vecl[0];
      model_real_plain_vector &ru2 = vecl[1];
      model_real_plain_vector &rlambda_n = vecl[nvec_lambda_n];
      model_real_plain_vector &rlambda_t = vecl[nvec_lambda_t];

      // pre-computations
      if (!is_init) init_BBN_BBT();
      precomp(u1, u2, lambda_n, lambda_t, wt1, wt2);
      
      if (version & model::BUILD_MATRIX) {
	// Unilateral contact
	gmm::clear(T_n_n); gmm::clear(T_u1_u1);
	if (two_variables) gmm::clear(T_u2_u2);
	gmm::copy(gmm::scaled(gmm::transposed(BBN1), -vt1), T_u1_n);
	if (two_variables)
	  gmm::copy(gmm::scaled(gmm::transposed(BBN2), -vt1), T_u2_n);
	for (size_type i=0; i < nbc; ++i) {
	  if (RLN[i] >= scalar_type(0)) {
	    gmm::clear(gmm::mat_col(T_u1_n, i));
	    if (two_variables) gmm::clear(gmm::mat_col(T_u2_n, i));
	    T_n_n(i, i) = -vt1/r;
	  }
	}
	gmm::copy(gmm::transposed(T_u1_n), T_n_u1);
	if (two_variables) gmm::copy(gmm::transposed(T_u2_n), T_n_u2);
      
	// Friction
	if (!contact_only) {
	  base_matrix pg(d, d);
	  base_vector vg(d);
	  gmm::clear(T_u1_t); gmm::clear(T_t_n); gmm::clear(T_t_t);
	  if (two_variables) gmm::clear(T_u2_t);

	  for (size_type i=0; i < nbc; ++i) {
	    gmm::sub_interval SUBI(i*d, d);
	    scalar_type th = Tresca_version ? threshold[i]
	      : - lambda_n[i] * friction_coeff[i];
	    ball_projection_grad(gmm::sub_vector(RLT, SUBI), th, pg);
	    if (!really_stationary) {
	      for (size_type k = 0; k < d; ++k) {
		gmm::copy(gmm::scaled(gmm::mat_row(BBT1, i*d+k), -pg[k]),
			  gmm::mat_col(T_u1_t, i*d+k));
		if (two_variables)
		  gmm::copy(gmm::scaled(gmm::mat_row(BBT2, i*d+k), -pg[k]),
			    gmm::mat_col(T_u2_t, i*d+k));
	      }
	    }
	    
	    if (!Tresca_version) {
	      ball_projection_grad_r(gmm::sub_vector(RLT, SUBI), th, vg);
	      for (size_type k = 0; k < d; ++k)
		T_t_n(i*d+k, i) = - friction_coeff[i] * vg[k] / r;
	    }
	    for (size_type k = 0; k < d; ++k) pg(k,k) -= vt1;
	    gmm::copy(gmm::scaled(pg, vt1/r), gmm::sub_matrix(T_t_t, SUBI));
	  }
	  gmm::copy(gmm::transposed(T_u1_t), T_t_u1);
	  if (two_variables) gmm::copy(gmm::transposed(T_u2_t), T_t_u2);
	}

	if (symmetrized) {
	  // gmm::copy(gmm::transposed(T_n_u1), T_u1_n);  // already done
	  // gmm::copy(gmm::transposed(T_n_u2), T_u2_n);  // already done
	  model_real_sparse_matrix tmp1(gmm::mat_ncols(BN1),
					gmm::mat_ncols(BN1));
	  model_real_sparse_matrix tmp2(gmm::mat_ncols(BN2),
					gmm::mat_ncols(BN2));
	  gmm::mult(gmm::transposed(gmm::scaled(BBN1,-r)), T_n_u1, tmp1);
	  gmm::add(tmp1, T_u1_u1);
	  if (two_variables) {
	    gmm::mult(gmm::transposed(gmm::scaled(BBN2,-r)), T_n_u2, tmp2);
	    gmm::add(tmp2, T_u2_u2);
	  }
	  
	  if (!contact_only) {
	    // gmm::copy(gmm::transposed(T_t_u1), T_u1_t);  // already done
	    // gmm::copy(gmm::transposed(T_t_u2), T_u2_t);  // already done
	    gmm::mult(gmm::transposed(gmm::scaled(BBT1,-r)), T_t_u1, tmp1);
	    gmm::add(tmp1, T_u1_u1);
	    if (two_variables) {
	      gmm::mult(gmm::transposed(gmm::scaled(BBT2,-r)), T_t_u2, tmp2);
	      gmm::add(tmp2, T_u2_u2);
	    }
	  }
	}
	else {
	  gmm::copy(gmm::scaled(gmm::transposed(BN1), -vt1), T_u1_n);
	  if (two_variables)
	    gmm::copy(gmm::scaled(gmm::transposed(BN2), -vt1), T_u2_n);
	  if (!contact_only) {
	    gmm::copy(gmm::scaled(gmm::transposed(BT1), -vt1), T_u1_t);
	    if (two_variables)
	      gmm::copy(gmm::scaled(gmm::transposed(BT2), -vt1), T_u2_t);
	  }
	}
      }
      
    if (version & model::BUILD_RHS) {
	for (size_type i=0; i < nbc; ++i) {
	  RLN[i] = std::min(scalar_type(0), RLN[i]);
	  if (!contact_only) {
	    scalar_type radius = Tresca_version ? threshold[i]
	      : -friction_coeff[i]*lambda_n[i];
	    ball_projection
	      (gmm::sub_vector(RLT, gmm::sub_interval(i*d,d)), radius);
	  }
	}
	
	if (symmetrized) {
	  gmm::mult_add(gmm::transposed(BN1), RLN, ru1);
	  if (two_variables) gmm::mult_add(gmm::transposed(BN2), RLN, ru2);
	  if (!contact_only) {
	    gmm::mult_add(gmm::transposed(BT1), RLT, ru1);
	    if (two_variables) gmm::mult_add(gmm::transposed(BT2), RLT, ru2);
	  }
	} else {
	  gmm::mult_add(gmm::transposed(BN1), lambda_n, ru1);
	  if (two_variables)
	    gmm::mult_add(gmm::transposed(BN2), lambda_n, ru2);
	  if (!contact_only) {
	    gmm::mult_add(gmm::transposed(BT1), lambda_t, ru1);
	    if (two_variables)
	      gmm::mult_add(gmm::transposed(BT2), lambda_t, ru2);
	  }
	}
	
	gmm::add(gmm::scaled(lambda_n, vt1/r), gmm::scaled(RLN, -vt1/r),
		 rlambda_n);

	if (!contact_only)
	  gmm::add(gmm::scaled(lambda_t, vt1/r), gmm::scaled(RLT, -vt1/r),
		   rlambda_t);
      }
    }

    // specific part for the basic bricks : BN, BT, gap, r, alpha are given.
    virtual void asm_real_tangent_terms(const model &md, size_type ib,
					const model::varnamelist &vl,
					const model::varnamelist &dl,
					const model::mimlist &mims,
					model::real_matlist &matl,
					model::real_veclist &vecl,
					model::real_veclist &,
					size_type /* region */,
					build_version version) const {
      GMM_ASSERT1(mims.size() == 0, "Contact brick need no mesh_im");
      size_type nbvar = 2 + (contact_only ? 0 : 1) + (two_variables ? 1 : 0); 
      GMM_ASSERT1(vl.size() == nbvar && dl.size() >= 2 && dl.size() <= 3,
		  "Wrong number of variables for contact brick");
      size_type nbc = gmm::mat_nrows(BN1);

      // Variables
      // Without friction and one displacement  : u1, lambda_n
      // Without friction and two displacements : u1, u2, lambda_n
      // With friction and one displacement     : u1, lambda_n, lambda_t 
      // With friction and two displacements    : u1, u2, lambda_n, lambda_t
      size_type nv = 0;
      const model_real_plain_vector &u1 = md.real_variable(vl[nv++]);
      const model_real_plain_vector &u2 = md.real_variable(vl[nv++]);
      if (!two_variables) nv--;
      const model_real_plain_vector &lambda_n = md.real_variable(vl[nv++]);
      if (contact_only) nv--;
      const model_real_plain_vector &lambda_t = md.real_variable(vl[nv]);

      // Parameters
      // (order : r, gap, alpha, friction_coeff, gamma, wt, threshold)
      size_type np = 0, np_wt1 = 0, np_wt2 = 0, np_alpha = 0;
      const model_real_plain_vector &vr = md.real_variable(dl[np++]);
      GMM_ASSERT1(gmm::vect_size(vr) == 1, "Parameter r should be a scalar");
      r = vr[0];
      const model_real_plain_vector &vgap = md.real_variable(dl[np++]);
      GMM_ASSERT1(gmm::vect_size(vgap) == 1 || gmm::vect_size(vgap) == nbc,
		  "Parameter gap has a wrong size");
      gmm::resize(gap, nbc);
      if (gmm::vect_size(vgap) == 1)
	gmm::fill(gap, vgap[0]);
      else
	gmm::copy(vgap, gap);
      np_alpha = np++;
      const model_real_plain_vector &valpha = md.real_variable(dl[np_alpha]);
      GMM_ASSERT1(gmm::vect_size(valpha)== 1 || gmm::vect_size(valpha) == nbc,
		  "Parameter alpha has a wrong size");
      gmm::resize(alpha, nbc);
      if (gmm::vect_size(valpha) == 1)
	gmm::fill(alpha, valpha[0]);
      else
	gmm::copy(valpha, alpha);
      if (!contact_only) {
	const model_real_plain_vector &vfr = md.real_variable(dl[np++]);
	GMM_ASSERT1(gmm::vect_size(vfr)==1 || gmm::vect_size(vfr) == nbc,
		    "Parameter friction_coeff has a wrong size");
	gmm::resize(friction_coeff, nbc);
	if (gmm::vect_size(vfr) == 1)
	  gmm::fill(friction_coeff, vfr[0]);
	else
	  gmm::copy(vfr, friction_coeff);
	if (friction_dynamic_term) {
	  const model_real_plain_vector &vg = md.real_variable(dl[np++]);
	  GMM_ASSERT1(gmm::vect_size(vg) == 1,
		      "Parameter gamma should be a scalar");
	  gamma = vg[0];
	  np_wt1 = np++;
	  if (two_variables) np_wt2 = np++;
	}
	if (Tresca_version) {
	  const model_real_plain_vector &vth = md.real_variable(dl[np++]);
	  GMM_ASSERT1(gmm::vect_size(vth) == 1 || gmm::vect_size(vth) == nbc,
		      "Parameter threshold has a wrong size");
	  gmm::resize(threshold, nbc);
	  if (gmm::vect_size(vth) == 1)
	    gmm::fill(threshold, vth[0]);
	  else
	    gmm::copy(vth, threshold);
	}
      }

      if (md.is_var_newer_than_brick(dl[np_alpha], ib)) is_init = false;

      basic_asm_real_tangent_terms
	(u1, u2, lambda_n, lambda_t, md.real_variable(dl[np_wt1]),
	 md.real_variable(dl[np_wt2]), matl, vecl, version); 

    }

    Coulomb_friction_brick(bool symmetrized_, bool contact_only_) {
      symmetrized = symmetrized_;
      contact_only = contact_only_;
      is_init = false;
      Tresca_version = false;   // future version ...
      really_stationary = false;   // future version ...
      friction_dynamic_term = false;  // future version ...
      two_variables = false;  // future version ...
      set_flags("Coulomb friction brick", false /* is linear*/,
		/* is symmetric */
		symmetrized && (contact_only || Tresca_version),
		false /* is coercive */, true /* is real */,
		false /* is complex */);
    }

    
    void set_BN1(model_real_sparse_matrix &BN1_) {
      gmm::resize(BN1, gmm::mat_nrows(BN1_), gmm::mat_ncols(BN1_));
      gmm::copy(BN1_, BN1);
      is_init = false;
    }

    void set_BT1(model_real_sparse_matrix &BT1_) {
      gmm::resize(BT1, gmm::mat_nrows(BT1_), gmm::mat_ncols(BT1_));
      gmm::copy(BT1_, BT1);
      is_init = false;
    }

    CONTACT_B_MATRIX &get_BN1(void) { return BN1; }
    CONTACT_B_MATRIX &get_BT1(void) { return BT1; }
    const CONTACT_B_MATRIX &get_BN1(void) const { return BN1; }
    const CONTACT_B_MATRIX &get_BT1(void) const { return BT1; }


  };


  CONTACT_B_MATRIX &contact_brick_set_BN
  (model &md, size_type indbrick) {
    pbrick pbr = md.brick_pointer(indbrick);
    md.touch_brick(indbrick);
    Coulomb_friction_brick *p = dynamic_cast<Coulomb_friction_brick *>
      (const_cast<virtual_brick *>(pbr.get()));
    GMM_ASSERT1(p, "Wrong type of brick");
    return p->get_BN1();
  }

  CONTACT_B_MATRIX &contact_brick_set_BT
  (model &md, size_type indbrick) {
    pbrick pbr = md.brick_pointer(indbrick);
    md.touch_brick(indbrick);
    Coulomb_friction_brick *p = dynamic_cast<Coulomb_friction_brick *>
      (const_cast<virtual_brick *>(pbr.get()));
    GMM_ASSERT1(p, "Wrong type of brick");
    return p->get_BT1();
  }
  
  //=========================================================================
  //  Add a frictionless contact condition with BN, r, alpha given.  
  //=========================================================================

  size_type add_basic_contact_brick
  (model &md, const std::string &varname_u, const std::string &multname_n,
   const std::string &dataname_r, model_real_sparse_matrix &BN,
   std::string dataname_gap, std::string dataname_alpha,
   bool symmetrized) {
    Coulomb_friction_brick *pbr_=new Coulomb_friction_brick(symmetrized,true);
    pbr_->set_BN1(BN);
    pbrick pbr = pbr_;

    model::termlist tl;
    tl.push_back(model::term_description(varname_u, varname_u, false));
    tl.push_back(model::term_description(varname_u, multname_n, false));
    tl.push_back(model::term_description(multname_n, varname_u, false));
    tl.push_back(model::term_description(multname_n, multname_n, false));
    model::varnamelist dl(1, dataname_r);

    if (dataname_gap.size() == 0) {
      dataname_gap = md.new_name("contact_gap_on_" + varname_u);
      md.add_initialized_fixed_size_data
	(dataname_gap, model_real_plain_vector(1, scalar_type(0)));
    }
    dl.push_back(dataname_gap);
    
    if (dataname_alpha.size() == 0) {
      dataname_alpha = md.new_name("contact_parameter_alpha_on_"+ multname_n);
      md.add_initialized_fixed_size_data
	(dataname_alpha, model_real_plain_vector(1, scalar_type(1)));
    }
    dl.push_back(dataname_alpha);

    model::varnamelist vl(1, varname_u);
    vl.push_back(multname_n);
    
    return md.add_brick(pbr, vl, dl, tl, model::mimlist(), size_type(-1));
  }
  

  //=========================================================================
  //
  //  Brick with a given rigid obstacle (one body, build BN, BT, gap, alpha)
  //
  //=========================================================================
  // TODO : add an option for a weak contact condition

  struct Coulomb_friction_brick_rigid_obstacle
    : public Coulomb_friction_brick {

    std::string obstacle; // obstacle given with a signed distance expression.

  public :
    
    virtual void asm_real_tangent_terms(const model &md, size_type ib,
					const model::varnamelist &vl,
					const model::varnamelist &dl,
					const model::mimlist &mims,
					model::real_matlist &matl,
					model::real_veclist &vecl,
					model::real_veclist &,
					size_type region,
					build_version version) const {
      GMM_ASSERT1(mims.size() == 1, "This contact brick need one mesh_im");
      size_type nbvar = 2 + (contact_only ? 0 : 1);
      GMM_ASSERT1(vl.size() == nbvar,
		  "Wrong number of variables for contact brick: "
		  << vl.size() << " should be " << nbvar);
      size_type nbdl = 1 + (Tresca_version ? 1 : 0)
	+ (friction_dynamic_term ? 1 : 0);      
      GMM_ASSERT1(dl.size() == nbdl,
		  "Wrong number of data for contact brick: "
		  << dl.size() << " should be " << nbdl);
      GMM_ASSERT1(!two_variables, "internal error");
      const mesh_im &mim = *mims[0];

      // Variables
      // Without friction and one displacement  : u1, lambda_n
      // With friction and one displacement     : u1, lambda_n, lambda_t
      size_type nv = 0;
      const model_real_plain_vector &u1 = md.real_variable(vl[nv++]);
      const mesh_fem &mf_u1 = md.mesh_fem_of_variable(vl[0]);
      const model_real_plain_vector &lambda_n = md.real_variable(vl[nv++]);
      if (contact_only) nv--;
      const model_real_plain_vector &lambda_t = md.real_variable(vl[nv]);


      // Parameters (order : r, friction_coeff, gamma, wt, threshold)
      size_type np = 0, np_wt1 = 0, nbc;
      const model_real_plain_vector &vr = md.real_variable(dl[np++]);
      GMM_ASSERT1(gmm::vect_size(vr) == 1, "Parameter r should be a scalar");
      r = vr[0];

      // Computation of BN, BT, gap and alpha
      if (md.is_var_mf_newer_than_brick(vl[0], ib)) {

	// Verification that mf_u1 is a pure Lagrange fem.
	GMM_ASSERT1(!(mf_u1.is_reduced()),
		    "This contact brick works only for pure Lagrange fems");
	dal::bit_vector dofs = mf_u1.basic_dof_on_region(region);
	for (dal::bv_visitor id(dofs); !id.finished(); ++id) {
	  size_type cv = mf_u1.first_convex_of_basic_dof(id);
	  GMM_ASSERT1(mf_u1.fem_of_element(cv)->is_lagrange(),
		      "This contact brick works only for pure Lagrange fems");
	}
	size_type d = mf_u1.get_qdim() - 1, i = 0;
        nbc = dofs.card() / (d+1);

	// computation of alpha vector.
	base_node Pmin, Pmax;
	mf_u1.linked_mesh().bounding_box(Pmin, Pmax);
	scalar_type l = scalar_type(0);
	for (i = 0; i < Pmin.size(); ++i)
	  l = std::max(l, gmm::abs(Pmax[i] - Pmin[i]));

	CONTACT_B_MATRIX MM(mf_u1.nb_dof(), mf_u1.nb_dof());
	asm_mass_matrix(MM, mim, mf_u1, region);
	gmm::resize(alpha, nbc);
	i = 0;
	for (dal::bv_visitor id(dofs); !id.finished(); ++id, ++i)
	  alpha[i] = MM(id, id) / l;


#if GETFEM_HAVE_MUPARSER_MUPARSER_H


	mu::Parser parser;
	parser.SetExpr(obstacle);

	gmm::resize(gap, nbc);
	gmm::resize(BN1, nbc, mf_u1.nb_dof());
	cerr << "mf_u1.nb_dof() = " << mf_u1.nb_dof() << endl;
	cerr << "nbc = " << nbc << endl;
	gmm::clear(BN1);
	if (!contact_only) {
	  gmm::resize(BT1, d*nbc, mf_u1.nb_dof());
	  gmm::clear(BT1);
	}
	base_node pt(d+1), grad(d+1), ut[3];

	static std::string varn[4] = { "x", "y", "z", "w"};
	for (size_type k = 0; k <= d; ++k)
	  parser.DefineVar(varn[k], &pt[k]);

	parser.DefineVar("x", &pt[0]);
	if (d > 0) parser.DefineVar("y", &pt[1]);
	if (d > 1) parser.DefineVar("z", &pt[2]);
	i = 0;
	for (dal::bv_visitor id(dofs); !id.finished(); ++id, ++i) {
	  if ((i % (d+1)) == 0) {
	    gmm::copy(mf_u1.point_of_basic_dof(id), pt);
	    try {

	      // Computation of gap
	      gap[i] = scalar_type(parser.Eval());
	      
	      // computation of BN
	      size_type cv = mf_u1.first_convex_of_basic_dof(id);
	      scalar_type eps
		= mf_u1.linked_mesh().convex_radius_estimate(cv) * 1E-3;
	      for (size_type k = 0; k <= d; ++k) {
		pt[k] += eps;
		grad[k] = (scalar_type(parser.Eval()) - gap[i]) / eps;
		pt[k] -= eps;
	      }
	      // unit normal vector
	      base_node un = - grad / gmm::vect_norm2(grad);
	      for (size_type k = 0; k <= d; ++k)
		BN1(i, id + k) = un[k];
	      
	      // computation of BT
	      if (!contact_only) {
		
		// Computation of an orthonormal basis to un.
		size_type n = 0;
		for (size_type k = 0; k <= d && n < d; ++k) {
		  gmm::resize(ut[n], d+1);
		  gmm::clear(ut[n]);
		  ut[n][k] = scalar_type(1);
		  
		  ut[n] -= gmm::vect_sp(un, ut[n]) * un;
		  for (size_type nn = 0; nn < n; ++nn)
		    ut[n] -= gmm::vect_sp(ut[nn], ut[n]) * ut[nn];
		  
		  if (gmm::vect_norm2(ut[n]) < 1e-3) continue;
		  ut[n] /= gmm::vect_norm2(ut[n]);
		  ++n;
		}
		GMM_ASSERT1(n == d, "Gram-Schmidt algorithm to find an "
		  "orthonormal basis for the tangential displacement failed");
		
		for (size_type k = 0; k <= d; ++k)
		  for (size_type nn = 0; nn < d; ++nn)
		    BT1(i*d+nn, id + k) = ut[nn][k];
	      }
	      
	    } catch (mu::Parser::exception_type &e) {
	      std::cerr << "Message  : " << e.GetMsg()   << std::endl;
	      std::cerr << "Formula  : " << e.GetExpr()  << std::endl;
	      std::cerr << "Token    : " << e.GetToken() << std::endl;
	      std::cerr << "Position : " << e.GetPos()   << std::endl;
	      std::cerr << "Errc     : " << e.GetCode()  << std::endl;
	      GMM_ASSERT1(false, "Error in signed distance expression");
	    }
	  }
	  
	}

	GMM_ASSERT1(gmm::vect_size(md.real_variable(vl[1])) == nbc, 
		    "Wrong size of multiplier for the contact condition");

	if (!contact_only)
	  GMM_ASSERT1(gmm::vect_size(md.real_variable(vl[2])) == nbc*d, 
		      "Wrong size of multiplier for the friction condition");

#else

	GMM_ASSERT1(false, "Muparser is not installed, "
		    "You cannot used this contact brick");
	
#endif

	is_init = false;
      }
      else 
	nbc = gmm::mat_nrows(BN1);

      if (!contact_only) {
	const model_real_plain_vector &vfr = md.real_variable(dl[np++]);
	GMM_ASSERT1(gmm::vect_size(vfr)==1 || gmm::vect_size(vfr) == nbc,
		    "Parameter friction_coeff has a wrong size");
	gmm::resize(friction_coeff, nbc);
	if (gmm::vect_size(vfr) == 1)
	  gmm::fill(friction_coeff, vfr[0]);
	else
	  gmm::copy(vfr, friction_coeff);
	if (friction_dynamic_term) {
	  const model_real_plain_vector &vg = md.real_variable(dl[np++]);
	  GMM_ASSERT1(gmm::vect_size(vg) == 1,
		      "Parameter gamma should be a scalar");
	  gamma = vg[0];
	  np_wt1 = np++;
	}
	if (Tresca_version) {
	  const model_real_plain_vector &vth = md.real_variable(dl[np++]);
	  GMM_ASSERT1(gmm::vect_size(vth) == 1 || gmm::vect_size(vth) == nbc,
		      "Parameter threshold has a wrong size");
	  gmm::resize(threshold, nbc);
	  if (gmm::vect_size(vth) == 1)
	    gmm::fill(threshold, vth[0]);
	  else
	    gmm::copy(vth, threshold);
	}
      }

      basic_asm_real_tangent_terms
	(u1, u1, lambda_n, lambda_t, md.real_variable(dl[np_wt1]),
	 md.real_variable(dl[np_wt1]), matl, vecl, version);

    }

    Coulomb_friction_brick_rigid_obstacle
    (bool symmetrized_, bool contact_only_, const std::string &obs)
      : Coulomb_friction_brick(symmetrized_, contact_only_), obstacle(obs) {}

  };


  //=========================================================================
  //  Add a frictionless contact condition with a rigid obstacle given
  //  by a signed distance.  
  //=========================================================================

  size_type add_contact_with_rigid_obstacle_brick
  (model &md, const mesh_im &mim, const std::string &varname_u,
   const std::string &multname_n, const std::string &dataname_r,
   size_type region, const std::string &obstacle, bool symmetrized) {
    pbrick pbr
      = new Coulomb_friction_brick_rigid_obstacle(symmetrized, true, obstacle);

    model::termlist tl;
    tl.push_back(model::term_description(varname_u, varname_u, false));
    tl.push_back(model::term_description(varname_u, multname_n, false));
    tl.push_back(model::term_description(multname_n, varname_u, false));
    tl.push_back(model::term_description(multname_n, multname_n, false));
    model::varnamelist dl(1, dataname_r);

    model::varnamelist vl(1, varname_u);
    vl.push_back(multname_n);
    
    return md.add_brick(pbr, vl, dl, tl, model::mimlist(1, &mim), region);
  }








}  /* end of namespace getfem.                                             */
