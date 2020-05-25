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

===========================================================================*/


#include <queue>
#include "getfem/dal_singleton.h"
#include "getfem/getfem_mesh_fem.h"
#include "getfem/getfem_torus.h"

namespace getfem {

  void mesh_fem::update_from_context() const {
    for (dal::bv_visitor cv(fe_convex); !cv.finished(); ++cv) {
      if (linked_mesh_->convex_index().is_in(cv)) {
        if (v_num_update < linked_mesh_->convex_version_number(cv)) {
          if (auto_add_elt_pf != 0)
            const_cast<mesh_fem *>(this)
              ->set_finite_element(cv, auto_add_elt_pf);
          else if (auto_add_elt_K != dim_type(-1)) {
            if (auto_add_elt_disc)
              const_cast<mesh_fem *>(this)
                ->set_classical_discontinuous_finite_element
                  (cv, auto_add_elt_K, auto_add_elt_alpha,
                   auto_add_elt_complete);
            else
              const_cast<mesh_fem *>(this)
                ->set_classical_finite_element(cv, auto_add_elt_K,
                                               auto_add_elt_complete);
          }
          else
            const_cast<mesh_fem *>(this)->set_finite_element(cv, 0);
        }
      }
      else const_cast<mesh_fem *>(this)->set_finite_element(cv, 0);
    }
    for (dal::bv_visitor cv(linked_mesh_->convex_index());
         !cv.finished(); ++cv) {
      if (!fe_convex.is_in(cv)
          && v_num_update < linked_mesh_->convex_version_number(cv)) {
        if (auto_add_elt_pf != 0)
          const_cast<mesh_fem *>(this)
            ->set_finite_element(cv, auto_add_elt_pf);
        else if (auto_add_elt_K != dim_type(-1)) {
          if (auto_add_elt_disc)
            const_cast<mesh_fem *>(this)
              ->set_classical_discontinuous_finite_element
                (cv, auto_add_elt_K, auto_add_elt_alpha, auto_add_elt_complete);
          else
            const_cast<mesh_fem *>(this)
              ->set_classical_finite_element(cv, auto_add_elt_K,
                                             auto_add_elt_complete);
        }
      }
    }
    if (!dof_enumeration_made) enumerate_dof();
    v_num = v_num_update = act_counter();
  }

  dal::bit_vector mesh_fem::basic_dof_on_region(const mesh_region &b) const {
    context_check(); if (!dof_enumeration_made) this->enumerate_dof();
    dal::bit_vector res;
    for (getfem::mr_visitor v(b,linked_mesh()); !v.finished(); ++v) {
      size_type cv = v.cv();
      if (convex_index().is_in(cv)) {
        if (v.is_face()) {
          short_type f = short_type(v.f());
          size_type nbb =
            dof_structure.structure_of_convex(cv)->nb_points_of_face(f);
          for (size_type i = 0; i < nbb; ++i) {
            size_type n = Qdim/fem_of_element(cv)->target_dim();
            for (size_type ll = 0; ll < n; ++ll)
              res.add(dof_structure.ind_points_of_face_of_convex(cv,f)[i]+ll);
          }
        } else {
          size_type nbb =
            dof_structure.structure_of_convex(cv)->nb_points();
          for (size_type i = 0; i < nbb; ++i) {
            size_type n = Qdim/fem_of_element(cv)->target_dim();
            for (size_type ll = 0; ll < n; ++ll)
              res.add(dof_structure.ind_points_of_convex(cv)[i]+ll);
          }
        }
      }
    }
    return res;
  }

  template <typename V>
  static void add_e_line__(const V &v, dal::bit_vector &r) {
    typedef typename gmm::linalg_traits<V>::value_type T;
    typename gmm::linalg_traits<V>::const_iterator it = gmm::vect_begin(v);
    typename gmm::linalg_traits<V>::const_iterator ite = gmm::vect_end(v);
    for (; it != ite; ++it) if (*it != T(0)) r.add(it.index());
  }

  dal::bit_vector mesh_fem::dof_on_region(const mesh_region &b) const {
    dal::bit_vector res = basic_dof_on_region(b);
    if (is_reduced()) {
      if (nb_dof() == 0) return dal::bit_vector();
      dal::bit_vector basic = res;
      res.clear();
      for (dal::bv_visitor i(basic); !i.finished(); ++i)
        add_e_line__(gmm::mat_row(E_, i), res);
    }
    return res;
  }


