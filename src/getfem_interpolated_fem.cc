/*===========================================================================

 Copyright (C) 2004-2020 Yves Renard

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

#include "getfem/getfem_interpolated_fem.h"

namespace getfem {

  void interpolated_fem::build_rtree(void) const {
    base_node min, max;
    boxtree.clear();
    box_to_convexes_map.clear();
    for (dal::bv_visitor cv(mf.convex_index()); !cv.finished(); ++cv) {
      bounding_box(min, max, mf.linked_mesh().points_of_convex(cv),
                   mf.linked_mesh().trans_of_convex(cv));
      box_to_convexes_map[boxtree.add_box(min, max, cv)].push_back(cv);
    }
    boxtree.build_tree();
  }

  bool interpolated_fem::find_a_point(base_node pt, base_node &ptr,
                                      size_type &cv) const {
    bool gt_invertible;
    if (pif) { base_node ptreal = pt; pif->val(ptreal, pt); }
    if (cv_stored != size_type(-1) && gic.invert(pt, ptr, gt_invertible))
      { cv = cv_stored; if (gt_invertible) return true; }
    boxtree.find_boxes_at_point(pt, boxlst);
    bgeot::rtree::pbox_set::const_iterator it = boxlst.begin(),
      ite = boxlst.end();
    for (; it != ite; ++it) {
      for (auto candidate : box_to_convexes_map.at((*it)->id)) {
        gic = bgeot::geotrans_inv_convex
          (mf.linked_mesh().convex(candidate),
           mf.linked_mesh().trans_of_convex(candidate));
        cv_stored = candidate;
        if (gic.invert(pt, ptr, gt_invertible)) {
          cv = candidate; return true;
        }
      }
    }
    return false;
  }

  void interpolated_fem::update_from_context(void) const {
    fictx_cv = cv_stored = size_type(-1);
    dim_ = dim_type(-1);
    build_rtree();

    GMM_ASSERT1(!mf.is_reduced(),
                "Interpolated fem works only on non reduced mesh_fems");

    std::vector<elt_interpolation_data> vv(mim.convex_index().last_true() + 1);
    elements.swap(vv);
    base_node gpt;
    ind_dof.resize(mf.nb_basic_dof());
    dal::bit_vector alldofs;
    size_type max_dof = 0;
    if (mim.convex_index().card() == 0) return;
    for (dal::bv_visitor cv(mim.convex_index()); !cv.finished(); ++cv) {
      if (dim_ == dim_type(-1))
        dim_ = mim.linked_mesh().structure_of_convex(cv)->dim();

      GMM_ASSERT1(dim_ == mim.linked_mesh().structure_of_convex(cv)->dim(),
                  "Convexes of different dimension: to be done");
      pintegration_method pim = mim.int_method_of_element(cv);
      GMM_ASSERT1(pim->type() == IM_APPROX, "You have to use approximated "
                  "integration to interpolate a fem");
      papprox_integration pai = pim->approx_method();
      bgeot::pgeometric_trans pgt = mim.linked_mesh().trans_of_convex(cv);
      elements[cv].gausspt.resize(pai->nb_points());
      dal::bit_vector dofs;
      size_type last_cv = size_type(-1);
      for (size_type k = 0; k < pai->nb_points(); ++k) {
        gausspt_interpolation_data &gpid = elements[cv].gausspt[k];
        /* todo: use a geotrans_interpolation_context */
        gpt = pgt->transform(pai->point(k),
                             mim.linked_mesh().points_of_convex(cv));
        gpid.iflags = find_a_point(gpt, gpid.ptref, gpid.elt) ? 1 : 0;
        if (gpid.iflags && last_cv != gpid.elt) {
          size_type nbd = mf.nb_basic_dof_of_element(gpid.elt);
          for (size_type i = 0; i < nbd; ++i) {
            size_type idof = mf.ind_basic_dof_of_element(gpid.elt)[i];
            if (!(blocked_dof[idof])) dofs.add(idof);
          }
          last_cv = gpid.elt;
        }
      }
      elements[cv].nb_dof = dofs.card();
      elements[cv].pim = pim;
      max_dof = std::max(max_dof, dofs.card());
      elements[cv].inddof.resize(dofs.card());
      size_type cnt = 0;
      for (dal::bv_visitor idof(dofs); !idof.finished(); ++idof)
        { elements[cv].inddof[cnt] = idof; ind_dof[idof] = cnt++; }
      for (size_type k = 0; k < pai->nb_points(); ++k) {
        gausspt_interpolation_data &gpid = elements[cv].gausspt[k];
        if (gpid.iflags) {
          size_type nbd = mf.nb_basic_dof_of_element(gpid.elt);
          gpid.local_dof.resize(nbd);
          for (size_type i = 0; i < nbd; ++i) {
            size_type ndof = mf.ind_basic_dof_of_element(gpid.elt)[i];
            gpid.local_dof[i] = dofs.is_in(ndof) ? ind_dof[ndof]
                                                 : size_type(-1);
          }
        }
      }
      alldofs |= dofs;
    }
    /** setup global dofs, with dummy coordinates */
    base_node P(dim()); gmm::fill(P,1./20);
    std::vector<base_node> node_tab_(max_dof, P);
    pspt_override = bgeot::store_point_tab(node_tab_);
    pspt_valid = false;
    dof_types_.resize(max_dof);
    std::fill(dof_types_.begin(), dof_types_.end(),
              global_dof(dim()));

    /* ind_dof should be kept full of -1 ( real_base_value and
       grad_base_value expect that)
    */
    std::fill(ind_dof.begin(), ind_dof.end(), size_type(-1));
  }

  size_type interpolated_fem::nb_dof(size_type cv) const
  { context_check();
    if (mim.linked_mesh().convex_index().is_in(cv))
      return elements[cv].nb_dof;
    else GMM_ASSERT1(false, "Wrong convex number: " << cv);
  }

  size_type interpolated_fem::index_of_global_dof
  (size_type cv, size_type i) const
  { return elements[cv].inddof[i]; }

  bgeot::pconvex_ref interpolated_fem::ref_convex(size_type cv) const
  { return mim.int_method_of_element(cv)->approx_method()->ref_convex(); }

  const bgeot::convex<base_node> &interpolated_fem::node_convex
  (size_type cv) const
  {
    if (mim.linked_mesh().convex_index().is_in(cv))
      return *(bgeot::generic_dummy_convex_ref
               (dim(), nb_dof(cv),
                mim.linked_mesh().structure_of_convex(cv)->nb_faces()));
    else GMM_ASSERT1(false, "Wrong convex number: " << cv);
  }

  void interpolated_fem::base_value(const base_node &, base_tensor &) const
  { GMM_ASSERT1(false, "No base values, real only element."); }
  void interpolated_fem::grad_base_value(const base_node &,
                                         base_tensor &) const
  { GMM_ASSERT1(false, "No grad values, real only element."); }
  void interpolated_fem::hess_base_value(const base_node &,
                                         base_tensor &) const
  { GMM_ASSERT1(false, "No hess values, real only element."); }

  inline void interpolated_fem::actualize_fictx(pfem pf, size_type cv,
                                                const base_node &ptr) const {
    if (fictx_cv != cv) {
      mf.linked_mesh().points_of_convex(cv, G);
      fictx = fem_interpolation_context
        (mf.linked_mesh().trans_of_convex(cv), pf, base_node(), G, cv,
         short_type(-1));
      fictx_cv = cv;
    }
    fictx.set_xref(ptr);
  }

  void interpolated_fem::real_base_value(const fem_interpolation_context& c,
                                         base_tensor &t, bool) const {
    elt_interpolation_data &e = elements.at(c.convex_num());
    size_type cv;

    mi2[1] = target_dim(); mi2[0] = short_type(e.nb_dof);
    t.adjust_sizes(mi2);
    std::fill(t.begin(), t.end(), scalar_type(0));
    if (e.nb_dof == 0) return;

    if (c.have_pgp() &&
        (c.pgp()->get_ppoint_tab()
         == e.pim->approx_method()->pintegration_points())) {
      gausspt_interpolation_data &gpid = e.gausspt.at(c.ii());
      if (gpid.iflags & 1) {
        cv = gpid.elt;
        pfem pf = mf.fem_of_element(cv);
        unsigned rdim = target_dim() / pf->target_dim(), mdim = (rdim==1) ? 0 : 1;
        if (gpid.iflags & 2) { t = gpid.base_val; return; }
        actualize_fictx(pf, cv, gpid.ptref);
        pf->real_base_value(fictx, taux);
        for (size_type i = 0; i < pf->nb_dof(cv); ++i)
          if (gpid.local_dof[i*rdim] != size_type(-1))
            for (size_type j = 0; j < target_dim(); ++j)
              t(gpid.local_dof[i*rdim+j*mdim],j) = taux(i, j*(1-mdim));
        if (store_values) { gpid.base_val = t; gpid.iflags |= 2; }
      }
    }
    else {
      if (find_a_point(c.xreal(), ptref, cv)) {
        pfem pf = mf.fem_of_element(cv);
        unsigned rdim = target_dim() / pf->target_dim(), mdim = (rdim==1) ? 0 : 1;
        actualize_fictx(pf, cv, ptref);
        pf->real_base_value(fictx, taux);
        for (size_type i = 0; i < e.nb_dof; ++i) {
          ind_dof.at(e.inddof[i]) = i;
        }
        for (size_type i = 0; i < pf->nb_dof(cv); ++i)
          for (size_type j = 0; j < target_dim(); ++j)
            if (ind_dof.at(mf.ind_basic_dof_of_element(cv)[i*rdim+j*mdim])
                != size_type(-1)) {
              t(ind_dof[mf.ind_basic_dof_of_element(cv)[i*rdim+j*mdim]], j)
                = taux(i, j*(1-mdim));
            }
        for (size_type i = 0; i < elements[c.convex_num()].nb_dof; ++i)
          ind_dof[e.inddof[i]] = size_type(-1);
      }
    }

  }

  void interpolated_fem::real_grad_base_value
  (const fem_interpolation_context& c, base_tensor &t, bool) const {
    size_type N0 = mf.linked_mesh().dim();
    elt_interpolation_data &e = elements.at(c.convex_num());
    size_type nbdof = e.nb_dof, cv;

    mi3[2] = short_type(N0); mi3[1] = target_dim(); mi3[0] = short_type(nbdof);
    t.adjust_sizes(mi3);
    std::fill(t.begin(), t.end(), scalar_type(0));
    if (nbdof == 0) return;

    if (c.have_pgp()  &&
        (c.pgp()->get_ppoint_tab()
         == e.pim->approx_method()->pintegration_points())) {
      gausspt_interpolation_data &gpid = e.gausspt.at(c.ii());
      if (gpid.iflags & 1) {
        cv = gpid.elt;
        pfem pf = mf.fem_of_element(cv);
        unsigned rdim = target_dim() / pf->target_dim(), mdim = (rdim==1) ? 0 : 1;
        if (gpid.iflags & 4) { t = gpid.grad_val; return; }
        actualize_fictx(pf, cv, gpid.ptref);
        pf->real_grad_base_value(fictx, taux);

        if (pif) {
          pif->grad(c.xreal(), trans);
          for (size_type i = 0; i < pf->nb_dof(cv); ++i)
            if (gpid.local_dof[i*rdim] != size_type(-1))
              for (size_type j = 0; j < target_dim(); ++j)
                for (size_type k = 0; k < N0; ++k) {
                  scalar_type ee(0);
                  for (size_type l = 0; l < N0; ++l)
                    ee += trans(l, k) * taux(i, j*(1-mdim), l);
                  t(gpid.local_dof[i*rdim+j*mdim], j, k) = ee;
                }
        }
        else {
          for (size_type i = 0; i < pf->nb_dof(cv); ++i)
            if (gpid.local_dof[i*rdim] != size_type(-1))
              for (size_type j = 0; j < target_dim(); ++j)
                for (size_type k = 0; k < N0; ++k)
                  t(gpid.local_dof[i*rdim+j*mdim], j, k)
                    = taux(i, j*(1-mdim), k);
          if (store_values) { gpid.grad_val = t; gpid.iflags |= 4; }
        }
      }
    }
    else {
      cerr << "NON PGP OU MAUVAIS PTS sz=" << elements.size() << ", cv="
           << c.convex_num() << " ";
      cerr << "ii=" << c.ii() << ", sz=" << e.gausspt.size() << "\n";

      if (find_a_point(c.xreal(), ptref, cv)) {
        pfem pf = mf.fem_of_element(cv);
        unsigned rdim = target_dim() / pf->target_dim(), mdim = (rdim==1) ? 0 : 1;
        actualize_fictx(pf, cv, ptref);
        pf->real_grad_base_value(fictx, taux);
        for (size_type i = 0; i < nbdof; ++i)
          ind_dof.at(e.inddof.at(i)) = i;
        if (pif) {
          pif->grad(c.xreal(), trans);
          for (size_type i = 0; i < pf->nb_dof(cv); ++i)
            for (size_type j = 0; j < target_dim(); ++j)
              for (size_type k = 0; k < N0; ++k)
                if (ind_dof[mf.ind_basic_dof_of_element(cv)[i*rdim+j*mdim]]
                    != size_type(-1)) {
                  scalar_type ee(0);
                  for (size_type l = 0; l < N0; ++l)
                    ee += trans(l, k) * taux(i, j*(1-mdim), l);
                  t(ind_dof[mf.ind_basic_dof_of_element(cv)[i*rdim+j*mdim]],j,k)=ee;
                }
        }
        else {
          for (size_type i = 0; i < pf->nb_dof(cv); ++i)
            for (size_type j = 0; j < target_dim(); ++j)
              for (size_type k = 0; k < N0; ++k)
                if (ind_dof[mf.ind_basic_dof_of_element(cv)[i*rdim+j*mdim]]
                    != size_type(-1))
                  t(ind_dof[mf.ind_basic_dof_of_element(cv)[i*rdim+j*mdim]],j,k)
                    = taux(i,j*(1-mdim),k);
        }
        for (size_type i = 0; i < nbdof; ++i)
          ind_dof[e.inddof[i]] = size_type(-1);
      }
    }
  }

  void interpolated_fem::real_hess_base_value
  (const fem_interpolation_context&, base_tensor &, bool) const
  { GMM_ASSERT1(false, "Sorry, to be done."); }


  dal::bit_vector interpolated_fem::interpolated_convexes() const {
    dal::bit_vector bv;
    for (dal::bv_visitor cv(mim.linked_mesh().convex_index()); !cv.finished();
         ++cv) {
      for (unsigned ii=0; ii < elements.at(cv).gausspt.size(); ++ii) {
        if (elements[cv].gausspt[ii].iflags)
          bv.add(elements[cv].gausspt[ii].elt);
      }
    }
    return bv;
  }

  void interpolated_fem::gauss_pts_stats(unsigned &ming, unsigned &maxg,
                                         scalar_type &meang) const {
    std::vector<unsigned> v(mf.linked_mesh().nb_allocated_convex());
    for (dal::bv_visitor cv(mim.linked_mesh().convex_index());
         !cv.finished(); ++cv) {
      for (unsigned ii=0; ii < elements.at(cv).gausspt.size(); ++ii) {
        if (elements[cv].gausspt[ii].iflags)
          v[elements[cv].gausspt[ii].elt]++;
      }
    }
    ming = 100000; maxg = 0; meang = 0;
    for (dal::bv_visitor cv(mf.linked_mesh().convex_index());
         !cv.finished(); ++cv) {
      ming = std::min(ming, v[cv]);
      maxg = std::max(maxg, v[cv]);
      meang += v[cv];
    }
    meang /= scalar_type(mf.linked_mesh().convex_index().card());
  }

  size_type interpolated_fem::memsize() const {
    size_type sz = 0;
    sz += blocked_dof.memsize();
    sz += sizeof(*this);
    sz += elements.capacity() * sizeof(elt_interpolation_data);
    for (unsigned i=0; i < elements.size(); ++i) {
      sz += elements[i].gausspt.capacity()*sizeof(gausspt_interpolation_data);
      sz += elements[i].inddof.capacity() * sizeof(size_type);
      for (unsigned j=0; j < elements[i].gausspt.size(); ++j) {
        sz += elements[i].gausspt[j].local_dof.capacity() * sizeof(size_type);
      }
    }
    return sz;
  }

  interpolated_fem::interpolated_fem(const mesh_fem &mef,
                                     const mesh_im &meim,
                                     pinterpolated_func pif_,
                                     dal::bit_vector blocked_dof_,
                                     bool store_val)
    : mf(mef), mim(meim), pif(pif_), store_values(store_val),
      blocked_dof(blocked_dof_), boxtree{1E-13}, mi2(2), mi3(3) {
    DAL_STORED_OBJECT_DEBUG_CREATED(this, "Interpolated fem");
    this->add_dependency(mf);
    this->add_dependency(mim);
    is_pol = is_lag = is_standard_fem = false; es_degree = 5;
    is_equiv = real_element_defined = true;
    gmm::resize(trans, mf.linked_mesh().dim(), mf.linked_mesh().dim());
    ntarget_dim = mef.get_qdim();
    update_from_context();
  }

  DAL_SIMPLE_KEY(special_intfem_key, pfem);

  pfem new_interpolated_fem(const mesh_fem &mef, const mesh_im &mim,
                            pinterpolated_func pif,
                            dal::bit_vector blocked_dof, bool store_val) {
    pfem pf = std::make_shared<interpolated_fem>
      (mef, mim, pif, blocked_dof, store_val);
    dal::pstatic_stored_object_key pk=std::make_shared<special_intfem_key>(pf);
    dal::add_stored_object(pk, pf);
    return pf;
  }


}  /* end of namespace getfem.                                            */

