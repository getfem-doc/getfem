/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================

 Copyright (C) 2014-2020 Liang Jin Lim

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
/**
@file getfem_torus.h
@brief Provides mesh and mesh fem of torus.
@date May 2014
@author Liang Jin Lim
*/

#pragma once

#ifndef GETFEM_TORUS_H__
#define GETFEM_TORUS_H__

#include "getfem/getfem_mesh_fem.h"

namespace getfem
{
  /**Torus fem, the real grad base value is modified to compute radial grad of F/R.
     It stores a reference to the original fem object. By default, torus_fem is vectorial.
     There is an option to change it to a scalar form through set_to_scalar(bool is_scalar).
     torus_mesh_fem will automatically check qdim of itself and set the form accordingly.
  */
  class torus_fem : public virtual_fem{

  public :
    virtual size_type index_of_global_dof(size_type cv, size_type i) const;
    void base_value(const base_node &, base_tensor &) const;
    void grad_base_value(const base_node &, base_tensor &) const;
    void hess_base_value(const base_node &, base_tensor &) const;
    void real_base_value(const fem_interpolation_context& c,
      base_tensor &t, bool = true) const;
    void real_grad_base_value(const fem_interpolation_context& c,
      base_tensor &t, bool = true) const;
    void real_hess_base_value(const fem_interpolation_context&,
      base_tensor &, bool = true) const;

    pfem get_original_pfem() const;

    torus_fem(pfem pf) : virtual_fem(*pf), poriginal_fem_(pf), is_scalar_(false){
      init();
    }

    void set_to_scalar(bool is_scalar);

  protected :
    void init();

  private :
    pfem poriginal_fem_;
    bool is_scalar_;
  };
  
  /**Copy an original 2D mesh to become a torus mesh with radial dimension.*/
  class torus_mesh : public mesh
  {
  private:
    bool is_adapted_;
    
  public:
    torus_mesh(std::string name = std::string());
    virtual scalar_type convex_radius_estimate(size_type ic) const;

    void adapt();
    void adapt(const getfem::mesh &original_mesh);
  };

  /**Mesh fem object that adapts */
  class torus_mesh_fem : public mesh_fem{
  public:

    torus_mesh_fem(const torus_mesh &mesh, bgeot::dim_type dim) : mesh_fem(mesh, dim){}    
    void enumerate_dof(void) const;

  private:
    void adapt_to_torus_();
    void del_torus_fem_();
  };

}

#endif /* GETFEM_TORUS_H__  */