  void mesh_fem::set_finite_element(size_type cv, pfem pf) {
    GMM_ASSERT1(linked_mesh_ != 0, "Uninitialized mesh_fem");
    context_check();
    if (pf == 0) {
      if (fe_convex.is_in(cv)) {
        fe_convex.sup(cv);
        dof_enumeration_made = false;
        touch(); v_num = act_counter();
      }
    }
    else {
      GMM_ASSERT1(basic_structure(linked_mesh_->structure_of_convex(cv))
                  == pf->basic_structure(cv),
                  "Incompatibility between fem " << name_of_fem(pf) <<
                  " and mesh element " <<
                  name_of_geometric_trans(linked_mesh_->trans_of_convex(cv)));
      GMM_ASSERT1((Qdim % pf->target_dim()) == 0 || pf->target_dim() == 1,
                  "Incompatibility between Qdim=" << int(Qdim) <<
                  " and target_dim " << int(pf->target_dim()) << " of " <<
                  name_of_fem(pf));


      if (cv == f_elems.size()) {
        f_elems.push_back(pf);
        fe_convex.add(cv);
        dof_enumeration_made = false;
        touch(); v_num = act_counter();
      } else {
        if (cv > f_elems.size()) f_elems.resize(cv+1);
        if (!fe_convex.is_in(cv) || f_elems[cv] != pf) {
          fe_convex.add(cv);
          f_elems[cv] = pf;
          dof_enumeration_made = false;
          touch(); v_num = act_counter();
        }
      }
    }
  }

  void mesh_fem::set_finite_element(const dal::bit_vector &cvs, pfem ppf) {
    for (dal::bv_visitor cv(cvs); !cv.finished(); ++cv)
      set_finite_element(cv, ppf);
  }

  void mesh_fem::set_finite_element(pfem ppf)
  { set_finite_element(linked_mesh().convex_index(), ppf); set_auto_add(ppf); }

  void mesh_fem::set_classical_finite_element(size_type cv,
                                              dim_type fem_degree,
                                              bool complete) {
    pfem pf = getfem::classical_fem(linked_mesh().trans_of_convex(cv),
                                    fem_degree, complete);
    set_finite_element(cv, pf);
  }

  void mesh_fem::set_classical_finite_element(const dal::bit_vector &cvs,
                                              dim_type fem_degree,
                                              bool complete) {
    for (dal::bv_visitor cv(cvs); !cv.finished(); ++cv) {
      pfem pf = getfem::classical_fem(linked_mesh().trans_of_convex(cv),
                                      fem_degree, complete);
      set_finite_element(cv, pf);
    }
  }

  void mesh_fem::set_classical_finite_element(dim_type fem_degree,
                                              bool complete) {
    set_classical_finite_element(linked_mesh().convex_index(), fem_degree,
                                 complete);
    set_auto_add(fem_degree, false);
  }

  void mesh_fem::set_classical_discontinuous_finite_element
  (size_type cv, dim_type fem_degree, scalar_type alpha, bool complete) {
    pfem pf = getfem::classical_discontinuous_fem
      (linked_mesh().trans_of_convex(cv), fem_degree, alpha, complete);
    set_finite_element(cv, pf);
  }

  void mesh_fem::set_classical_discontinuous_finite_element
  (const dal::bit_vector &cvs, dim_type fem_degree, scalar_type alpha,
   bool complete) {
    for (dal::bv_visitor cv(cvs); !cv.finished(); ++cv) {
      pfem pf = getfem::classical_discontinuous_fem
        (linked_mesh().trans_of_convex(cv), fem_degree, alpha, complete);
      set_finite_element(cv, pf);
    }
  }

  void mesh_fem::set_classical_discontinuous_finite_element
  (dim_type fem_degree, scalar_type alpha, bool complete) {
    set_classical_discontinuous_finite_element
      (linked_mesh().convex_index(), fem_degree, alpha, complete);
    set_auto_add(fem_degree, true, alpha);
  }

  base_node mesh_fem::point_of_basic_dof(size_type cv, size_type i) const {
    context_check(); if (!dof_enumeration_made) enumerate_dof();
    pfem pf = f_elems[cv];
    return linked_mesh().trans_of_convex(cv)->transform
      (pf->node_of_dof(cv, i * pf->target_dim() / Qdim),
       linked_mesh().points_of_convex(cv));
  }

  base_node mesh_fem::point_of_basic_dof(size_type d) const {
    context_check(); if (!dof_enumeration_made) enumerate_dof();
    for (size_type i = d; i != d - Qdim && i != size_type(-1); --i) {
      size_type cv = dof_structure.first_convex_of_point(i);
      if (cv != size_type(-1)) {
        pfem pf = f_elems[cv];
        return linked_mesh().trans_of_convex(cv)->transform
          (pf->node_of_dof(cv, dof_structure.ind_in_convex_of_point(cv, i)),
           linked_mesh().points_of_convex(cv));
      }
    }
    GMM_ASSERT1(false, "Inexistent dof");
  }

