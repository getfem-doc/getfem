// -*- c++ -*- (enables emacs c++ mode)
//========================================================================
//
// Library : Basic GEOmetric Tool  (bgeot)
// File    : bgeot_convex_ref.cc : convexes of reference
//           
// Date    : Septembre 28, 2001.
// Author  : Yves Renard <Yves.Renard@insa-toulouse.fr>
//
//========================================================================
//
// Copyright (C) 2001-2005 Yves Renard
//
// This file is a part of GETFEM++
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//========================================================================

#include <dal_singleton.h>
#include <bgeot_convex_ref.h>
#include <bgeot_simplexify.h>

namespace bgeot {
  
  /* ********************************************************************* */
  /*       Point tab storage.                                              */
  /* ********************************************************************* */

  struct stored_point_tab_key : virtual public dal::static_stored_object_key  {
    const stored_point_tab * pspt;
    virtual bool compare(const static_stored_object_key &oo) const {
      const stored_point_tab_key &o
	= dynamic_cast<const stored_point_tab_key &>(oo);
      const stored_point_tab &x = *pspt;
      const stored_point_tab &y = *(o.pspt);
      std::vector<base_node>::const_iterator it1 = x.begin(), it2 = y.begin();
      base_node::const_iterator itn1, itn2, itne;
      for ( ; it1 != x.end() && it2 != y.end() ; ++it1, ++it2) {
	if ((*it1).size() < (*it2).size()) return true;
	if ((*it1).size() > (*it2).size()) return false;
	itn1 = (*it1).begin(); itne = (*it1).end(); itn2 = (*it2).begin();
	for ( ; itn1 != itne ; ++itn1, ++itn2)
	  if (*itn1 < *itn2) return true;
	  else if (*itn1 > *itn2) return false;
      }
      if (it2 != y.end()) return true;
      return false;
    }
    stored_point_tab_key(const stored_point_tab *p) : pspt(p) {}
  };
  
  
  pstored_point_tab store_point_tab(const stored_point_tab& spt) {
    dal::pstatic_stored_object o
      = dal::search_stored_object(stored_point_tab_key(&spt));
    if (o) return dal::stored_cast<stored_point_tab>(o);
    pstored_point_tab p = new stored_point_tab(spt);
    dal::add_stored_object(new stored_point_tab_key(p.get()), p,
			   dal::AUTODELETE_STATIC_OBJECT);
    return p;
  }

  struct cleanup_simplexified_convexes
    : public dal::ptr_collection<mesh_structure> {};

  /* should be called on the basic_convex_ref */
  const mesh_structure*
  convex_of_reference::simplexified_convex() const {    
    if (psimplexified_convex == 0) {
      psimplexified_convex = new mesh_structure();
      dal::singleton<cleanup_simplexified_convexes>::instance().push_back(psimplexified_convex);
      if (this != basic_convex_ref()) 
	DAL_THROW(to_be_done_error, 
                  "always use simplexified_convex on the basic_convex_ref() [this=" << 
                  nb_points() << ", basic=" << basic_convex_ref()->nb_points());
      mesh_structure ms;
      std::vector<size_type> ipts(nb_points());
      for (size_type i=0; i < ipts.size(); ++i) ipts[i] = i;
      ms.add_convex(structure(), ipts.begin());
      ms.to_edges();
      bgeot::simplexify(ms,*psimplexified_convex,
			convex<base_node>::points(), 
			std::max(structure()->dim(),dim_type(1)), 1e-12);
    }
    return psimplexified_convex;
  }

  // Key type for static storing
  class convex_of_reference_key : virtual public dal::static_stored_object_key{
    int type; // 0 = simplex structure degree K
              // 1 = equilateral simplex of ref.
              // 2 = dummy
    dim_type N; short_type K; short_type nf;
  public :
    virtual bool compare(const static_stored_object_key &oo) const {
      const convex_of_reference_key &o
	= dynamic_cast<const convex_of_reference_key &>(oo);
      if (type < o.type) return true;
      if (type > o.type) return false;
      if (N < o.N) return true;
      if (N > o.N) return false;
      if (K < o.K) return true;
      if (K > o.K) return false;
      if (nf < o.nf) return true;      
      return false;
    }
    convex_of_reference_key(int t, dim_type NN, short_type KK = 0,
			    short_type nnf = 0)
      : type(t), N(NN), K(KK), nf(nnf) {}
  };


