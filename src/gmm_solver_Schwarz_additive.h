/* -*- c++ -*- (enables emacs c++ mode)                                    */
/* *********************************************************************** */
/*                                                                         */
/* Library :  Generic Matrix Methods  (gmm)                                */
/* File    :  gmm_solvers_Schwarz_additive.h : generic solver.             */
/*     									   */
/* Date : October 13, 2002.                                                */
/* Author : Yves Renard, Yves.Renard@gmm.insa-tlse.fr                      */
/*          Michel Fournie, fournie@mip.ups-tlse.fr                        */
/*                                                                         */
/* *********************************************************************** */
/*                                                                         */
/* Copyright (C) 2002  Yves Renard, Michel Fournie.                        */
/*                                                                         */
/* This file is a part of GETFEM++                                         */
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


#ifndef __GMM_SOLVERS_SCHWARZ_ADDITIVE_H
#define __GMM_SOLVERS_SCHWARZ_ADDITIVE_H

namespace gmm {
      
  /* ******************************************************************** */
  /*		Schwartz Additive method                                  */
  /* ******************************************************************** */

  template <class Matrix1, class Matrix2, class Matrix3, class SUBI>
  struct schwarz_additif_matrix {
    typedef typename linalg_traits<Matrix2>::value_type value_type;
    typedef typename plain_vector_type<value_type>::vector_type vector_type; 
    const Matrix1 *A;
    const std::vector<Matrix2> *ml1;
    const std::vector<Matrix3> *ml2;
    const std::vector<SUBI> *cor;
    int itemax, noisy;
    double residu;
    std::vector<vector_type> *gi;
    std::vector<vector_type> *fi;
  };

  template <class Matrix1, class Matrix2, class Matrix3,
    class SUBI, class Vector2, class Vector3>
  void schwarz_additif(const Matrix1 &A,
		       Vector3 &u,
		       const std::vector<Matrix2> &ml1,
		       const std::vector<Matrix3> &ml2,
		       const std::vector<SUBI> &cor,
		       const Vector2 &f,
		       int itemax,  double residu, int noisy = 1) {

    typedef typename linalg_traits<Matrix2>::value_type value_type;
    typedef typename plain_vector_type<value_type>::vector_type vector_type;

    size_type nb_sub = ml1.size() + ml2.size();
    std::vector<vector_type> gi(nb_sub);
    std::vector<vector_type> fi(nb_sub);

    residu /= nb_sub; // utile ?

    size_type ms = ml1.size();

    for (int i = 0; i < nb_sub; ++i) {
      size_type k = i < ms ? mat_nrows(ml1[i]) : mat_nrows(ml2[i-ms]);
      fi[i] = gi[i] = vector_type(k);
      clear(gi[i]);
    }

    size_type nb_dof = f.size();
    global_to_local(f, fi, cor);

    for (int i = 0; i < ms; ++i) {
      cg(ml1[i], gi[i], fi[i], identity_matrix(),
	 identity_matrix(), itemax, residu, noisy - 1);
    }
    for (int i = 0; i < ml2.size(); ++i) {
      cg(ml2[i], gi[i+ms], fi[i+ms], identity_matrix(),
	 identity_matrix(), itemax, residu, noisy - 1);
    }

    vector_type g(nb_dof);
    local_to_global(gi, g, cor);
    // copy(g, u);
    
    // cout << " g = " << g << " g0 = " << gi[0] << endl;
    
    schwarz_additif_matrix<Matrix1, Matrix2, Matrix3, SUBI> SAM;
    SAM.A = &A; SAM.ml1 = &ml1; SAM.ml2 = &ml2; SAM.cor = &cor;
    SAM.itemax = itemax; SAM.residu = residu; SAM.noisy = noisy;
    SAM.gi = &gi; SAM.fi = &fi;
    // cg(SAM, u, g, identity_matrix(), identity_matrix(), itemax, residu * 1000, noisy);

//    for (size_type i = 0; i < mat_nrows(A); ++i) {
//       Vector3 ff(mat_nrows(A)), gg(mat_nrows(A));
//       clear(ff);
//       ff[i] = 1.0;
//       mult(SAM, ff, gg);
//       cout << "SAM[" << i << "] = " << gg << endl;
//     }


    cg(SAM, u, g, A, identity_matrix(), 100, residu * 1000, noisy);
   // bicgstab(SAM, u, g, identity_matrix(), 100, residu * 1000, noisy);

    // il faudrait tester le residu final ...

  }
  