  dim_type mesh_fem::basic_dof_qdim(size_type d) const {
    context_check(); if (!dof_enumeration_made) enumerate_dof();
    for (size_type i = d; i != d - Qdim && i != size_type(-1); --i) {
      size_type cv = dof_structure.first_convex_of_point(i);
      if (cv != size_type(-1)) {
        size_type tdim = f_elems[cv]->target_dim();
        return dim_type((d-i) / tdim);
      }
    }
    GMM_ASSERT1(false, "Inexistent dof");
    return 0;
  }

  size_type mesh_fem::first_convex_of_basic_dof(size_type d) const {
    context_check(); if (!dof_enumeration_made) enumerate_dof();
    for (size_type i = d; i != d - Qdim && i != size_type(-1); --i) {
      size_type cv = dof_structure.first_convex_of_point(i);
      if (cv != size_type(-1)) return cv;
    }
    return size_type(-1);
  }

  const mesh::ind_cv_ct &mesh_fem::convex_to_basic_dof(size_type d) const {
    context_check(); if (!dof_enumeration_made) enumerate_dof();
    for (size_type i = d; i != d - Qdim && i != size_type(-1); --i) {
      size_type cv = dof_structure.first_convex_of_point(i);
      if (cv != size_type(-1)) return dof_structure.convex_to_point(i);
    }
    GMM_ASSERT1(false, "Inexistent dof");
  }

  struct fem_dof {
    size_type ind_node;
    pdof_description pnd;
    size_type part;
  };

  struct dof_comp_ {
    bool operator()(const fem_dof& m, const fem_dof& n) const {
      if (m.ind_node < n.ind_node) return true;
      if (m.ind_node > n.ind_node) return false;
      if (m.part == n.part)
        return dof_description_compare(m.pnd, n.pnd) < 0;
      else if (m.part < n.part) return true;
      else /*if (m.part > n.part)*/ return false;
    }
  };

  void mesh_fem::get_global_dof_index(std::vector<size_type> &ind) const {
    context_check(); if (!dof_enumeration_made) enumerate_dof();
    ind.resize(nb_total_dof);
    gmm::fill(ind, size_type(-1));
    for (dal::bv_visitor cv(convex_index()); !cv.finished(); ++cv) {
      pfem pf = fem_of_element(cv);
      for (size_type j=0; j < pf->nb_dof(cv); j++) {
        size_type gid = pf->index_of_global_dof(cv,j);
        if (gid != size_type(-1)) {
          size_type dof = dof_structure.ind_points_of_convex(cv)[j];
          for (size_type i=0; i < Qdim/pf->target_dim(); ++i)
            ind[dof+i] = gid;
        }
      }
    }
  }

  bool mesh_fem::is_uniform() const {
    context_check(); if (!dof_enumeration_made) enumerate_dof();
    return is_uniform_;
  }

  bool mesh_fem::is_uniformly_vectorized() const {
    context_check(); if (!dof_enumeration_made) enumerate_dof();
    return is_uniformly_vectorized_;
  }