  /* simplexes.                                                            */

  class K_simplex_of_ref_ : public convex_of_reference {
  public :
    scalar_type is_in(const base_node &pt) const {
      // return a negative or null number if pt is in the convex
      if (pt.size() != cvs->dim())
	throw dimension_error
	  ("K_simplex_of_ref_::is_in : Dimension does not match");
      scalar_type e = -1.0, r = 0.0;
      base_node::const_iterator it = pt.begin(), ite = pt.end();
      for (; it != ite; e += *it, ++it) r = std::min(r, *it);
      return std::max(-r, e);
    }
    scalar_type is_in_face(short_type f, const base_node &pt) const {
      // return a null number if pt is in the face of the convex
      if (pt.size() != cvs->dim())
	throw dimension_error
	  ("K_simplex_of_ref_::is_in_face : Dimension does not match");
      if (f > 0) return gmm::abs(pt[f-1]);
      scalar_type e = -1.0;
      base_node::const_iterator it = pt.begin(), ite = pt.end();
      for (; it != ite; e += *it, ++it);
      return gmm::abs(e);
    }
    K_simplex_of_ref_(dim_type NN, short_type KK) {
      cvs = simplex_structure(NN, KK);
      size_type R = cvs->nb_points();
      convex<base_node>::points().resize(R);
      normals_.resize(NN+1);
      base_node null(NN); null.fill(0.0);
      std::fill(normals_.begin(), normals_.end(), null);
      std::fill(convex<base_node>::points().begin(),
		convex<base_node>::points().end(), null);
      for (size_type i = 1; i <= NN; ++i)
	normals_[i][i-1] = -1.0;
      if (NN > 0)
	std::fill(normals_[0].begin(), normals_[0].end(),
		  scalar_type(1.0)/sqrt(scalar_type(NN)));
      base_node c(NN);  c.fill(0.0);
      
      if (KK == 0) {
	c.fill(1.0/(NN+1));
	convex<base_node>::points()[0] = c;
      }
      else {
	size_type sum = 0, l;
	for (size_type r = 0; r < R; ++r) {
	  convex<base_node>::points()[r] = c;
	  if (KK != 0 && NN > 0) {
	    l = 0; c[l] += 1.0 / scalar_type(KK); sum++;
	    while (sum > KK) {
	      sum -= int(floor(0.5+(c[l] * KK)));
	      c[l] = 0.0; l++; if (l == NN) break;
	      c[l] += 1.0 / scalar_type(KK); sum++;
	    }
	  }
	}
      }
      ppoints = store_point_tab(convex<base_node>::points());
    }
  };

  pconvex_ref simplex_of_reference(dim_type nc, short_type K) {
    dal::pstatic_stored_object o
      = dal::search_stored_object(convex_of_reference_key(0, nc, K));
    if (o) return dal::stored_cast<convex_of_reference>(o);
    pconvex_ref p = new K_simplex_of_ref_(nc, K);
    dal::add_stored_object(new convex_of_reference_key(0, nc, K), p,
			   p->structure(), &(p->points()),
			   dal::PERMANENT_STATIC_OBJECT);
    pconvex_ref p1 = simplex_of_reference(nc, 1);
    p->attach_basic_convex_ref(p1);
    if (p != p1) add_dependency(p, p1); 
    return p;
  }

  /* products.                                                             */

  struct product_ref_key_ : virtual public dal::static_stored_object_key {
    pconvex_ref cvr1, cvr2;
    virtual bool compare(const static_stored_object_key &oo) const {
      const product_ref_key_ &o = dynamic_cast<const product_ref_key_ &>(oo);
      if (cvr1 < o.cvr1) return true;
      if (o.cvr1 < cvr1) return false; 
      if (cvr2 < o.cvr2) return true;
      return false;
    }
    product_ref_key_(pconvex_ref a, pconvex_ref b) { cvr1 = a; cvr2 = b; }
  };

