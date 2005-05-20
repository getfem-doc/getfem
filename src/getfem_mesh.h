// -*- c++ -*- (enables emacs c++ mode)
//========================================================================
//
// Library : GEneric Tool for Finite Element Methods (getfem)
// File    : getfem_mesh.h : meshes for computations.
//           
// Date    : November 05, 1999.
// Author  : Yves Renard <Yves.Renard@insa-toulouse.fr>
//
//========================================================================
//
// Copyright (C) 1999-2005 Yves Renard
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


#ifndef GETFEM_MESH_H__
#define GETFEM_MESH_H__

#include <bitset>
#include <dal_shared_ptr.h>
#include <bgeot_mesh.h>
#include <bgeot_geotrans_inv.h>
#include <linkmsg.h>
#include <getfem_context.h>
namespace getfem {

  /* ********************************************************************* */
  /*								   	   */
  /*	I. Classes of message                                		   */
  /*									   */
  /* ********************************************************************* */

  struct MESH_CLEAR  /* clear message for the structure.                   */
  { operator int(void) const { return 0; } };
  struct MESH_DELETE  /* clear message for the structure.                  */
  { operator int(void) const { return 1; } };
  struct MESH_ADD_CONVEX { 
    size_t icv;
    operator int(void) const { return 5; }
    MESH_ADD_CONVEX(size_t i) { icv = i; }
    MESH_ADD_CONVEX(void) {}
  };
  struct MESH_SUP_CONVEX { 
    size_t icv;
    operator int(void) const { return 6; }
    MESH_SUP_CONVEX(size_t i) { icv = i; }
    MESH_SUP_CONVEX(void) {}
  };
  struct MESH_SWAP_CONVEX { 
    size_t icv1, icv2;
    operator int(void) const { return 7; }
    MESH_SWAP_CONVEX(size_t i, size_t j) { icv1 = i; icv2 = j; }
    MESH_SWAP_CONVEX(void) {}
  };

  class getfem_mesh_receiver : public lmsg::virtual_linkmsg_receiver
  {
    public :

      virtual void receipt(const MESH_CLEAR           &)
      { DAL_THROW(internal_error, "internal error");}
      virtual void receipt(const MESH_DELETE          &)
      { DAL_THROW(internal_error, "internal error");}
      virtual void receipt(const MESH_ADD_CONVEX      &) 
      { DAL_THROW(internal_error, "internal error");}
      virtual void receipt(const MESH_SUP_CONVEX      &) 
      { DAL_THROW(internal_error, "internal error");}
      virtual void receipt(const MESH_SWAP_CONVEX     &)
      { DAL_THROW(internal_error, "internal error");}

      virtual ~getfem_mesh_receiver() {}
  };

  /* refinement methods are :                                              */
  /* mtype = 0 : simplexification.                                         */
  /* mtype = 1 : bank total.                                               */
  /* mtype = 2 : bank green.                                               */


  /* ********************************************************************* */
  /*								   	   */
  /*	II. Class convex or face set                        		   */
  /*									   */
  /* ********************************************************************* */

  ///  Describe a selection of convexes or a selection of faces of convexes
  struct mesh_cvf_set  {
    
