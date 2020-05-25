/* -*- c++ -*- (enables emacs c++ mode) */
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

 As a special exception, you  may use  this file  as it is a part of a free
 software  library  without  restriction.  Specifically,  if   other  files
 instantiate  templates  or  use macros or inline functions from this file,
 or  you compile this  file  and  link  it  with other files  to produce an
 executable, this file  does  not  by itself cause the resulting executable
 to be covered  by the GNU Lesser General Public License.  This   exception
 does not  however  invalidate  any  other  reasons why the executable file
 might be covered by the GNU Lesser General Public License.

===========================================================================*/
/**@file getfem_mesh_fem_sum.h
   @author Yves Renard <Yves.Renard@insa-lyon.fr>
   @author Julien Pommier <Julien.Pommier@insa-toulouse.fr>
   @date March 18, 2005.
   @brief Implement a special mesh_fem with merges the FEMs of two
    (or more) mesh_fems.
*/

#ifndef GETFEM_MESH_SUM_H__
#define GETFEM_MESH_SUM_H__

#include "getfem_mesh_fem.h"

namespace getfem {
  typedef std::vector<const std::string *> dof_enrichments;
  
  /** @internal FEM used in mesh_fem_sum objects. */
  class fem_sum : public virtual_fem {
    std::vector<pfem> pfems; /* the fems to be summed */
    bool smart_global_dof_linking_;
    size_type cv;
    
  public:

    size_type index_of_global_dof(size_type cv_, size_type j) const;
    fem_sum(const std::vector<pfem> &pfs, size_type i,
	    bool smart_global_dof_linking)
      : pfems(pfs), smart_global_dof_linking_(smart_global_dof_linking),
	cv(i) { init(); }
    void init();
    void valid();
    void base_value(const base_node &x, base_tensor &t) const;
    void grad_base_value(const base_node &x, base_tensor &t) const;
    void hess_base_value(const base_node &x, base_tensor &t) const;

    void real_base_value(const fem_interpolation_context& c, 
			 base_tensor &t, bool = true) const;    
    void real_grad_base_value(const fem_interpolation_context& c, 
			      base_tensor &t, bool = true) const;
    void real_hess_base_value(const fem_interpolation_context& c, 
			      base_tensor &t, bool = true) const;
    void mat_trans(base_matrix &M, const base_matrix &G,
		   bgeot::pgeometric_trans pgt) const;
  };


  /** @brief Implement a special mesh_fem with merges the FEMs of
      two (or more) mesh_fems.
  */
  class mesh_fem_sum : public mesh_fem {
  protected :
    std::vector<const mesh_fem *> mfs;

    mutable std::map< std::vector<pfem>, pfem> situations;
    mutable std::vector<pfem> build_methods;
    mutable bool is_adapted;
    bool smart_global_dof_linking_;
    void clear_build_methods();

  public :
    void adapt(void);
    void update_from_context(void) const { is_adapted = false; }
    void clear(void);
    
    size_type memsize() const {
      return mesh_fem::memsize(); // + ... ;
    }
    
    mesh_fem_sum(const mesh &me, bool smart_global_dof_linking = false)
      : mesh_fem(me), smart_global_dof_linking_(smart_global_dof_linking)
    { is_adapted = false; }
    void set_mesh_fems(const std::vector<const mesh_fem *> &mefs)
    { mfs = mefs; adapt(); }
    /** enabled "smart" dof linking between the mesh_fems. 
	It was introduced for the point-wise matching part of 
	tests/crack.cc but it does not work with discontinuous global
	functions... 
    */
    void set_smart_global_dof_linking(bool b)
    { smart_global_dof_linking_ = b; }
    void set_mesh_fems(const mesh_fem &mf1)
    { mfs.clear(); mfs.push_back(&mf1); adapt(); }
    void set_mesh_fems(const mesh_fem &mf1, const mesh_fem &mf2)
    { mfs.clear(); mfs.push_back(&mf1); mfs.push_back(&mf2);  adapt(); }
    void set_mesh_fems(const mesh_fem &mf1, const mesh_fem &mf2,
		       const mesh_fem &mf3) {
      mfs.clear();
      mfs.push_back(&mf1); mfs.push_back(&mf2); mfs.push_back(&mf3);
      adapt();
    }

    ~mesh_fem_sum() { clear_build_methods(); }
  };


}  /* end of namespace getfem.                                            */

#endif
  