  struct product_ref_ : public convex_of_reference {
    pconvex_ref cvr1, cvr2;
    
    scalar_type is_in(const base_node &pt) const {
      dim_type n1 = cvr1->structure()->dim(), n2 = cvr2->structure()->dim();
      base_node pt1(n1), pt2(n2);
      if (pt.size() != cvs->dim())
	throw dimension_error
	  ("product_ref_::is_in : Dimension does not match");
      std::copy(pt.begin(), pt.begin()+n1, pt1.begin());
      std::copy(pt.begin()+n1,   pt.end(), pt2.begin());
      return std::max(cvr1->is_in(pt1), cvr2->is_in(pt2));
    }

    scalar_type is_in_face(short_type f, const base_node &pt) const {
      // ne controle pas si le point est dans le convexe mais si un point
      // suppos� appartenir au convexe est dans une face donn�e
      dim_type n1 = cvr1->structure()->dim(), n2 = cvr2->structure()->dim();
      base_node pt1(n1), pt2(n2);
      if (pt.size() != cvs->dim())
	DAL_THROW(dimension_error, "Dimensions mismatch");
      std::copy(pt.begin(), pt.begin()+n1, pt1.begin());
      std::copy(pt.begin()+n1,   pt.end(), pt2.begin());
      if (f < cvr1->structure()->nb_faces()) return cvr1->is_in_face(f, pt1);
      else return cvr2->is_in_face(f - cvr1->structure()->nb_faces(), pt2);
    }

    product_ref_(pconvex_ref a, pconvex_ref b) { 
      if (a->structure()->dim() < b->structure()->dim())
	DAL_WARNING(1, "Illegal convex : swap your operands: dim(cv1)=" << 
		    int(a->structure()->dim()) << " < dim(cv2)=" << 
		    int(b->structure()->dim()));
      cvr1 = a; cvr2 = b;
      *((convex<base_node> *)(this)) = convex_direct_product(*a, *b);
      normals_.resize(cvs->nb_faces());
      base_small_vector null(cvs->dim()); null.fill(0.0);
      std::fill(normals_.begin(), normals_.end(), null);
      for (size_type r = 0; r < cvr1->structure()->nb_faces(); r++)
	std::copy(cvr1->normals()[r].begin(), cvr1->normals()[r].end(),
		  normals_[r].begin());
      for (size_type r = 0; r < cvr2->structure()->nb_faces(); r++)
	std::copy(cvr2->normals()[r].begin(), cvr2->normals()[r].end(),
		  normals_[r+cvr1->structure()->nb_faces()].begin()
		  + cvr1->structure()->dim());
      ppoints = store_point_tab(convex<base_node>::points());
    }
  };


  pconvex_ref convex_ref_product(pconvex_ref a, pconvex_ref b) {
    dal::pstatic_stored_object o
      = dal::search_stored_object(product_ref_key_(a, b));
    if (o) return dal::stored_cast<convex_of_reference>(o);
    pconvex_ref p = new product_ref_(a, b);
    dal::add_stored_object(new product_ref_key_(a, b), p, a, b,
			   convex_product_structure(a->structure(),
						    b->structure()),
			   &(p->points()), dal::PERMANENT_STATIC_OBJECT);
    pconvex_ref p1 = convex_ref_product(a->basic_convex_ref(),
					b->basic_convex_ref());
    p->attach_basic_convex_ref(p1);
    if (p != p1) add_dependency(p, p1); 
    return p;
  }

  struct parallelepiped_of_reference_tab
    : public dal::dynamic_array<pconvex_ref> {};

  pconvex_ref parallelepiped_of_reference(dim_type nc) {
    parallelepiped_of_reference_tab &tab
      = dal::singleton<parallelepiped_of_reference_tab>::instance();
    static dim_type ncd = 1;
    if (nc <= 1) return simplex_of_reference(nc);
    if (nc > ncd) { 
      tab[nc] = convex_ref_product(parallelepiped_of_reference(nc-1),
				   simplex_of_reference(1));
      ncd = nc;
    }
    return tab[nc];
  }