    typedef std::bitset<MAX_FACES_PER_CV> face_bitset;
    bool is_bound;
    dal::bit_vector cvindex;
    dal::dynamic_tree_sorted<size_type> cv_in;
    dal::dynamic_array<face_bitset> faces;
    /** Add the convex to the convex set.
     */
    void add_convex(size_type c) { cvindex.add(c); }
    /** Return true if the convex is referenced.
     */
    bool is_convex(size_type c) const { return cvindex[c]; }
    /** Return true if the face f of convex c is part of the boundary
     */
    bool is_elt(size_type c, short_type f) const;
    /** Add a boudary element from the face f of the convex of index
     *          i of the mesh.
     */
    void add_elt(size_type c) { cvindex.add(c); }
    /** Add a convex to a set of convexes.
     */
    void add_elt(size_type c, short_type f);
    /** Delete a boudary element which is the face f of the convex of 
     *          index i of the mesh.
     */
    void sup_elt(size_type c, short_type f);
    /** Delete all boudary element linked with the convex of 
     *          index i of the mesh.
     */
    void sup_convex(size_type c);
    /** Gives in a structure face_bitset all the faces of convex
     *          of index i on the boundary.
     */
    const face_bitset &faces_of_convex(size_type c) const;
    bool is_boundary(void) const { return is_bound; }
    void swap_convex(size_type c1, size_type c2);
    size_type nb_convex(void) { return cvindex.card(); }
    void clear(void) { cv_in.clear(); faces.clear(); cvindex.clear(); }
    mesh_cvf_set(bool is_bound_) { is_bound = is_bound_; }
    mesh_cvf_set() {}
  };

  typedef mesh_cvf_set region;

  /* ********************************************************************* */
  /*								   	   */
  /*	III. Class getfem_mesh                                 		   */
  /*									   */
  /* ********************************************************************* */

