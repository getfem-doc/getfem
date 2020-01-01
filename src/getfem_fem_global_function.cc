/*===========================================================================

 Copyright (C) 2004-2020 Yves Renard
 Copyright (C) 2016      Konstantinos Poulios

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

===========================================================================*/

#include <getfem/getfem_fem_global_function.h>

namespace getfem {


  void fem_global_function::init() {
    is_pol = is_lag = is_standard_fem = false; es_degree = 5;
    is_equiv = real_element_defined = true;
    ntarget_dim = 1; // An extension for vectorial elements should be easy

    std::stringstream nm;
    nm << "GLOBAL_FEM(" << (void*)this << ")";
    debug_name_ = nm.str();

    GMM_ASSERT1(functions.size() > 0, "Empty list of base functions.");
    dim_ = functions[0]->dim();
    for (size_type i=1; i < functions.size(); ++i)
      GMM_ASSERT1(dim_ == functions[i]->dim(),
                  "Incompatible function space dimensions.");

    update_from_context();
  }


  fem_global_function::fem_global_function
  (const std::vector<pglobal_function> &funcs, const mesh &m_)
    : functions(funcs), m(m_), mim(dummy_mesh_im()), has_mesh_im(false) {

    DAL_STORED_OBJECT_DEBUG_CREATED(this, "Global function fem");
    GMM_ASSERT1(&m != &dummy_mesh(), "A non-empty mesh object"
                                     " is expected.");
    this->add_dependency(m_);
    init();
  }

  fem_global_function::fem_global_function
  (const std::vector<pglobal_function> &funcs, const mesh_im &mim_)
    : functions(funcs), m(mim_.linked_mesh()), mim(mim_), has_mesh_im(true) {

    DAL_STORED_OBJECT_DEBUG_CREATED(this, "Global function fem");
    GMM_ASSERT1(&mim != &dummy_mesh_im(), "A non-empty mesh_im object"
                                          " is expected.");
    this->add_dependency(mim_);
    init();
  }

  void fem_global_function::update_from_context() const {

    if (precomps) {
      for (const auto &cv_precomps : *precomps)
        for (const auto &keyval : cv_precomps)
          dal::del_dependency(precomps, keyval.first);
      precomps->clear();
    } else {
      precomps = std::make_shared<precomp_pool>();
      dal::pstatic_stored_object_key pkey
        = std::make_shared<precomp_pool_key>(precomps);
      dal::add_stored_object(pkey, precomps);
    }

    size_type nb_total_dof(functions.size());
    base_node bmin(dim()), bmax(dim());
    bgeot::rtree boxtree{1E-13};
    std::map<size_type, std::vector<size_type>> box_to_convexes_map;
    for (size_type i=0; i < nb_total_dof; ++i) {
      functions[i]->bounding_box(bmin, bmax);
      box_to_convexes_map[boxtree.add_box(bmin, bmax, i)].push_back(i);
    }
    boxtree.build_tree();

    size_type max_dof(0);
    index_of_global_dof_.clear();
    index_of_global_dof_.resize(m.nb_allocated_convex());
    for (dal::bv_visitor cv(m.convex_index()); !cv.finished(); ++cv) {
      GMM_ASSERT1(dim_ == m.structure_of_convex(cv)->dim(),
                  "Convexes of different dimension: to be done");
      bgeot::pgeometric_trans pgt = m.trans_of_convex(cv);

      bounding_box(bmin, bmax, m.points_of_convex(cv), pgt);

      bgeot::rtree::pbox_set boxlst;
      boxtree.find_intersecting_boxes(bmin, bmax, boxlst);
      index_of_global_dof_[cv].clear();

      if (has_mesh_im) {
        pintegration_method pim = mim.int_method_of_element(cv);
        GMM_ASSERT1(pim->type() == IM_APPROX, "You have to use approximated "
                    "integration in connection to a fem with global functions");
        papprox_integration pai = pim->approx_method();

        for (const auto &box : boxlst) {
          for (auto candidate : box_to_convexes_map.at(box->id)) {
            for (size_type k = 0; k < pai->nb_points(); ++k) {
              base_node gpt = pgt->transform(pai->point(k),
                                             m.points_of_convex(cv));
              if (functions[candidate]->is_in_support(gpt)) {
                index_of_global_dof_[cv].push_back(candidate);
                break;
              }
            }
          }
        }
      } else { // !has_mesh_im
        for (const auto &box : boxlst) {
          for (auto candidate : box_to_convexes_map.at(box->id))
            index_of_global_dof_[cv].push_back(candidate);
        }
      }
      max_dof = std::max(max_dof, index_of_global_dof_[cv].size());
    }

    /** setup global dofs, with dummy coordinates */
    base_node P(dim()); gmm::fill(P,1./20);
    std::vector<base_node> node_tab_(max_dof, P);
    pspt_override = bgeot::store_point_tab(node_tab_);
    pspt_valid = false;
    dof_types_.resize(nb_total_dof);
    std::fill(dof_types_.begin(), dof_types_.end(),
              global_dof(dim()));
  }