  /// Enumeration of dofs
  void mesh_fem::enumerate_dof() const {
    bgeot::index_node_pair ipt;
    is_uniform_ = true;
    is_uniformly_vectorized_ = (get_qdim() > 1);
    GMM_ASSERT1(linked_mesh_ != 0, "Uninitialized mesh_fem");
    context_check();
    if (fe_convex.card() == 0)
      { dof_enumeration_made = true; nb_total_dof = 0; return; }
    pfem first_pf = f_elems[fe_convex.first_true()];
    if (first_pf && first_pf->is_on_real_element()) is_uniform_ = false;
    if (first_pf && first_pf->target_dim() > 1) is_uniformly_vectorized_=false;

    // Dof counter
    size_type nbdof = 0;

    // Information stored per element
    size_type nb_max_cv = linked_mesh().nb_allocated_convex();
    std::vector<bgeot::kdtree> dof_nodes(nb_max_cv);
    std::vector<scalar_type> elt_car_sizes(nb_max_cv);
    std::vector<std::map<fem_dof, size_type, dof_comp_>> dof_sorts(nb_max_cv);

    // Information for global dof
    dal::bit_vector encountered_global_dof, processed_elt;
    dal::dynamic_array<size_type> ind_global_dof;

    // Auxilliary variables
    std::vector<size_type> itab;
    base_node P(linked_mesh().dim());
    base_node bmin(linked_mesh().dim()), bmax(linked_mesh().dim());
    fem_dof fd;
    bgeot::mesh_structure::ind_set s;

    dof_structure.clear();
    bgeot::pstored_point_tab pspt_old = 0;
    bgeot::pgeometric_trans pgt_old = 0;
    bgeot::pgeotrans_precomp pgp = 0;

    for (dal::bv_visitor cv(linked_mesh().convex_index());
         !cv.finished(); ++cv) {
      if (fe_convex.is_in(cv)) {
        gmm::copy(linked_mesh().points_of_convex(cv)[0], bmin);
        gmm::copy(bmin, bmax);
        for (size_type i = 0; i < linked_mesh().nb_points_of_convex(cv); ++i) {
          const base_node &pt = linked_mesh().points_of_convex(cv)[i];
          for (size_type d = 1; d < bmin.size(); ++d) {
            bmin[d] = std::min(bmin[d], pt[d]);
            bmax[d] = std::max(bmax[d], pt[d]);
          }
        }
        elt_car_sizes[cv] = gmm::vect_dist2_sqr(bmin, bmax);
      }
    }

    dal::bit_vector cv_done;

    for (dal::bv_visitor cv(linked_mesh().convex_index());
         !cv.finished(); ++cv) { // Loop on elements
      if (!fe_convex.is_in(cv)) continue;
      pfem pf = fem_of_element(cv);
      if (pf != first_pf) is_uniform_ = false;
      if (pf->target_dim() > 1) is_uniformly_vectorized_ = false;
      bgeot::pgeometric_trans pgt = linked_mesh().trans_of_convex(cv);
      bgeot::pstored_point_tab pspt = pf->node_tab(cv);
      if (pgt != pgt_old || pspt != pspt_old)
        pgp = bgeot::geotrans_precomp(pgt, pspt, pf);
      pgt_old = pgt; pspt_old = pspt;
      size_type nbd = pf->nb_dof(cv);
      pdof_description andof = global_dof(pf->dim());
      itab.resize(nbd);

      for (size_type i = 0; i < nbd; i++) { // Loop on dofs
        fd.pnd = pf->dof_types()[i];
        fd.part = get_dof_partition(cv);

        if (fd.pnd == andof) {              // If the dof is a global one
          size_type num = pf->index_of_global_dof(cv, i);
          if (!(encountered_global_dof[num])) {
            ind_global_dof[num] = nbdof;
            nbdof += Qdim / pf->target_dim();
            encountered_global_dof[num] = true;
          }
          itab[i] = ind_global_dof[num];
        } else if (!dof_linkable(fd.pnd)) { // If the dof is not linkable
          itab[i] = nbdof;
          nbdof += Qdim / pf->target_dim();
        } else {                            // For a standard linkable dof
          pgp->transform(linked_mesh().points_of_convex(cv), i, P);
          size_type idof = nbdof;

          if (dof_nodes[cv].nb_points() > 0) {
            scalar_type dist = dof_nodes[cv].nearest_neighbor(ipt, P);
            if (gmm::abs(dist) <= 1e-6*elt_car_sizes[cv]) {
              fd.ind_node=ipt.i;
              auto it = dof_sorts[cv].find(fd);
              if (it != dof_sorts[cv].end()) idof = it->second;
            }
          }

          if (idof == nbdof) {
            nbdof += Qdim / pf->target_dim();

            linked_mesh().neighbors_of_convex(cv, pf->faces_of_dof(cv, i), s);
            for (size_type ncv : s) { // For each unscanned neighbor
              if (!cv_done[ncv] && fe_convex.is_in(ncv)) { // add the dof

                fd.ind_node = size_type(-1);
                if (dof_nodes[ncv].nb_points() > 0) {
                  scalar_type dist = dof_nodes[ncv].nearest_neighbor(ipt, P);
                  if (gmm::abs(dist) <= 1e-6*elt_car_sizes[ncv])
                    fd.ind_node=ipt.i;
                }
                if (fd.ind_node == size_type(-1))
                  fd.ind_node = dof_nodes[ncv].add_point(P);
                dof_sorts[ncv][fd] = idof;
              }
            }
          }
          itab[i] = idof;
        }
      }
      cv_done.add(cv);
      dof_sorts[cv].clear(); dof_nodes[cv].clear();
      dof_structure.add_convex_noverif(pf->structure(cv), itab.begin(), cv);
    }

    dof_enumeration_made = true;
    nb_total_dof = nbdof;
  }

  void mesh_fem::reduce_to_basic_dof(const dal::bit_vector &kept_dof) {
    gmm::row_matrix<gmm::rsvector<scalar_type> >
      RR(kept_dof.card(), nb_basic_dof());
    size_type j = 0;
    for (dal::bv_visitor i(kept_dof); !i.finished(); ++i, ++j)
      RR(j, i) = scalar_type(1);
    set_reduction_matrices(RR, gmm::transposed(RR));
  }

  void mesh_fem::reduce_to_basic_dof(const std::set<size_type> &kept_dof) {
    gmm::row_matrix<gmm::rsvector<scalar_type> >
      RR(kept_dof.size(), nb_basic_dof());
    size_type j = 0;
    for (std::set<size_type>::const_iterator it = kept_dof.begin();
         it != kept_dof.end(); ++it, ++j)
      RR(j, *it) = scalar_type(1);
    set_reduction_matrices(RR, gmm::transposed(RR));
  }

