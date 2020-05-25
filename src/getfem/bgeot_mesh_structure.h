/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================

 Copyright (C) 1999-2020 Yves Renard

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

/**@file bgeot_mesh_structure.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date November 5, 1999.
   @brief Mesh structure definition
*/

#ifndef BGEOT_MESH_STRUCTURE_H__
#define BGEOT_MESH_STRUCTURE_H__

#include <set>
#include "bgeot_convex_structure.h"
#include "dal_tree_sorted.h"

namespace bgeot {

  struct APIDECL mesh_convex_structure {
    typedef std::vector<size_type> ind_pt_ct;

    pconvex_structure cstruct;       /* type of convex.                  */
    ind_pt_ct pts;                   /* point list indices.               */

    pconvex_structure structure() const { return cstruct; }
    pconvex_structure &structure() { return cstruct; }
    mesh_convex_structure() : cstruct(0) {}
  };

  struct convex_face
  {
    convex_face(size_type element, short_type face) : cv(element), f(face) {}
    size_type cv = -1;
    short_type f = -1;
    bool valid() const {return (cv != size_type(-1)) && (f != short_type(-1));}
    static convex_face invalid_face() {return {size_type(-1), short_type(-1)};}
  };

  /**@addtogroup mesh */
  ///@{
  /** Mesh structure definition.
   *   At this point, the mesh is just a graph: the points have no
   *  associated coordinates
   */
  class APIDECL mesh_structure {

  public :

    typedef std::vector<size_type> ind_cv_ct;
    typedef std::vector<size_type> ind_set;
    typedef gmm::tab_ref_index_ref<ind_cv_ct::const_iterator,
                            convex_ind_ct::const_iterator> ind_pt_face_ct;
    typedef dal::dynamic_array<ind_cv_ct, 8> point_ct;

  protected :

    dal::dynamic_tas<mesh_convex_structure, 8> convex_tab;
    point_ct points_tab;

  public :

    /// Return the list of valid convex IDs
    const dal::bit_vector &convex_index() const
      { return convex_tab.index(); }
    /// Return the list of valid convex IDs of a given dimension
    dal::bit_vector convex_index(dim_type) const;
    /// The total number of convexes in the mesh
    size_type nb_convex() const { return convex_tab.card(); }
    /// The number of convex indexes from 0 to the index of the last convex
    size_type nb_allocated_convex() const
      { return convex_tab.index().last_true()+1; }
    /// Return true if i is in convex_index()
    bool is_convex_valid(size_type i) { return (convex_tab.index())[i]; }
    size_type nb_max_points() const { return points_tab.size(); }
    /// Return true if the point i is used by at least one convex
    bool is_point_valid(size_type i) const { return !(points_tab[i].empty()); }
    /** Return a container to the list of points attached to convex ic.
        They are ordered according to structure_of_convex(ic) */
    const ind_set &ind_points_of_convex(size_type ic) const
    { return convex_tab[ic].pts; }
    /// Return the "local" index for point ip of the mesh
    size_type local_ind_of_convex_point(size_type ic, size_type ip) const;
    /// Return the pconvex_structure of the convex ic.
    pconvex_structure structure_of_convex(size_type ic) const
    { return convex_tab[ic].cstruct; }
    /// Return the number of points of convex ic.
    short_type nb_points_of_convex(size_type ic) const
    { return convex_tab[ic].cstruct->nb_points(); }
    /// Return the number of faces of convex ic.
    short_type nb_faces_of_convex(size_type ic) const
    { return short_type(convex_tab[ic].cstruct->nb_faces()); }
    /// Exchange two point IDs
    void swap_points(size_type i, size_type j);
    /// Exchange two convex IDs
    void swap_convex(size_type cv1, size_type cv2);

