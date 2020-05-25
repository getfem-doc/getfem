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
/** \file getfem_fem_level_set.cc
    \brief an FEM which should be used with getfem::mesh_fem_level_set.
*/
#include "getfem/getfem_fem_level_set.h"

namespace getfem {
    
  void fem_level_set::init() {
    cvr = bfem->ref_convex(0);
    dim_ = cvr->structure()->dim();
    is_equiv = true; real_element_defined = true;
    is_polycomp = is_pol = is_lag = is_standard_fem = false;
    es_degree = 5; /* humm ... */
    ntarget_dim = bfem->target_dim();

    std::stringstream nm;
    nm << "FEM_LEVEL_SET(" << bfem->debug_name() << ")";
    debug_name_ = nm.str();
 
    ls_index.sup(0, mls.nb_level_sets());

    common_ls_zones.resize(mls.nb_level_sets());
    /* fill ls_index .. */
    for (size_type i=0; i < mls.nb_level_sets(); ++i) {
      char c = '*';
      for (size_type k=0; k < bfem->nb_dof(0); ++k) {
	const mesh_level_set::zoneset *ze = dofzones[k];
	if (ze) {
	  for (mesh_level_set::zoneset::const_iterator itz = ze->begin();
	       itz != ze->end(); ++itz) {
	    const mesh_level_set::zone *z = *itz;
	    for (mesh_level_set::zone::const_iterator it = z->begin(); 
		 it != z->end(); ++it) {
	      assert((**it).size() == mls.nb_level_sets());
	      char d = (*(*it))[i];
	      if (c == '*') c = d;
	      else if (c != d) { ls_index.add(i); break; }	      
	    }
	  }
	}
      }
      common_ls_zones[i] = c;
    }
    
    init_cvs_node();
    for (size_type k = 0; k < bfem->nb_dof(0); ++k) {
      if (!dofzones[k]) {
	add_node(bfem->dof_types()[k], bfem->node_of_dof(0,k));
      } else {
	for (size_type j = 0; j < dofzones[k]->size(); ++j) {
	  // cout << " -> +dof: '" << *(*dofzones[k])[j] << "'\n";
	  add_node(xfem_dof(bfem->dof_types()[k], j+xfem_index),
		   bfem->node_of_dof(0,k));
	}
      }
    }
  }

  void fem_level_set::base_value(const base_node &, base_tensor &) const
  { GMM_ASSERT1(false, "No base values, real only element."); }
  void fem_level_set::grad_base_value(const base_node &, 
				      base_tensor &) const
  { GMM_ASSERT1(false, "No base values, real only element."); }
  void fem_level_set::hess_base_value(const base_node &, 
				      base_tensor &) const
  { GMM_ASSERT1(false, "No base values, real only element.");  }

  static bool are_zones_compatible_(const std::string a, const std::string b) {
    if (a.size() != b.size()) return false;
    for (size_type i = 0; i < a.size(); ++i)
      if (a[i] != '0' && a[i] != b[i]) return false;
    return true;
  }

  void fem_level_set::find_zone_id(const fem_interpolation_context &c, 
				   std::vector<bool> &ids, int side) const {
    size_type s = 0, cv = c.convex_num();
    for (size_type i = 0; i < dofzones.size(); ++i)
      if (dofzones[i]) s += dofzones[i]->size();
    ids.resize(0); ids.resize(s, false);
    std::string z(common_ls_zones);
    base_vector coeff(32);
    
    mesher_level_set eval;
    size_type iclosest = size_type(-1); scalar_type vclosest = 1E100;
    for (dal::bv_visitor i(ls_index); !i.finished(); ++i) {
      const level_set *ls = mls.get_level_set(i);
      const mesh_fem &mf = ls->get_mesh_fem();
      slice_vector_on_basic_dof_of_element(mf, ls->values(), cv, coeff);
      eval.init_base(mf.fem_of_element(cv), coeff);
      eval.set_shift(ls->get_shift()); // Deprecated

      // mesher_level_set eval = mls.get_level_set(i)->mls_of_convex(cv);

      scalar_type v = eval(c.xref());
      if (side != 0) {
        if (gmm::abs(v) <  vclosest) { vclosest = gmm::abs(v); iclosest = i; }
      }
      z[size_type(i)] = (v > 0.) ? '+' : '-';
    }

    if (side != 0 && iclosest != size_type(-1)) // Forces the side of the
                           // closest level-set (in order to compute jumps).
      z[iclosest] = (side > 0) ? '+' : '-';


    unsigned cnt = 0;
    for (unsigned d = 0; d < dofzones.size(); ++d) {
      if (!dofzones[d]) continue;
      for (mesh_level_set::zoneset::const_iterator it = dofzones[d]->begin();
	   it != dofzones[d]->end(); ++it, ++cnt) {
	ids[cnt] = false;
	for (mesh_level_set::zone::const_iterator it2 = (*it)->begin();
	     it2 != (*it)->end(); ++it2) {
	  if (are_zones_compatible_(z,*(*it2))) { ids[cnt] = true; break; }
	}
      }
    }
  }

