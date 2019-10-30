/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================

 Copyright (C) 2000-2017 Yves Renard

 This file is a part of GetFEM++

 GetFEM++  is  free software;  you  can  redistribute  it  and/or modify it
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

/** @file bgeot_geometric_trans.h
    @author  Yves Renard <Yves.Renard@insa-lyon.fr>
    @date December 20, 2000.
    @brief Geometric transformations on convexes.
*/

#ifndef BGEOT_GEOMETRIC_TRANSFORMATION_H__
#define BGEOT_GEOMETRIC_TRANSFORMATION_H__

#include <set>
#include "bgeot_config.h"
#include "bgeot_convex_ref.h"
#include "getfem/dal_naming_system.h"

namespace bgeot {

  /**  Description of a geometric transformation between a
   * reference element and a real element.
   *
   * Geometric nodes and vector of polynomials. This class is not to
   * be manipulate by itself.  Use bgeot::pgeometric_trans and the
   * functions written to produce the basic geometric transformations.
   *
   * <h3>Description of the geometry</h3>
   *     Let @f$T \in\ {I\hspace{-0.3em}R}^N@f$ be a real element and
   *     @f$\overline{T} \in\ {I\hspace{-0.3em}R}^P@f$ be a reference element,
   *      with @f$N >= P@f$.
   *
   *     The geometric nodes of @f$\overline{T}@f$ are the points
   *     @f$\overline{g}^i \in\ {I\hspace{-0.3em}R}^P@f$, for @f$i = 0 .. n_g-1@f$,
   *     and the corresponding (via the geometric transformation) nodes of
   *     @f$T@f$ are the points @f$g^i \in\ {I\hspace{-0.3em}R}^N@f$.
   *
   *  <h3>Geometric transformation</h3>
   *     The geometric transformation is the application
   *     @f[ \begin{array}{rl}
   *        \tau : \overline{T} & \longrightarrow \ T, \\
   *               \overline{x} & \longmapsto \ \ x,
   *     \end{array} @f]
   *     which should be a diffeomorphism between @f$\overline{T}@f$ and @f$T@f$.
   *     It is assumed that there exists a (generally polynomial) vector
   *     @f$ \underline{\cal N}(\overline{x})
   *        = \left({\cal N}i_(\overline{x})\right)i_, \ \ i = 0 .. n_g-1, @f$
   *     defined on @f$\overline{T}@f$ of size @f$n_g@f$, such that the
   *     transformation
   *     @f$\tau@f$ can be written
   *     @f$ \tau(\overline{x}) = \sum_{i = 0}^{n_g-1} {\cal N}i_(\overline{x})
   *     g^i@f$.
   *
   *     Denoting by
   *     @f$ \underline{\underline{G}} = (g^0; g^1; ...;g^{n_g-1}), @f$
   *     The matrix in which each column is a geometric node of @f$T@f$,
   *     the transformation @f$\tau@f$ can be written as
   *     @f$ \tau(\overline{x}) = \underline{\underline{G}} \
   *        \underline{\cal N}(\overline{x}). @f$
   *
   *  <h3>Gradient of the transformation</h3>
   *     The gradient of the transformation is
   *     @f[ \nabla \tau(\overline{x}) =
   *     \left( \frac{\partial \tau_i}{\partial \overline{x}_j} \right)_{ij}
   *     = \left( \sum_{l = 0}^{n_g-1}g^l_i
   *     \frac{\partial {\cal N}l_(\overline{x})}{\partial \overline{x}_j}
   *     \right)_{ij} = \underline{\underline{G}}\ \nabla
   *     \underline{\cal N}(\overline{x}), @f]
   *
   *     Remark : @f$\underline{\underline{G}}@f$ is a @f$N \times n_g@f$ matrix,
   *       @f$\nabla \underline{\cal N}(\overline{x})@f$ is a @f$n_g \times P@f$
   *       matrix, and thus @f$\nabla \tau(\overline{x})@f$ is a @f$N \times P@f$
   *       matrix.
   *
   *  <h3>Inverse transformation and pseudo-inverse</h3>
   *     to do ...
   */
  class APIDECL geometric_trans : virtual public dal::static_stored_object {
  protected :