    template<class ITER>
    size_type add_convex_noverif(pconvex_structure cs, ITER ipts,
                                 size_type to_index = size_type(-1));
    /** Insert a new convex in the mesh_structure.
        @param cs the structure of the new convex.
        @param ipts an iterator over a sequence of integers (point IDs of the convex nodes).
        @param present an optional argument, contains true on return if the convex already exists in the mesh_structure.
        @return the convex ID
    */
    template<class ITER>
    size_type add_convex(pconvex_structure cs,
                         ITER ipts, bool *present = 0);
    template<class ITER> size_type add_simplex(dim_type dim, ITER ipts)
      { return add_convex(simplex_structure(dim), ipts); }
    size_type add_segment(size_type a, size_type b);
    /** Remove the convex ic */
    void sup_convex(size_type ic);
    /** Remove a convex given its points
        @param nb the number of points for the convex
        @param ipts an iterator over the list of point IDs of the convex
    */
    template<class ITER>
    void sup_convex_with_points(ITER ipts, short_type nb);
    void sup_segment(size_type a, size_type b)
      { size_type t[2]; t[0] = a; t[1] = b; sup_convex_with_points(&t[0], 2); }
    /** Insert a new convex corresponding to face f of the convex ic */
    size_type add_face_of_convex(size_type ic, short_type f);
    /** Insert new convexes corresponding to the faces of the convex ic */
    void add_faces_of_convex(size_type ic);
    /** build a new mesh, such that its convexes are the faces of the
        convexes of the previous one */
    void to_faces(dim_type n);
    /** build a new mesh, such that its convexes are the edges of the
        convexes of the previous one */
    void to_edges();

    size_type nb_convex_with_edge(size_type i1, size_type i2);
    void convex_with_edge(size_type i1, size_type i2,
                          std::vector<size_type> &ipt) const;

    /** Return a container of the convexes attached to point ip */
    const ind_cv_ct &convex_to_point(size_type ip) const
    { return points_tab[ip]; }
    /** Return a container of the points attached (via an edge) to point ip */
    void ind_points_to_point(size_type, ind_set &) const;

    /** Return true if the convex contains the listed points.
        @param ic the convex ID.
        @param nb the number of points which are searched in ic.
        @param pit an iterator to the list of points searched.
    */
    template<class ITER>
      bool is_convex_having_points(size_type ic,short_type nb, ITER pit) const;

    /** Return true if the face of the convex contains the given list of points */
    template<class ITER>
    bool is_convex_face_having_points(size_type ic, short_type face_num,
                                      short_type nb, ITER pit) const;

    /** Return a container of the (global) point number for face f or convex ic */
    ind_pt_face_ct ind_points_of_face_of_convex(size_type ic,
                                                short_type f) const;

    size_type memsize() const;
    /** Reorder the convex IDs and point IDs, such that there is no
        hole in their numbering. */
    void optimize_structure();
    /// erase the mesh
    void clear();
    void stat();

    /** Return in s a list of neighbors of a given convex face.
        @param ic the convex id.
        @param f the face number of the convex.
        @param s the resulting ind_set.
     */
    void neighbors_of_convex(size_type ic, short_type f, ind_set &s) const;
    void neighbours_of_convex(size_type ic, short_type f, ind_set &s) const
      IS_DEPRECATED { neighbors_of_convex(ic, f, s); }

    /** Return in s a list of neighbors of a given convex sharing the
        intersection of a given list of faces
        @param ic the convex id.
        @param f the face number of the convex.
        @param s the resulting ind_set.
     */
    void neighbors_of_convex(size_type ic,
                             const std::vector<short_type> &ftab,
                             ind_set &s) const;
    void neighbours_of_convex(size_type ic,
                              const std::vector<short_type> &ftab,
                              ind_set &s) const IS_DEPRECATED
    { neighbors_of_convex(ic, ftab, s); }

    /** Return a list of neighbors of a given convex.
        @param ic the convex id.
        @param  s the resulting ind_set.
    */
    void neighbors_of_convex(size_type ic, ind_set &s) const;
    void neighbours_of_convex(size_type ic, ind_set &s) const
      IS_DEPRECATED { neighbors_of_convex(ic, s); }

    /** Return a neighbor convex of a given convex face.
        @param ic the convex id.
        @param f the face number of the convex.
        @return size_type(-1) if there is no neighbor to this convex and
        the index of the first neighbor found otherwise.
    */
    size_type neighbor_of_convex(size_type ic, short_type f) const;
    size_type neighbour_of_convex(size_type ic, short_type f) const
      IS_DEPRECATED { return neighbor_of_convex(ic, f); }

    /**Return a face of the neighbouring element that is adjacent to the
       given face
       @param ic the convex id.
       @param f the face number of the convex.
       @return convex_face that contains the neighbors convex id and the
       adjacent face number, or convex_face::invalid_face() otherwise.
    */
    convex_face adjacent_face(size_type ic, short_type f) const;

    bool is_convex_having_neighbor(size_type ic, short_type f) const
    { return (neighbor_of_convex(ic, f) != size_type(-1)); }
    bool is_convex_having_neighbour(size_type ic, short_type f) const
    IS_DEPRECATED { return (neighbor_of_convex(ic, f) != size_type(-1)); }