  size_type fem_global_function::nb_dof(size_type cv) const {
    //return functions.size();
    context_check();
    return (cv < index_of_global_dof_.size()) ? index_of_global_dof_[cv].size()
                                              : size_type(0);
  }

  size_type fem_global_function::index_of_global_dof
  (size_type cv, size_type i) const {
    //return i;
    context_check();
    return (cv < index_of_global_dof_.size() &&
            i < index_of_global_dof_[cv].size()) ? index_of_global_dof_[cv][i]
                                                 : size_type(-1);
  }

  bgeot::pconvex_ref fem_global_function::ref_convex(size_type cv) const {
    if (has_mesh_im)
      return mim.int_method_of_element(cv)->approx_method()->ref_convex();
    else
      return bgeot::basic_convex_ref(m.trans_of_convex(cv)->convex_ref());
  }

  const bgeot::convex<base_node> &
  fem_global_function::node_convex(size_type cv) const { 
    if (m.convex_index().is_in(cv))
      return *(bgeot::generic_dummy_convex_ref
               (dim(), nb_dof(cv), m.structure_of_convex(cv)->nb_faces()));
    else GMM_ASSERT1(false, "Wrong convex number: " << cv);
  }

  void fem_global_function::base_value(const base_node &, base_tensor &) const
  { GMM_ASSERT1(false, "No base values, real only element."); }
  void fem_global_function::grad_base_value(const base_node &,
                                            base_tensor &) const
  { GMM_ASSERT1(false, "No grad values, real only element."); }
  void fem_global_function::hess_base_value(const base_node &,
                                            base_tensor &) const
  { GMM_ASSERT1(false, "No hess values, real only element."); }

  void fem_global_function::real_base_value(const fem_interpolation_context& c,
                                            base_tensor &t, bool) const {
    assert(target_dim() == 1);
    size_type cv = c.convex_num();
    size_type nbdof = nb_dof(cv);
    t.adjust_sizes(nbdof, target_dim());
    if (c.have_pfp() && c.ii() != size_type(-1)) {
      GMM_ASSERT1(precomps, "Internal error");
      if (precomps->size() == 0)
        precomps->resize(m.nb_allocated_convex());
      GMM_ASSERT1(precomps->size() == m.nb_allocated_convex(), "Internal error");
      const bgeot::pstored_point_tab ptab = c.pfp()->get_ppoint_tab();
      auto it = (*precomps)[cv].find(ptab);
      if (it == (*precomps)[cv].end()) {
        it = (*precomps)[cv].emplace(ptab, precomp_data()).first;
        dal::add_dependency(precomps, ptab);
        // we could have added the dependency to this->shared_from_this()
        // instead, but there is a risk that this will shadow the same
        // dependency through a different path, so that it becomes dangerous
        // to delete the dependency later
      }
      if (it->second.val.size() == 0) {
        it->second.val.resize(ptab->size());
        base_matrix G;
        bgeot::vectors_to_base_matrix(G, m.points_of_convex(cv));
        for (size_type k = 0; k < ptab->size(); ++k) {
          const fem_interpolation_context
            ctx(m.trans_of_convex(cv), shared_from_this(), (*ptab)[k], G, cv);
          real_base_value(ctx, it->second.val[k]);
        }
      }
      gmm::copy(it->second.val[c.ii()].as_vector(), t.as_vector());
    } else
      for (size_type i=0; i < nbdof; ++i) {
        /*cerr << "fem_global_function: real_base_value(" << c.xreal() << ")\n";
        if (c.have_G()) cerr << "G = " << c.G() << "\n";
        else cerr << "no G\n";*/
        t[i] = functions[index_of_global_dof_[cv][i]]->val(c);
      }
  }

