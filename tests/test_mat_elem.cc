/*===========================================================================

 Copyright (C) 2002-2020 Yves Renard.

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
/* *********************************************************************** */
/*                                                                         */
/*   Program to test the efficiency of elementary matrices computation.    */
/*                                                                         */
/* *********************************************************************** */

#include "getfem/getfem_assembling.h"
#include "getfem/getfem_export.h"
#include "getfem/getfem_regular_meshes.h"
#include "getfem/getfem_mat_elem.h"
using std::endl; using std::cout; using std::cerr;
using std::ends; using std::cin;


using bgeot::base_vector;
using bgeot::base_small_vector;
using bgeot::base_node;
using bgeot::scalar_type;
using bgeot::size_type;
using bgeot::dim_type;

/**************************************************************************/
/*  structure representing the problem.                                   */
/**************************************************************************/

struct lap_pb {
  getfem::mesh mesh;
  getfem::mesh_im mim;
  getfem::mesh_fem mef;
  getfem::mesh_fem mef_data;

  scalar_type LX, LY, LZ, incline, residual;
  size_type N;
  int NX, K, fem_type, KI;
 
  int integration, mesh_type;

  std::string datafilename;
  bgeot::md_param PARAM;

  
  void init(void);
  lap_pb(void) : mim(mesh), mef(mesh), mef_data(mesh) {}
};

