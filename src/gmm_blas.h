/* -*- c++ -*- (enables emacs c++ mode)                                    */
/* *********************************************************************** */
/*                                                                         */
/* Library :  Generic Matrix Methods  (gmm)                                */
/* File    :  gmm_blas.h : generic basic linear algebra algorithms.        */
/*                         (and others ...)                                */
/*     									   */
/* Date : October 13, 2002.                                                */
/* Author : Yves Renard, Yves.Renard@gmm.insa-tlse.fr                      */
/*                                                                         */
/* *********************************************************************** */
/*                                                                         */
/* Copyright (C) 2002  Yves Renard.                                        */
/*                                                                         */
/* This file is a part of GMM++                                            */
/*                                                                         */
/* This program is free software; you can redistribute it and/or modify    */
/* it under the terms of the GNU Lesser General Public License as          */
/* published by the Free Software Foundation; version 2.1 of the License.  */
/*                                                                         */
/* This program is distributed in the hope that it will be useful,         */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           */
/* GNU Lesser General Public License for more details.                     */
/*                                                                         */
/* You should have received a copy of the GNU Lesser General Public        */
/* License along with this program; if not, write to the Free Software     */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,  */
/* USA.                                                                    */
/*                                                                         */
/* *********************************************************************** */

#ifndef __GMM_BLAS_H
#define __GMM_BLAS_H

#include <gmm_scaled.h>
#include <gmm_transposed.h>
#include <gmm_conjugated.h>

namespace gmm {

  /* ******************************************************************** */
  /*		                                         		  */
  /*		Generic algorithms                           		  */
  /*		                                         		  */
  /* ******************************************************************** */


  /* ******************************************************************** */
  /*		Miscellaneous                           		  */
  /* ******************************************************************** */

  template <typename V> inline size_type vect_size(const V &v)
  { return linalg_traits<V>::size(v); }

  template <typename MAT> inline size_type mat_nrows(const MAT &m)
  { return linalg_traits<MAT>::nrows(m); }

  template <typename MAT> inline size_type mat_ncols(const MAT &m)
  { return linalg_traits<MAT>::ncols(m); }


  template <typename L> 
  typename select_return<const typename linalg_traits<L>::origin_type *,
			 typename linalg_traits<L>::origin_type *,
			 L *>::return_type
  linalg_origin(L &l)
  { return linalg_traits<L>::origin(linalg_cast(l)); }

  template <typename L> 
  typename select_return<const typename linalg_traits<L>::origin_type *,
			 typename linalg_traits<L>::origin_type *,
			 const L *>::return_type
  linalg_origin(const L &l)
  { return linalg_traits<L>::origin(linalg_cast(l)); }


  template <typename V> inline
  typename select_return<typename linalg_traits<V>::const_iterator,
           typename linalg_traits<V>::iterator, V *>::return_type
  vect_begin(V &v)
  { return linalg_traits<V>::begin(linalg_cast(v)); }

  template <typename V> inline
  typename select_return<typename linalg_traits<V>::const_iterator,
	   typename linalg_traits<V>::iterator, const V *>::return_type
  vect_begin(const V &v)
  { return linalg_traits<V>::begin(linalg_cast(v)); }

  template <typename V> inline
  typename linalg_traits<V>::const_iterator
  vect_const_begin(const V &v)
  { return linalg_traits<V>::begin(v); }

  template <typename V> inline
  typename select_return<typename linalg_traits<V>::const_iterator,
    typename linalg_traits<V>::iterator, V *>::return_type
  vect_end(V &v)
  { return linalg_traits<V>::end(linalg_cast(v)); }

  template <typename V> inline
  typename select_return<typename linalg_traits<V>::const_iterator,
    typename linalg_traits<V>::iterator, const V *>::return_type
  vect_end(const V &v)
  { return linalg_traits<V>::end(linalg_cast(v)); }

  template <typename V> inline
  typename linalg_traits<V>::const_iterator
  vect_const_end(const V &v)
  { return linalg_traits<V>::end(v); }

  template <typename M> inline
  typename select_return<typename linalg_traits<M>::const_row_iterator,
    typename linalg_traits<M>::row_iterator, M *>::return_type
  mat_row_begin(M &m) { return linalg_traits<M>::row_begin(linalg_cast(m)); }
  
  template <typename M> inline
  typename select_return<typename linalg_traits<M>::const_row_iterator,
    typename linalg_traits<M>::row_iterator, const M *>::return_type
  mat_row_begin(const M &m)
  { return linalg_traits<M>::row_begin(linalg_cast(m)); }
  
  template <typename M> inline typename linalg_traits<M>::const_row_iterator
  mat_row_const_begin(const M &m)
  { return linalg_traits<M>::row_begin(m); }

  template <typename M> inline
  typename select_return<typename linalg_traits<M>::const_row_iterator,
    typename linalg_traits<M>::row_iterator, M *>::return_type
  mat_row_end(M &v) {
    return linalg_traits<M>::row_end(linalg_cast(v));
  }

  template <typename M> inline
  typename select_return<typename linalg_traits<M>::const_row_iterator,
    typename linalg_traits<M>::row_iterator, const M *>::return_type
  mat_row_end(const M &v) {
    return linalg_traits<M>::row_end(linalg_cast(v));
  }

  template <typename M> inline
  typename linalg_traits<M>::const_row_iterator
  mat_row_const_end(const M &v)
  { return linalg_traits<M>::row_end(v); }

  template <typename M> inline
  typename select_return<typename linalg_traits<M>::const_col_iterator,
    typename linalg_traits<M>::col_iterator, M *>::return_type
  mat_col_begin(M &v) {
    return linalg_traits<M>::col_begin(linalg_cast(v));
  }

  template <typename M> inline
  typename select_return<typename linalg_traits<M>::const_col_iterator,
    typename linalg_traits<M>::col_iterator, const M *>::return_type
  mat_col_begin(const M &v) {
    return linalg_traits<M>::col_begin(linalg_cast(v));
  }

  template <typename M> inline
  typename linalg_traits<M>::const_col_iterator
  mat_col_const_begin(const M &v)
  { return linalg_traits<M>::col_begin(v); }

  template <typename M> inline
  typename linalg_traits<M>::const_col_iterator
  mat_col_const_end(const M &v)
  { return linalg_traits<M>::col_end(v); }

  template <typename M> inline
  typename select_return<typename linalg_traits<M>::const_col_iterator,
                         typename linalg_traits<M>::col_iterator,
                         M *>::return_type
  mat_col_end(M &m)
  { return linalg_traits<M>::col_end(linalg_cast(m)); }

  template <typename M> inline
  typename select_return<typename linalg_traits<M>::const_col_iterator,
                         typename linalg_traits<M>::col_iterator,
                         const M *>::return_type
  mat_col_end(const M &m)
  { return linalg_traits<M>::col_end(linalg_cast(m)); }

  template <typename MAT> inline
  typename select_return<typename linalg_traits<MAT>::const_sub_row_type,
                         typename linalg_traits<MAT>::sub_row_type,
                         const MAT *>::return_type
  mat_row(const MAT &m, size_type i)
  { return linalg_traits<MAT>::row(mat_row_begin(m) + i); }

  template <typename MAT> inline
  typename select_return<typename linalg_traits<MAT>::const_sub_row_type,
                         typename linalg_traits<MAT>::sub_row_type,
                         MAT *>::return_type
  mat_row(MAT &m, size_type i)
  { return linalg_traits<MAT>::row(mat_row_begin(m) + i); }

  template <typename MAT> inline
  typename linalg_traits<MAT>::const_sub_row_type
  mat_const_row(const MAT &m, size_type i)
  { return linalg_traits<MAT>::row(mat_row_const_begin(m) + i); }

  template <typename MAT> inline
  typename select_return<typename linalg_traits<MAT>::const_sub_col_type,
                         typename linalg_traits<MAT>::sub_col_type,
                         const MAT *>::return_type
  mat_col(const MAT &m, size_type i)
  { return linalg_traits<MAT>::col(mat_col_begin(m) + i); }


  template <typename MAT> inline
  typename select_return<typename linalg_traits<MAT>::const_sub_col_type,
                         typename linalg_traits<MAT>::sub_col_type,
                         MAT *>::return_type
  mat_col(MAT &m, size_type i)
  { return linalg_traits<MAT>::col(mat_col_begin(m) + i); }
  
  template <typename MAT> inline
  typename linalg_traits<MAT>::const_sub_col_type
  mat_const_col(const MAT &m, size_type i)
  { return linalg_traits<MAT>::col(mat_col_const_begin(m) + i); }

  template <typename L> inline void clear(L &l)
  { linalg_traits<L>::do_clear(l); }

  template <typename L> inline void clear(const L &l)
  { linalg_traits<L>::do_clear(linalg_const_cast(l)); }

  template <typename L> inline size_type nnz(const L& l)
  { return nnz(l, typename linalg_traits<L>::linalg_type()); }

  template <typename L> inline size_type nnz(const L& l, abstract_vector) { 
    typename linalg_traits<L>::const_iterator it = vect_const_begin(l),
      ite = vect_const_end(l);
    size_type res(0);
    for (; it != ite; ++it) ++res;
    return res;
  }

  template <typename L> inline size_type nnz(const L& l, abstract_matrix) {
    return nnz(l,  typename principal_orientation_type<typename
	       linalg_traits<L>::sub_orientation>::potype());
  }

  template <typename L> inline size_type nnz(const L& l, row_major) {
    size_type res(0);
    for (size_type i = 0; i < mat_nrows(l); ++i)
      res += nnz(mat_const_row(l, i));
    return res;
  } 

  template <typename L> inline size_type nnz(const L& l, col_major) {
    size_type res(0);
    for (size_type i = 0; i < mat_ncols(l); ++i)
      res += nnz(mat_const_col(l, i));
    return res;
  }

  template <typename L> inline void fill_random(L& l)
  { fill_random(l, typename linalg_traits<L>::linalg_type()); }

  template <typename L> inline void fill_random(const L& l) {
    fill_random(linalg_const_cast(l),
		typename linalg_traits<L>::linalg_type());
  }

  template <typename L> inline void fill_random(L& l, abstract_vector) {
    for (size_type i = 0; i < vect_size(l); ++i)
      l[i] = gmm::random(typename linalg_traits<L>::value_type());
  }

  template <typename L> inline void fill_random(L& l, abstract_matrix) {
    for (size_type i = 0; i < mat_nrows(l); ++i)
      for (size_type j = 0; j < mat_ncols(l); ++j)
	l(i,j) = gmm::random(typename linalg_traits<L>::value_type());
  }

  template <typename L> inline void fill_random(L& l, double cfill)
  { fill_random(l, cfill, typename linalg_traits<L>::linalg_type()); }