    bool is_lin;
    pconvex_ref cvr;
    std::vector<size_type> vertices_;
    size_type complexity_; /* either the degree or the refinement of the
                            *  transformation */
    std::string name_;

    void fill_standard_vertices();
  public :

    /// Dimension of the reference element.
    dim_type dim() const { return cvr->structure()->dim(); }
    /// True if the transformation is linear (affine in fact).
    bool is_linear() const { return is_lin; }
    /// Number of geometric nodes.
    size_type nb_points() const { return cvr->nb_points(); }
    /// Pointer on the convex of reference.
    pconvex_ref convex_ref() const { return cvr; }
    /// Structure of the reference element.
    pconvex_structure structure() const { return cvr->structure(); }
    /// Basic structure of the reference element.
    pconvex_structure basic_structure() const
      { return bgeot::basic_structure(cvr->structure()); }
    /// Gives the value of the functions vector at a certain point.
    virtual void poly_vector_val(const base_node &pt, base_vector &val) const = 0;
    /// Gives the value of a subgroup of the functions vector at a certain point.
    virtual void poly_vector_val(const base_node &pt, const convex_ind_ct &ind_ct,
                                 base_vector &val) const = 0;
    /// Gives the gradient of the functions vector at a certain point.
    virtual void poly_vector_grad(const base_node &pt, base_matrix &val) const = 0;
    /// Gives the gradient of a subgroup of the functions vector at a certain point.
    virtual void poly_vector_grad(const base_node &pt, const convex_ind_ct &ind_ct,
                                  base_matrix &val) const = 0;
    /// Gives the hessian of the functions vector at a certain point.
    virtual void poly_vector_hess(const base_node &pt, base_matrix &val) const = 0;
    /// compute K matrix from multiplication of G with gradient
    virtual void compute_K_matrix(const base_matrix &G, const base_matrix &pc, base_matrix &K) const;
    /// Gives the number of vertices.
    size_type nb_vertices() const { return vertices_.size(); }
    /// Gives the indices of vertices between the nodes.
    const std::vector<size_type> &vertices() const { return vertices_; }
    /// Gives the array of geometric nodes (on reference convex)
    const stored_point_tab &geometric_nodes() const
    { return cvr->points(); }
    /// Gives the array of geometric nodes (on reference convex)
    pstored_point_tab pgeometric_nodes() const
    { return cvr->pspt(); }
    /// Gives the array of the normals to faces (on reference convex)
    const std::vector<base_small_vector> &normals() const
    { return cvr->normals(); }
    /** Apply the geometric transformation to point pt,
        PTAB contains the points of the real convex */
    template<class CONT> base_node transform(const base_node &pt,
                                             const CONT &PTAB) const;
    base_node transform(const base_node &pt, const base_matrix &G) const;
    void set_name(const std::string &name){name_ = name;}
    const std::string& debug_name() const {return name_;}
    virtual void project_into_reference_convex(base_node &pt) const
      { cvr->project_into(pt); }
    size_type complexity() const { return complexity_; }
    virtual ~geometric_trans()
      { DAL_STORED_OBJECT_DEBUG_DESTROYED(this, "Geometric transformation"); }
    geometric_trans()
      { DAL_STORED_OBJECT_DEBUG_CREATED(this, "Geometric transformation"); }
  };

  template<class CONT>
  base_node geometric_trans::transform(const base_node &pt,
                                       const CONT &ptab) const {
    base_node P(ptab[0].size());
    size_type k = nb_points();
    base_vector val(k);
    poly_vector_val(pt, val);
    for (size_type l = 0; l < k; ++l) gmm::add(gmm::scaled(ptab[l], val[l]),P);
    return P;
  }

  /** pointer type for a geometric transformation */
  typedef std::shared_ptr<const bgeot::geometric_trans> pgeometric_trans;
  class geotrans_interpolation_context;