void lap_pb::init(void)
{
  dal::bit_vector nn;

  /***********************************************************************/
  /* READING PARAMETER FILE                                              */
  /***********************************************************************/
  
  N = PARAM.int_value("N", "Domaine dimension");
  LX = PARAM.real_value("LX", "Size in X");
  LY = PARAM.real_value("LY", "Size in Y");
  LZ = PARAM.real_value("LZ", "Size in Y");
  incline = PARAM.real_value("INCLINE", "incline of the mesh");
  NX = int(PARAM.int_value("NX", "Nomber of sace steps "));
  integration = int(PARAM.int_value("INTEGRATION", "integration method"));
  mesh_type = int(PARAM.int_value("MESH_TYPE", "Mesh type "));
  residual = PARAM.real_value("RESIDUAL", "Residu for c.g.");
  K = int(PARAM.int_value("K", "Finite element degree"));
  KI = int(PARAM.int_value("KI", "Integration degree"));
  fem_type = int(PARAM.int_value("FEM_TYPE", "Finite element method"));
  datafilename = std::string( PARAM.string_value("ROOTFILENAME",
			     "File name for saving"));

  /***********************************************************************/
  /*  BUILD MESH.                                                        */
  /***********************************************************************/

  cout << "Mesh generation\n";

  base_node org(N); 
  std::vector<base_small_vector> vtab(N);
  std::vector<size_type> ref(N); std::fill(ref.begin(), ref.end(), NX);
  for (dim_type i = 0; i < N; i++)
  { 
    vtab[i] = base_small_vector(N); 
    (vtab[i])[i] = ((i == 0) ? LX : ((i == 1) ? LY : LZ)) / scalar_type(NX);
  }
  if (N > 1) vtab[N-1][0] = incline * LX / scalar_type(NX);

  switch (mesh_type) {
  case 0 : getfem::parallelepiped_regular_simplex_mesh
      (mesh, bgeot::dim_type(N), org, vtab.begin(), ref.begin()); break;
  case 1 : getfem::parallelepiped_regular_mesh
      (mesh, bgeot::dim_type(N), org, vtab.begin(), ref.begin()); break;
  case 2 : getfem::parallelepiped_regular_prism_mesh
      (mesh, bgeot::dim_type(N), org, vtab.begin(), ref.begin()); break;
  default : GMM_ASSERT1(false, "Unknown type of mesh");
  }

  mesh.optimize_structure();

  if (mesh_type == 2 && N <= 1) mesh_type = 0;

  cout << "Selecting finite element method.\n";

  switch(fem_type) {
  case 0 : break;
  case 1 :
    if (N != 1 || mesh_type != 0)
      GMM_ASSERT1(false, "This element is only defined on segments");
     K = 3;
    break;
  case 2 : 
    if (mesh_type != 0)
      GMM_ASSERT1(false, "This element is only defined on simplexes");
    break;
  case 3 : 
    if (mesh_type != 0)
      GMM_ASSERT1(false, "This element is only defined on simplexes");
    break;
  default : GMM_ASSERT1(false, "Unknown finite element method");
  }

  getfem::pintegration_method ppi;
  char meth[500];
  nn = mesh.convex_index(bgeot::dim_type(N));
  switch (integration) {
  case 0 :
    switch (mesh_type) { 
    case 0 : sprintf(meth, "IM_EXACT_SIMPLEX(%d)", int(N)); break;
    case 1 : sprintf(meth, "IM_EXACT_PARALLELEPIPED(%d)", int(N)); break;
    default : GMM_ASSERT1(false,
			  "Exact integration not allowed in this context");
    }
    break;
  case 1 :
    switch (mesh_type) { 
    case 0 : 
      sprintf(meth, "IM_NC(%d,%d)", int(N), int(2*K));
      break;
    case 1 : 
      sprintf(meth, "IM_NC_PARALLELEPIPED(%d,%d)", int(N), int(2*K));
      break;
    case 2 :
      sprintf(meth, "IM_NC_PRISM(%d,%d)", int(N), int(2*K));
      break;
    }
    break;
  case 2 :
    if (mesh_type == 1)
      sprintf(meth, "IM_GAUSS_PARALLELEPIPED(%d,%d)", int(N), int(KI));
    else
      GMM_ASSERT1(false, "Product of 1D Gauss only for parallelepipeds");
    break;
  case 3 :
    if (mesh_type == 0) {
      if (N == 1)
	sprintf(meth, "IM_STRUCTURED_COMPOSITE(IM_GAUSS1D(%d), %d)",2,int(KI));
      else if (N == 2)
	sprintf(meth, "IM_STRUCTURED_COMPOSITE(IM_TRIANGLE(%d), %d)",
		2,int(KI));
      else
	sprintf(meth, "IM_STRUCTURED_COMPOSITE(IM_NC(%d, %d), %d)",
		int(N), int(2*K), int(KI));
    }
    else
      GMM_ASSERT1(false, "Composite integration only for simplexes");
    break;
  case 11 : sprintf(meth, "IM_TRIANGLE(1)"); break;
  case 12 : sprintf(meth, "IM_TRIANGLE(2)"); break;
  case 13 : sprintf(meth, "IM_TRIANGLE(3)"); break;
  case 14 : sprintf(meth, "IM_TRIANGLE(4)"); break;
  case 15 : sprintf(meth, "IM_TRIANGLE(5)"); break;
  case 16 : sprintf(meth, "IM_TRIANGLE(6)"); break;
  case 17 : sprintf(meth, "IM_TRIANGLE(7)"); break;
  case 21 : sprintf(meth, "IM_TETRAHEDRON(1)"); break;
  case 22 : sprintf(meth, "IM_TETRAHEDRON(2)"); break;
  case 23 : sprintf(meth, "IM_TETRAHEDRON(3)"); break;
  case 25 : sprintf(meth, "IM_TETRAHEDRON(5)"); break;
  case 32 : sprintf(meth, "IM_QUAD(2)"); break;
  case 33 : sprintf(meth, "IM_QUAD(3)"); break;
  case 35 : sprintf(meth, "IM_QUAD(5)"); break;
  default : GMM_ASSERT1(false, "Undefined integration method");
  }
  ppi = getfem::int_method_descriptor(meth);
  getfem::pfem pfprinc = 0;
  switch (mesh_type) {
  case 0 :
    sprintf(meth, "FEM_PK(%d,%d)", int(N), int(K));
    pfprinc = getfem::fem_descriptor(meth);
    mim.set_integration_method(nn, ppi);
    mef.set_finite_element(nn, getfem::fem_descriptor(meth));
    mef_data.set_finite_element(nn, getfem::fem_descriptor(meth));
    sprintf(meth, "FEM_PK(%d,%d)", int(N), 0);
    
    break;
  case 1 :
    sprintf(meth, "FEM_QK(%d,%d)", int(N), K);
    pfprinc = getfem::fem_descriptor(meth);
    mim.set_integration_method(nn, ppi);
    mef.set_finite_element(nn, getfem::fem_descriptor(meth)); 
    mef_data.set_finite_element(nn, getfem::fem_descriptor(meth));
    sprintf(meth, "FEM_QK(%d,%d)", int(N), 0);
    
    break;
  case 2 :
    sprintf(meth, "FEM_PK_PRISM(%d,%d)", int(N), K);
    pfprinc = getfem::fem_descriptor(meth);
    mim.set_integration_method(nn, ppi);
    mef.set_finite_element(nn, getfem::fem_descriptor(meth));
    mef_data.set_finite_element(nn, getfem::fem_descriptor(meth));
    sprintf(meth, "FEM_PK_PRISM(%d,%d)", int(N), 0);
    
    break;
  }

  switch(fem_type) {

  case 0 : break;

  case 1 :
    sprintf(meth, "FEM_HERMITE(1)");
    pfprinc = getfem::fem_descriptor(meth);
    mef.set_finite_element(nn, getfem::fem_descriptor(meth));
    break;
    
  case 2 :
    sprintf(meth, "FEM_PK_HIERARCHICAL(%d, %d)", int(N), int(K));
    pfprinc = getfem::fem_descriptor(meth);
    mef.set_finite_element(nn, getfem::fem_descriptor(meth));
    break;

  case 3 :
    sprintf(meth, "FEM_PK_HIERARCHICAL_COMPOSITE(%d,%d,%d)", int(N), 1, int(K));
    pfprinc = getfem::fem_descriptor(meth);
    mef.set_finite_element(nn, getfem::fem_descriptor(meth));
    break;
  
  }
  
  cout << "Name of principal finite element method : "
       << getfem::name_of_fem(pfprinc) << endl;
  cout << "Name of principal integration method : "
       << getfem::name_of_int_method(ppi) << endl;
}