  /* equilateral ref convexes are used for estimation of convex quality */
  class equilateral_simplex_of_ref_ : public convex_of_reference {
  public:
    scalar_type is_in(const base_node &pt) const {
      scalar_type d;
      for (size_type f = 0; f < normals().size(); ++f) {
        const base_node &x0 = (f ? convex<base_node>::points()[f-1]
			       : convex<base_node>::points().back());
        scalar_type v = gmm::vect_sp(pt-x0, normals()[f]);
        if (f == 0) d = v; else d = std::max(d,v);
      }
      return d;
    }
    scalar_type is_in_face(short_type f, const base_node &pt) const {
      const base_node &x0 = (f ? convex<base_node>::points()[f-1]
			     : convex<base_node>::points().back());
      return gmm::abs(gmm::vect_sp(pt-x0, normals()[f])); 
    }
    equilateral_simplex_of_ref_(size_type N) {
      pconvex_ref prev = equilateral_simplex_of_reference(N-1);
      basic_convex_ref_ = this;
      cvs = simplex_structure(N, 1);
      convex<base_node>::points().resize(N+1);
      normals_.resize(N+1);
      base_node G(N); G.fill(0.);
      for (size_type i=0; i < N+1; ++i) {
        convex<base_node>::points()[i].resize(N);
        if (i != N) {
          std::copy(prev->convex<base_node>::points()[i].begin(),
		    prev->convex<base_node>::points()[i].end(),
		    convex<base_node>::points()[i].begin());
          convex<base_node>::points()[i][N-1] = 0.;
        } else {
          convex<base_node>::points()[i] = 1./N * G;
          convex<base_node>::points()[i][N-1]
	    = sqrt(1. - gmm::vect_norm2_sqr(convex<base_node>::points()[i]));
        }
        G += convex<base_node>::points()[i];
      }
      gmm::scale(G, 1./(N+1));
      for (size_type f=0; f < N+1; ++f) {
        normals_[f] = G - convex<base_node>::points()[f]; 
        gmm::scale(normals_[f], 1/gmm::vect_norm2(normals_[f]));
      }
      ppoints = store_point_tab(convex<base_node>::points());
    }
  };

  pconvex_ref equilateral_simplex_of_reference(dim_type nc) {
    if (nc <= 1) return simplex_of_reference(nc);
    dal::pstatic_stored_object o
      = dal::search_stored_object(convex_of_reference_key(1, nc));
    if (o) return dal::stored_cast<convex_of_reference>(o);
    pconvex_ref p = new equilateral_simplex_of_ref_(nc);
    dal::add_stored_object(new convex_of_reference_key(1, nc), p,
			   p->structure(), &(p->points()),
			   dal::PERMANENT_STATIC_OBJECT);
    return p;
  }


  /* generic convex with n global nodes      */

  class generic_dummy_ : public convex_of_reference {
  public:
    scalar_type is_in(const base_node &) const
    { DAL_THROW(failure_error, "Information not available here"); }
    scalar_type is_in_face(short_type, const base_node &) const 
    { DAL_THROW(failure_error, "Information not available here"); }
  
    generic_dummy_(dim_type d, size_type n, size_type nf) {
      cvs = generic_dummy_structure(d, n, nf);
      convex<base_node>::points().resize(n);
      normals_.resize(0);
      base_node P(d);
      std::fill(P.begin(), P.end(), scalar_type(1)/scalar_type(20));
      std::fill(convex<base_node>::points().begin(), convex<base_node>::points().end(), P);
      ppoints = store_point_tab(convex<base_node>::points());
    }
  };

  pconvex_ref generic_dummy_convex_ref(dim_type nc, size_type n,
				       size_type nf) {
    dal::pstatic_stored_object o
      = dal::search_stored_object(convex_of_reference_key(2, nc, n, nf));
    if (o) return dal::stored_cast<convex_of_reference>(o);
    pconvex_ref p = new generic_dummy_(nc, n, nf);
    dal::add_stored_object(new convex_of_reference_key(2, nc, n, nf), p,
			   p->structure(), &(p->points()),
			   dal::PERMANENT_STATIC_OBJECT);
    return p;
  }


}  /* end of namespace bgeot.                                              */