  void fem_global_function::real_grad_base_value
  (const fem_interpolation_context& c, base_tensor &t, bool) const {
    assert(target_dim() == 1);
    size_type cv = c.convex_num();
    size_type nbdof = nb_dof(cv);
    t.adjust_sizes(nbdof, target_dim(), dim());
    if (c.have_pfp() && c.ii() != size_type(-1)) {
      GMM_ASSERT1(precomps, "Internal error");
      if (precomps->size() == 0)
        precomps->resize(m.nb_allocated_convex());
      GMM_ASSERT1(precomps->size() == m.nb_allocated_convex(), "Internal error");
      const bgeot::pstored_point_tab ptab = c.pfp()->get_ppoint_tab();
      auto it = (*precomps)[cv].find(ptab);
      if (it == (*precomps)[cv].end()) {
        it = (*precomps)[cv].emplace(ptab, precomp_data()).first;
        dal::add_dependency(precomps, ptab);
      }
      if (it->second.grad.size() == 0) {
        it->second.grad.resize(ptab->size());
        base_matrix G;
        bgeot::vectors_to_base_matrix(G, m.points_of_convex(cv));
        for (size_type k = 0; k < ptab->size(); ++k) {
          const fem_interpolation_context
            ctx(m.trans_of_convex(cv), shared_from_this(), (*ptab)[k], G, cv);
          real_grad_base_value(ctx, it->second.grad[k]);
        }
      }
      gmm::copy(it->second.grad[c.ii()].as_vector(), t.as_vector());
    } else {
      base_small_vector G(dim());
      for (size_type i=0; i < nbdof; ++i) {
        functions[index_of_global_dof_[cv][i]]->grad(c,G);
        for (size_type j=0; j < dim(); ++j)
          t[j*nbdof + i] = G[j];
      }
    }
  }

  void fem_global_function::real_hess_base_value
  (const fem_interpolation_context &c, base_tensor &t, bool) const {
    assert(target_dim() == 1);
    size_type cv = c.convex_num();
    size_type nbdof = nb_dof(cv);
    t.adjust_sizes(nbdof, target_dim(), gmm::sqr(dim()));
    if (c.have_pfp() && c.ii() != size_type(-1)) {
      GMM_ASSERT1(precomps, "Internal error");
      if (precomps->size() == 0)
        precomps->resize(m.nb_allocated_convex());
      GMM_ASSERT1(precomps->size() == m.nb_allocated_convex(), "Internal error");
      const bgeot::pstored_point_tab ptab = c.pfp()->get_ppoint_tab();
      auto it = (*precomps)[cv].find(ptab);
      if (it == (*precomps)[cv].end()) {
        it = (*precomps)[cv].emplace(ptab, precomp_data()).first;
        dal::add_dependency(precomps, ptab);
      }
      if (it->second.hess.size() == 0) {
        it->second.hess.resize(ptab->size());
        base_matrix G;
        bgeot::vectors_to_base_matrix(G, m.points_of_convex(cv));
        for (size_type k = 0; k < ptab->size(); ++k) {
          const fem_interpolation_context
            ctx(m.trans_of_convex(cv), shared_from_this(), (*ptab)[k], G, cv);
          real_hess_base_value(ctx, it->second.hess[k]);
        }
      }
      gmm::copy(it->second.hess[c.ii()].as_vector(), t.as_vector());
    } else {
      base_matrix H(dim(),dim());
      for (size_type i=0; i < nbdof; ++i) {
        functions[index_of_global_dof_[cv][i]]->hess(c,H);
        for (size_type jk=0; jk < size_type(dim()*dim()); ++jk)
          t[jk*nbdof + i] = H[jk];
      }
    }
  }


  DAL_SIMPLE_KEY(special_fem_globf_key, pfem);

  pfem new_fem_global_function(const std::vector<pglobal_function> &funcs,
                               const mesh &m) {
    pfem pf = std::make_shared<fem_global_function>(funcs, m);
    dal::pstatic_stored_object_key
      pk = std::make_shared<special_fem_globf_key>(pf);
    dal::add_stored_object(pk, pf);
    return pf;
  }

  pfem new_fem_global_function(const std::vector<pglobal_function> &funcs,
                               const mesh_im &mim) {
    pfem pf = std::make_shared<fem_global_function>(funcs, mim);
    dal::pstatic_stored_object_key
      pk = std::make_shared<special_fem_globf_key>(pf);
    dal::add_stored_object(pk, pf);
    return pf;
  }

}

/* end of namespace getfem  */