  template <typename L> inline void fill_random(const L& l, double cfill) {
    fill_random(linalg_const_cast(l), cfill,
		typename linalg_traits<L>::linalg_type());
  }

  template <typename L> inline
  void fill_random(L& l, double cfill, abstract_vector) {
    gmm::clear(l);
    typedef typename linalg_traits<L>::value_type T;
    size_type ntot = std::min(vect_size(l), size_type(vect_size(l)*cfill) + 1);
    for (size_type nb = 0; nb < ntot;) {
      size_type i = gmm::irandom(vect_size(l));
      if (l[i] == T(0)) { 
	l[i] = gmm::random(typename linalg_traits<L>::value_type());
	++nb;
      }
    }
  }

  template <typename L> inline
  void fill_random(L& l, double cfill, abstract_matrix) {
    fill_random(l, cfill, typename principal_orientation_type<typename
		linalg_traits<L>::sub_orientation>::potype());
  }

  template <typename L> inline
  void fill_random(L& l, double cfill, row_major) {
    for (size_type i=0; i < mat_nrows(l); ++i) fill_random(mat_row(l,i),cfill);
  }

  template <typename L> inline
  void fill_random(L& l, double cfill, col_major) {
    for (size_type j=0; j < mat_ncols(l); ++j) fill_random(mat_col(l,j),cfill);
  }


  /* ******************************************************************** */
  /*		Scalar product                             		  */
  /* ******************************************************************** */

  template <typename V1, typename V2> inline
    typename linalg_traits<V1>::value_type
    vect_sp(const V1 &v1, const V2 &v2) {
    if (vect_size(v1) != vect_size(v2))
      DAL_THROW(dimension_error,"dimensions mismatch "
		<< vect_size(v1) << " and " << vect_size(v2));
    return vect_sp(v1, v2,
		   typename linalg_traits<V1>::storage_type(), 
		   typename linalg_traits<V2>::storage_type());
  }

  template <typename MATSP, typename V1, typename V2> inline
    typename linalg_traits<V1>::value_type
    vect_sp(const MATSP &ps, const V1 &v1, const V2 &v2) {
    return vect_sp_with_mat(ps, v1, v2,
			    typename linalg_traits<MATSP>::sub_orientation());
  }

  template <typename MATSP, typename V1, typename V2> inline
    typename linalg_traits<V1>::value_type
    vect_sp_with_mat(const MATSP &ps, const V1 &v1, const V2 &v2, row_major) {
    return vect_sp_with_matr(ps, v1, v2, 
			     typename linalg_traits<V2>::storage_type());
  }

  template <typename MATSP, typename V1, typename V2> inline 
    typename linalg_traits<V1>::value_type
    vect_sp_with_matr(const MATSP &ps, const V1 &v1, const V2 &v2,
		      abstract_sparse) {
    if (vect_size(v1) != mat_ncols() || vect_size(v2) != mat_nrows())
      DAL_THROW(dimension_error,"dimensions mismatch");
    size_type nr = mat_nrows(ps);
    typename linalg_traits<V2>::const_iterator
      it = vect_const_begin(v2), ite = vect_const_end(v2);
    typename linalg_traits<V1>::value_type res(0);
    for (; it != ite; ++it)
      res += vect_sp(mat_const_row(ps, it.index()), v1)* (*it);
    return res;
  }

  template <typename MATSP, typename V1, typename V2> inline
    typename linalg_traits<V1>::value_type
    vect_sp_with_matr(const MATSP &ps, const V1 &v1, const V2 &v2,
		      abstract_skyline)
  { return vect_sp_with_matr(ps, v1, v2, abstract_sparse()); }

  template <typename MATSP, typename V1, typename V2> inline
    typename linalg_traits<V1>::value_type
    vect_sp_with_matr(const MATSP &ps, const V1 &v1, const V2 &v2,
		      abstract_dense) {
    if (vect_size(v1) != mat_ncols(ps) || vect_size(v2) != mat_nrows(ps))
      DAL_THROW(dimension_error,"dimensions mismatch");
    typename linalg_traits<V2>::const_iterator
      it = vect_const_begin(v2), ite = vect_const_end(v2);
    typename linalg_traits<V1>::value_type res(0);
    for (size_type i = 0; it != ite; ++i, ++it)
      res += vect_sp(mat_const_row(ps, i), v1) * (*it);
    return res;
  }

  template <typename MATSP, typename V1, typename V2> inline
  typename linalg_traits<V1>::value_type
  vect_sp_with_mat(const MATSP &ps, const V1 &v1,const V2 &v2,row_and_col)
  { return vect_sp_with_mat(ps, v1, v2, row_major()); }

  template <typename MATSP, typename V1, typename V2> inline
  typename linalg_traits<V1>::value_type
  vect_sp_with_mat(const MATSP &ps, const V1 &v1, const V2 &v2,col_major){
    return vect_sp_with_matc(ps, v1, v2,
			     typename linalg_traits<V1>::storage_type());
  }

  template <typename MATSP, typename V1, typename V2> inline
    typename linalg_traits<V1>::value_type
    vect_sp_with_matc(const MATSP &ps, const V1 &v1, const V2 &v2,
		      abstract_sparse) {
    if (vect_size(v1) != mat_ncols() || vect_size(v2) != mat_nrows())
      DAL_THROW(dimension_error,"dimensions mismatch");
    typename linalg_traits<V1>::const_iterator
      it = vect_const_begin(v1), ite = vect_const_end(v1);
    typename linalg_traits<V1>::value_type res(0);
    for (; it != ite; ++it)
      res += vect_sp(mat_const_col(ps, it.index()), v2) * (*it);
    return res;
  }

  template <typename MATSP, typename V1, typename V2> inline
    typename linalg_traits<V1>::value_type
    vect_sp_with_matc(const MATSP &ps, const V1 &v1, const V2 &v2,
		      abstract_skyline)
  { return vect_sp_with_matc(ps, v1, v2, abstract_sparse()); }

  template <typename MATSP, typename V1, typename V2> inline
    typename linalg_traits<V1>::value_type
    vect_sp_with_matc(const MATSP &ps, const V1 &v1, const V2 &v2,
		      abstract_dense) {
    if (vect_size(v1) != mat_ncols() || vect_size(v2) != mat_nrows())
      DAL_THROW(dimension_error,"dimensions mismatch");
    typename linalg_traits<V1>::const_iterator
      it = vect_const_begin(v1), ite = vect_const_end(v1);
    typename linalg_traits<V1>::value_type res(0);
    for (size_type i = 0; it != ite; ++i, ++it)
      res += vect_sp(mat_const_col(ps, i), v2) * (*it);
    return res;
  }

  template <typename MATSP, typename V1, typename V2> inline
  typename linalg_traits<V1>::value_type
  vect_sp_with_mat(const MATSP &ps, const V1 &v1,const V2 &v2,col_and_row)
  { return vect_sp_with_mat(ps, v1, v2, col_major()); }

  template <typename MATSP, typename V1, typename V2> inline
  typename linalg_traits<V1>::value_type
  vect_sp_with_mat(const MATSP &ps, const V1 &v1, const V2 &v2,
		   abstract_null_type) {
    typename temporary_vector<V1>::vector_type w(mat_nrows(ps));
    DAL_WARNING(2, "Warning, a temporary is used in scalar product\n");
    mult(ps, v1, w); 
    return vect_sp(w, v2);
  }

  template <typename IT1, typename IT2> inline
    typename std::iterator_traits<IT1>::value_type
    _vect_sp_dense(IT1 it, IT1 ite, IT2 it2) {
    typename std::iterator_traits<IT1>::value_type res(0);
    for (; it != ite; ++it, ++it2) res += (*it) * (*it2);
    return res;
  }
  
  template <typename IT1, typename V> inline
    typename std::iterator_traits<IT1>::value_type
    _vect_sp_sparse(IT1 it, IT1 ite, const V &v) {
    typedef typename std::iterator_traits<IT1>::value_type T;
    T res(0);
    for (; it != ite; ++it) res += (*it) * (T(v[it.index()]));
    return res;
  }

  template <typename V1, typename V2> inline
    typename linalg_traits<V1>::value_type
    vect_sp(const V1 &v1, const V2 &v2, abstract_dense, abstract_dense) {
    return _vect_sp_dense(vect_const_begin(v1), vect_const_end(v1),
			  vect_const_begin(v2));
  }

  template <typename V1, typename V2> inline
    typename linalg_traits<V1>::value_type
    vect_sp(const V1 &v1, const V2 &v2, abstract_skyline, abstract_dense) {
    typename linalg_traits<V1>::const_iterator it1 = vect_const_begin(v1),
      ite =  vect_const_end(v1);
    typename linalg_traits<V2>::const_iterator it2 = vect_const_begin(v2);
    return _vect_sp_dense(it1, ite, it2 + it1.index());
  }

  template <typename V1, typename V2> inline
    typename linalg_traits<V1>::value_type
    vect_sp(const V1 &v1, const V2 &v2, abstract_dense, abstract_skyline) {
    typename linalg_traits<V2>::const_iterator it1 = vect_const_begin(v2),
      ite =  vect_const_end(v2);
    typename linalg_traits<V1>::const_iterator it2 = vect_const_begin(v1);
    return _vect_sp_dense(it1, ite, it2 + it1.index());
  }

  template <typename V1, typename V2> inline
    typename linalg_traits<V1>::value_type
    vect_sp(const V1 &v1, const V2 &v2, abstract_skyline, abstract_skyline) {
    typedef typename linalg_traits<V1>::value_type T;
    typename linalg_traits<V1>::const_iterator it1 = vect_const_begin(v1),
      ite1 =  vect_const_end(v1);
    typename linalg_traits<V2>::const_iterator it2 = vect_const_begin(v2),
      ite2 =  vect_const_end(v2);
    size_type n = std::min(ite1.index(), ite2.index());
    size_type l = std::max(it1.index(), it2.index());

    if (l < n) {
      size_type m = l - it1.index(), p = l - it2.index(), q = m + n - l;
      return _vect_sp_dense(it1+m, it1+q, it2 + p);
    }
    return T(0);
  }

  template <typename V1, typename V2> inline
    typename linalg_traits<V1>::value_type
  vect_sp(const V1 &v1, const V2 &v2,abstract_sparse,abstract_dense) {
    return _vect_sp_sparse(vect_const_begin(v1), vect_const_end(v1), v2);
  }

  template <typename V1, typename V2> inline
    typename linalg_traits<V1>::value_type
    vect_sp(const V1 &v1, const V2 &v2, abstract_sparse, abstract_skyline) {
    return _vect_sp_sparse(vect_const_begin(v1), vect_const_end(v1), v2);
  }

  template <typename V1, typename V2> inline
    typename linalg_traits<V1>::value_type
    vect_sp(const V1 &v1, const V2 &v2, abstract_skyline, abstract_sparse) {
    return _vect_sp_sparse(vect_const_begin(v2), vect_const_end(v2), v1);
  }