  template<class CONT>
  void bounding_box(base_node& min, base_node& max,
                    const CONT &ptab, pgeometric_trans pgt = 0) {
    typename CONT::const_iterator it = ptab.begin();
    min = max = *it; size_type P = min.size();
    base_node::iterator itmin = min.begin(), itmax = max.begin();
    for ( ++it; it != ptab.end(); ++it) {
      base_node pt = *it; /* need a temporary storage since cv.points()[j] may
                             not be a reference to a base_node, but a
                             temporary base_node !! (?) */
      base_node::const_iterator it2 = pt.begin();
      for (size_type i = 0; i < P; ++i) {
        itmin[i] = std::min(itmin[i], it2[i]);
        itmax[i] = std::max(itmax[i], it2[i]);
      }
    }
    /* enlarge the box for non-linear transformations .. */
    if (pgt && !pgt->is_linear())
      for (size_type i = 0; i < P; ++i) {
        scalar_type e = (itmax[i]-itmin[i]) * 0.2;
        itmin[i] -= e; itmax[i] += e;
      }
  }

  /** @name functions on geometric transformations
   */
  //@{

  pgeometric_trans APIDECL simplex_geotrans(size_type n, short_type k);
  pgeometric_trans APIDECL parallelepiped_geotrans(size_type n, short_type k);
  pgeometric_trans APIDECL parallelepiped_linear_geotrans(size_type n);
  pgeometric_trans APIDECL prism_geotrans(size_type n, short_type k);
  pgeometric_trans APIDECL prism_linear_geotrans(size_type n);
  pgeometric_trans APIDECL product_geotrans(pgeometric_trans pg1,
                                    pgeometric_trans pg2);
  pgeometric_trans APIDECL linear_product_geotrans(pgeometric_trans pg1,
                                           pgeometric_trans pg2);
  pgeometric_trans APIDECL Q2_incomplete_geotrans(dim_type nc);
  pgeometric_trans APIDECL prism_incomplete_P2_geotrans();
  pgeometric_trans APIDECL pyramid_QK_geotrans(short_type k);
  IS_DEPRECATED inline pgeometric_trans APIDECL
  pyramid_geotrans(short_type k) { return pyramid_QK_geotrans(k); }
  pgeometric_trans APIDECL pyramid_Q2_incomplete_geotrans();

  pgeometric_trans APIDECL default_trans_of_cvs(pconvex_structure);

  /**
     Get the geometric transformation from its string name.
     @see name_of_geometric_trans
  */
  pgeometric_trans APIDECL geometric_trans_descriptor(std::string name);
  /**
     Get the string name of a geometric transformation.

     List of possible names:
     GT_PK(N,K)   : Transformation on simplexes, dim N, degree K

     GT_QK(N,K)         : Transformation on parallelepipeds, dim N, degree K
     GT_PRISM(N,K)      : Transformation on prisms, dim N, degree K
     GT_PYRAMID_QK(K)   : Transformation on pyramids, dim 3, degree K=0,1,2
     GT_Q2_INCOMPLETE(N)      : Q2 incomplete transformation in dim N=2 or 3.
     GT_PYRAMID_Q2_INCOMPLETE : incomplete quadratic pyramid transformation
                                in dim 3
     GT_PRISM_INCOMPLETE_P2   : incomplete quadratic prism transformation in
                                dim 3
     GT_PRODUCT(a,b)          : tensorial product of two transformations
     GT_LINEAR_PRODUCT(a,b)   : Linear tensorial product of two transformations
     GT_LINEAR_QK(N) : shortcut for GT_LINEAR_PRODUCT(GT_LINEAR_QK(N-1),
                                                      GT_PK(1,1))
   */

  std::string APIDECL name_of_geometric_trans(pgeometric_trans p);

  /** norm of returned vector is the ratio between the face surface on
   *  the real element and the face surface on the reference element
   *  IT IS NOT UNITARY
   *
   *  pt is the position of the evaluation point on the reference element
   */
  base_small_vector APIDECL
  compute_normal(const geotrans_interpolation_context& c, size_type face);

  /** return the local basis (i.e. the normal in the first column, and the
   *  tangent vectors in the other columns
   */
  base_matrix APIDECL
  compute_local_basis(const geotrans_interpolation_context& c,
                      size_type face);
    //@}

