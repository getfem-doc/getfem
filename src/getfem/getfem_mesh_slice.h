/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================

 Copyright (C) 2003-2020 Julien Pommier

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

/**@file getfem_mesh_slice.h
   @author  Julien Pommier <Julien.Pommier@insa-toulouse.fr>
   @date May 2003.
   @brief Define the class getfem::stored_mesh_slice.
 */
#ifndef GETFEM_MESH_SLICE_H
#define GETFEM_MESH_SLICE_H

#include "getfem_mesh_slicers.h"

namespace getfem {
  class slicer_build_stored_mesh_slice;

  /** The output of a getfem::mesh_slicer which has been recorded. 
   @see getfem::slicer_build_stored_mesh_slice */
  class stored_mesh_slice : virtual public dal::static_stored_object {
  protected:
    /* nodes lists and simplexes lists for each convex of the original mesh */
    struct convex_slice {
      size_type cv_num; 
      dim_type cv_dim;
      dim_type fcnt, cv_nbfaces; // number of faces of the convex
                                 // (fcnt also counts the faces created by
                                 // the slicing of the convex)
      bool discont; // when true, it is assumed that the interpolated data
                    // inside the convex may be discontinuous
                    // (for ex. levelset discont function)
      mesh_slicer::cs_nodes_ct nodes;
      mesh_slicer::cs_simplexes_ct simplexes;
      size_type global_points_count;
    };
    typedef std::deque<convex_slice> cvlst_ct;

    struct merged_node_t {
      const slice_node *P;
      unsigned pos; /* [0..nb_points()-1] */
    };
    /* these three arrays allow the association of a global point
       number to a merged point number, and of a merged point to a
       list of global points */
    mutable std::vector<merged_node_t> merged_nodes; /* size = points_cnt */
    mutable std::vector<size_type> merged_nodes_idx; /* size = merged pts + 1*/
    mutable std::vector<size_type> to_merged_index;  /* size = points_cnt */
    mutable bool merged_nodes_available;

    /* keep track of the original mesh (hence it should not be destroyed
     * before the slice) */
    const mesh *poriginal_mesh;
    std::vector<size_type> simplex_cnt; // count simplexes of dim 0,1,...,dim
    size_type points_cnt;
    cvlst_ct cvlst;
    size_type dim_;
    std::vector<size_type> cv2pos; // convex id -> pos in cvlst
    friend class slicer_build_stored_mesh_slice;
    friend class mesh_slicer;
  public:
    stored_mesh_slice() : poriginal_mesh(0), points_cnt(0), dim_(size_type(-1))
    { }
    /** Shortcut constructor to simplexify a mesh with refinement. */
    explicit stored_mesh_slice(const getfem::mesh& m, size_type nrefine = 1) 
      : poriginal_mesh(0), points_cnt(0), dim_(size_type(-1)) { 
      this->build(m, slicer_none(), nrefine);
    };
    virtual ~stored_mesh_slice() {}
    /** return the number of convexes of the original mesh referenced
        in the slice */
    size_type nb_convex() const { return cvlst.size(); }
    /** return the original convex number of the 'ic'th convex
        referenced in the slice */
    size_type convex_num(size_type ic) const { return cvlst[ic].cv_num; }
    /** return the position ic of the referenced convex in the slice convexes
        list cvlist, corresponding to the original convex number cv*/
    size_type convex_pos(size_type cv) const { return cv2pos[cv]; }
    /** change the slice dimension (append zeros or truncate node coordinates..) */
    void set_dim(size_type newdim);
    /** return the slice dimension */
    size_type dim() const { return dim_; }
    /** return a pointer to the original mesh */
    const mesh& linked_mesh() const { return *poriginal_mesh; }
    /** return the simplex count, in an array.
        @param c contains the number of simplexes of dimension 0, 1, ... dim().
    */
    void nb_simplexes(std::vector<size_type>& c) const { c = simplex_cnt; }
    /** Return the number of simplexes of dimension sdim */
    size_type nb_simplexes(size_type sdim) const { return simplex_cnt[sdim]; }
    /** Return the number of nodes in the slice */
    size_type nb_points() const { return points_cnt; }
    /** Return the list of nodes for the 'ic'th convex of the slice. */
    const mesh_slicer::cs_nodes_ct& nodes(size_type ic) const { return cvlst[ic].nodes; }
    mesh_slicer::cs_nodes_ct& nodes(size_type ic) { return cvlst[ic].nodes; }
    