  void mesh_fem::clear() {
    fe_convex.clear();
    dof_enumeration_made = false;
    is_uniform_ = true;
    touch(); v_num = act_counter();
    dof_structure.clear();
    use_reduction = false;
    R_ = REDUCTION_MATRIX();
    E_ = EXTENSION_MATRIX();
  }

  void mesh_fem::init_with_mesh(const mesh &me, dim_type Q) {
    GMM_ASSERT1(linked_mesh_ == 0, "Mesh level set already initialized");
    dof_enumeration_made = false;
    is_uniform_ = false;
    auto_add_elt_pf = 0;
    auto_add_elt_K = dim_type(-1);
    Qdim = Q;
    mi.resize(1); mi[0] = Q;
    linked_mesh_ = &me;
    use_reduction = false;
    this->add_dependency(me);
    v_num = v_num_update = act_counter();
  }

  void mesh_fem::copy_from(const mesh_fem &mf) {
    clear_dependencies();
    linked_mesh_ = 0;
    init_with_mesh(*mf.linked_mesh_, mf.get_qdim());

    f_elems = mf.f_elems;
    fe_convex = mf.fe_convex;
    R_ = mf.R_;
    E_ = mf.E_;
    dof_structure = mf.dof_structure;
    dof_enumeration_made = mf.dof_enumeration_made;
    is_uniform_ = mf.is_uniform_;
    nb_total_dof = mf.nb_total_dof;
    auto_add_elt_pf = mf.auto_add_elt_pf;
    auto_add_elt_K = mf.auto_add_elt_K;
    auto_add_elt_disc = mf.auto_add_elt_disc;
    auto_add_elt_complete = mf.auto_add_elt_complete;
    auto_add_elt_alpha = mf.auto_add_elt_alpha;
    mi = mf.mi;
    dof_partition = mf.dof_partition;
    v_num_update = mf.v_num_update;
    v_num = mf.v_num;
    use_reduction = mf.use_reduction;
  }

  mesh_fem::mesh_fem(const mesh_fem &mf) : context_dependencies() {
    linked_mesh_ = 0; copy_from(mf);
  }

  mesh_fem &mesh_fem::operator=(const mesh_fem &mf) {
    copy_from(mf);
    return *this;
  }

  mesh_fem::mesh_fem(const mesh &me, dim_type Q)
    { linked_mesh_ = 0; init_with_mesh(me, Q); }

  mesh_fem::mesh_fem() {
    linked_mesh_ = 0;
    dof_enumeration_made = false;
    is_uniform_ = true;
    set_qdim(1);
  }

  mesh_fem::~mesh_fem() { }