  /* ********************************************************************* */
  /*       Precomputation on geometric transformations.                    */
  /* ********************************************************************* */


  class geotrans_precomp_;
  typedef std::shared_ptr<const geotrans_precomp_> pgeotrans_precomp;


  /**
   *  precomputed geometric transformation operations use this for
   *  repetitive evaluation of a geometric transformations on a set of
   *  points "pspt" in the reference convex which do not change.
   */
  class APIDECL geotrans_precomp_ : virtual public dal::static_stored_object {
  protected:
    pgeometric_trans pgt;
    pstored_point_tab pspt;  /* a set of points in the reference elt*/
    mutable std::vector<base_vector> c;  /* precomputed values for the     */
                                         /* transformation                 */
    mutable std::vector<base_matrix> pc; /* precomputed values for gradient*/
                                         /* of the transformation.         */
    mutable std::vector<base_matrix> hpc; /* precomputed values for hessian*/
                                          /*  of the transformation.       */
  public:
    inline const base_vector &val(size_type i) const
    { if (c.empty()) init_val(); return c[i]; }
    inline const base_matrix &grad(size_type i) const
    { if (pc.empty()) init_grad(); return pc[i]; }
    inline const base_matrix &hessian(size_type i) const
    { if (hpc.empty()) init_hess(); return hpc[i]; }

    /**
     *  Apply the geometric transformation from the reference convex to
     *  the convex whose vertices are stored in G, to the set of points
     *  listed in pspt.
     *  @param G any container of vertices of the transformed
     *  convex.
     *  @param pt_tab on output, the transformed points.
     */
    template <typename CONT>
    void transform(const CONT& G,
                   stored_point_tab& pt_tab) const;
    template <typename CONT, typename VEC>
    void transform(const CONT& G, size_type ii, VEC& pt) const;

    base_node transform(size_type i, const base_matrix &G) const;
    pgeometric_trans get_trans() const { return pgt; }
    // inline const stored_point_tab& get_point_tab() const { return *pspt; }
    inline pstored_point_tab get_ppoint_tab() const { return pspt; }
    geotrans_precomp_(pgeometric_trans pg, pstored_point_tab ps);
    ~geotrans_precomp_()
      { DAL_STORED_OBJECT_DEBUG_DESTROYED(this, "Geotrans precomp"); }

  private:
    void init_val() const;
    void init_grad() const;
    void init_hess() const;

    /**
     *  precomputes a geometric transformation for a fixed set of
     *  points in the reference convex.
     */
    friend pgeotrans_precomp
    geotrans_precomp(pgeometric_trans pg, pstored_point_tab ps,
                     dal::pstatic_stored_object dep);

  };


  pgeotrans_precomp
  geotrans_precomp(pgeometric_trans pg, pstored_point_tab ps,
                   dal::pstatic_stored_object dep);

  template <typename CONT, typename VEC>
  void geotrans_precomp_::transform(const CONT& G, size_type j,
                                    VEC& pt) const {
    size_type k = 0;
    gmm::clear(pt);
    if (c.empty()) init_val();
    for (typename CONT::const_iterator itk = G.begin();
         itk != G.end(); ++itk, ++k)
      gmm::add(gmm::scaled(*itk, c[j][k]), pt);
    GMM_ASSERT1(k == pgt->nb_points(),
                "Wrong number of points in transformation");
  }

  template <typename CONT>
  void geotrans_precomp_::transform(const CONT& G,
                                    stored_point_tab& pt_tab) const {
    if (c.empty()) init_val();
    pt_tab.clear(); pt_tab.resize(c.size(), base_node(G[0].size()));
    for (size_type j = 0; j < c.size(); ++j) {
      transform(G, j, pt_tab[j]);
    }
  }

  void APIDECL delete_geotrans_precomp(pgeotrans_precomp pgp);

  /**
   *  The object geotrans_precomp_pool Allow to allocate a certain number
   *  of geotrans_precomp and automatically delete them when it is
   *  deleted itself.
   */
  class APIDECL geotrans_precomp_pool {
    std::set<pgeotrans_precomp> precomps;

  public :

