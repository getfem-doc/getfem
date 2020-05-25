/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================

 Copyright (C) 2002-2020 Yves Renard

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

 As a special exception, you  may use  this file  as it is a part of a free
 software  library  without  restriction.  Specifically,  if   other  files
 instantiate  templates  or  use macros or inline functions from this file,
 or  you compile this  file  and  link  it  with other files  to produce an
 executable, this file  does  not  by itself cause the resulting executable
 to be covered  by the GNU Lesser General Public License.  This   exception
 does not  however  invalidate  any  other  reasons why the executable file
 might be covered by the GNU Lesser General Public License.

===========================================================================*/

/**@file bgeot_poly_composite.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date August 26, 2002.
   @brief Handle composite polynomials.

   Composite polynomials are used in hierarchical FEM, composite geometric
   transformations and composite fems.
*/

#ifndef BGEOT_POLY_COMPOSITE_H__
#define BGEOT_POLY_COMPOSITE_H__

#include "bgeot_poly.h"
#include "bgeot_mesh.h"
#include "bgeot_rtree.h"

// TODO : Use of rtree instead of dal::dynamic_tree_sorted<base_node,
//        imbricated_box_less>


namespace bgeot {

  /// A comparison function for bgeot::base_node
  struct imbricated_box_less
  {
    mutable int exp_max, exp_min;
    mutable scalar_type c_max;
    unsigned base;

    /// comparaison function
    int operator()(const base_node &x, const base_node &y) const;

    imbricated_box_less(unsigned ba = 10, int emi = -15, int ema = -2) {
      base = ba; exp_max = ema; exp_min = emi;
      c_max = pow(double(base), double(-exp_max));
    }
  };



  struct mesh_precomposite {

    typedef dal::dynamic_tree_sorted<base_node, imbricated_box_less> PTAB;

    const basic_mesh *msh;
    PTAB vertices;
    rtree box_tree;
    std::map<size_type, std::vector<size_type>> box_to_convexes_map;
    std::vector<base_matrix> gtrans, gtransinv;
    std::vector<scalar_type> det;
    std::vector<base_node> orgs;
    
    const basic_mesh &linked_mesh(void) const { return *msh; }
    size_type nb_convex(void) const { return gtrans.size(); }
    dim_type dim(void) const { return msh->dim(); }
    pgeometric_trans trans_of_convex(size_type ic) const
    { return msh->trans_of_convex(ic); }
    void initialise(const basic_mesh &m);
    
    mesh_precomposite(const basic_mesh &m);
    mesh_precomposite(void) : msh(0), box_tree(1e-13) {}
  };

  typedef const mesh_precomposite *pmesh_precomposite;

  struct stored_base_poly : base_poly, public dal::static_stored_object {
    stored_base_poly(const base_poly &p) : base_poly(p) {}
  };
  typedef std::shared_ptr<const stored_base_poly> pstored_base_poly;

  
  class polynomial_composite {

  protected :
    const mesh_precomposite *mp;
    std::map<size_type, pstored_base_poly> polytab;
    bool local_coordinate;  // Local coordinates on each sub-element for
                            // polynomials or global coordinates ?
    bool faces_first; // If true try to evaluate on faces before on the
                      // interior, usefull for HHO elements.
    std::vector<base_poly> default_polys;

  public :
    scalar_type eval(const base_node &p, size_type l) const;

    template <class ITER> scalar_type eval(const ITER &it,
                                           size_type l = -1) const;
    void derivative(short_type k);
    void set_poly_of_subelt(size_type l, const base_poly &poly);
    const base_poly &poly_of_subelt(size_type l) const;
    size_type nb_subelt() const { return polytab.size(); }

    polynomial_composite(bool lc = true, bool ff = false)
      : local_coordinate(lc), faces_first(ff) {}
    polynomial_composite(const mesh_precomposite &m, bool lc = true,
                         bool ff = false);

  };

  inline std::ostream &operator <<
  (std::ostream &o, const polynomial_composite& P) {
    o << "poly_composite [";
    for (size_type i = 0; i < P.nb_subelt(); ++i) {
      if (i != 0) o << ", " << P.poly_of_subelt(i);
    }
    o << "]";
    return o;
  }

  template <class ITER>
  scalar_type polynomial_composite::eval(const ITER &it, size_type l) const {
    base_node p(mp->dim());
    std::copy(it, it+mp->dim(), p.begin());
    return eval(p,l);
  }

  void structured_mesh_for_convex(pconvex_ref cvr, short_type k,
				  pbasic_mesh &pm, pmesh_precomposite &pmp,
				  bool force_simplexification=false);

  /** simplexify a convex_ref.
      @param cvr the convex_ref.
      @param k the refinement level.
      @return a pointer to a statically allocated mesh. Do no free it!
  */
  const basic_mesh *
  refined_simplex_mesh_for_convex(pconvex_ref cvr, short_type k);

  /** simplexify the faces of a convex_ref

      @param cvr the convex_ref.

      @param k the refinement level.

      @return vector of pointers to a statically allocated
      mesh_structure objects. Do no free them! The point numbers in
      the mesh_structure refer to the points of the mesh given by
      refined_simplex_mesh_for_convex.
  */      
  const std::vector<std::unique_ptr<mesh_structure>>&
  refined_simplex_mesh_for_convex_faces(pconvex_ref cvr, short_type k);
}  /* end of namespace bgeot.                                            */


#endif