  template <typename V1, typename V2> inline
    typename linalg_traits<V1>::value_type
    vect_sp(const V1 &v1, const V2 &v2, abstract_dense,abstract_sparse) {
    return _vect_sp_sparse(vect_const_begin(v2), vect_const_end(v2), v1);
  }

  template <typename V1, typename V2> inline
    typename linalg_traits<V1>::value_type
    vect_sp(const V1 &v1, const V2 &v2,abstract_sparse,abstract_sparse) {
    typename linalg_traits<V1>::const_iterator it1 = vect_const_begin(v1),
      ite1 = vect_const_end(v1);
    typename linalg_traits<V2>::const_iterator it2 = vect_const_begin(v2),
      ite2 = vect_const_end(v2);
    typedef typename linalg_traits<V1>::value_type T;
    T res(0);
    while (it1 != ite1 && it2 != ite2) {
      if (it1.index() == it2.index())
	{ res += (*it1) * T(*it2); ++it1; ++it2; }
      else if (it1.index() < it2.index()) ++it1; else ++it2;
    }
    return res;
  }

  /* ******************************************************************** */
  /*		Hermitian product                             		  */
  /* ******************************************************************** */

  template <typename V1, typename V2> inline typename linalg_traits<V1>::value_type
  vect_hp(const V1 &v1, const V2 &v2)
  { return vect_sp(conjugated(v1), v2); }

  /* ******************************************************************** */
  /*		Trace of a matrix                             		  */
  /* ******************************************************************** */
  
   template <typename M>
   typename linalg_traits<M>::value_type
   mat_trace(const M &m) {
     typename linalg_traits<M>::value_type res(0);
     for (size_type i = 0; i < std::max(mat_nrows(m), mat_ncols(m)); ++i)
       res += m(i,i);
     return res;
  }

  /* ******************************************************************** */
  /*		Euclidian norm                             		  */
  /* ******************************************************************** */

  template <typename V>
  typename number_traits<typename linalg_traits<V>::value_type>
  ::magnitude_type
  vect_norm2_sqr(const V &v) {
    typename linalg_traits<V>::const_iterator
      it = vect_const_begin(v), ite = vect_const_end(v);
    typename number_traits<typename linalg_traits<V>::value_type>
      ::magnitude_type res(0);
    for (; it != ite; ++it) res += gmm::sqr(gmm::abs(*it));
    return res;
  }

  template <typename V> inline
   typename number_traits<typename linalg_traits<V>::value_type>
   ::magnitude_type
   vect_norm2(const V &v)
  { return sqrt(vect_norm2_sqr(v)); }

  template <typename M>
   typename number_traits<typename linalg_traits<M>::value_type>
   ::magnitude_type
   mat_norm2(const M &m, row_major) {
    typename number_traits<typename linalg_traits<M>::value_type>
      ::magnitude_type res(0);
    for (size_type i = 0; i < mat_nrows(m); ++i) {
      typedef typename linalg_traits<M>::const_sub_row_type row_type;
      row_type row = mat_const_row(m, i);
      typename linalg_traits<row_type>::const_iterator
	it = vect_const_begin(row), ite = vect_const_end(row);
      for (; it != ite; ++it) res += gmm::sqr(gmm::abs(*it));
    }
    return sqrt(res);
  }

  template <typename M>
   typename number_traits<typename linalg_traits<M>::value_type>
   ::magnitude_type
   mat_norm2(const M &m, col_major) {
    typename number_traits<typename linalg_traits<M>::value_type>
      ::magnitude_type res(0);
    for (size_type i = 0; i < mat_ncols(m); ++i) {
      typedef typename linalg_traits<M>::const_sub_col_type col_type;
      col_type col = mat_const_col(m, i);
      typename linalg_traits<col_type>::const_iterator
	it = vect_const_begin(col), ite = vect_const_end(col);
      for (; it != ite; ++it) res += gmm::sqr(gmm::abs(*it));
    }
    return sqrt(res);
  }

  template <typename M>
   typename number_traits<typename linalg_traits<M>::value_type>
   ::magnitude_type
   mat_norm2(const M &m) {
    return mat_norm2(m,
		     typename principal_orientation_type<typename
		     linalg_traits<M>::sub_orientation>::potype());
  }

  /* ******************************************************************** */
  /*		Inifity norm                              		  */
  /* ******************************************************************** */

  template <typename V>
  typename number_traits<typename linalg_traits<V>::value_type>
  ::magnitude_type 
  vect_norminf(const V &v) {
    typename linalg_traits<V>::const_iterator
      it = vect_const_begin(v), ite = vect_const_end(v);
      typename number_traits<typename linalg_traits<V>::value_type>
	::magnitude_type res(0);
    for (; it != ite; ++it) res = std::max(res, gmm::abs(*it));
    return res;
  }
  
  /* ******************************************************************** */
  /*		norm1                                    		  */
  /* ******************************************************************** */

  template <typename V>
  typename number_traits<typename linalg_traits<V>::value_type>
  ::magnitude_type
  vect_norm1(const V &v) {
    typename linalg_traits<V>::const_iterator
      it = vect_const_begin(v), ite = vect_const_end(v);
    typename number_traits<typename linalg_traits<V>::value_type>
	::magnitude_type res(0);
    for (; it != ite; ++it) res += gmm::abs(*it);
    return res;
  }

  /* ******************************************************************** */
  /*		Clean                                    		  */
  /* ******************************************************************** */

  template <typename L> inline void clean(L &l, double seuil)
  { clean(l, seuil, typename linalg_traits<L>::linalg_type()); }

  template <typename L> inline void clean(const L &l, double seuil)
  { clean(linalg_const_cast(l), seuil);}

  template <typename L> inline void clean(L &l, double seuil, abstract_vector) {
    clean(l, seuil, typename linalg_traits<L>::storage_type(),
	  typename linalg_traits<L>::value_type());
  }

  template <typename L, typename T>
  void clean(L &l, double seuil, abstract_dense, T) {
    typename linalg_traits<L>::iterator it = vect_begin(l), ite = vect_end(l);
    for (; it != ite; ++it)
      if (gmm::abs(*it) < seuil) *it = T(0);
  }

  template <typename L, typename T>
  void clean(L &l, double seuil, abstract_skyline, T)
  { clean(l, seuil, abstract_dense(), T()); }

  template <typename L, typename T>
  void clean(L &l, double seuil, abstract_sparse, T) {
    typename linalg_traits<L>::iterator it = vect_begin(l), ite = vect_end(l);
    for (; it != ite; ++it) // to be optimized ...
      if (gmm::abs(*it) < seuil) {
	l[it.index()] = T(0);
	it = vect_begin(l); ite = vect_end(l);
      }
  }

  template <typename L, typename T>
  void clean(L &l, double seuil, abstract_dense, std::complex<T>) {
    typename linalg_traits<L>::iterator it = vect_begin(l), ite = vect_end(l);
    for (; it != ite; ++it){
      if (gmm::abs((*it).real()) < seuil)
	*it = std::complex<T>(T(0), (*it).imag());
      if (gmm::abs((*it).imag()) < seuil)
	*it = std::complex<T>((*it).real(), T(0));
    }
  }

  template <typename L, typename T>
  void clean(L &l, double seuil, abstract_skyline, std::complex<T>)
  { clean(l, seuil, abstract_dense(), std::complex<T>()); }

  template <typename L, typename T>
  void clean(L &l, double seuil, abstract_sparse, std::complex<T>) {
    typename linalg_traits<L>::iterator it = vect_begin(l), ite = vect_end(l);
    for (; it != ite; ++it) { // to be optimized ...
      if (gmm::abs((*it).real()) < seuil) {
	l[it.index()] = std::complex<T>(T(0), (*it).imag());
	it = vect_begin(l); ite = vect_end(l); continue;
      }
      if (gmm::abs((*it).imag()) < seuil) {
	l[it.index()] = std::complex<T>((*it).real(), T(0));
	it = vect_begin(l); ite = vect_end(l); continue;
      }
    }
  }

  template <typename L> inline void clean(L &l, double seuil, abstract_matrix) {
    clean(l, seuil, typename principal_orientation_type<typename
	  linalg_traits<L>::sub_orientation>::potype());
  }
  
  template <typename L> void clean(L &l, double seuil, row_major) {
    for (size_type i = 0; i < mat_nrows(l); ++i)
      clean(mat_row(l, i), seuil);
  }

  template <typename L> void clean(L &l, double seuil, col_major) {
    for (size_type i = 0; i < mat_ncols(l); ++i)
      clean(mat_col(l, i), seuil);
  }

  /* ******************************************************************** */
  /*		Copy                                    		  */
  /* ******************************************************************** */

  template <typename L1, typename L2> inline
  void copy(const L1& l1, L2& l2) { 
    if ((const void *)(&l1) != (const void *)(&l2)) {
      if (same_origin(l1,l2))
	DAL_WARNING(2, "Warning : a conflict is possible in copy\n");
     
      copy(l1, l2, typename linalg_traits<L1>::linalg_type(),
	   typename linalg_traits<L2>::linalg_type());
    }
  }

  template <typename L1, typename L2> inline
  void copy(const L1& l1, const L2& l2) { copy(l1, linalg_const_cast(l2)); }

  template <typename L1, typename L2> inline
  void copy(const L1& l1, L2& l2, abstract_vector, abstract_vector) {
    if (vect_size(l1) != vect_size(l2))
      DAL_THROW(dimension_error, "dimensions mismatch");

    copy_vect(l1, l2, typename linalg_traits<L1>::storage_type(),
	      typename linalg_traits<L2>::storage_type());
  }

  template <typename L1, typename L2> inline
  void copy(const L1& l1, L2& l2, abstract_matrix, abstract_matrix) {
    size_type m = mat_nrows(l1), n = mat_ncols(l1);
    if (!m || !n) return;
    if (n != mat_ncols(l2) || m != mat_nrows(l2))
	DAL_THROW(dimension_error, "dimensions mismatch");
    copy_mat(l1, l2, typename linalg_traits<L1>::sub_orientation(),
	     typename linalg_traits<L2>::sub_orientation());
  }

  template <typename V1, typename V2, typename C1, typename C2> inline 
  void copy_vect(const V1 &v1, const V2 &v2, C1, C2)
  { copy_vect(v1, const_cast<V2 &>(v2), C1(), C2()); }
  

  template <typename L1, typename L2>
  void copy_mat_by_row(const L1& l1, L2& l2) {
    size_type nbr = mat_nrows(l1);
    for (size_type i = 0; i < nbr; ++i)
      copy_vect(mat_const_row(l1, i), mat_row(l2, i),
      		typename linalg_traits<L1>::storage_type(),
		typename linalg_traits<L2>::storage_type());
  }