    /** Return the list of simplexes for the 'ic'th convex of the slice. */
    const mesh_slicer::cs_simplexes_ct& simplexes(size_type ic) const { return cvlst[ic].simplexes; }
    size_type memsize() const;
    void clear() {
      poriginal_mesh = 0;
      cvlst.clear();
      points_cnt = 0; 
      dim_ = size_type(-1);
      gmm::fill(cv2pos, size_type(-1));
      simplex_cnt.clear();
      clear_merged_nodes();
    }
    /** @brief merge with another mesh slice. */
    void merge(const stored_mesh_slice& sl);

    /** @brief build a list of merged nodes.

         build a list of merged nodes, i.e. nodes which have the same
         geometrical location but were extracted from two different
         convexes will be considered as one same node. Use for
         exportation purposes, as VTK and OpenDX do not like
         'discontinuous' meshes
    */
    void merge_nodes() const;

    /** @brief Return the number of merged nodes in slice. */
    size_type nb_merged_nodes() const 
      { return merged_nodes_idx.size() - 1; }

    /** @brief Return the physical position of the merged node.
        @param i_merged should be 0 <= i_merged < nb_merged_nodes()
     */
    const base_node merged_point(size_type i_merged) const 
      { return merged_nodes[merged_nodes_idx[i_merged]].P->pt; }
 
    size_type merged_index(size_type ic, size_type ipt) const
      { return to_merged_index[global_index(ic,ipt)]; }
    size_type global_index(size_type ic, size_type ipt) const 
      { return cvlst[ic].global_points_count+ipt; }

    /** @brief Return the number of nodes that were merged 
       to form the merged one.
      @param i_merged  index of the merged node: 
                             0 <= i_merged < nb_merged_nodes()
     */
    size_type merged_point_cnt(size_type i_merged) const 
      { return merged_nodes_idx[i_merged+1] - merged_nodes_idx[i_merged]; }
    
    std::vector<merged_node_t>::const_iterator 
    merged_point_nodes(size_type i_merged) const { 
      return merged_nodes.begin() + merged_nodes_idx[i_merged]; 
    }

    void clear_merged_nodes() const;

    /** @brief Extract the list of mesh edges.

         extract the list of mesh edges into 'edges' (size = 2* number
         of edges). 'slice_edges' indicates which one were created
         after slicing.  The from_merged_nodes flag may be used if you
         want to use (and merge common edges according to) the merged
         points 
    */
    void get_edges(std::vector<size_type> &edges,
                   dal::bit_vector &slice_edges,
                   bool from_merged_nodes) const;

    void set_convex(size_type cv, bgeot::pconvex_ref cvr, 
                    mesh_slicer::cs_nodes_ct cv_nodes, 
                    mesh_slicer::cs_simplexes_ct cv_simplexes, 
                    dim_type fcnt, const dal::bit_vector& splx_in,
                    bool discont);

    /** Build the slice, by applying a slicer_action operation. */
    void build(const getfem::mesh& m, const slicer_action &a, 
               size_type nrefine = 1) { build(m,&a,0,0,nrefine); }
    /** Build the slice, by applying two slicer_action operations. */
    void build(const getfem::mesh& m, const slicer_action &a,
               const slicer_action &b, 
               size_type nrefine = 1) { build(m,&a,&b,0,nrefine); }
    /** Build the slice, by applying three slicer_action operations. */
    void build(const getfem::mesh& m, const slicer_action &a,
               const slicer_action &b, const slicer_action &c, 
               size_type nrefine = 1) { build(m,&a,&b,&c,nrefine); }
    void build(const getfem::mesh& m, const slicer_action *a,
               const slicer_action *b, const slicer_action *c, 
               size_type nrefine);
      
    /** @brief Apply the listed slicer_action(s) to the slice object.
        the stored_mesh_slice is not modified. This can be used to build a
        new stored_mesh_slice from a stored_mesh_slice.
    */
    void replay(slicer_action &a) const { replay(&a,0,0); }
    void replay(slicer_action &a, slicer_action &b) const
    { replay(&a, &b, 0); }
    void replay(slicer_action &a, slicer_action &b, slicer_action &c) const
    { replay(&a, &b, &c); }
    void replay(slicer_action *a, slicer_action *b, slicer_action *c) const;