    pgeotrans_precomp operator()(pgeometric_trans pg,
                                 pstored_point_tab pspt) {
      pgeotrans_precomp p = geotrans_precomp(pg, pspt, 0);
      precomps.insert(p);
      return p;
    }
    ~geotrans_precomp_pool() {
      for (std::set<pgeotrans_precomp>::iterator it = precomps.begin();
           it != precomps.end(); ++it)
        delete_geotrans_precomp(*it);
    }
  };



  /** the geotrans_interpolation_context structure is passed as the
     argument of geometric transformation interpolation
     functions. This structure can be partially filled (for example
     the xreal will be computed if needed as long as pgp+ii is known).
     See also fem_interpolation_context in getfem_fem.h.
     The name of member data, and the computations done by this structure
     are heavily described in the GetFEM++ Kernel Documentation.
  */
  class APIDECL geotrans_interpolation_context {
  protected:
    mutable base_node xref_;  /** reference point */
    mutable base_node xreal_; /** transformed point */
    const base_matrix *G_;    /** pointer to the matrix of real nodes of the convex */
    mutable base_node cv_center_; /** real center of convex (average of columns of G) */
    mutable base_matrix K_, B_, B3_, B32_; /** see documentation (getfem kernel doc) for more details */
    pgeometric_trans pgt_;
    pgeotrans_precomp pgp_;
    pstored_point_tab pspt_; /** if pgp != 0, it is the same as pgp's one */
    size_type ii_;           /** index of current point in the pgp */
    mutable scalar_type J_, J__; /** Jacobian */
    mutable base_matrix PC, B_factors;
    mutable base_vector aux1, aux2;
    mutable std::vector<long> ipvt;
    mutable bool have_J_, have_B_, have_B3_, have_B32_, have_K_, have_cv_center_;
    void compute_J() const;
  public:
    bool have_xref() const { return !xref_.empty(); }
    bool have_xreal() const { return !xreal_.empty(); }
    bool have_G() const { return G_ != 0; }
    bool have_K() const { return have_K_; }
    bool have_B() const { return have_B_; }
    bool have_B3() const { return have_B3_; }
    bool have_B32() const { return have_B32_; }
    bool have_pgt() const { return pgt_ != 0; }
    bool have_pgp() const { return pgp_ != 0; }
    /// coordinates of the current point, in the reference convex.
    const base_node& xref() const;
    /// coordinates of the current point, in the real convex.
    const base_node& xreal() const;
    /// coordinates of the center of the real convex.
    const base_node& cv_center() const;
    /// See getfem kernel doc for these matrices
    const base_matrix& K() const;
    const base_matrix& B() const;
    const base_matrix& B3() const;
    const base_matrix& B32() const;
    bgeot::pgeometric_trans pgt() const { return pgt_; }
    /** matrix whose columns are the vertices of the convex */
    const base_matrix& G() const { return *G_; }
    /** get the Jacobian of the geometric trans (taken at point @c xref() ) */
    scalar_type J() const { if (!have_J_) compute_J(); return J_; }
    size_type N() const {
      if (have_G()) return G().nrows();
      else if (have_xreal()) return xreal_.size();
      else GMM_ASSERT2(false, "cannot get N");
      return 0;
    }
    size_type ii() const { return ii_; }
    bgeot::pgeotrans_precomp pgp() const { return pgp_; }
    /** change the current point (assuming a geotrans_precomp_ is used) */
    void set_ii(size_type ii__) {
      if (ii_ != ii__) {
        if (pgt_ && !pgt()->is_linear())
          { have_K_ = have_B_ = have_B3_ = have_B32_ = have_J_ = false; }
        xref_.resize(0); xreal_.resize(0);
        ii_=ii__;
      }
    }
    /** change the current point (coordinates given in the reference convex) */
    void set_xref(const base_node& P);
    void change(bgeot::pgeotrans_precomp pgp__,
                size_type ii__,
                const base_matrix& G__) {
      G_ = &G__; pgt_ = pgp__->get_trans(); pgp_ = pgp__;
      pspt_ = pgp__->get_ppoint_tab(); ii_ = ii__;
      have_J_ = have_B_ = have_B3_ = have_B32_ = have_K_ = false;
      have_cv_center_ = false;
      xref_.resize(0); xreal_.resize(0); cv_center_.resize(0);
    }
    void change(bgeot::pgeometric_trans pgt__,
                bgeot::pstored_point_tab pspt__,
                size_type ii__,
                const base_matrix& G__) {
      G_ = &G__; pgt_ = pgt__; pgp_ = 0; pspt_ = pspt__; ii_ = ii__;
      have_J_ = have_B_ = have_B3_ = have_B32_ = have_K_ = false;
      have_cv_center_ = false;
      xref_.resize(0); xreal_.resize(0); cv_center_.resize(0);
    }
    void change(bgeot::pgeometric_trans pgt__,
                const base_node& xref__,
                const base_matrix& G__) {
      xref_ = xref__; G_ = &G__; pgt_ = pgt__; pgp_ = 0; pspt_ = 0;
      ii_ = size_type(-1);
      have_J_ = have_B_ = have_B3_ = have_B32_ = have_K_ = false;
      have_cv_center_ = false;
      xreal_.resize(0); cv_center_.resize(0);
    }