  template <typename L1, typename L2>
  void copy_mat_by_col(const L1 &l1, L2 &l2) {
    size_type nbc = mat_ncols(l1);
    for (size_type i = 0; i < nbc; ++i) {
      copy_vect(mat_const_col(l1, i), mat_col(l2, i),
      		typename linalg_traits<L1>::storage_type(),
		typename linalg_traits<L2>::storage_type());
    }
  }

  template <typename L1, typename L2> inline
  void copy_mat(const L1& l1, L2& l2, row_major, row_major)
  { copy_mat_by_row(l1, l2); }

  template <typename L1, typename L2> inline
  void copy_mat(const L1& l1, L2& l2, row_major, row_and_col)
  { copy_mat_by_row(l1, l2); }

  template <typename L1, typename L2> inline
  void copy_mat(const L1& l1, L2& l2, row_and_col, row_and_col)
  { copy_mat_by_row(l1, l2); }

  template <typename L1, typename L2> inline
  void copy_mat(const L1& l1, L2& l2, row_and_col, row_major)
  { copy_mat_by_row(l1, l2); }

  template <typename L1, typename L2> inline
  void copy_mat(const L1& l1, L2& l2, col_and_row, row_major)
  { copy_mat_by_row(l1, l2); }

  template <typename L1, typename L2> inline
  void copy_mat(const L1& l1, L2& l2, row_major, col_and_row)
  { copy_mat_by_row(l1, l2); }

  template <typename L1, typename L2> inline
  void copy_mat(const L1& l1, L2& l2, col_and_row, row_and_col)
  { copy_mat_by_row(l1, l2); }

  template <typename L1, typename L2> inline
  void copy_mat(const L1& l1, L2& l2, row_and_col, col_and_row)
  { copy_mat_by_row(l1, l2); }

  template <typename L1, typename L2> inline
  void copy_mat(const L1& l1, L2& l2, col_major, col_major)
  { copy_mat_by_col(l1, l2); }

  template <typename L1, typename L2> inline
  void copy_mat(const L1& l1, L2& l2, col_major, col_and_row)
  { copy_mat_by_col(l1, l2); }

  template <typename L1, typename L2> inline
  void copy_mat(const L1& l1, L2& l2, col_major, row_and_col)
  { copy_mat_by_col(l1, l2); }

  template <typename L1, typename L2> inline
  void copy_mat(const L1& l1, L2& l2, row_and_col, col_major)
  { copy_mat_by_col(l1, l2); }

  template <typename L1, typename L2> inline
  void copy_mat(const L1& l1, L2& l2, col_and_row, col_major)
  { copy_mat_by_col(l1, l2); }

  template <typename L1, typename L2> inline
  void copy_mat(const L1& l1, L2& l2, col_and_row, col_and_row)
  { copy_mat_by_col(l1, l2); }
  
  template <typename L1, typename L2> inline
  void copy_mat_mixed_rc(const L1& l1, L2& l2, size_type i) {
    copy_mat_mixed_rc(l1, l2, i, typename linalg_traits<L1>::storage_type());
  }

  template <typename L1, typename L2>
  void copy_mat_mixed_rc(const L1& l1, L2& l2, size_type i, abstract_sparse) {
    typename linalg_traits<L1>::const_iterator
      it  = vect_const_begin(l1), ite = vect_const_end(l1);
    for (; it != ite; ++it)
      l2(i, it.index()) = *it;
  }

  template <typename L1, typename L2>
  void copy_mat_mixed_rc(const L1& l1, L2& l2, size_type i, abstract_skyline) {
    typename linalg_traits<L1>::const_iterator
      it  = vect_const_begin(l1), ite = vect_const_end(l1);
    for (; it != ite; ++it)
      l2(i, it.index()) = *it;
  }

  template <typename L1, typename L2>
  void copy_mat_mixed_rc(const L1& l1, L2& l2, size_type i, abstract_dense) {
    typename linalg_traits<L1>::const_iterator
      it  = vect_const_begin(l1), ite = vect_const_end(l1);
    for (size_type j = 0; it != ite; ++it, ++j) l2(i, j) = *it;
  }

  template <typename L1, typename L2> inline
  void copy_mat_mixed_cr(const L1& l1, L2& l2, size_type i) {
    copy_mat_mixed_cr(l1, l2, i, typename linalg_traits<L1>::storage_type());
  }

  template <typename L1, typename L2>
  void copy_mat_mixed_cr(const L1& l1, L2& l2, size_type i, abstract_sparse) {
    typename linalg_traits<L1>::const_iterator
      it  = vect_const_begin(l1), ite = vect_const_end(l1);
    for (; it != ite; ++it) l2(it.index(), i) = *it;
  }

  template <typename L1, typename L2>
  void copy_mat_mixed_cr(const L1& l1, L2& l2, size_type i, abstract_skyline) {
    typename linalg_traits<L1>::const_iterator
      it  = vect_const_begin(l1), ite = vect_const_end(l1);
    for (; it != ite; ++it) l2(it.index(), i) = *it;
  }

  template <typename L1, typename L2>
  void copy_mat_mixed_cr(const L1& l1, L2& l2, size_type i, abstract_dense) {
    typename linalg_traits<L1>::const_iterator
      it  = vect_const_begin(l1), ite = vect_const_end(l1);
    for (size_type j = 0; it != ite; ++it, ++j) l2(j, i) = *it;
  }

  template <typename L1, typename L2>
  void copy_mat(const L1& l1, L2& l2, row_major, col_major) {
    clear(l2);
    size_type nbr = mat_nrows(l1);
    for (size_type i = 0; i < nbr; ++i)
      copy_mat_mixed_rc(mat_const_row(l1, i), l2, i);
  }
  
  template <typename L1, typename L2>
  void copy_mat(const L1& l1, L2& l2, col_major, row_major) {
    clear(l2);
    size_type nbc = mat_ncols(l1);
    for (size_type i = 0; i < nbc; ++i)
      copy_mat_mixed_cr(mat_const_col(l1, i), l2, i);
  }
  
  template <typename L1, typename L2> inline
  void copy_vect(const L1 &l1, L2 &l2, abstract_dense, abstract_dense) {
    std::copy(vect_const_begin(l1), vect_const_end(l1), vect_begin(l2));
  }

  template <typename L1, typename L2> inline // to be optimised ?
  void copy_vect(const L1 &l1, L2 &l2, abstract_skyline, abstract_skyline) {
    typename linalg_traits<L1>::const_iterator it1 = vect_const_begin(l1),
      ite1 = vect_const_end(l1);
    while (it1 != ite1 && *it1 == typename linalg_traits<L1>::value_type(0))
      ++it1;

    if (ite1 - it1 > 0) {
      clear(l2);
      typename linalg_traits<L2>::iterator it2 = vect_begin(l2), 
	ite2 = vect_end(l2);
      while (*(ite1-1) == typename linalg_traits<L1>::value_type(0)) ite1--;

      if (it2 == ite2) {
	l2[it1.index()] = *it1; ++it1;
	l2[ite1.index()-1] = *(ite1-1); --ite1;
	if (it1 < ite1)
	  { it2 = vect_begin(l2); ++it2; std::copy(it1, ite1, it2); }
      }
      else {
	ptrdiff_t m = it1.index() - it2.index();
	if (m >= 0 && ite1.index() <= ite2.index())
	  std::copy(it1, ite1, it2 + m);
	else {
	  if (m < 0) l2[it1.index()] = *it1;
	  if (ite1.index() > ite2.index()) l2[ite1.index()-1] = *(ite1-1);
	  it2 = vect_begin(l2); ite2 = vect_end(l2);
	  m = it1.index() - it2.index();
	  std::copy(it1, ite1, it2 + m);
	}
      }
    }
  }
  
  template <typename L1, typename L2>
  void copy_vect(const L1& l1, L2& l2, abstract_sparse, abstract_dense) {
    clear(l2);
    typename linalg_traits<L1>::const_iterator
      it  = vect_const_begin(l1), ite = vect_const_end(l1);
    for (; it != ite; ++it) { l2[it.index()] = *it; }
  }

  template <typename L1, typename L2>
  void copy_vect(const L1& l1, L2& l2, abstract_sparse, abstract_skyline) {
    clear(l2);
    typename linalg_traits<L1>::const_iterator
      it  = vect_const_begin(l1), ite = vect_const_end(l1);
    for (; it != ite; ++it) l2[it.index()] = *it;
  }

  template <typename L1, typename L2>
  void copy_vect(const L1& l1, L2& l2, abstract_skyline, abstract_dense) {
    typedef typename linalg_traits<L1>::value_type T;
    typedef typename linalg_traits<L1>::const_iterator l1_const_iterator;
    typedef typename linalg_traits<L2>::iterator l2_iterator;

    l1_const_iterator it = vect_const_begin(l1), ite = vect_const_end(l1);
    if (it == ite)
      gmm::clear(l2);
    else {
      l2_iterator it2 = vect_begin(l2), ite2 = vect_end(l2);
      
      size_type i = it.index(), j;
      for (j = 0; j < i; ++j, ++it2) *it2 = T(0);
      for (; it != ite; ++it, ++it2) *it2 = *it;
      for (; it2 != ite2; ++it2) *it2 = T(0);
    }
  }
    
  template <typename L1, typename L2>
  void copy_vect(const L1& l1, L2& l2, abstract_sparse, abstract_sparse) {
    typename linalg_traits<L1>::const_iterator
      it  = vect_const_begin(l1), ite = vect_const_end(l1);
    clear(l2);
    for (; it != ite; ++it)
      if (*it != (typename linalg_traits<L1>::value_type)(0))
	l2[it.index()] = *it;
  }
  
  template <typename L1, typename L2>
  void copy_vect(const L1& l1, L2& l2, abstract_dense, abstract_sparse) {
    clear(l2);
    typename linalg_traits<L1>::const_iterator
      it  = vect_const_begin(l1), ite = vect_const_end(l1);
    for (size_type i = 0; it != ite; ++it, ++i)
      if (*it != (typename linalg_traits<L1>::value_type)(0))
	l2[i] = *it;
  }

  template <typename L1, typename L2> // to be optimised ...
  void copy_vect(const L1& l1, L2& l2, abstract_dense, abstract_skyline) {
    clear(l2);
    typename linalg_traits<L1>::const_iterator
      it  = vect_const_begin(l1), ite = vect_const_end(l1);
    for (size_type i = 0; it != ite; ++it, ++i)
      if (*it != (typename linalg_traits<L1>::value_type)(0))
	l2[i] = *it;
  }

  
  template <typename L1, typename L2>
  void copy_vect(const L1& l1, L2& l2, abstract_skyline, abstract_sparse) {
    clear(l2);
    typename linalg_traits<L1>::const_iterator
      it  = vect_const_begin(l1), ite = vect_const_end(l1);
    for (; it != ite; ++it)
      if (*it != (typename linalg_traits<L1>::value_type)(0))
	l2[it.index()] = *it;
  }