  void mesh_fem::read_from_file(std::istream &ist) {
    GMM_ASSERT1(linked_mesh_ != 0, "Uninitialized mesh_fem");
    gmm::stream_standard_locale sl(ist);
    dal::bit_vector npt;
    dal::dynamic_array<double> tmpv;
    std::string tmp("nop"), tmp2("nop"); tmp.clear(); tmp2.clear();
    bool dof_read = false;
    gmm::col_matrix< gmm::wsvector<scalar_type> > RR, EE;
    ist.precision(16);
    clear();
    ist.seekg(0);ist.clear();
    bgeot::read_until(ist, "BEGIN MESH_FEM");

    while (true) {
      ist >> std::ws; bgeot::get_token(ist, tmp);
      if (bgeot::casecmp(tmp, "END")==0) {
        break;
      } else if (bgeot::casecmp(tmp, "CONVEX")==0) {
        bgeot::get_token(ist, tmp);
        size_type ic = atoi(tmp.c_str());
        GMM_ASSERT1(linked_mesh().convex_index().is_in(ic), "Convex " << ic <<
                    " does not exist, are you sure "
                    "that the mesh attached to this object is right one ?");

        int rgt = bgeot::get_token(ist, tmp);
        if (rgt != 3) { // for backward compatibility
          char c; ist.get(c);
          while (!isspace(c)) { tmp.push_back(c); ist.get(c); }
        }
        getfem::pfem fem = getfem::fem_descriptor(tmp);
        GMM_ASSERT1(fem, "could not create the FEM '" << tmp << "'");
        set_finite_element(ic, fem);
      } else if (bgeot::casecmp(tmp, "BEGIN")==0) {
        bgeot::get_token(ist, tmp);
        if (bgeot::casecmp(tmp, "DOF_PARTITION") == 0) {
          for (dal::bv_visitor cv(convex_index()); !cv.finished(); ++cv) {
            size_type d; ist >> d; set_dof_partition(cv, unsigned(d));
          }
          ist >> bgeot::skip("END DOF_PARTITION");
        } else if (bgeot::casecmp(tmp, "DOF_ENUMERATION") == 0) {
          dal::bit_vector doflst;
          dof_structure.clear(); dof_enumeration_made = false;
          is_uniform_ = true;
          size_type nbdof_unif = size_type(-1);
          touch(); v_num = act_counter();
          while (true) {
            bgeot::get_token(ist, tmp);
            if (bgeot::casecmp(tmp, "END")==0) {
              break;
            }
            bgeot::get_token(ist, tmp2);

            size_type ic = atoi(tmp.c_str());
            std::vector<size_type> tab;
            if (convex_index().is_in(ic) && tmp.size() &&
                isdigit(tmp[0]) && tmp2 == ":") {
              size_type nbd = nb_basic_dof_of_element(ic);
              if (nbdof_unif == size_type(-1))
                nbdof_unif = nbd;
              else if (nbdof_unif != nbd)
                is_uniform_ = false;
              tab.resize(nb_basic_dof_of_element(ic));
              for (size_type i=0; i < fem_of_element(ic)->nb_dof(ic);
                   i++) {
                ist >> tab[i];
                for (size_type q=0; q < size_type(get_qdim())
                       / fem_of_element(ic)->target_dim(); ++q)
                  doflst.add(tab[i]+q);
              }
              dof_structure.add_convex_noverif
                (fem_of_element(ic)->structure(ic), tab.begin(), ic);
            } else GMM_ASSERT1(false, "Missing convex or wrong number "
                               << "in dof enumeration: '"
                               << tmp << "' [pos="
                               << std::streamoff(ist.tellg())<<"]");
            /*bgeot::get_token(ist, tmp);
              cerr << " tok: '" << tmp << "'\n";*/
          }
          dof_read = true;
          this->dof_enumeration_made = true;
          touch(); v_num = act_counter();
          this->nb_total_dof = doflst.card();
          ist >> bgeot::skip("DOF_ENUMERATION");
        } else if  (bgeot::casecmp(tmp, "REDUCTION_MATRIX")==0) {
          bgeot::get_token(ist, tmp);
          GMM_ASSERT1(bgeot::casecmp(tmp, "NROWS")==0,
                      "Missing number of rows");
          size_type nrows; ist >> nrows;
          bgeot::get_token(ist, tmp);
          GMM_ASSERT1(bgeot::casecmp(tmp, "NCOLS")==0,
                      "Missing number of columns");
          size_type ncols; ist >> ncols;
          bgeot::get_token(ist, tmp);
          GMM_ASSERT1(bgeot::casecmp(tmp, "NNZ")==0,
                      "Missing number of nonzero elements");
          size_type nnz; ist >> nnz;
          gmm::clear(RR); gmm::resize(RR, nrows, ncols);
          for (size_type i = 0; i < ncols; ++i) {
            bgeot::get_token(ist, tmp);
            GMM_ASSERT1(bgeot::casecmp(tmp, "COL")==0,
                        "Missing some columns");
            size_type nnz_col; ist >> nnz_col;
            for (size_type j=0; j<nnz_col; ++j) {
              size_type ind; ist >> ind;
              scalar_type val; ist >> val;
              RR(ind, i) = val; // can be optimized using a csc matrix
            }
          }
          R_ = REDUCTION_MATRIX(nrows, ncols);
          gmm::copy(RR, R_);
          use_reduction = true;
          ist >> bgeot::skip("END");
          ist >> bgeot::skip("REDUCTION_MATRIX");
        } else if  (bgeot::casecmp(tmp, "EXTENSION_MATRIX")==0) {
          bgeot::get_token(ist, tmp);
          GMM_ASSERT1(bgeot::casecmp(tmp, "NROWS")==0,
                      "Missing number of rows");
          size_type nrows; ist >> nrows;
          bgeot::get_token(ist, tmp);
          GMM_ASSERT1(bgeot::casecmp(tmp, "NCOLS")==0,
                      "Missing number of columns");
          size_type ncols; ist >> ncols;
          bgeot::get_token(ist, tmp);
          GMM_ASSERT1(bgeot::casecmp(tmp, "NNZ")==0,
                      "Missing number of nonzero elements");
          size_type nnz; ist >> nnz;
          gmm::clear(EE); gmm::resize(EE, nrows, ncols);
          for (size_type i = 0; i < nrows; ++i) {
            bgeot::get_token(ist, tmp);
            GMM_ASSERT1(bgeot::casecmp(tmp, "ROW")==0,
                        "Missing some rows");
            size_type nnz_row; ist >> nnz_row;
            for (size_type j=0; j < nnz_row; ++j) {
              size_type ind; ist >> ind;
              scalar_type val; ist >> val;
              EE(i, ind) = val; // can be optimized using a csc matrix
            }
          }
          E_ = EXTENSION_MATRIX(nrows, ncols);
          gmm::copy(EE, E_);
          use_reduction = true;
          ist >> bgeot::skip("END");
          ist >> bgeot::skip("EXTENSION_MATRIX");
        }
        else if (tmp.size())
          GMM_ASSERT1(false, "Syntax error in file at token '"
                      << tmp << "' [pos=" << std::streamoff(ist.tellg())
                      << "]");
      } else if (bgeot::casecmp(tmp, "QDIM")==0) {
        GMM_ASSERT1(!dof_read, "Can't change QDIM after dof enumeration");
        bgeot::get_token(ist, tmp);
        int q = atoi(tmp.c_str());
        GMM_ASSERT1(q > 0 && q <= 250, "invalid qdim: " << q);
        set_qdim(dim_type(q));
      } else if (tmp.size()) {
        GMM_ASSERT1(false, "Unexpected token '" << tmp <<
                    "' [pos=" << std::streamoff(ist.tellg()) << "]");
      } else if (ist.eof()) {
        GMM_ASSERT1(false, "Unexpected end of stream "
                    << "(missing BEGIN MESH_FEM/END MESH_FEM ?)");
      }
    }
  }

