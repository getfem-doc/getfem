// -*- c++ -*- (enables emacs c++ mode)
//========================================================================
//
// Library : Dynamic Array Library (dal)
// File    : dal_alloc.h : allocation in an array.
//           
// Date    : March 06, 1997.
// Author  : Yves Renard <Yves.Renard@insa-toulouse.fr>
//
//========================================================================
//
// Copyright (C) 1997-2005 Yves Renard
//
// This file is a part of GETFEM++
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//========================================================================


#ifndef DAL_ALLOC_H__
#define DAL_ALLOC_H__

#include <dal_tree_sorted.h>


namespace dal {

  struct fr_sp_ { size_t ind, size; };
  struct less1_fr_sp_ : public std::binary_function<fr_sp_, fr_sp_, int> {
    int operator()(const fr_sp_ &f1, const fr_sp_ &f2) const
      { return (f1.ind < f2.ind) ? -1 : ((f1.ind > f2.ind) ? 1 : 0); }
  };
  struct less2_fr_sp_ : public std::binary_function<fr_sp_, fr_sp_, int> {
    int operator()(const fr_sp_ &f1, const fr_sp_ &f2) const
      { return (f1.size < f2.size) ? -1 : ((f1.size > f2.size) ? 1 : 0); }
  };
  
  
  template<class T,  unsigned char pks = 5> class dynamic_alloc
    : public dynamic_array<T, pks> {
  public :
    
    typedef typename dynamic_array<T, pks>::size_type size_type;
    
  protected :
    
    typedef dynamic_tree_sorted<fr_sp_, less1_fr_sp_, 6> fsptab_t;
    
    fsptab_t fr_tab;
    dynamic_tree_sorted_index<fr_sp_, fsptab_t, less2_fr_sp_, 6> ind_fr_tab;
    
    void init(void) { 
      fr_sp_ fsp; fsp.size = size_type(-2); fsp.ind = 0;
      size_type i = fr_tab.add(fsp); ind_fr_tab.add(i);
    }
    
  public :
    
    dynamic_alloc(void) : ind_fr_tab(fr_tab) { init(); }
    dynamic_alloc(const dynamic_alloc<T, pks> &da) :
      dynamic_array<T, pks>(da), fr_tab(da.fr_tab),
      ind_fr_tab(da.ind_fr_tab) { ind_fr_tab.change_tab(fr_tab); }
    dynamic_alloc<T, pks> &operator =(const dynamic_alloc<T, pks> &da) {
      dynamic_array<T, pks>::operator =(da);
      fr_tab = da.fr_tab;
      ind_fr_tab = da.ind_fr_tab;
      ind_fr_tab.change_tab(fr_tab);
      return *this;
    }
    
    size_type alloc(size_type); 
    void free(size_type, size_type);
    void clear(void);
  };
  
  template<class T, unsigned char pks>
    void dynamic_alloc<T,pks>::clear(void)
  { dynamic_array<T,pks>::clear(); fr_tab.clear(); ind_fr_tab.clear(); init();}

  template<class T, unsigned char pks>
    typename dynamic_alloc<T,pks>::size_type
      dynamic_alloc<T,pks>::alloc(size_type si) {
    size_type res = ST_NIL;
    if (si > 0) {
      fr_sp_ fsp; fsp.size = si; 
      size_type i = ind_fr_tab.search_ge(fsp);
      if (i != ST_NIL) {
	res = fr_tab[i].ind;
	if (si > fr_tab[i].size)
	  throw dal::internal_error
	    ("dynamic_alloc<T,pks>::alloc : internal error (1)");
	if (si <  fr_tab[i].size) {
	  ind_fr_tab.sup(i); fr_tab[i].ind += si;
	  fr_tab[i].size -= si; ind_fr_tab.add(i);
	}
	else
	{ ind_fr_tab.sup(i); fr_tab.sup(i); }
      }
      else {
	throw dal::internal_error
	  ("dynamic_alloc<T,pks>::alloc : internal error (2)");
      }
    }
    return res;
  }

  
  template<class T, unsigned char pks>
    void dynamic_alloc<T,pks>::free(size_type l, size_type si) {
    if (si > 0 && l != ST_NIL)
    {
      fr_sp_ fsp; fsp.size = si; fsp.ind = l;
      size_type i = fr_tab.add(fsp);
      fsptab_t::const_sorted_iterator it1(fr_tab);
      fr_tab.find_sorted_iterator(i, it1);
      fsptab_t::const_sorted_iterator it2 = it1;
      size_type i1 = (++it1).index(), i2 = (--it2).index();
      if (i1 != ST_NIL && (*it1).ind <= l + si) { 
	fr_tab[i].size = (*it1).ind + (*it1).size - l;
	ind_fr_tab.sup(i1); fr_tab.sup(i1);
      }
      if (i2 != ST_NIL && (*it2).ind + (*it2).size >= l) {
	fr_tab[i].size = l + si - (*it2).ind;
	fr_tab[i].ind = (*it2).ind;
	ind_fr_tab.sup(i2); fr_tab.sup(i2);
      }
      ind_fr_tab.add(i);
    }
  }

}

#endif  /* DAL_ALLOC_H__ */