void test1_mat_elem(const getfem::mesh_im &mim,
		    const getfem::mesh_fem &mf,
		   const getfem::mesh_fem &mfdata) { 
  
  size_type cv;
  dal::bit_vector nn = mf.convex_index();
  bgeot::base_tensor t;
  getfem::pfem pf1, pf2, pf1prec = 0, pf2prec = 0;
  getfem::pintegration_method pim, pimprec = 0;
  bgeot::pgeometric_trans pgt, pgtprec = 0;
  getfem::pmat_elem_computation pmec = 0;
  
  if (&(mf.linked_mesh()) != &(mfdata.linked_mesh()))
    GMM_ASSERT1(false,"This assembling procedure only works on a single mesh");
  
  for (cv << nn; cv != size_type(-1); cv << nn) {
    pf1 =     mf.fem_of_element(cv);
    pf2 = mfdata.fem_of_element(cv);
    pgt = mf.linked_mesh().trans_of_convex(cv);
    pim = mim.int_method_of_element(cv);
    if (pf1prec != pf1 || pf2prec != pf2 || pgtprec != pgt || pimprec != pim) {
      getfem::pmat_elem_type pme 
	= getfem::mat_elem_product
	(getfem::mat_elem_product(getfem::mat_elem_grad(pf1),
				  getfem::mat_elem_grad(pf1)),
	 getfem::mat_elem_base(pf2));
      pmec = getfem::mat_elem(pme, pim, pgt);
      pf1prec = pf1; pf2prec = pf2; pgtprec = pgt; pimprec = pim;
    }
    pmec->gen_compute(t, mf.linked_mesh().points_of_convex(cv), cv);
    // cout << "t = " << t << endl;
  }
}

void test2_mat_elem(const getfem::mesh_im &mim, const getfem::mesh_fem &mf,
		   const getfem::mesh_fem &mfdata) { 
  
  size_type cv;
  dal::bit_vector nn = mf.convex_index();
  bgeot::base_tensor t;
  getfem::pfem pf1, pf2, pf1prec = 0, pf2prec = 0;
  getfem::pintegration_method pim, pimprec = 0;
  bgeot::pgeometric_trans pgt, pgtprec = 0;
  getfem::pmat_elem_computation pmec = 0;
  
  if (&(mf.linked_mesh()) != &(mfdata.linked_mesh()))
    GMM_ASSERT1(false,"This assembling procedure only works on a single mesh");
  
  for (cv << nn; cv != size_type(-1); cv << nn) {
    pf1 =     mf.fem_of_element(cv);
    pf2 = mfdata.fem_of_element(cv);
    pgt = mf.linked_mesh().trans_of_convex(cv);
    pim = mim.int_method_of_element(cv);
    if (pf1prec != pf1 || pf2prec != pf2 || pgtprec != pgt || pimprec != pim) {
      getfem::pmat_elem_type pme 
	= getfem::mat_elem_product(getfem::mat_elem_base(pf1),
				   getfem::mat_elem_base(pf1));
      pmec = getfem::mat_elem(pme, pim, pgt);
      pf1prec = pf1; pf2prec = pf2; pgtprec = pgt; pimprec = pim;
    }
    pmec->gen_compute_on_face(t, mf.linked_mesh().points_of_convex(cv), 0, cv);
  }
}


/**************************************************************************/
/*  main program.                                                         */
/**************************************************************************/

int main(int argc, char *argv[]) {
  
  try {
    
    lap_pb p;
    scalar_type exectime = gmm::uclock_sec(), total_time = 0.0;
    
    // cout << "initialisation ...\n";
    p.PARAM.read_command_line(argc, argv);
    p.init();
    // cout << "Initialisation terminee\n";
    
    total_time += gmm::uclock_sec() - exectime;
    
    
    
    exectime = gmm::uclock_sec();
    test1_mat_elem(p.mim, p.mef, p.mef_data);
    cout << "Mat elem computation time 1 : "
	 << gmm::uclock_sec() - exectime << endl;
 
    exectime = gmm::uclock_sec();
    test2_mat_elem(p.mim, p.mef, p.mef_data);
    cout << "Mat elem computation time 2 : "
	 << gmm::uclock_sec() - exectime << endl;


    /* check mesh/mesh_fem I/O */
    p.mef.write_to_file(p.datafilename + ".mesh", true);
    p.mesh.read_from_file(p.datafilename + ".mesh");
    p.mef.read_from_file(p.datafilename + ".mesh");
  }
  GMM_STANDARD_CATCH_ERROR;
  return 0; 
}