  template <class Matrix1, class Matrix2, class Matrix3, class SUBI,
    class Vector2, class Vector3>
  void mult(const schwarz_additif_matrix<Matrix1, Matrix2, Matrix3, SUBI> &M,
	    const Vector2 &p, Vector3 &q) {

    size_type ms = (M.ml1)->size();
    mult(*(M.A), p, q);
    global_to_local(q, *(M.fi), *(M.cor));
    for (int i = 0; i < (M.ml1)->size(); ++i)
      cg((*(M.ml1))[i], (*(M.gi))[i], (*(M.fi))[i], identity_matrix(),
	 identity_matrix(), M.itemax, M.residu, M.noisy-1);

    for (int i = 0; i < (M.ml2)->size(); ++i)
      cg((*(M.ml2))[i],(*(M.gi))[i+ms],(*(M.fi))[i+ms], identity_matrix(),
	 identity_matrix(), M.itemax, M.residu, M.noisy-1);

    local_to_global(*(M.gi), q, *(M.cor));

    // Vector3 zz = q; add(scaled((*(M.gi))[0], -1.0), zz);
    // cout << "difference = " << vect_norm2(zz) << endl;
  }

  template <class Matrix1, class Matrix2, class Matrix3, class SUBI,
    class Vector2, class Vector3, class Vector4>
  void mult(const schwarz_additif_matrix<Matrix1, Matrix2, Matrix3, SUBI> &M,
	    const Vector2 &p, const Vector3 &p2, Vector4 &q) {
    
    size_type ms = (M.ml1)->size();
    mult(*(M.A), p, q);
    global_to_local(q, *(M.fi), *(M.cor));
    for (int i = 0; i < (M.ml1)->size(); ++i)
      cg((*(M.ml1))[i], (*(M.gi))[i], (*(M.fi))[i], identity_matrix(),
	 identity_matrix(), M.itemax, M.residu, M.noisy-1);

    for (int i = 0; i < (M.ml2)->size(); ++i)
      cg((*(M.ml2))[i],(*(M.gi))[i+ms],(*(M.fi))[i+ms], identity_matrix(),
	 identity_matrix(), M.itemax,M.residu, M.noisy-1);

    local_to_global(*(M.gi), q, *(M.cor));
    add(p2, q);
  }

  template <class SUBI, class Vector2, class Vector3>
  void global_to_local(const Vector2 &f, std::vector<Vector3> &fi,
		       const std::vector<SUBI> &cor) {
    // cout << " f = " << f << endl;
    for (int i = 0; i < fi.size(); ++i) {
      // typename linalg_traits<Vector3>::iterator it2 = fi[i].begin();
      for (size_type j = 0, l = cor[i].size(); j < l; ++j/* , ++it2 */)
	fi[i][j] /* *it2 */ = f[cor[i].index(j)];
      // cout << "res fi[" << i << "] = " << fi[i] << endl;
    }
  }

  template <class SUBI, class Vector2, class Vector3>
  void local_to_global(const std::vector<Vector3> &fi, Vector2 &f, 
		       const std::vector<SUBI> &cor) {
    clear(f);
    // cout << " f = " << f << endl;
    for (size_type i = 0; i < fi.size(); ++i) {
      // cout << "fi[" << i << "] = " << fi[i] << endl;
      // typename linalg_traits<Vector3>::const_iterator it2=fi[i].begin();
      for (size_type j = 0, l = cor[i].size(); j < l; ++j /*, ++it2 */) {
	f[cor[i].index(j)] += fi[i][j]; // *it2
	//	if (cor[i].index(j) == 4) cout << "i = " << i << " j = " << j << " *it2 = " << *it2 << " total = " << f[cor[i].index(j)] << endl;
      }
    }
    // cout << " f = " << f << endl;
  }
  
}


#endif //  __GMM_SOLVERS_SCHWARZ_ADDITIVE_H