  /** Describe a mesh for the computation of pde problems. This
   *      mesh is able to be link with classes which define computation
   *      methods. 
   */
  class getfem_mesh : virtual public dal::static_stored_object,
		      public bgeot::mesh<base_node>,
		      public context_dependencies {
  public :
    
    typedef lmsg::linkmsg_sender<getfem_mesh_receiver> msg_sender;
    
  protected :
    /* if a new field is added here, do NOT forget to add it in the
     * copy_from method! */
    
    double eps_p;  /* infinity distance under wich two points are equal. */
    msg_sender lkmsg; /* gestionnaire de msg.                            */
    dal::dynamic_array<bgeot::pgeometric_trans> gtab;
    dal::bit_vector trans_exists;
    
    dal::dynamic_array<mesh_cvf_set> cvf_sets;
    dal::bit_vector valid_cvf_sets;
    
  public :
    
    /// Constructor.
    getfem_mesh(dim_type NN = dim_type(-1)); 
    double eps(void) const { return eps_p; }
    const msg_sender &lmsg_sender(void) const { return lkmsg; }
    msg_sender &lmsg_sender(void) { return lkmsg; }
    void update_from_context(void) const {}
    
    /** Add the point pt to the mesh and return the index of the
     *          point. If the point is to close to an existing point, the
     *          function do not add the point and return the index of the
     *          already existing point. pt should be of type base\_node.
     */
    size_type add_point(const base_node &pt, bool norepeat = true);
    /// Gives the number of points in the mesh.
    size_type nb_points(void) const { return pts.card(); }
    /// points index
    const dal::bit_vector &points_index(void) const { return pts.index(); }
    /// Delete the point of index i from the mesh.
    void sup_point(size_type i);
    /// Swap the indexes of points of index i and j in the whole structure.
    void swap_points(size_type i, size_type j);
    /** Search if the point pt is in (or approximatively in)
     *          the mesh, and return the index of the point, or
     *          size\_type(-1) if not found.
     */
    size_type search_point(const base_node &pt) const
    { return pts.search(pt); }
    
    bgeot::pgeometric_trans trans_of_convex(size_type ic) const { 
      if (!(trans_exists[ic]))
	DAL_THROW(internal_error, "internal error");
      return gtab[ic]; 
    }
    
    /** Add a convex to the mesh. cvs is of type 
     *          bgeot::convex\_structure and "it" is an iterator on a list
     *          of indexes of points. Return the index
     *          of the convex in the mesh.
     */
    template<class ITER>
    size_type add_convex(bgeot::pgeometric_trans pgt, ITER ipts) { 
      bool present;
      size_type i = bgeot::mesh<base_node>::add_convex(pgt->structure(),
						       ipts, &present);
      gtab[i] = pgt; trans_exists[i] = true;
      if (!present) { lmsg_sender().send(MESH_ADD_CONVEX(i)); touch(); }
      return i;
    }
    
    /** Add a convex to the mesh. cvs is of type 
     *          bgeot::convex\_structure and "it" is an iterator on a list
     *          of points of type base\_node. Return the index
     *          of the convex in the mesh.
     */
    template<class ITER>
    size_type add_convex_by_points(bgeot::pgeometric_trans pgt, ITER ipts);
    
    /** Add a simplex of dimension dim to the mesh. 
     *          "it" is an iterator on a list of indexes of the points.
     *          Return the index of the convex in the mesh.
     */
    template<class ITER>
    size_type add_simplex(dim_type di, ITER ipts)
    { return add_convex(bgeot::simplex_geotrans(di, 1), ipts); }
    /** Add a simplex of dimension dim to the mesh. 
     *          "it" is an iterator on a list of points of type base\_node.
     *          Return the index of the convex in the mesh.
     */
    template<class ITER>
    size_type add_simplex_by_points(dim_type dim, ITER ipts);
    size_type add_segment(size_type a, size_type b);
    size_type add_segment_by_points(const base_node &pt1,
				    const base_node &pt2)
    { return add_segment(add_point(pt1), add_point(pt2)); }
    size_type add_triangle(size_type a,size_type b, size_type c);
    size_type add_triangle_by_points(const base_node &p1,
				     const base_node &p2,
				     const base_node &p3);
    size_type add_tetrahedron(size_type a,
			      size_type b, size_type c, size_type d);
    size_type add_tetrahedron_by_points(const base_node &p1,
					const base_node &p2,
					const base_node &p3,
					const base_node &p4);
    /** Add a parallelepiped of dimension dim to the mesh. 
     *          "it" is an iterator on a list of indexes of the points.
     *          Return the index of the convex in the mesh.
     */
    template<class ITER>
    size_type add_parallelepiped(dim_type di, const ITER &ipts);
    /** Add a parallelepiped of dimension dim to the mesh. 
     *          "it" is an iterator on a list of points of type base\_node.
     *          Return the index of the convex in the mesh.
     */
    template<class ITER>
    size_type add_parallelepiped_by_points(dim_type di, const ITER &ps);
    /** Add a parallelepiped of dimension dim to the
     *          mesh. org is the point of type base\_node representing
     *          the origine and "it" is an iterator on a list of
     *          vectors of type base\_vector.
     *          Return the index of the convex in the mesh.
     */
    template<class ITER>
    size_type add_parallelepiped_by_vectors(dim_type di,
				  const base_node &org, const ITER &vects);
    
    /** Add a prism of dimension dim to the mesh. 
     *          "it" is an iterator on a list of indexes of the points.
     *          Return the index of the convex in the mesh.
     */
    template<class ITER>
    size_type add_prism(dim_type di, const ITER &ipts);
    
    /** Add a prism of dimension dim to the mesh. 
     *          "it" is an iterator on a list of points of type base\_node.
     *          Return the index of the convex in the mesh.
     */
    template<class ITER>
    size_type add_prism_by_points(dim_type di, const ITER &ps);
    
    /// Delete the convex of index i from the mesh.
    void sup_convex(size_type ic);
    /** Swap the indexes of the convex of indexes i and j 
     *          in the whole structure.
     */
    void swap_convex(size_type i, size_type j);
    
    /* returns the normal of face 'f' evaluated at the point 'pt'       */
    /* (pt is a position in the reference convex)                       */
    /* pt should of course be on the face, except if the geometric
       transformation is linear */
    base_small_vector normal_of_face_of_convex(size_type ic, short_type f,
					       const base_node &pt) const;
    /* same as above, but n is the index of a point of the reference convex 
       (on the face..) -- should be faster since it uses a geotrans_precomp */
    base_small_vector normal_of_face_of_convex(size_type ic, short_type f,
					       size_type n=0) const;
    base_matrix local_basis_of_face_of_convex(size_type ic, short_type f,
					      const base_node &pt) const;
    base_matrix local_basis_of_face_of_convex(size_type ic, short_type f,
					      size_type n) const;
    scalar_type convex_quality_estimate(size_type ic) const;
    scalar_type convex_radius_estimate(size_type ic) const;
    scalar_type minimal_convex_radius_estimate() const;
    void translation(base_small_vector);
    void transformation(base_matrix);
    

    size_type add_cvf_set(bool is_bound) {
      size_type d = valid_cvf_sets.first_false(); valid_cvf_sets.add(d);
      cvf_sets[d] = mesh_cvf_set(is_bound);
      return d;
    }
    const region& get_region(size_type id) const { 
      if (set_exists(id)) return cvf_sets[id]; 
      else DAL_THROW(dal::failure_error, "no such region");
    }
    size_type add_convex_set(void) { return add_cvf_set(false); }
    size_type add_face_set(void) { return add_cvf_set(true); }
    bool set_exists(size_type s) const { return valid_cvf_sets[s]; }
    bool set_is_boundary(size_type s) const
    { return cvf_sets[s].is_boundary(); }
    void add_convex_to_set(size_type s, size_type c) {
      if (!(valid_cvf_sets[s]))
	{ cvf_sets[s] = mesh_cvf_set(false); valid_cvf_sets.add(s); }
      cvf_sets[s].add_convex(c); touch();
    }
    void add_face_to_set(size_type s, size_type c, short_type f) {
      if (!(valid_cvf_sets[s]))
	{ cvf_sets[s] = mesh_cvf_set(true); valid_cvf_sets.add(s); }
      cvf_sets[s].add_elt(c, f); touch();
    }
    bool is_convex_in_set(size_type s, size_type c) const
    { return (valid_cvf_sets[s] && cvf_sets[s].is_convex(c)); }
    bool is_face_in_set(size_type s, size_type c, short_type f) const
    { return (valid_cvf_sets[s] && cvf_sets[s].is_elt(c, f)); }
    const dal::bit_vector &convexes_in_set(size_type s) const;
    const mesh_cvf_set::face_bitset
      &faces_of_convex_in_set(size_type s, size_type cv) const;
    const dal::bit_vector &get_valid_sets() const { return valid_cvf_sets; }
    void sup_convex_from_sets(size_type c);
    

    void sup_face_from_set(size_type b, size_type c, short_type f)
    { if (valid_cvf_sets[b]) { cvf_sets[b].sup_elt(c,f); touch(); } }
    void sup_set(size_type b) {
      if (valid_cvf_sets[b])
	{ valid_cvf_sets.sup(b); cvf_sets[b].clear(); touch(); }
    }
    void swap_convex_in_sets(size_type c1, size_type c2);

    void optimize_structure(void);
    void clear(void);
    
    void write_to_file(const std::string &name) const;
    void write_to_file(std::ostream &ost) const;
    void read_from_file(const std::string &name);
    void read_from_file(std::istream &ist);
    void copy_from(const getfem_mesh& m); /* might be the copy constructor */
    size_type memsize() const;
    ~getfem_mesh() { lmsg_sender().send(MESH_DELETE()); }
  private:
    void to_edges() {} /* to be done, the to_edges of mesh_structure does   */
                       /* not handle geotrans */
  };