    geotrans_interpolation_context()
      : G_(0), pgt_(0), pgp_(0), pspt_(0), ii_(size_type(-1)),
      have_J_(false), have_B_(false), have_B3_(false), have_B32_(false),
      have_K_(false), have_cv_center_(false) {}
    geotrans_interpolation_context(bgeot::pgeotrans_precomp pgp__,
                                   size_type ii__,
                                   const base_matrix& G__)
      : G_(&G__), pgt_(pgp__->get_trans()), pgp_(pgp__),
      pspt_(pgp__->get_ppoint_tab()), ii_(ii__), have_J_(false), have_B_(false),
      have_B3_(false), have_B32_(false), have_K_(false), have_cv_center_(false) {}
    geotrans_interpolation_context(bgeot::pgeometric_trans pgt__,
                                   bgeot::pstored_point_tab pspt__,
                                   size_type ii__,
                                   const base_matrix& G__)
      : G_(&G__), pgt_(pgt__), pgp_(0),
      pspt_(pspt__), ii_(ii__), have_J_(false), have_B_(false), have_B3_(false),
      have_B32_(false), have_K_(false), have_cv_center_(false) {}
    geotrans_interpolation_context(bgeot::pgeometric_trans pgt__,
                                   const base_node& xref__,
                                   const base_matrix& G__)
      : xref_(xref__), G_(&G__), pgt_(pgt__), pgp_(0), pspt_(0),
      ii_(size_type(-1)),have_J_(false), have_B_(false), have_B3_(false),
      have_B32_(false), have_K_(false), have_cv_center_(false) {}
  };

  /* Function allowing the add of an geometric transformation method outwards
     of getfem_integration.cc */

  typedef dal::naming_system<geometric_trans>::param_list gt_param_list;

  void APIDECL add_geometric_trans_name
  (std::string name, dal::naming_system<geometric_trans>::pfunction f);


  /* Optimized operation for small matrices                               */
  scalar_type lu_det(const scalar_type *A, size_type N);
  scalar_type lu_inverse(scalar_type *A, size_type N, bool doassert = true);
  inline scalar_type lu_det(const base_matrix &A)
  { return lu_det(&(*(A.begin())), A.nrows()); }
  inline scalar_type lu_inverse(base_matrix &A, bool doassert = true)
  { return lu_inverse(&(*(A.begin())), A.nrows(), doassert); }
  // Optimized matrix mult for small matrices.
  // Multiply the matrix A of size MxN by B of size NxP in C of size MxP
  void mat_mult(const scalar_type *A, const scalar_type *B, scalar_type *C,
                size_type M, size_type N, size_type P);
  // Optimized matrix mult for small matrices.
  // Multiply the matrix A of size MxN by the transpose of B of size PxN
  // in C of size MxP
  void mat_tmult(const scalar_type *A, const scalar_type *B, scalar_type *C,
                 size_type M, size_type N, size_type P);

}  /* end of namespace bgeot.                                             */


#endif /* BGEOT_GEOMETRIC_TRANSFORMATION_H__                              */