  /* ******************************************************************** */
  /*		Matrix and vector addition                             	  */
  /*   algorithms are built in order to avoid some conflicts whith        */
  /*   repeated arguments or with overlapping part of a same object.      */
  /*   In the latter case, conflicts are still possible.                  */
  /* ******************************************************************** */
  
  template <typename L1, typename L2> inline
    void add(const L1& l1, L2& l2) {
      add_spec(l1, l2, typename linalg_traits<L2>::linalg_type());
  }

  template <typename L1, typename L2> inline
  void add(const L1& l1, const L2& l2) { add(l1, linalg_const_cast(l2)); }

  template <typename L1, typename L2> inline
    void add_spec(const L1& l1, L2& l2, abstract_vector) {
    if (vect_size(l1) != vect_size(l2))
      DAL_THROW(dimension_error, "dimensions mismatch");
    add(l1, l2, typename linalg_traits<L1>::storage_type(),
	typename linalg_traits<L2>::storage_type());
  }

  template <typename L1, typename L2> inline
    void add_spec(const L1& l1, L2& l2, abstract_matrix) {
    if (mat_nrows(l1) != mat_nrows(l2) || mat_ncols(l1) != mat_ncols(l2))
      DAL_THROW(dimension_error, "dimensions mismatch");
    add(l1, l2, typename linalg_traits<L1>::sub_orientation(),
	typename linalg_traits<L2>::sub_orientation());
  }

  template <typename L1, typename L2>
  void add(const L1& l1, L2& l2, row_major, row_major) {
    typename linalg_traits<L1>::const_row_iterator it1 = mat_row_begin(l1),
      ite = mat_row_end(l1);
    typename linalg_traits<L2>::row_iterator it2 = mat_row_begin(l2);
    for ( ; it1 != ite; ++it1, ++it2)
      add(linalg_traits<L1>::row(it1), linalg_traits<L2>::row(it2));
  }

  template <typename L1, typename L2>
  void add(const L1& l1, L2& l2, col_major, col_major) {
    typename linalg_traits<L1>::const_col_iterator
      it1 = mat_col_const_begin(l1),
      ite = mat_col_const_end(l1);
    typename linalg_traits<L2>::col_iterator it2 = mat_col_begin(l2);
    for ( ; it1 != ite; ++it1, ++it2)
      add(linalg_traits<L1>::col(it1),  linalg_traits<L2>::col(it2));
  }
  
    template <typename L1, typename L2> inline
  void add_mat_mixed_rc(const L1& l1, L2& l2, size_type i) {
    add_mat_mixed_rc(l1, l2, i, typename linalg_traits<L1>::storage_type());
  }

  template <typename L1, typename L2>
  void add_mat_mixed_rc(const L1& l1, L2& l2, size_type i, abstract_sparse) {
    typename linalg_traits<L1>::const_iterator
      it  = vect_const_begin(l1), ite = vect_const_end(l1);
    for (; it != ite; ++it) l2(i, it.index()) += *it;
  }

  template <typename L1, typename L2>
  void add_mat_mixed_rc(const L1& l1, L2& l2, size_type i, abstract_skyline) {
    typename linalg_traits<L1>::const_iterator
      it  = vect_const_begin(l1), ite = vect_const_end(l1);
    for (; it != ite; ++it) l2(i, it.index()) += *it;
  }

  template <typename L1, typename L2>
  void add_mat_mixed_rc(const L1& l1, L2& l2, size_type i, abstract_dense) {
    typename linalg_traits<L1>::const_iterator
      it  = vect_const_begin(l1), ite = vect_const_end(l1);
    for (size_type j = 0; it != ite; ++it, ++j) l2(i, j) += *it;
  }

  template <typename L1, typename L2> inline
  void add_mat_mixed_cr(const L1& l1, L2& l2, size_type i) {
    add_mat_mixed_cr(l1, l2, i, typename linalg_traits<L1>::storage_type());
  }

  template <typename L1, typename L2>
  void add_mat_mixed_cr(const L1& l1, L2& l2, size_type i, abstract_sparse) {
    typename linalg_traits<L1>::const_iterator
      it  = vect_const_begin(l1), ite = vect_const_end(l1);
    for (; it != ite; ++it) l2(it.index(), i) += *it;
  }

  template <typename L1, typename L2>
  void add_mat_mixed_cr(const L1& l1, L2& l2, size_type i, abstract_skyline) {
    typename linalg_traits<L1>::const_iterator
      it  = vect_const_begin(l1), ite = vect_const_end(l1);
    for (; it != ite; ++it) l2(it.index(), i) += *it;
  }

  template <typename L1, typename L2>
  void add_mat_mixed_cr(const L1& l1, L2& l2, size_type i, abstract_dense) {
    typename linalg_traits<L1>::const_iterator
      it  = vect_const_begin(l1), ite = vect_const_end(l1);
    for (size_type j = 0; it != ite; ++it, ++j) l2(j, i) += *it;
  }

  template <typename L1, typename L2>
  void add(const L1& l1, L2& l2, row_major, col_major) {
    size_type nbr = mat_nrows(l1);
    for (size_type i = 0; i < nbr; ++i)
      add_mat_mixed_rc(mat_const_row(l1, i), l2, i);
  }
  
  template <typename L1, typename L2>
  void add(const L1& l1, L2& l2, col_major, row_major) {
    size_type nbc = mat_ncols(l1);
    for (size_type i = 0; i < nbc; ++i)
      add_mat_mixed_cr(mat_const_col(l1, i), l2, i);
  }

  template <typename L1, typename L2> inline
  void add(const L1& l1, L2& l2, row_and_col, row_major)
  { add(l1, l2, row_major(), row_major()); }

  template <typename L1, typename L2> inline
  void add(const L1& l1, L2& l2, row_and_col, row_and_col)
  { add(l1, l2, row_major(), row_major()); }

  template <typename L1, typename L2> inline
  void add(const L1& l1, L2& l2, row_and_col, col_and_row)
  { add(l1, l2, row_major(), row_major()); }

  template <typename L1, typename L2> inline
  void add(const L1& l1, L2& l2, col_and_row, row_and_col)
  { add(l1, l2, row_major(), row_major()); }

  template <typename L1, typename L2> inline
  void add(const L1& l1, L2& l2, row_major, row_and_col)
  { add(l1, l2, row_major(), row_major()); }

  template <typename L1, typename L2> inline
  void add(const L1& l1, L2& l2, col_and_row, row_major)
  { add(l1, l2, row_major(), row_major()); }

  template <typename L1, typename L2> inline
  void add(const L1& l1, L2& l2, row_major, col_and_row)
  { add(l1, l2, row_major(), row_major()); }

  template <typename L1, typename L2> inline
  void add(const L1& l1, L2& l2, row_and_col, col_major)
  { add(l1, l2, col_major(), col_major()); }

  template <typename L1, typename L2> inline
  void add(const L1& l1, L2& l2, col_major, row_and_col)
  { add(l1, l2, col_major(), col_major()); }

  template <typename L1, typename L2> inline
  void add(const L1& l1, L2& l2, col_and_row, col_major)
  { add(l1, l2, col_major(), col_major()); }

  template <typename L1, typename L2> inline
  void add(const L1& l1, L2& l2, col_and_row, col_and_row)
  { add(l1, l2, col_major(), col_major()); }

  template <typename L1, typename L2> inline
  void add(const L1& l1, L2& l2, col_major, col_and_row)
  { add(l1, l2, col_major(), col_major()); }

  template <typename L1, typename L2, typename L3> inline
    void add(const L1& l1, const L2& l2, L3& l3) {
    if (vect_size(l1) != vect_size(l2) || vect_size(l1) != vect_size(l3))
      DAL_THROW(dimension_error,"dimensions mismatch"); 
    if ((const void *)(&l1) == (const void *)(&l3))
      add(l2, l3);
    else if ((const void *)(&l2) == (const void *)(&l3))
      add(l1, l3);
    else
      add(l1, l2, l3, typename linalg_traits<L1>::storage_type(),
	  typename linalg_traits<L2>::storage_type(),
	  typename linalg_traits<L3>::storage_type());
  }

  template <typename L1, typename L2, typename L3> inline
  void add(const L1& l1, const L2& l2, const L3& l3)
  { add(l1, l2, linalg_const_cast(l3)); }

  template <typename IT1, typename IT2, typename IT3>
    void _add_full(IT1 it1, IT2 it2, IT3 it3, IT3 ite) {
    for (; it3 != ite; ++it3, ++it2, ++it1) *it3 = *it1 + *it2;
  }

  template <typename IT1, typename IT2, typename IT3>
    void _add_almost_full(IT1 it1, IT1 ite1, IT2 it2, IT3 it3, IT3 ite3) {
    IT3 it = it3;
    for (; it != ite3; ++it, ++it2) *it = *it2;
    for (; it1 != ite1; ++it1)
      *(it3 + it1.index()) += *it1;
  }

  template <typename IT1, typename IT2, typename IT3>
  void _add_to_full(IT1 it1, IT1 ite1, IT2 it2, IT2 ite2,
		    IT3 it3, IT3 ite3) {
    typedef typename std::iterator_traits<IT3>::value_type T;
    IT3 it = it3;
    for (; it != ite3; ++it) *it = T(0);
    for (; it1 != ite1; ++it1) *(it3 + it1.index()) = *it1;
    for (; it2 != ite2; ++it2) *(it3 + it2.index()) += *it2;    
  }
  
  template <typename L1, typename L2, typename L3> inline
  void add(const L1& l1, const L2& l2, L3& l3,
	   abstract_dense, abstract_dense, abstract_dense) {
    _add_full(vect_const_begin(l1), vect_const_begin(l2),
	      vect_begin(l3), vect_end(l3));
  }
  
  // generic function for add(v1, v2, v3).
  // Need to be specialized to optimize particular additions.
  template <typename L1, typename L2, typename L3, typename ST1, typename ST2, typename ST3>
  inline void add(const L1& l1, const L2& l2, L3& l3, ST1, ST2, ST3)
  { copy(l2, l3); add(l1, l3, ST1(), ST3()); }

  template <typename L1, typename L2, typename L3> inline
  void add(const L1& l1, const L2& l2, L3& l3,
	   abstract_sparse, abstract_dense, abstract_dense) {
    _add_almost_full(vect_const_begin(l1), vect_const_end(l1),
		     vect_const_begin(l2), vect_begin(l3), vect_end(l3));
  }
  