    /** @brief Save a slice content to a text file.
     */
    void write_to_file(std::ostream &os) const;
    void write_to_file(const std::string &fname, bool with_mesh=false) const;
    /** @brief Read a slice from a file.
     */
    void read_from_file(std::istream &ist, const getfem::mesh &m);
    void read_from_file(const std::string &fname, const getfem::mesh &m);


    /** @brief Interpolation of a mesh_fem on a slice.

        The mesh_fem and the slice must share the same mesh, of course.

        @param mf the mesh_fem

        @param U a vector whose dimension is a multiple of
        mf.nb_dof(), the field to be interpolated.

        @param V on output, a vector corresponding to the interpolated
        field on the slice (values given on each node of the slice).
    */
    template<typename V1, typename V2> void 
    interpolate(const getfem::mesh_fem &mf, const V1& UU, V2& V) const {
      typedef typename gmm::linalg_traits<V2>::value_type T;
      std::vector<base_node> refpts;
      std::vector<std::vector<T> > coeff;
      base_matrix G;
      size_type qdim = mf.get_qdim();
      size_type qqdim = gmm::vect_size(UU) / mf.nb_dof();
      size_type pos = 0;
      coeff.resize(qqdim);
      std::vector<T> U(mf.nb_basic_dof()*qqdim);
      mf.extend_vector(UU, U);

      gmm::clear(V);
      for (size_type i=0; i < nb_convex(); ++i) {
        size_type cv = convex_num(i);
        refpts.resize(nodes(i).size());
        for (size_type j=0; j < refpts.size(); ++j)
          refpts[j] = nodes(i)[j].pt_ref;
        if (!mf.convex_index().is_in(cv)) {
          pos += refpts.size() * qdim * qqdim;
          continue;
        }
        
        pfem pf = mf.fem_of_element(cv);
        if (pf->need_G()) 
          bgeot::vectors_to_base_matrix(G,
                                        mf.linked_mesh().points_of_convex(cv));
        fem_precomp_pool fppool;
        pfem_precomp pfp = fppool(pf, store_point_tab(refpts));
        
        mesh_fem::ind_dof_ct dof = mf.ind_basic_dof_of_element(cv);
        for (size_type qq=0; qq < qqdim; ++qq) {
          coeff[qq].resize(mf.nb_basic_dof_of_element(cv));
          typename std::vector<T>::iterator cit = coeff[qq].begin();
          for (mesh_fem::ind_dof_ct::const_iterator it=dof.begin();
               it != dof.end(); ++it, ++cit)
            *cit = U[(*it)*qqdim+qq];
        }
        fem_interpolation_context ctx(mf.linked_mesh().trans_of_convex(cv),
                                      pfp, 0, G, cv, short_type(-1));
        for (size_type j=0; j < refpts.size(); ++j) {
          ctx.set_ii(j);
          for (size_type qq = 0; qq < qqdim; ++qq) {
            typename gmm::sub_vector_type<V2*,
              gmm::sub_interval>::vector_type dest = 
              gmm::sub_vector(V,gmm::sub_interval(pos,qdim));
            pf->interpolation(ctx,coeff[qq],dest,dim_type(qdim));
            pos += qdim;
          }
        }
      }
      GMM_ASSERT1(pos == V.size(), "bad dimensions");
    }
  };

  /** @brief a getfem::mesh_slicer whose side effect is to build a
      stored_mesh_slice object.
  */
  class slicer_build_stored_mesh_slice : public slicer_action {
    stored_mesh_slice &sl;
  public:
    slicer_build_stored_mesh_slice(stored_mesh_slice& sl_) : sl(sl_) {
      GMM_ASSERT1(sl.cvlst.size() == 0, 
                  "the stored_mesh_slice already contains data");
    }
    void exec(mesh_slicer& ms);
  };

  std::ostream& operator<<(std::ostream& o, const stored_mesh_slice& m);
}

#endif /*GETFEM_MESH_SLICE_H*/