 template<class ITER>
    size_type getfem_mesh::add_convex_by_points(bgeot::pgeometric_trans pgt,
					                           ITER ipts)
  {
    short_type nb = pgt->nb_points();
    std::vector<size_type> ind(nb);
    for (short_type i = 0; i < nb; ++ipts, ++i) ind[i] = add_point(*ipts);
    return add_convex(pgt, ind.begin());
  }

  template<class ITER>
   size_type getfem_mesh::add_simplex_by_points(dim_type di, ITER ipts)
  { return add_convex_by_points(bgeot::simplex_geotrans(di, 1), ipts); }

  template<class ITER>
    size_type getfem_mesh::add_parallelepiped(dim_type di, const ITER &ipts)
  { return add_convex(bgeot::parallelepiped_geotrans(di, 1), ipts); }

  template<class ITER>
    size_type getfem_mesh::add_parallelepiped_by_points
    (dim_type di, const ITER &ps)
  { return add_convex_by_points(bgeot::parallelepiped_geotrans(di, 1), ps); }

  template<class ITER>
    size_type getfem_mesh::add_parallelepiped_by_vectors
    (dim_type di, const base_node &org, const ITER &vects) {
    size_type nbp = (size_type(1) << size_type(di)), i, j;
    std::vector<size_type> ipt;
    ipt.resize(nbp);
    base_node a; ITER b;

    for (i = 0; i < nbp; ++i) {
      for (a = org, b = vects, j = 0; j < di; ++j, ++b)
	if (i & (1 << j)) a += *b;
      ipt[i] = add_point(a);
    }
    return add_parallelepiped(di, ipt.begin());
  }