  template <typename L1, typename L2, typename L3> inline
  void add(const L1& l1, const L2& l2, L3& l3,
	   abstract_dense, abstract_sparse, abstract_dense)
  { add(l2, l1, l3, abstract_sparse(), abstract_dense(), abstract_dense()); }
  
  template <typename L1, typename L2, typename L3> inline
  void add(const L1& l1, const L2& l2, L3& l3,
	   abstract_sparse, abstract_sparse, abstract_dense) {
    _add_to_full(vect_const_begin(l1), vect_const_end(l1),
		 vect_const_begin(l2), vect_const_end(l2),
		 vect_begin(l3), vect_end(l3));
  }
  
  template <typename L1, typename L2, typename L3>
  void add(const L1& l1, const L2& l2, L3& l3,
	   abstract_sparse, abstract_sparse, abstract_sparse) {
    typename linalg_traits<L1>::const_iterator
      it1 = vect_const_begin(l1), ite1 = vect_const_end(l1);
    typename linalg_traits<L2>::const_iterator
      it2 = vect_const_begin(l2), ite2 = vect_const_end(l2);
    clear(l3);
    while (it1 != ite1 && it2 != ite2) {
      ptrdiff_t d = it1.index() - it2.index();
      if (d < 0)
	{ l3[it1.index()] += *it1; ++it1; }
      else if (d > 0)
	{ l3[it2.index()] += *it2; ++it2; }
      else
	{ l3[it1.index()] = *it1 + *it2; ++it1; ++it2; }
    }
    for (; it1 != ite1; ++it1) l3[it1.index()] += *it1;
    for (; it2 != ite2; ++it2) l3[it2.index()] += *it2;   
  }

  template <typename L1, typename L2>
  void add(const L1& l1, L2& l2,
	   abstract_dense, abstract_dense) {
    typename linalg_traits<L1>::const_iterator it1 = vect_const_begin(l1); 
    typename linalg_traits<L2>::iterator
             it2 = vect_begin(l2), ite = vect_end(l2);
    for (; it2 != ite; ++it2, ++it1) *it2 += *it1;
  }

  template <typename L1, typename L2>
  void add(const L1& l1, L2& l2, abstract_dense, abstract_skyline) {
    typedef typename linalg_traits<L1>::const_iterator const_l1_iterator;
    typedef typename linalg_traits<L2>::iterator l2_iterator;
    typedef typename linalg_traits<L1>::value_type T;

    const_l1_iterator it1 = vect_const_begin(l1), ite1 = vect_const_end(l1); 
    size_type i1 = 0, ie1 = vect_size(l1);
    while (it1 != ite1 && *it1 == T(0)) { ++it1; ++i1; }
    if (it1 != ite1) {
      l2_iterator it2 = vect_begin(l2), ite2 = vect_end(l2);
      while (ie1 && *(ite1-1) == T(0)) { ite1--; --ie1; }

      if (it2 == ite2 || i1 < it2.index()) {
	l2[i1] = *it1; ++i1; ++it1;
	if (it1 == ite1) return;
	it2 = vect_begin(l2); ite2 = vect_end(l2);
      }
      if (ie1 > ite2.index()) {
	--ite1; l2[ie1 - 1] = *ite1;
	it2 = vect_begin(l2);
      }
      it2 += i1 - it2.index();
      for (; it1 != ite1; ++it1, ++it2) { *it2 += *it1; }
    }
  }


  template <typename L1, typename L2>
  void add(const L1& l1, L2& l2,
	   abstract_skyline, abstract_dense) {
    typename linalg_traits<L1>::const_iterator it1 = vect_const_begin(l1),
      ite1 = vect_const_end(l1);
    if (it1 != ite1) {
      typename linalg_traits<L2>::iterator it2 = vect_begin(l2);
      it2 += it1.index();
      for (; it1 != ite1; ++it2, ++it1) *it2 += *it1;
    }
  }

  
  template <typename L1, typename L2>
  void add(const L1& l1, L2& l2,
	   abstract_sparse, abstract_dense) {
    typename linalg_traits<L1>::const_iterator
      it1 = vect_const_begin(l1), ite1 = vect_const_end(l1);
    for (; it1 != ite1; ++it1) l2[it1.index()] += *it1;
  }
  
  template <typename L1, typename L2>
  void add(const L1& l1, L2& l2,
	   abstract_sparse, abstract_sparse) {
    typename linalg_traits<L1>::const_iterator
      it1 = vect_const_begin(l1), ite1 = vect_const_end(l1);
    for (; it1 != ite1; ++it1) l2[it1.index()] += *it1;
  }

  template <typename L1, typename L2>
  void add(const L1& l1, L2& l2,
	   abstract_sparse, abstract_skyline) {
    typename linalg_traits<L1>::const_iterator
      it1 = vect_const_begin(l1), ite1 = vect_const_end(l1);
    for (; it1 != ite1; ++it1) l2[it1.index()] += *it1;
  }


  template <typename L1, typename L2>
  void add(const L1& l1, L2& l2,
	   abstract_skyline, abstract_sparse) {
    typename linalg_traits<L1>::const_iterator
      it1 = vect_const_begin(l1), ite1 = vect_const_end(l1);
    for (; it1 != ite1; ++it1)
      if (*it1 != typename linalg_traits<L1>::value_type(0))
	l2[it1.index()] += *it1;
  }

  template <typename L1, typename L2>
  void add(const L1& l1, L2& l2,
	   abstract_skyline, abstract_skyline) {
    typedef typename linalg_traits<L1>::const_iterator const_l1_iterator;
    typedef typename linalg_traits<L2>::iterator l2_iterator;
    typedef typename linalg_traits<L1>::value_type T;

    const_l1_iterator it1 = vect_const_begin(l1), ite1 = vect_const_end(l1);
    
    while (it1 != ite1 && *it1 == T(0)) ++it1;
    if (ite1 != it1) {
      l2_iterator it2 = vect_begin(l2), ite2 = vect_end(l2);
      while (*(ite1-1) == T(0)) ite1--;

      if (it2 == ite2 || it1.index() < it2.index()) {
	l2[it1.index()] = *it1; ++it1;
	if (it1 == ite1) return;
	it2 = vect_begin(l2); ite2 = vect_end(l2);
      }
      if (ite1.index() > ite2.index()) {
	l2[ite1.index() - 1] = *(ite1 - 1);
	it2 = vect_begin(l2);
      }
      ptrdiff_t m = it1.index() - it2.index();
      it2 += m;
      for (; it1 != ite1; ++it1, ++it2) *it2 += *it1;
    }
  }
  
  template <typename L1, typename L2>
  void add(const L1& l1, L2& l2, abstract_dense, abstract_sparse) {
    typename linalg_traits<L1>::const_iterator
      it1 = vect_const_begin(l1), ite1 = vect_const_end(l1);
    for (size_type i = 0; it1 != ite1; ++it1, ++i)
      if (*it1 != typename linalg_traits<L1>::value_type(0)) l2[i] += *it1;
  } 

  /* ******************************************************************** */
  /*		Matrix-vector mult                                    	  */
  /* ******************************************************************** */

  template <typename L1, typename L2, typename L3> inline
  void mult(const L1& l1, const L2& l2, L3& l3) {
    mult_dispatch(l1, l2, l3, typename linalg_traits<L2>::linalg_type());
  }

  template <typename L1, typename L2, typename L3> inline
  void mult(const L1& l1, const L2& l2, const L3& l3)
  { mult(l1, l2, linalg_const_cast(l3)); }

  template <typename L1, typename L2, typename L3> inline
  void mult_dispatch(const L1& l1, const L2& l2, L3& l3, abstract_vector) {
    size_type m = mat_nrows(l1), n = mat_ncols(l1);
    if (!m || !n) { gmm::clear(l3); return; }
    if (n != vect_size(l2) || m != vect_size(l3))
      DAL_THROW(dimension_error,"dimensions mismatch");
    if (!same_origin(l2, l3))
      mult_spec(l1, l2, l3, typename principal_orientation_type<typename
		linalg_traits<L1>::sub_orientation>::potype());
    else {
      DAL_WARNING(2, "Warning, A temporary is used for mult\n");
      typename temporary_vector<L3>::vector_type temp(vect_size(l3));
      mult_spec(l1, l2, temp, typename principal_orientation_type<typename
		linalg_traits<L1>::sub_orientation>::potype());
      copy(temp, l3);
    }
  }

  template <typename L1, typename L2, typename L3>
  void mult_by_row(const L1& l1, const L2& l2, L3& l3, abstract_sparse) {
    typedef typename  linalg_traits<L1>::value_type T;
    clear(l3);
    size_type nr = mat_nrows(l1);
    for (size_type i = 0; i < nr; ++i) {
      T aux = vect_sp(mat_const_row(l1, i), l2);
      if (aux != T(0)) l3[i] = aux;
    }
  }

  template <typename L1, typename L2, typename L3>
  void mult_by_row(const L1& l1, const L2& l2, L3& l3, abstract_skyline) {
    typedef typename  linalg_traits<L1>::value_type T;
    clear(l3);
    size_type nr = mat_nrows(l1);
    for (size_type i = 0; i < nr; ++i) {
      T aux = vect_sp(mat_const_row(l1, i), l2);
      if (aux != T(0)) l3[i] = aux;
    }
  }

  template <typename L1, typename L2, typename L3>
  void mult_by_row(const L1& l1, const L2& l2, L3& l3, abstract_dense) {
    typename linalg_traits<L3>::iterator it=vect_begin(l3), ite=vect_end(l3);
    typename linalg_traits<L1>::const_row_iterator
      itr = mat_row_const_begin(l1);
    for (; it != ite; ++it, ++itr)
      *it = vect_sp(linalg_traits<L1>::row(itr), l2,
		    typename linalg_traits<L1>::storage_type(),
		    typename linalg_traits<L2>::storage_type());
  }

  template <typename L1, typename L2, typename L3>
  void mult_by_col(const L1& l1, const L2& l2, L3& l3, abstract_dense) {
    clear(l3);
    size_type nc = mat_ncols(l1);
    for (size_type i = 0; i < nc; ++i)
      add(scaled(mat_const_col(l1, i), l2[i]), l3);
  }

  template <typename L1, typename L2, typename L3>
  void mult_by_col(const L1& l1, const L2& l2, L3& l3, abstract_sparse) {
    typedef typename linalg_traits<L1>::value_type T;
    clear(l3);
    typename linalg_traits<L2>::const_iterator it = vect_const_begin(l2),
      ite = vect_const_end(l2);
    for (; it != ite; ++it)
      if (*it != T(0)) add(scaled(mat_const_col(l1, it.index()), *it), l3);
  }