  void mesh_fem::read_from_file(const std::string &name) {
    std::ifstream o(name.c_str());
    GMM_ASSERT1(o, "Mesh_fem file '" << name << "' does not exist");
    read_from_file(o);
  }

  template<typename VECT> static void
  write_col(std::ostream &ost, const VECT &v) {
    typename gmm::linalg_traits<VECT>::const_iterator it = v.begin();
    ost << gmm::nnz(v);
    for (; it != v.end(); ++it)
      ost << " " << it.index() << " " << *it;
    ost << "\n";
  }

  void mesh_fem::write_reduction_matrices_to_file(std::ostream &ost) const {
    if (use_reduction) {
      ost.precision(16);
      ost << " BEGIN REDUCTION_MATRIX " << '\n';
      ost << "  NROWS " <<  gmm::mat_nrows(R_) << '\n';
      ost << "  NCOLS " <<  gmm::mat_ncols(R_) << '\n';
      ost << "  NNZ " << gmm::nnz(R_) << '\n';
      for (size_type i = 0; i < gmm::mat_ncols(R_); ++i) {
        ost << "  COL ";
        write_col(ost, gmm::mat_col(R_, i));
      }
      ost << " END REDUCTION_MATRIX " << '\n';
      ost << " BEGIN EXTENSION_MATRIX " << '\n';
      ost << "  NROWS " <<  gmm::mat_nrows(E_) << '\n';
      ost << "  NCOLS " <<  gmm::mat_ncols(E_) << '\n';
      ost << "  NNZ " << gmm::nnz(E_) << '\n';
      for (size_type i = 0; i < gmm::mat_nrows(E_); ++i) {
        ost << "  ROW ";
        write_col(ost, gmm::mat_row(E_, i));
      }
      ost << " END EXTENSION_MATRIX " << '\n';
    }
  }

  void mesh_fem::write_basic_to_file(std::ostream &ost) const {
    ost << "QDIM " << size_type(get_qdim()) << '\n';
    for (dal::bv_visitor cv(convex_index()); !cv.finished(); ++cv) {
      ost << " CONVEX " << cv;
      ost << " \'" << name_of_fem(fem_of_element(cv));
      ost << "\'\n";
    }

    if (!dof_partition.empty()) {
      ost << " BEGIN DOF_PARTITION\n";
      unsigned i = 0;
      for (dal::bv_visitor cv(convex_index()); !cv.finished(); ++cv) {
        ost << " " << get_dof_partition(cv); if ((++i % 20) == 0) ost << "\n";
      }
      ost << "\n";
      ost << " END DOF_PARTITION\n";
    }

    ost << " BEGIN DOF_ENUMERATION " << '\n';
    for (dal::bv_visitor cv(convex_index()); !cv.finished(); ++cv) {
      ost << "  " << cv << ": ";
      ind_dof_ct::const_iterator it = ind_basic_dof_of_element(cv).begin();
      while (it != ind_basic_dof_of_element(cv).end()) {
        ost << " " << *it;
        // skip repeated dofs for "pseudo" vector elements
        for (size_type i=0;
             i < size_type(get_qdim())/fem_of_element(cv)->target_dim();
             ++i) ++it;
      }
      ost << '\n';
    }
    ost << " END DOF_ENUMERATION " << '\n';
  }

  void mesh_fem::write_to_file(std::ostream &ost) const {
    context_check();
    gmm::stream_standard_locale sl(ost);
    ost << '\n' << "BEGIN MESH_FEM" << '\n' << '\n';
    write_basic_to_file(ost);
    write_reduction_matrices_to_file(ost);
    ost << "END MESH_FEM" << '\n';
  }

  void mesh_fem::write_to_file(const std::string &name, bool with_mesh) const {
    std::ofstream o(name.c_str());
    GMM_ASSERT1(o, "impossible to open file '" << name << "'");
    o << "% GETFEM MESH_FEM FILE " << '\n';
    o << "% GETFEM VERSION " << GETFEM_VERSION << '\n' << '\n' << '\n';
    if (with_mesh) linked_mesh().write_to_file(o);
    write_to_file(o);
  }