  void fem_level_set::real_base_value(const fem_interpolation_context &c,
				      base_tensor &t, bool) const {
    // bgeot::multi_index mi(2);
    // mi[1] = target_dim(); mi[0] = short_type(nb_base(0));
    t.adjust_sizes(nb_base(0), target_dim());
    base_tensor::iterator it = t.begin();
    fem_interpolation_context c0 = c;
    if (c0.have_pfp())
      c0.set_pfp(fem_precomp(bfem, c0.pfp()->get_ppoint_tab(), c0.pfp()));
    else  c0.set_pf(bfem); 
    base_tensor tt; c0.base_value(tt);
    base_tensor::const_iterator itf = tt.begin();

    std::vector<bool> zid;
    find_zone_id(c, zid, c.xfem_side());
    for (dim_type q = 0; q < target_dim(); ++q) {
      unsigned cnt = 0;
      for (size_type d = 0; d < bfem->nb_dof(0); ++d, ++itf) {
	if (dofzones[d]) { /* enriched dof ? */
	  for (size_type k = 0; k < dofzones[d]->size(); ++k, ++cnt)
	    *it++ = zid[cnt] ? *itf : 0;
	} else *it++ = *itf;
      }
    }
    assert(it == t.end());
  }

  void fem_level_set::real_grad_base_value(const fem_interpolation_context &c,
					   base_tensor &t, bool) const {
    // bgeot::multi_index mi(3);
    // mi[2] = short_type(c.N()); mi[1] = target_dim();
    // mi[0] = short_type(nb_base(0));
    t.adjust_sizes(nb_base(0), target_dim(), c.N());
    fem_interpolation_context c0 = c;
    if (c0.have_pfp())
      c0.set_pfp(fem_precomp(bfem, c0.pfp()->get_ppoint_tab(), c0.pfp()));
    else  c0.set_pf(bfem); 
    base_tensor tt; c0.grad_base_value(tt);

    base_tensor::iterator it = t.begin();
    base_tensor::const_iterator itf = tt.begin();

    std::vector<bool> zid;
    find_zone_id(c, zid, c.xfem_side());

    for (dim_type i = 0; i < c.N() ; ++i) {
      for (dim_type q = 0; q < target_dim(); ++q) {
	unsigned cnt = 0;
	for (size_type d = 0; d < bfem->nb_dof(0); ++d, ++itf) {
	  if (dofzones[d]) { /* enriched dof ? */
	    for (size_type k = 0; k < dofzones[d]->size(); ++k, ++cnt)
	      *it++ = zid[cnt] ? *itf : 0;
	  } else *it++ = *itf;
	}
      }
    }
    assert(it == t.end());
  }
  
  void fem_level_set::real_hess_base_value(const fem_interpolation_context &c,
                                           base_tensor &t, bool) const {
    t.adjust_sizes(nb_base(0), target_dim(), gmm::sqr(c.N()));
    fem_interpolation_context c0 = c;
    if (c0.have_pfp())
      c0.set_pfp(fem_precomp(bfem, c0.pfp()->get_ppoint_tab(), c0.pfp()));
    else  c0.set_pf(bfem); 
    base_tensor tt; c0.hess_base_value(tt);

    base_tensor::iterator it = t.begin();
    base_tensor::const_iterator itf = tt.begin();

    std::vector<bool> zid;
    find_zone_id(c, zid, c.xfem_side());

    dim_type NNdim = dim_type(gmm::sqr(c.N())*target_dim());
    for (dim_type ijq = 0; ijq < NNdim ; ++ijq) {
      unsigned cnt = 0;
      for (size_type d = 0; d < bfem->nb_dof(0); ++d, ++itf) {
        if (dofzones[d]) /* enriched dof ? */
          for (size_type k = 0; k < dofzones[d]->size(); ++k, ++cnt)
            *it++ = zid[cnt] ? *itf : 0;
        else
          *it++ = *itf;
      }
    }
    assert(it == t.end());
  }


}  /* end of namespace getfem.                                             */