  template <typename L1, typename L2, typename L3>
  void mult_by_col(const L1& l1, const L2& l2, L3& l3, abstract_skyline) {
    typedef typename linalg_traits<L1>::value_type T;
    clear(l3);
    typename linalg_traits<L2>::const_iterator it = vect_const_begin(l2),
      ite = vect_const_end(l2);
    for (; it != ite; ++it)
      if (*it != T(0)) add(scaled(mat_const_col(l1, it.index()), *it), l3);
  }

  template <typename L1, typename L2, typename L3> inline
  void mult_spec(const L1& l1, const L2& l2, L3& l3, row_major)
  { mult_by_row(l1, l2, l3, typename linalg_traits<L3>::storage_type()); }

  template <typename L1, typename L2, typename L3> inline
  void mult_spec(const L1& l1, const L2& l2, L3& l3, col_major)
  { mult_by_col(l1, l2, l3, typename linalg_traits<L2>::storage_type()); }

  template <typename L1, typename L2, typename L3> inline
  void mult_spec(const L1& l1, const L2& l2, L3& l3, abstract_null_type)
  { mult_ind(l1, l2, l3, typename linalg_traits<L1>::storage_type()); }

  template <typename L1, typename L2, typename L3>
  void mult_ind(const L1& l1, const L2& l2, L3& l3, abstract_indirect) {
    DAL_THROW(failure_error,
	  "You have to define gmm::mult(m, v1, v2) for this kind of matrix");
  }

  template <typename L1, typename L2, typename L3, typename L4> inline
  void mult(const L1& l1, const L2& l2, const L3& l3, L4& l4) {
    size_type m = mat_nrows(l1), n = mat_ncols(l1);
    if (!m || !n) { gmm::copy(l3, l4); return; }
    if (n != vect_size(l2) || m != vect_size(l3) || m != vect_size(l4))
      DAL_THROW(dimension_error,"dimensions mismatch");
    if (!same_origin(l2, l4))
      mult_spec(l1, l2, l3, l4, typename principal_orientation_type<typename
		linalg_traits<L1>::sub_orientation>::potype());
    else {
      DAL_WARNING(2, "Warning, A temporary is used for mult\n");
      typename temporary_vector<L4>::vector_type temp(vect_size(l3));
      mult_spec(l1,l2,l3, temp, typename principal_orientation_type<typename
		linalg_traits<L1>::sub_orientation>::potype());
      copy(temp, l4);
    }
  }

  template <typename L1, typename L2, typename L3, typename L4> inline
  void mult(const L1& l1, const L2& l2, const L3& l3, const L4& l4)
  { mult(l1, l2, l3, linalg_const_cast(l4)); } 

  template <typename L1, typename L2, typename L3, typename L4>
  void mult_by_row(const L1& l1, const L2& l2, const L3& l3,
		   L4& l4, abstract_sparse) {
    typedef typename linalg_traits<L1>::value_type T;
    copy(l3, l4);
    size_type nr = mat_nrows(l1);
    for (size_type i = 0; i < nr; ++i) {
      T aux = vect_sp(mat_const_row(l1, i), l2);
      if (aux != T(0)) l4[i] += aux;
    }
  }

  template <typename L1, typename L2, typename L3, typename L4>
  void mult_by_row(const L1& l1, const L2& l2, const L3& l3,
		   L4& l4, abstract_skyline) {
    typedef typename linalg_traits<L1>::value_type T;
    copy(l3, l4);
    size_type nr = mat_nrows(l1);
    for (size_type i = 0; i < nr; ++i) {
      T aux = vect_sp(mat_const_row(l1, i), l2);
      if (aux != T(0)) l4[i] += aux;
    }
  }

  template <typename L1, typename L2, typename L3, typename L4>
  void mult_by_row(const L1& l1, const L2& l2, const L3& l3, L4& l4,
		   abstract_dense) {
    copy(l3, l4); 
    typename linalg_traits<L4>::iterator it=vect_begin(l4), ite=vect_end(l4);
    typename linalg_traits<L1>::const_row_iterator
      itr = mat_row_const_begin(l1);
    for (; it != ite; ++it, ++itr)
      *it += vect_sp(linalg_traits<L1>::row(itr), l2);
  }

  template <typename L1, typename L2, typename L3, typename L4>
  void mult_by_col(const L1& l1, const L2& l2, const L3& l3, L4& l4,
		   abstract_dense) {
    copy(l3, l4);
    size_type nc = mat_ncols(l1);
    for (size_type i = 0; i < nc; ++i)
      add(scaled(mat_const_col(l1, i), l2[i]), l4);
  }

  template <typename L1, typename L2, typename L3, typename L4>
  void mult_by_col(const L1& l1, const L2& l2, const L3& l3, L4& l4,
		   abstract_sparse) {
    copy(l3, l4);
    typename linalg_traits<L2>::const_iterator it = vect_const_begin(l2),
      ite = vect_const_end(l2);
    for (; it != ite; ++it)
      if (*it != typename linalg_traits<L2>::value_type(0))
	add(scaled(mat_const_col(l1, it.index()), *it), l4);
  }

  template <typename L1, typename L2, typename L3, typename L4>
  void mult_by_col(const L1& l1, const L2& l2, const L3& l3, L4& l4,
		   abstract_skyline) {
    copy(l3, l4);
    typename linalg_traits<L2>::const_iterator it = vect_const_begin(l2),
      ite = vect_const_end(l2);
    for (; it != ite; ++it)
      if (*it != typename linalg_traits<L2>::value_type(0))
	add(scaled(mat_const_col(l1, it.index()), *it), l4);
  }

  template <typename L1, typename L2, typename L3, typename L4> inline
  void mult_spec(const L1& l1, const L2& l2, const L3& l3, L4& l4, row_major)
  { mult_by_row(l1, l2, l3, l4, typename linalg_traits<L4>::storage_type()); }

  template <typename L1, typename L2, typename L3, typename L4> inline
  void mult_spec(const L1& l1, const L2& l2, const L3& l3, L4& l4, col_major)
  { mult_by_col(l1, l2, l3, l4, typename linalg_traits<L2>::storage_type()); }

  template <typename L1, typename L2, typename L3, typename L4> inline
  void mult_spec(const L1& l1, const L2& l2, const L3& l3,
		 L4& l4, abstract_null_type)
  { mult_ind(l1, l2, l3, l4, typename linalg_traits<L1>::storage_type()); }

  template <typename L1, typename L2, typename L3, typename L4>
  void mult_ind(const L1& l1, const L2& l2, const L3& l3,
		L4& l4, abstract_indirect) {
    DAL_THROW(failure_error,
	  "You have to define gmm::mult(m, v1, v2) for this kind of matrix");
  }

  template <typename L1, typename L2, typename L3>
  void transposed_mult(const L1& l1, const L2& l2, const L3& l3)
  { mult(gmm::transposed(l1), l2, l3); }


  /* ******************************************************************** */
  /*		Matrix-matrix mult                                    	  */
  /* ******************************************************************** */
  

  struct g_mult {};  // generic mult, less optimized
  struct c_mult {};  // col x col mult
  struct r_mult {};  // row x row mult
  struct rcmult {};  // row x col mult
  struct crmult {};  // col x row mult


  template<typename SO1, typename SO2, typename SO3> struct mult_t;
  #define __DEFMU template<> struct mult_t
  __DEFMU<row_major  , row_major  , row_major  > { typedef r_mult t; };
  __DEFMU<row_major  , row_major  , col_major  > { typedef g_mult t; };
  __DEFMU<row_major  , row_major  , col_and_row> { typedef r_mult t; };
  __DEFMU<row_major  , row_major  , row_and_col> { typedef r_mult t; };
  __DEFMU<row_major  , col_major  , row_major  > { typedef rcmult t; };
  __DEFMU<row_major  , col_major  , col_major  > { typedef rcmult t; };
  __DEFMU<row_major  , col_major  , col_and_row> { typedef rcmult t; };
  __DEFMU<row_major  , col_major  , row_and_col> { typedef rcmult t; };
  __DEFMU<row_major  , col_and_row, row_major  > { typedef rcmult t; };
  __DEFMU<row_major  , col_and_row, col_major  > { typedef rcmult t; };
  __DEFMU<row_major  , col_and_row, col_and_row> { typedef rcmult t; };
  __DEFMU<row_major  , col_and_row, row_and_col> { typedef rcmult t; };
  __DEFMU<row_major  , row_and_col, row_major  > { typedef r_mult t; };
  __DEFMU<row_major  , row_and_col, col_major  > { typedef rcmult t; };
  __DEFMU<row_major  , row_and_col, col_and_row> { typedef r_mult t; };
  __DEFMU<row_major  , row_and_col, row_and_col> { typedef r_mult t; };
  __DEFMU<col_major  , row_major  , row_major  > { typedef crmult t; };
  __DEFMU<col_major  , row_major  , col_major  > { typedef g_mult t; };
  __DEFMU<col_major  , row_major  , col_and_row> { typedef crmult t; };
  __DEFMU<col_major  , row_major  , row_and_col> { typedef crmult t; };
  __DEFMU<col_major  , col_major  , row_major  > { typedef g_mult t; };
  __DEFMU<col_major  , col_major  , col_major  > { typedef c_mult t; };
  __DEFMU<col_major  , col_major  , col_and_row> { typedef c_mult t; };
  __DEFMU<col_major  , col_major  , row_and_col> { typedef c_mult t; };
  __DEFMU<col_major  , col_and_row, row_major  > { typedef crmult t; };
  __DEFMU<col_major  , col_and_row, col_major  > { typedef c_mult t; };
  __DEFMU<col_major  , col_and_row, col_and_row> { typedef c_mult t; };
  __DEFMU<col_major  , col_and_row, row_and_col> { typedef c_mult t; };
  __DEFMU<col_major  , row_and_col, row_major  > { typedef crmult t; };
  __DEFMU<col_major  , row_and_col, col_major  > { typedef c_mult t; };
  __DEFMU<col_major  , row_and_col, col_and_row> { typedef c_mult t; };
  __DEFMU<col_major  , row_and_col, row_and_col> { typedef c_mult t; };
  __DEFMU<col_and_row, row_major  , row_major  > { typedef r_mult t; };
  __DEFMU<col_and_row, row_major  , col_major  > { typedef c_mult t; };
  __DEFMU<col_and_row, row_major  , col_and_row> { typedef r_mult t; };
  __DEFMU<col_and_row, row_major  , row_and_col> { typedef r_mult t; };
  __DEFMU<col_and_row, col_major  , row_major  > { typedef rcmult t; };
  __DEFMU<col_and_row, col_major  , col_major  > { typedef c_mult t; };
  __DEFMU<col_and_row, col_major  , col_and_row> { typedef c_mult t; };
  __DEFMU<col_and_row, col_major  , row_and_col> { typedef c_mult t; };
  __DEFMU<col_and_row, col_and_row, row_major  > { typedef r_mult t; };
  __DEFMU<col_and_row, col_and_row, col_major  > { typedef c_mult t; };
  __DEFMU<col_and_row, col_and_row, col_and_row> { typedef c_mult t; };
  __DEFMU<col_and_row, col_and_row, row_and_col> { typedef c_mult t; };
  __DEFMU<col_and_row, row_and_col, row_major  > { typedef r_mult t; };
  __DEFMU<col_and_row, row_and_col, col_major  > { typedef c_mult t; };
  __DEFMU<col_and_row, row_and_col, col_and_row> { typedef c_mult t; };
  __DEFMU<col_and_row, row_and_col, row_and_col> { typedef r_mult t; };
  __DEFMU<row_and_col, row_major  , row_major  > { typedef r_mult t; };
  __DEFMU<row_and_col, row_major  , col_major  > { typedef c_mult t; };
  __DEFMU<row_and_col, row_major  , col_and_row> { typedef r_mult t; };
  __DEFMU<row_and_col, row_major  , row_and_col> { typedef r_mult t; };
  __DEFMU<row_and_col, col_major  , row_major  > { typedef rcmult t; };
  __DEFMU<row_and_col, col_major  , col_major  > { typedef c_mult t; };
  __DEFMU<row_and_col, col_major  , col_and_row> { typedef c_mult t; };
  __DEFMU<row_and_col, col_major  , row_and_col> { typedef c_mult t; };
  __DEFMU<row_and_col, col_and_row, row_major  > { typedef rcmult t; };
  __DEFMU<row_and_col, col_and_row, col_major  > { typedef rcmult t; };
  __DEFMU<row_and_col, col_and_row, col_and_row> { typedef rcmult t; };
  __DEFMU<row_and_col, col_and_row, row_and_col> { typedef rcmult t; };
  __DEFMU<row_and_col, row_and_col, row_major  > { typedef r_mult t; };
  __DEFMU<row_and_col, row_and_col, col_major  > { typedef c_mult t; };
  __DEFMU<row_and_col, row_and_col, col_and_row> { typedef r_mult t; };
  __DEFMU<row_and_col, row_and_col, row_and_col> { typedef r_mult t; };