  struct mf__key_ : public context_dependencies {
    const mesh *pmsh;
    dim_type order, qdim;
    bool complete;
    mf__key_(const mesh &msh, dim_type o, dim_type q, bool complete_)
      : pmsh(&msh), order(o), qdim(q), complete(complete_)
    { add_dependency(msh); }
    bool operator <(const mf__key_ &a) const {
      if (pmsh < a.pmsh) return true;
      else if (pmsh > a.pmsh) return false;
      else if (order < a.order) return true;
      else if (order > a.order) return false;
      else if (qdim < a.qdim) return true;
      else if (qdim > a.qdim) return false;
      else if (complete < a.complete) return true;
      return false;
    }
    void update_from_context() const {}
    mf__key_(const mf__key_ &mfk) : context_dependencies( ) {
      pmsh = mfk.pmsh;
      order = mfk.order;
      qdim = mfk.qdim;
      complete = mfk.complete;
      add_dependency(*pmsh);
    }
  private :
    mf__key_& operator=(const mf__key_ &mfk);
  };


  class classical_mesh_fem_pool {

    typedef std::shared_ptr<const mesh_fem> pmesh_fem;
    typedef std::map<mf__key_, pmesh_fem> mesh_fem_tab;

    mesh_fem_tab mfs;

  public :

    const mesh_fem &operator()(const mesh &msh, dim_type o, dim_type qdim,
                               bool complete=false) {
      mesh_fem_tab::iterator itt = mfs.begin(), itn = mfs.begin();
      if (itn != mfs.end()) itn++;
      while (itt != mfs.end()) {
        if (!(itt->first.is_context_valid()))
          { mfs.erase(itt); }
        itt=itn;
        if (itn != mfs.end()) itn++;
      }

      mf__key_ key(msh, o, qdim, complete);
      mesh_fem_tab::iterator it = mfs.find(key);
      assert(it == mfs.end() || it->second->is_context_valid());

      if (it == mfs.end()) {
        auto p_torus_mesh = dynamic_cast<const getfem::torus_mesh *>(&msh);
        auto pmf = (p_torus_mesh) ? std::make_shared<torus_mesh_fem>(*p_torus_mesh, qdim)
                                  : std::make_shared<mesh_fem>(msh, qdim);
        pmf->set_classical_finite_element(o);
        pmf->set_auto_add(o, false);
        return *(mfs[key] = pmf);
      }
      else return *(it->second);
    }

  };

  const mesh_fem &classical_mesh_fem(const mesh &msh,
                                     dim_type order, dim_type qdim,
                                     bool complete) {
    classical_mesh_fem_pool &pool
      = dal::singleton<classical_mesh_fem_pool>::instance();
    return pool(msh, order, qdim, complete);
  }

  struct dummy_mesh_fem_ {
    mesh_fem mf;
    dummy_mesh_fem_() : mf(dummy_mesh()) {}
  };

  const mesh_fem &dummy_mesh_fem()
  { return dal::singleton<dummy_mesh_fem_>::instance().mf; }


  void vectorize_base_tensor(const base_tensor &t, base_matrix &vt,
                             size_type ndof, size_type qdim, size_type N) {
    GMM_ASSERT1(qdim == N || qdim == 1, "mixed intrinsic vector and "
                "tensorised fem is not supported");
    gmm::resize(vt, ndof, N);
    ndof = (ndof*qdim)/N;

    if (qdim == N) {
      gmm::copy(t.as_vector(), vt.as_vector());
    } else if (qdim == 1) {
      gmm::clear(vt);
      base_tensor::const_iterator it = t.begin();
      for (size_type i = 0; i < ndof; ++i, ++it)
        for (size_type j = 0; j < N; ++j) vt(i*N+j, j) = *it;
    }
  }

  void vectorize_grad_base_tensor(const base_tensor &t, base_tensor &vt,
                                  size_type ndof, size_type qdim, size_type Q) {
    size_type N = t.sizes()[2];
    GMM_ASSERT1(qdim == Q || qdim == 1, "mixed intrinsic vector and "
                  "tensorised fem is not supported");
    vt.adjust_sizes(bgeot::multi_index(ndof, Q, N));
    ndof = (ndof*qdim)/Q;

    if (qdim == Q) {
      gmm::copy(t.as_vector(), vt.as_vector());
    } else if (qdim == 1) {
      gmm::clear(vt.as_vector());
      base_tensor::const_iterator it = t.begin();
      for (size_type k = 0; k < N; ++k)
        for (size_type i = 0; i < ndof; ++i, ++it)
          for (size_type j = 0; j < Q; ++j) vt(i*Q+j, j, k) = *it;
    }
  }



}  /* end of namespace getfem.                                             */


