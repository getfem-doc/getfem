/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================

 Copyright (C) 2000-2020 Yves Renard

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
/**@file getfem_mat_elem_type.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date December 21, 2000.
   @brief Build elementary tensors descriptors, used by generic assembly.
*/

#ifndef GETFEM_MAT_ELEM_TYPE_H__
#define GETFEM_MAT_ELEM_TYPE_H__

#include "getfem_config.h"
#include "getfem_integration.h"
#include "getfem_fem.h"

namespace getfem {

  enum constituant_type
    { GETFEM_BASE_, GETFEM_GRAD_, GETFEM_HESSIAN_, GETFEM_NONLINEAR_,
      GETFEM_UNIT_NORMAL_, GETFEM_GRAD_GEOTRANS_, GETFEM_GRAD_GEOTRANS_INV_ };

  class mat_elem_type;
  typedef std::shared_ptr<const mat_elem_type> pmat_elem_type;

  /**
     abstract class for integration of non-linear terms into the mat_elem
     computations
     the nonlinear term is added into the mat_elem_type via mat_elem_nonlinear

     When this object is destroyed, its destructor will remove all
     references to it from its internal structures.
     
     The object must depend on one pfem (and may depend on other optional pfem)

     During elementary computations, the function "prepare" is called
     for each optional pfem. Then the method "compute" is called with
     the main fem context.
  */
  class nonlinear_elem_term {
    mutable std::set<pmat_elem_type> melt_list; /* list of melt that will
						 * be destroyed by
						 * ~nonlinear_elem_term */
    mutable size_type term_num_;
    
  public :
    virtual const bgeot::multi_index &sizes(size_type icv) const = 0;
    virtual void compute(fem_interpolation_context& /*ctx*/,
                         base_tensor &/*output*/) = 0;
    virtual void prepare(fem_interpolation_context& /*ctx*/,
                         size_type /*nl_part*/) {}
    virtual ~nonlinear_elem_term();
    size_type &term_num(void) const { return term_num_; }
    void register_mat_elem(pmat_elem_type p) /* internal use */
    { melt_list.insert(p); }
  };

  typedef nonlinear_elem_term* pnonlinear_elem_term;

  struct constituant {
    constituant_type t;
    pfem pfi;
    unsigned nl_part; /* only useful with GETFEM_NONLINEAR_ : since the
			 nonlinear term may use more than one pfem, it will
			 be splitted into one "constituant" per fem
			 for (nl_part = 0), the mat_elem_* computations will
			 call nlt->compute,
			 for (nl_part != 0) they will call
			 nlt->prepare(ctx,nl_part)
		      */
    pnonlinear_elem_term nlt;
  };

  /** Description of an elementary matrix.  This class 
   *  is not to be manipulate by itself. Use pmat_elem_type and
   *  the functions written to produce those descriptions.
   */ 
  class mat_elem_type
    : virtual public dal::static_stored_object, public std::vector<constituant> {
  protected :
    bgeot::multi_index mi;
  public :
    bgeot::multi_index sizes(size_type) const;
    bgeot::multi_index &get_mi(void) { return mi; }
    const bgeot::multi_index &get_mi(void) const { return mi; }
    mat_elem_type() { DAL_STORED_OBJECT_DEBUG_CREATED(this, "Mat elem type"); }
    mat_elem_type(const mat_elem_type &o)
      : std::vector<constituant>(o), mi(o.mi)
    { DAL_STORED_OBJECT_DEBUG_CREATED(this, "Mat elem type"); }
    ~mat_elem_type()
    { DAL_STORED_OBJECT_DEBUG_DESTROYED(this, "Mat elem type"); }
  };

   /** @name functions on elementary matrix descriptions
   */
  //@{
  
  
  /** Give a pointer to the structure describing the elementary matrix
   *   which compute the integral of the basic functions described by pfi.
   */
  pmat_elem_type mat_elem_base(pfem pfi);
  /** Give a pointer to the structure describing the elementary matrix
   *   which compute the integral of the gradient of the basic functions
   *    described by pfi. 
   */
  pmat_elem_type mat_elem_grad(pfem pfi);
  /** Give a pointer to the structure describing the elementary matrix
   *   which compute the unit normal on the boundary of the element 
   */
  pmat_elem_type mat_elem_unit_normal(void);
  /** 
      Return the gradient of the geometrical transformation ("K" in
      the getfem++ kernel doc.), or its pseudo-inverse (transposed(B) in the
      getfem++ kernel doc.).
  */
  pmat_elem_type mat_elem_grad_geotrans(bool inverted);
  /** Give a pointer to the structure describing the elementary matrix
   *   which compute the integral of the hessian of the basic functions
   *    described by pfi.
   */
    pmat_elem_type mat_elem_hessian(pfem pfi);
  /** Give a pointer to the structure describing the elementary
    matrix which compute the integral of a nonlinear term.  
    
    The pnonlinear_elem_term must not be destroyed, at any time!
    vector pfi can not be empty pfi[0] is the main fem for the
    pnonlinear_term.

    During computations of elementary tensors in getfem_mat_elem.C, 
      pnonlinear_elem_term->prepare() will be called for each pfi[i>=1],
    and then
      pnonlinear_elem_term->compute() will be called for pfi[0]
   */
  pmat_elem_type mat_elem_nonlinear(pnonlinear_elem_term,
				    std::vector<pfem> pfi);

  /** Give a pointer to the structure describing the elementary matrix
   *   which computes the integral of product described by
   *   *a and *b.
   */
  pmat_elem_type mat_elem_product(pmat_elem_type a, pmat_elem_type b);
  
  pmat_elem_type mat_elem_empty();
   //@}

}  /* end of namespace getfem.                                             */


#endif /* GETFEM_PRE_INTERPOLATION_H__                                     */