  template<class ITER>
    size_type getfem_mesh::add_prism(dim_type di, const ITER &ipts)
  { return add_convex(bgeot::prism_geotrans(di, 1), ipts); }

  template<class ITER>
    size_type getfem_mesh::add_prism_by_points
    (dim_type di, const ITER &ps)
  { return add_convex_by_points(bgeot::prism_geotrans(di, 1), ps); }

  typedef getfem_mesh *pgetfem_mesh;

  /** rough estimate of the maximum value of the condition 
   * number of the jacobian of the geometric transformation */
  scalar_type convex_quality_estimate(bgeot::pgeometric_trans pgt,
				      const base_matrix& pts);

  /** rough estimate of the radius of the convex using the largest eigenvalue
   * of the jacobian of the geometric transformation */
  scalar_type convex_radius_estimate(bgeot::pgeometric_trans pgt,
				     const base_matrix& pts);

  
  class region_ref {
  public:
    size_type id;
    mutable const region* p;
    mutable dal::shared_ptr<region> p_;
  public:
    static region_ref all_convexes() { return region_ref(size_type(-1)); }
    region_ref(dal::bit_vector &bv) { set_bv(bv); }
    region_ref(size_type id_) : id(id_), p(0) {}
    region_ref(const region &s) : id(size_type(-2)), p(&s) {}
    void from_mesh(const getfem_mesh &m) const { 
      if (!p) {
	if (id == size_type(-1))
	  set_bv(m.convex_index());
	else if (id != size_type(-2))
	  p = &m.get_region(id);
      }
      /* TODO : verifier que la liste des convexes est bien inclue dans m.convex_index */
    }
    const region& get() const { return *p; }
  private:
    void set_bv(const dal::bit_vector &bv) const
    { p_.reset(new region(false)); p=p_.get(); p_->cvindex = bv; }
  };


  /* 
     stores a convex face. if f == -1, it is the whole convex
  */
  struct convex_face  {
    size_type cv;
    size_type f;    
    inline bool operator < (const convex_face &e) const
    {
      if (cv < e.cv) return true; if (cv > e.cv) return false; 
      if (f < e.f) return true; else if (f > e.f) return false;
      return false;
    }
    bool is_face() const { return f != size_type(-1); }
    convex_face(size_type cv_, size_type f_=size_type(-1)) : cv(cv_), f(f_) {}
    convex_face() : cv(size_type(-1)), f(size_type(-1)) {}
  };
  typedef std::vector<convex_face> convex_face_ct;

  /** returns a list of "exterior" faces of a mesh
   * (i.e. faces which are not shared by two convexes) 
   * + convexes whose dimension is smaller that m.dim()
   */
  void  outer_faces_of_mesh(const getfem::getfem_mesh &m, 
			const dal::bit_vector& cvlst, convex_face_ct& flist);
  inline void  outer_faces_of_mesh(const getfem::getfem_mesh &m, 
				   convex_face_ct& flist) {
    outer_faces_of_mesh(m,m.convex_index(),flist);
  }

}  /* end of namespace getfem.                                             */


#endif /* GETFEM_MESH_H__  */