  template <typename L1, typename L2, typename L3>
  void mult_dispatch(const L1& l1, const L2& l2, L3& l3, abstract_matrix) {
    typedef typename temporary_matrix<L3>::matrix_type temp_mat_type;

    if (mat_ncols(l1) != mat_nrows(l2) || mat_nrows(l1) != mat_nrows(l3)
	|| mat_ncols(l2) != mat_ncols(l3))
      DAL_THROW(dimension_error,"dimensions mismatch");

    if (same_origin(l2, l3) || same_origin(l1, l3)) {
      DAL_WARNING(2, "A temporary is used for mult");
      temp_mat_type temp(mat_nrows(l3), mat_ncols(l3));
      mult_spec(l1, l2, temp, typename mult_t<
		typename linalg_traits<L1>::sub_orientation,
		typename linalg_traits<L2>::sub_orientation,
		typename linalg_traits<temp_mat_type>::sub_orientation>::t());
      copy(temp, l3);
    }
    else
      mult_spec(l1, l2, l3, typename mult_t<
		typename linalg_traits<L1>::sub_orientation,
		typename linalg_traits<L2>::sub_orientation,
		typename linalg_traits<L3>::sub_orientation>::t());
  }

  // Completely generic but inefficient

  template <typename L1, typename L2, typename L3>
  void mult_spec(const L1& l1, const L2& l2, L3& l3, g_mult) {
    DAL_WARNING(2, "Inefficient generic matrix-matrix mult is used");
    clear(l3);
    for (size_type i = 0; i < mat_nrows(l3) ; ++i)
      for (size_type j = 0; j < mat_nrows(l3) ; ++j)
	for (size_type k = 0; k < mat_nrows(l3) ; ++k)
	  l3(i, j) += l1(i, k) * l2(k, j);
  }

  // row x col matrix-matrix mult

  template <typename L1, typename L2, typename L3>
  void mult_spec(const L1& l1, const L2& l2, L3& l3, rcmult) {

    if (is_sparse(l1) || is_sparse(l2))
      DAL_WARNING(3, "Inefficient matrix-matrix mult for sparse matrices");

    typename linalg_traits<L2>::const_col_iterator
      it2b = linalg_traits<L2>::col_begin(l2), it2,
      ite = linalg_traits<L2>::col_end(l2);
    size_type i,j, k = mat_nrows(l1);

    for (i = 0; i < k; ++i) {
      typename linalg_traits<L1>::const_sub_row_type r1 = mat_const_row(l1, i);
      for (it2 = it2b, j = 0; it2 != ite; ++it2, ++j)
	l3(i,j) = vect_sp(r1, linalg_traits<L2>::col(it2));
    }
  }

  // row - row matrix-matrix mult

  template <typename L1, typename L2, typename L3> inline
  void mult_spec(const L1& l1, const L2& l2, L3& l3, r_mult)
  { mult_spec(l1, l2, l3, r_mult(), typename linalg_traits<L1>::storage_type()); }



  template <typename L1, typename L2, typename L3>
  void mult_spec(const L1& l1, const L2& l2, L3& l3, r_mult, abstract_dense) {
    // optimizable
    clear(l3);
    size_type nn = mat_nrows(l3), mm = mat_nrows(l2);
    for (size_type i = 0; i < nn; ++i) {
      for (size_type j = 0; j < mm; ++j)
      add(scaled(mat_const_row(l2, j), l1(i, j)), mat_row(l3, i));
    }
  }

  template <typename L1, typename L2, typename L3>
  void mult_spec(const L1& l1, const L2& l2, L3& l3, r_mult, abstract_sparse) {
    // optimizable
    clear(l3);
    size_type nn = mat_nrows(l3);
    for (size_type i = 0; i < nn; ++i) {
      typename linalg_traits<L1>::const_sub_row_type rl1=mat_const_row(l1, i);
      typename linalg_traits<typename linalg_traits<L1>::const_sub_row_type>::
	const_iterator it = vect_const_begin(rl1), ite = vect_const_end(rl1);
      for (; it != ite; ++it)
	add(scaled(mat_const_row(l2, it.index()), *it), mat_row(l3, i));
    }
  }

  template <typename L1, typename L2, typename L3>
  void mult_spec(const L1& l1, const L2& l2, L3& l3, r_mult, abstract_skyline)
  { mult_spec(l1, l2, l3, r_mult(), abstract_sparse()); }

  // col - col matrix-matrix mult

  template <typename L1, typename L2, typename L3> inline
  void mult_spec(const L1& l1, const L2& l2, L3& l3, c_mult) {
    mult_spec(l1, l2,l3,c_mult(),typename linalg_traits<L2>::storage_type(),
	      typename linalg_traits<L2>::sub_orientation());
  }


  template <typename L1, typename L2, typename L3, typename ORIEN>
  void mult_spec(const L1& l1, const L2& l2, L3& l3, c_mult,
		 abstract_dense, ORIEN) {
    typedef typename linalg_traits<L1>::value_type T;
    size_type nn = mat_ncols(l3), mm = mat_ncols(l1);

    for (size_type i = 0; i < nn; ++i) {
      clear(mat_col(l3, i));
      for (size_type j = 0; j < mm; ++j) {
	T b = l2(j, i);
	if (b != T(0)) add(scaled(mat_const_col(l1, j), b), mat_col(l3, i));
      }
    }
  }

  template <typename L1, typename L2, typename L3, typename ORIEN>
  void mult_spec(const L1& l1, const L2& l2, L3& l3, c_mult,
		 abstract_sparse, ORIEN) {
    // optimizable
    clear(l3);
    size_type nn = mat_ncols(l3);
    for (size_type i = 0; i < nn; ++i) {
      typename linalg_traits<L2>::const_sub_col_type rc2=mat_const_col(l2, i);
      typename linalg_traits<typename linalg_traits<L2>::const_sub_col_type>::
	const_iterator it = vect_const_begin(rc2), ite = vect_const_end(rc2);
      for (; it != ite; ++it)
	add(scaled(mat_const_col(l1, it.index()), *it), mat_col(l3, i));
    }
  }

  template <typename L1, typename L2, typename L3>
  void mult_spec(const L1& l1, const L2& l2, L3& l3, c_mult,
		 abstract_sparse, row_major) {
     typedef typename linalg_traits<L2>::value_type T;
     DAL_WARNING(3, "Inefficient matrix-matrix mult for sparse matrices");
     clear(l3);
     size_type mm = mat_nrows(l2), nn = mat_ncols(l3);
     for (size_type i = 0; i < nn; ++i)
       for (size_type j = 0; j < mm; ++j) {
	 T a = l2(i,j);
	 if (a != T(0)) add(scaled(mat_const_col(l1, j), a), mat_col(l3, i));
       }
   }

  template <typename L1, typename L2, typename L3, typename ORIEN> inline
  void mult_spec(const L1& l1, const L2& l2, L3& l3, c_mult,
		 abstract_skyline, ORIEN)
  { mult_spec(l1, l2, l3, c_mult(), abstract_sparse(), ORIEN()); }

  
  // col - row matrix-matrix mult

  template <typename L1, typename L2, typename L3> inline
  void mult_spec(const L1& l1, const L2& l2, L3& l3, crmult)
  { mult_spec(l1,l2,l3,crmult(), typename linalg_traits<L1>::storage_type()); }


  template <typename L1, typename L2, typename L3>
  void mult_spec(const L1& l1, const L2& l2, L3& l3, crmult, abstract_dense) {
    // optimizable
    clear(l3);
    size_type nn = mat_ncols(l1), mm = mat_nrows(l1);
    for (size_type i = 0; i < nn; ++i) {
      for (size_type j = 0; j < mm; ++j)
      add(scaled(mat_const_row(l2, i), l2(j, i)), mat_row(l3, j));
    }
  }

  template <typename L1, typename L2, typename L3>
  void mult_spec(const L1& l1, const L2& l2, L3& l3, crmult, abstract_sparse) {
    // optimizable
    clear(l3);
    size_type nn = mat_ncols(l1);
    for (size_type i = 0; i < nn; ++i) {
      typename linalg_traits<L1>::const_sub_col_type rc1=mat_const_col(l1, i);
      typename linalg_traits<typename linalg_traits<L1>::const_sub_col_type>::
	const_iterator it = vect_const_begin(rc1), ite = vect_const_end(rc1);
      for (; it != ite; ++it)
	add(scaled(mat_const_row(l2, i), *it), mat_row(l3, it.index()));
    }
  }

  template <typename L1, typename L2, typename L3> inline
  void mult_spec(const L1& l1, const L2& l2, L3& l3, crmult, abstract_skyline)
  { mult_spec(l1, l2, l3, crmult(), abstract_sparse()); }}
  

#endif //  __GMM_BLAS_H