    /** Convex ID of the first convex attached to the point ip. */
    size_type first_convex_of_point(size_type ip) const
    { return points_tab[ip].empty() ?  size_type(-1) : points_tab[ip][0]; }
    /** Find the local index of the point of global index ip with respect to
     *        the convex cv.
     *  @return local index (a number smaller than
     *        nb_points_of_convex(first_convex_of_point(ip))) or size_type(-1) if
     *  the point is not found.
     */
    size_type ind_in_convex_of_point(size_type ic, size_type ip) const;
  };
  ///@}




  /** Return the cuthill_mc_kee ordering on the convexes */
  void APIDECL cuthill_mckee_on_convexes(const bgeot::mesh_structure &ms,
                                         std::vector<size_type> &cmk);

  template<class ITER>
    bool mesh_structure::is_convex_having_points(size_type ic,
                                              short_type nb, ITER pit) const {
    const ind_set &pt = ind_points_of_convex(ic);
    for (short_type i = 0; i < nb; ++i, ++pit)
      if (std::find(pt.begin(), pt.end(), *pit) == pt.end())
        return false;
    return true;
  }

  template<class ITER> bool
  mesh_structure::is_convex_face_having_points(size_type ic, short_type face_num,
                                               short_type nb, ITER pit) const {
    ind_pt_face_ct pt = ind_points_of_face_of_convex(ic, face_num);
    for (short_type i = 0; i < nb; ++i, ++pit)
      if (std::find(pt.begin(), pt.end(), *pit) == pt.end()) return false;
    return true;
  }

  template<class ITER>
  size_type mesh_structure::add_convex_noverif(pconvex_structure cs,
                                               ITER ipts, size_type is) {
    mesh_convex_structure s; s.cstruct = cs;
    size_type nb = cs->nb_points();

    if (is != size_type(-1)) { sup_convex(is); convex_tab.add_to_index(is,s); }
    else is = convex_tab.add(s);

    convex_tab[is].pts.resize(nb);
    for (size_type i = 0; i < nb; ++i, ++ipts)
      { convex_tab[is].pts[i] = *ipts; points_tab[*ipts].push_back(is); }
    return is;
  }

  template<class ITER>
  size_type mesh_structure::add_convex(pconvex_structure cs,
                                       ITER ipts, bool *present) {
    if (present) *present = false;
    for (size_type i = 0; i < points_tab[*ipts].size(); ++i)
      if (structure_of_convex(points_tab[*ipts][i]) == cs &&
          is_convex_having_points(points_tab[*ipts][i], cs->nb_points(), ipts))
        { if (present) *present = true; return points_tab[*ipts][i]; }
    return add_convex_noverif(cs, ipts);
  }

  template<class ITER>
  void mesh_structure::sup_convex_with_points(ITER ipts, short_type nb) {
    if (nb) {
      for (size_type i = 0; i < points_tab[*ipts].size(); ++i)
        if (is_convex_having_points(points_tab[*ipts][i], nb, ipts))
          sup_convex(points_tab[*ipts][i]);
    }
  }


  /* ********************************************************************* */
  /*                                                                       */
  /*  Gives the list of edges of a mesh.                                   */
  /*                                                                       */
  /* ********************************************************************* */

  /* maybe this should be removed from the matlab interface and obsoleted */
  struct APIDECL edge_list_elt  {
    size_type i, j;
    size_type cv;
    inline bool operator < (const edge_list_elt &e) const {
      if (i < e.i) return true;
      if (i > e.i) return false;
      if (j < e.j) return true; else if (j > e.j) return false;
      if (cv < e.cv) return true;
      return false;
    }
    edge_list_elt(size_type ii, size_type jj, size_type ic = 0) : cv(ic)
    { i = std::min(ii, jj); j = std::max(ii, jj); }
    edge_list_elt() {}
  };

  typedef dal::dynamic_tree_sorted<edge_list_elt> edge_list;

  /* do not use that */
  void APIDECL mesh_edge_list_convex(pconvex_structure cvs,
                             std::vector<size_type> points_of_convex,
                             size_type cv_id, edge_list &el,
                             bool merge_convex);
  void APIDECL mesh_edge_list(const mesh_structure &m, edge_list &el,
                      bool merge_convex = true);



}  /* end of namespace bgeot.                                              */


#endif /* BGEOT_MESH_STRUCTURE_H__                                         */
