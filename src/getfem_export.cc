/*===========================================================================

 Copyright (C) 2001-2020 Yves Renard

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

#include <iomanip>
#include "getfem/dal_singleton.h"
#include "getfem/bgeot_comma_init.h"
#include "getfem/getfem_export.h"

namespace getfem
{
  /* -------------------------------------------------------------
   * VTK export
   * ------------------------------------------------------------- */

  struct gf2vtk_dof_mapping : public std::vector<std::vector<unsigned> > {};
  struct gf2vtk_vtk_type : public std::vector<int> {};

  typedef enum { NO_VTK_MAPPING,
                 N1_TO_VTK_VERTEX,
                 N2_TO_VTK_LINE,
                 N3_TO_VTK_TRIANGLE,
                 N4_TO_VTK_PIXEL,
                 N4_TO_VTK_QUAD,
                 N4_TO_VTK_TETRA,
                 N8_TO_VTK_VOXEL,
                 N8_TO_VTK_HEXAHEDRON,
                 N6_TO_VTK_WEDGE,
                 N5_TO_VTK_PYRAMID,
                 N3_TO_VTK_QUADRATIC_EDGE,
                 N6_TO_VTK_QUADRATIC_TRIANGLE,
                 N8_TO_VTK_QUADRATIC_QUAD,
                 N10_TO_VTK_QUADRATIC_TETRA,
                 N20_TO_VTK_QUADRATIC_HEXAHEDRON,
                 N15_TO_VTK_QUADRATIC_WEDGE,
                 N13_TO_VTK_QUADRATIC_PYRAMID,
                 N14_TO_VTK_QUADRATIC_PYRAMID,
                 N9_TO_VTK_BIQUADRATIC_QUAD,
                 N27_TO_VTK_TRIQUADRATIC_HEXAHEDRON,
                 N18_TO_VTK_BIQUADRATIC_QUADRATIC_WEDGE } vtk_mapping_type;


  void init_gf2vtk() {
    // see https://www.kitware.com/products/books/VTKUsersGuide.pdf
    // and https://www.kitware.com/products/books/VTKTextbook.pdf
    // (there are some conflicts between the two)
    gf2vtk_dof_mapping &vtkmaps = dal::singleton<gf2vtk_dof_mapping>::instance();
    gf2vtk_vtk_type &vtktypes = dal::singleton<gf2vtk_vtk_type>::instance();
    vtkmaps.resize(25);
    vtktypes.resize(25);

    vtktypes[N1_TO_VTK_VERTEX] = vtk_export::VTK_VERTEX;
    vtkmaps [N1_TO_VTK_VERTEX] = {0};
    vtktypes[N2_TO_VTK_LINE] = vtk_export::VTK_LINE;
    vtkmaps [N2_TO_VTK_LINE] = {0, 1};
    vtktypes[N3_TO_VTK_TRIANGLE] = vtk_export::VTK_TRIANGLE;
    vtkmaps [N3_TO_VTK_TRIANGLE] = {0, 1, 2};
    vtktypes[N4_TO_VTK_PIXEL] = vtk_export::VTK_PIXEL;
    vtkmaps [N4_TO_VTK_PIXEL] = {0, 1, 2, 3};
    vtktypes[N4_TO_VTK_QUAD] = vtk_export::VTK_QUAD;
    vtkmaps [N4_TO_VTK_QUAD] = {0, 1, 3, 2};
    vtktypes[N4_TO_VTK_TETRA] = vtk_export::VTK_TETRA;
    vtkmaps [N4_TO_VTK_TETRA] = {0, 1, 2, 3};
    vtktypes[N8_TO_VTK_VOXEL] = vtk_export::VTK_VOXEL;
    vtkmaps [N8_TO_VTK_VOXEL] = {0, 1, 2, 3, 4, 5, 6, 7};
    vtktypes[N8_TO_VTK_HEXAHEDRON] = vtk_export::VTK_HEXAHEDRON;
    vtkmaps [N8_TO_VTK_HEXAHEDRON] = {0, 1, 3, 2, 4, 5, 7, 6};
    vtktypes[N6_TO_VTK_WEDGE] = vtk_export::VTK_WEDGE;
    vtkmaps [N6_TO_VTK_WEDGE] = {0, 1, 2, 3, 4, 5};
    vtktypes[N5_TO_VTK_PYRAMID] = vtk_export::VTK_PYRAMID;
    vtkmaps [N5_TO_VTK_PYRAMID] = {0, 1, 3, 2, 4};
    vtktypes[N3_TO_VTK_QUADRATIC_EDGE] = vtk_export::VTK_QUADRATIC_EDGE;
    vtkmaps [N3_TO_VTK_QUADRATIC_EDGE] = {0, 2, 1};
    vtktypes[N6_TO_VTK_QUADRATIC_TRIANGLE] = vtk_export::VTK_QUADRATIC_TRIANGLE;
    vtkmaps [N6_TO_VTK_QUADRATIC_TRIANGLE] = {0, 2, 5, 1, 4, 3};
    vtktypes[N8_TO_VTK_QUADRATIC_QUAD] = vtk_export::VTK_QUADRATIC_QUAD;
    vtkmaps [N8_TO_VTK_QUADRATIC_QUAD] = {0, 2, 7, 5, 1, 4, 6, 3};
    vtktypes[N10_TO_VTK_QUADRATIC_TETRA] = vtk_export::VTK_QUADRATIC_TETRA;
    vtkmaps [N10_TO_VTK_QUADRATIC_TETRA] = {0, 2, 5, 9, 1, 4, 3, 6, 7, 8};
    vtktypes[N20_TO_VTK_QUADRATIC_HEXAHEDRON] = vtk_export::VTK_QUADRATIC_HEXAHEDRON;
    vtkmaps [N20_TO_VTK_QUADRATIC_HEXAHEDRON] = {0, 2, 7, 5, 12, 14, 19, 17, 1, 4, 6, 3, 13, 16, 18, 15, 8, 9, 11, 10};
    vtktypes[N15_TO_VTK_QUADRATIC_WEDGE] = vtk_export::VTK_QUADRATIC_WEDGE;
    vtkmaps [N15_TO_VTK_QUADRATIC_WEDGE] = {0, 2, 5, 9, 11, 14, 1, 4, 3, 10, 13, 12, 6, 7, 8};
                                      // = {0, 5, 2, 9, 14, 11, 3, 4, 1, 12, 13, 10, 6, 8, 7};
    vtktypes[N13_TO_VTK_QUADRATIC_PYRAMID] = vtk_export::VTK_QUADRATIC_PYRAMID;
    vtkmaps [N13_TO_VTK_QUADRATIC_PYRAMID] = {0, 2, 7, 5, 12, 1, 4, 6, 3, 8, 9, 11, 10};
    vtktypes[N14_TO_VTK_QUADRATIC_PYRAMID] = vtk_export::VTK_QUADRATIC_PYRAMID;
    vtkmaps [N14_TO_VTK_QUADRATIC_PYRAMID] = {0, 2, 8, 6, 13, 1, 5, 7, 3, 9, 10, 12, 11};
    vtktypes[N9_TO_VTK_BIQUADRATIC_QUAD] = vtk_export::VTK_BIQUADRATIC_QUAD;
    vtkmaps [N9_TO_VTK_BIQUADRATIC_QUAD] = {0, 2, 8, 6, 1, 5, 7, 3, 4};
    vtktypes[N27_TO_VTK_TRIQUADRATIC_HEXAHEDRON] = vtk_export::VTK_TRIQUADRATIC_HEXAHEDRON;
    vtkmaps [N27_TO_VTK_TRIQUADRATIC_HEXAHEDRON] = {0, 2, 8, 6, 18, 20, 26, 24, 1, 5, 7, 3, 19, 23, 25, 21, 9, 11, 17, 15, 12, 14, 10, 16, 4, 22};
    vtktypes[N18_TO_VTK_BIQUADRATIC_QUADRATIC_WEDGE] = vtk_export::VTK_BIQUADRATIC_QUADRATIC_WEDGE;
    vtkmaps [N18_TO_VTK_BIQUADRATIC_QUADRATIC_WEDGE]  = {0, 2, 5, 12, 14, 17, 1, 4, 3, 13, 16, 15, 6, 8, 11, 7, 10, 9};
  }

  static const std::vector<unsigned> &
  select_vtk_dof_mapping(unsigned t) {
    gf2vtk_dof_mapping &vtkmaps = dal::singleton<gf2vtk_dof_mapping>::instance();
    if (vtkmaps.size() == 0) init_gf2vtk();
    return vtkmaps[t];
  }

  int select_vtk_type(unsigned t) {
    gf2vtk_vtk_type &vtktypes = dal::singleton<gf2vtk_vtk_type>::instance();
    if (vtktypes.size() == 0) init_gf2vtk();
    return vtktypes[t];
  }

  vtk_export::vtk_export(std::ostream &os_, bool ascii_)
    : os(os_), ascii(ascii_) { init(); }

  vtk_export::vtk_export(const std::string& fname, bool ascii_)
    : os(real_os), ascii(ascii_),
    real_os(fname.c_str(), !ascii ? std::ios_base::binary | std::ios_base::out :
                                    std::ios_base::out) {
    GMM_ASSERT1(real_os, "impossible to write to vtk file '" << fname << "'");
    init();
  }

  void vtk_export::init() {
    static int test_endian = 0x01234567;
    strcpy(header, "Exported by getfem++");
    psl = 0; dim_ = dim_type(-1);
    if (*((char*)&test_endian) == 0x67)
      reverse_endian = true;
    else reverse_endian = false;
    state = EMPTY;
  }

  void vtk_export::switch_to_cell_data() {
    if (state != IN_CELL_DATA) {
      state = IN_CELL_DATA;
      write_separ();
      if (psl) {
        os << "CELL_DATA " << psl->nb_simplexes(0) + psl->nb_simplexes(1)
         + psl->nb_simplexes(2) + psl->nb_simplexes(3) << "\n";
      } else {
        os << "CELL_DATA " << pmf->convex_index().card() << "\n";
      }
      write_separ();
    }
  }

  void vtk_export::switch_to_point_data() {
    if (state != IN_POINT_DATA) {
      state = IN_POINT_DATA;
      write_separ();
      if (psl) {
        write_separ(); os << "POINT_DATA " << psl->nb_points() << "\n";
      } else {
        os << "POINT_DATA " << pmf_dof_used.card() << "\n";
      }
      write_separ();
    }
  }


  /* try to check if a quad or hexahedric cell is "rectangular" and oriented
     along the axes */
  template<typename C> static bool check_voxel(const C& c) {
    scalar_type h[3];
    unsigned N = c[0].size();
    if (c.size() != (1U << N)) return false;
    const base_node P0 = c[0];
    h[0] = c[1][0] - P0[0];
    h[1] = c[2][0] - P0[0];
    if (c.size() != 4) h[2] = c[4][0] - P0[0];
    for (unsigned i=1; i < c.size(); ++i) {
      const base_node d = c[i] - P0;
      for (unsigned j=0; j < N; ++j)
        if (gmm::abs(d[j]) > 1e-7*h[j] && gmm::abs(d[j] - h[j]) > 1e-7*h[j])
          return false;
    }
    return true;
  }


  void vtk_export::exporting(const stored_mesh_slice& sl) {
    psl = &sl; dim_ = dim_type(sl.dim());
    GMM_ASSERT1(psl->dim() <= 3, "attempt to export a " << int(dim_)
              << "D slice (not supported)");
  }

  void vtk_export::exporting(const mesh& m) {
    dim_ = m.dim();
    GMM_ASSERT1(dim_ <= 3, "attempt to export a " << int(dim_)
              << "D mesh (not supported)");
    pmf = std::make_unique<mesh_fem>(const_cast<mesh&>(m), dim_type(1));
    for (dal::bv_visitor cv(m.convex_index()); !cv.finished(); ++cv) {
      bgeot::pgeometric_trans pgt = m.trans_of_convex(cv);
      pfem pf = getfem::classical_fem(pgt, pgt->complexity() > 1 ? 2 : 1);
      pmf->set_finite_element(cv, pf);
    }
    exporting(*pmf);
  }

  void vtk_export::exporting(const mesh_fem& mf) {
    dim_ = mf.linked_mesh().dim();
    GMM_ASSERT1(dim_ <= 3, "attempt to export a " << int(dim_)
              << "D mesh_fem (not supported)");
    if (&mf != pmf.get())
      pmf = std::make_unique<mesh_fem>(mf.linked_mesh());
    /* initialize pmf with finite elements suitable for VTK (which only knows
       isoparametric FEMs of order 1 and 2) */
    for (dal::bv_visitor cv(mf.convex_index()); !cv.finished(); ++cv) {
      bgeot::pgeometric_trans pgt = mf.linked_mesh().trans_of_convex(cv);
      pfem pf = mf.fem_of_element(cv);

      if (pf == fem_descriptor("FEM_Q2_INCOMPLETE(2)") ||
          pf == fem_descriptor("FEM_Q2_INCOMPLETE(3)") ||
          pf == fem_descriptor("FEM_PYRAMID_Q2_INCOMPLETE") ||
          pf == fem_descriptor("FEM_PYRAMID_Q2_INCOMPLETE_DISCONTINUOUS") ||
          pf == fem_descriptor("FEM_PRISM_INCOMPLETE_P2") ||
          pf == fem_descriptor("FEM_PRISM_INCOMPLETE_P2_DISCONTINUOUS"))
        pmf->set_finite_element(cv, pf);
      else {
        bool discontinuous = false;
        for (unsigned i=0; i < pf->nb_dof(cv); ++i) {
          /* could be a better test for discontinuity .. */
          if (!dof_linkable(pf->dof_types()[i])) { discontinuous = true; break; }
        }

        pfem classical_pf1 = discontinuous ? classical_discontinuous_fem(pgt, 1)
                                           : classical_fem(pgt, 1);

        short_type degree = 1;
        if ((pf != classical_pf1 && pf->estimated_degree() > 1) ||
            pgt->structure() != pgt->basic_structure())
          degree = 2;

        pmf->set_finite_element(cv, discontinuous ?
                                classical_discontinuous_fem(pgt, degree, 0, true) :
                                classical_fem(pgt, degree, true));
      }
    }
    /* find out which dof will be exported to VTK */

    const mesh &m = pmf->linked_mesh();
    pmf_mapping_type.resize(pmf->convex_index().last_true() + 1, unsigned(-1));
    pmf_dof_used.sup(0, pmf->nb_basic_dof());
    for (dal::bv_visitor cv(pmf->convex_index()); !cv.finished(); ++cv) {
      vtk_mapping_type t = NO_VTK_MAPPING;
      size_type nbd = pmf->fem_of_element(cv)->nb_dof(cv);
      switch (pmf->fem_of_element(cv)->dim()) {
      case 0: t = N1_TO_VTK_VERTEX; break;
      case 1:
        if (nbd == 2) t = N2_TO_VTK_LINE;
        else if (nbd == 3) t = N3_TO_VTK_QUADRATIC_EDGE;
        break;
      case 2:
        if (nbd == 3) t = N3_TO_VTK_TRIANGLE;
        else if (nbd == 4)
          t = check_voxel(m.points_of_convex(cv)) ? N4_TO_VTK_PIXEL
                                                  : N4_TO_VTK_QUAD;
        else if (nbd == 6) t = N6_TO_VTK_QUADRATIC_TRIANGLE;
        else if (nbd == 8) t = N8_TO_VTK_QUADRATIC_QUAD;
        else if (nbd == 9) t = N9_TO_VTK_BIQUADRATIC_QUAD;
        break;
      case 3:
        if (nbd == 4) t = N4_TO_VTK_TETRA;
        else if (nbd == 10) t = N10_TO_VTK_QUADRATIC_TETRA;
        else if (nbd == 8)
          t = check_voxel(m.points_of_convex(cv)) ? N8_TO_VTK_VOXEL
                                                  : N8_TO_VTK_HEXAHEDRON;
        else if (nbd == 20) t = N20_TO_VTK_QUADRATIC_HEXAHEDRON;
        else if (nbd == 27) t = N27_TO_VTK_TRIQUADRATIC_HEXAHEDRON;
        else if (nbd == 5) t = N5_TO_VTK_PYRAMID;
        else if (nbd == 13) t = N13_TO_VTK_QUADRATIC_PYRAMID;
        else if (nbd == 14) t = N14_TO_VTK_QUADRATIC_PYRAMID;
        else if (nbd == 6) t = N6_TO_VTK_WEDGE;
        else if (nbd == 15) t = N15_TO_VTK_QUADRATIC_WEDGE;
        else if (nbd == 18) t = N18_TO_VTK_BIQUADRATIC_QUADRATIC_WEDGE;
        break;
      }
      GMM_ASSERT1(t != -1, "semi internal error. Could not map " <<
                  name_of_fem(pmf->fem_of_element(cv))
                << " to a VTK cell type");
      pmf_mapping_type[cv] = t;

      const std::vector<unsigned> &dmap = select_vtk_dof_mapping(t);
      //cout << "nbd = " << nbd << ", t = " << t << ", dmap = "<<dmap << "\n";
      GMM_ASSERT1(dmap.size() <= pmf->nb_basic_dof_of_element(cv),
                "inconsistency in vtk_dof_mapping");
      for (unsigned i=0; i < dmap.size(); ++i)
        pmf_dof_used.add(pmf->ind_basic_dof_of_element(cv)[dmap[i]]);
    }
    // cout << "mf.nb_dof = " << mf.nb_dof() << ", pmf->nb_dof="
    //      << pmf->nb_dof() << ", dof_used = " << pmf_dof_used.card() << "\n";
  }


  const stored_mesh_slice& vtk_export::get_exported_slice() const {
    GMM_ASSERT1(psl, "no slice!");
    return *psl;
  }

  const mesh_fem& vtk_export::get_exported_mesh_fem() const {
    GMM_ASSERT1(pmf.get(), "no mesh_fem!");
    return *pmf;
  }

  void vtk_export::set_header(const std::string& s)
  {
    strncpy(header, s.c_str(), 256);
    header[255] = 0;
  }

  void vtk_export::check_header() {
    if (state >= HEADER_WRITTEN) return;
    os << "# vtk DataFile Version 2.0\n";
    os << header << "\n";
    if (ascii) os << "ASCII\n"; else os << "BINARY\n";
    state = HEADER_WRITTEN;
  }

  void vtk_export::write_separ()
  { if (ascii) os << "\n"; }

  void vtk_export::write_mesh() {
    if (psl) write_mesh_structure_from_slice();
    else write_mesh_structure_from_mesh_fem();
  }

  /* export the slice data as an unstructured mesh composed of simplexes */
  void vtk_export::write_mesh_structure_from_slice() {
    /* element type code for (linear) simplexes of dimensions 0,1,2,3 in VTK */
    static int vtk_simplex_code[4] = { VTK_VERTEX, VTK_LINE, VTK_TRIANGLE, VTK_TETRA };
    if (state >= STRUCTURE_WRITTEN) return;
    check_header();
    /* possible improvement: detect structured grids */
    os << "DATASET UNSTRUCTURED_GRID\n";
    os << "POINTS " << psl->nb_points() << " float\n";
    /*
       points are not merge, vtk is mostly fine with that (except for
       transparency and normals at vertices of 3D elements: all simplex faces
       are considered to be "boundary" faces by vtk)
    */
    for (size_type ic=0; ic < psl->nb_convex(); ++ic) {
      for (size_type i=0; i < psl->nodes(ic).size(); ++i)
       write_vec(psl->nodes(ic)[i].pt.begin(),psl->nodes(ic)[i].pt.size());
      write_separ();
    }
    /* count total number of simplexes, and total number of entries
     * in the CELLS section */
    size_type cells_cnt = 0, splx_cnt = 0;
    for (size_type ic=0; ic < psl->nb_convex(); ++ic) {
      for (size_type i=0; i < psl->simplexes(ic).size(); ++i)
       cells_cnt += psl->simplexes(ic)[i].dim() + 2;
      splx_cnt += psl->simplexes(ic).size();
    }
    size_type nodes_cnt = 0;
    write_separ(); os << "CELLS " << splx_cnt << " " << cells_cnt << "\n";
    for (size_type ic=0; ic < psl->nb_convex(); ++ic) {
      const getfem::mesh_slicer::cs_simplexes_ct& s = psl->simplexes(ic);
      for (size_type i=0; i < s.size(); ++i) {
       write_val(int(s[i].dim()+1));
       for (size_type j=0; j < s[i].dim()+1; ++j)
         write_val(int(s[i].inodes[j] + nodes_cnt));
       write_separ();
      }
      nodes_cnt += psl->nodes(ic).size();
    }
    assert(nodes_cnt == psl->nb_points()); // sanity check
    write_separ(); os << "CELL_TYPES " << splx_cnt << "\n";
    for (size_type ic=0; ic < psl->nb_convex(); ++ic) {
      const getfem::mesh_slicer::cs_simplexes_ct& s = psl->simplexes(ic);
      for (size_type i=0; i < s.size(); ++i) {
       write_val(int(vtk_simplex_code[s[i].dim()]));
      }
      write_separ();
      splx_cnt -= s.size();
    }
    assert(splx_cnt == 0); // sanity check
    state = STRUCTURE_WRITTEN;
  }


  void vtk_export::write_mesh_structure_from_mesh_fem() {
    if (state >= STRUCTURE_WRITTEN) return;
    check_header();
    /* possible improvement: detect structured grids */
    os << "DATASET UNSTRUCTURED_GRID\n";
    os << "POINTS " << pmf_dof_used.card() << " float\n";
    std::vector<int> dofmap(pmf->nb_dof());
    int cnt = 0;
    for (dal::bv_visitor d(pmf_dof_used); !d.finished(); ++d) {
      dofmap[d] = cnt++;
      base_node P = pmf->point_of_basic_dof(d);
      write_vec(P.const_begin(),P.size());
      write_separ();
    }

    size_type nb_cell_values = 0;
    for (dal::bv_visitor cv(pmf->convex_index()); !cv.finished(); ++cv)
      nb_cell_values += (1 + select_vtk_dof_mapping(pmf_mapping_type[cv]).size());

    write_separ(); os << "CELLS " << pmf->convex_index().card() << " " << nb_cell_values << "\n";

    for (dal::bv_visitor cv(pmf->convex_index()); !cv.finished(); ++cv) {
      const std::vector<unsigned> &dmap = select_vtk_dof_mapping(pmf_mapping_type[cv]);
      write_val(int(dmap.size()));
      for (size_type i=0; i < dmap.size(); ++i)
        write_val(int(dofmap[pmf->ind_basic_dof_of_element(cv)[dmap[i]]]));
      write_separ();
    }

    write_separ(); os << "CELL_TYPES " << pmf->convex_index().card() << "\n";
    for (dal::bv_visitor cv(pmf->convex_index()); !cv.finished(); ++cv) {
      write_val(select_vtk_type(pmf_mapping_type[cv]));
      write_separ();
    }

    state = STRUCTURE_WRITTEN;
  }

  void vtk_export::write_mesh_quality(const mesh &m) {
    if (psl) {
      mesh_fem mf(const_cast<mesh&>(m),1);
      mf.set_classical_finite_element(0);
      std::vector<scalar_type> q(mf.nb_dof());
      for (size_type d=0; d < mf.nb_dof(); ++d) {
        q[d] = m.convex_quality_estimate(mf.first_convex_of_basic_dof(d));
      }
      write_point_data(mf, q, "convex_quality");
    } else {
      std::vector<scalar_type> q(pmf->convex_index().card());
      for (dal::bv_visitor cv(pmf->convex_index()); !cv.finished(); ++cv) {
        q[cv] = m.convex_quality_estimate(cv);
      }
      write_cell_data(q, "convex_quality");
    }
  }

  vtu_export::vtu_export(std::ostream &os_, bool ascii_)
    : os(os_), ascii(ascii_) { init(); }

  vtu_export::vtu_export(const std::string& fname, bool ascii_)
    : os(real_os), ascii(ascii_),
    real_os(fname.c_str(), !ascii ? std::ios_base::binary | std::ios_base::out :
                                    std::ios_base::out) {
    GMM_ASSERT1(real_os, "impossible to write to vtu file '" << fname << "'");
    init();
  }

  void vtu_export::init() {
    strcpy(header, "Exported by getfem++");
    state = EMPTY;
  }

  void vtu_export::exporting(const mesh& m) {
    dim_ = m.dim();
    GMM_ASSERT1(dim_ <= 3, "attempt to export a " << int(dim_)
              << "D mesh (not supported)");
    pmf = std::make_unique<mesh_fem>(const_cast<mesh&>(m), dim_type(1));
    for (dal::bv_visitor cv(m.convex_index()); !cv.finished(); ++cv) {
      bgeot::pgeometric_trans pgt = m.trans_of_convex(cv);
      pfem pf = getfem::classical_fem(pgt, pgt->complexity() > 1 ? 2 : 1);
      pmf->set_finite_element(cv, pf);
    }
    exporting(*pmf);
  }

  void vtu_export::exporting(const mesh_fem& mf) {
    dim_ = mf.linked_mesh().dim();
    GMM_ASSERT1(dim_ <= 3, "attempt to export a " << int(dim_)
              << "D mesh_fem (not supported)");
    if (&mf != pmf.get())
      pmf = std::make_unique<mesh_fem>(mf.linked_mesh());
    /* initialize pmf with finite elements suitable for VTK (which only knows
       isoparametric FEMs of order 1 and 2) */
    for (dal::bv_visitor cv(mf.convex_index()); !cv.finished(); ++cv) {
      bgeot::pgeometric_trans pgt = mf.linked_mesh().trans_of_convex(cv);
      pfem pf = mf.fem_of_element(cv);

      if (pf == fem_descriptor("FEM_Q2_INCOMPLETE(2)") ||
          pf == fem_descriptor("FEM_Q2_INCOMPLETE(3)") ||
          pf == fem_descriptor("FEM_PYRAMID_Q2_INCOMPLETE") ||
          pf == fem_descriptor("FEM_PYRAMID_Q2_INCOMPLETE_DISCONTINUOUS") ||
          pf == fem_descriptor("FEM_PRISM_INCOMPLETE_P2") ||
          pf == fem_descriptor("FEM_PRISM_INCOMPLETE_P2_DISCONTINUOUS"))
        pmf->set_finite_element(cv, pf);
      else {
        bool discontinuous = false;
        for (unsigned i=0; i < pf->nb_dof(cv); ++i) {
          /* could be a better test for discontinuity .. */
          if (!dof_linkable(pf->dof_types()[i])) { discontinuous = true; break; }
        }

        pfem classical_pf1 = discontinuous ? classical_discontinuous_fem(pgt, 1)
                                           : classical_fem(pgt, 1);

        short_type degree = 1;
        if ((pf != classical_pf1 && pf->estimated_degree() > 1) ||
            pgt->structure() != pgt->basic_structure())
          degree = 2;

        pmf->set_finite_element(cv, discontinuous ?
                                classical_discontinuous_fem(pgt, degree, 0, true) :
                                classical_fem(pgt, degree, true));
      }
    }
    /* find out which dof will be exported to VTK */

    const mesh &m = pmf->linked_mesh();
    pmf_mapping_type.resize(pmf->convex_index().last_true() + 1, unsigned(-1));
    pmf_dof_used.sup(0, pmf->nb_basic_dof());
    for (dal::bv_visitor cv(pmf->convex_index()); !cv.finished(); ++cv) {
      vtk_mapping_type t = NO_VTK_MAPPING;
      size_type nbd = pmf->fem_of_element(cv)->nb_dof(cv);
      switch (pmf->fem_of_element(cv)->dim()) {
      case 0: t = N1_TO_VTK_VERTEX; break;
      case 1:
        if (nbd == 2) t = N2_TO_VTK_LINE;
        else if (nbd == 3) t = N3_TO_VTK_QUADRATIC_EDGE;
        break;
      case 2:
        if (nbd == 3) t = N3_TO_VTK_TRIANGLE;
        else if (nbd == 4)
          t = check_voxel(m.points_of_convex(cv)) ? N4_TO_VTK_PIXEL
                                                  : N4_TO_VTK_QUAD;
        else if (nbd == 6) t = N6_TO_VTK_QUADRATIC_TRIANGLE;
        else if (nbd == 8) t = N8_TO_VTK_QUADRATIC_QUAD;
        else if (nbd == 9) t = N9_TO_VTK_BIQUADRATIC_QUAD;
        break;
      case 3:
        if (nbd == 4) t = N4_TO_VTK_TETRA;
        else if (nbd == 10) t = N10_TO_VTK_QUADRATIC_TETRA;
        else if (nbd == 8)
          t = check_voxel(m.points_of_convex(cv)) ? N8_TO_VTK_VOXEL
                                                  : N8_TO_VTK_HEXAHEDRON;
        else if (nbd == 20) t = N20_TO_VTK_QUADRATIC_HEXAHEDRON;
        else if (nbd == 27) t = N27_TO_VTK_TRIQUADRATIC_HEXAHEDRON;
        else if (nbd == 5) t = N5_TO_VTK_PYRAMID;
        else if (nbd == 13) t = N13_TO_VTK_QUADRATIC_PYRAMID;
        else if (nbd == 14) t = N14_TO_VTK_QUADRATIC_PYRAMID;
        else if (nbd == 6) t = N6_TO_VTK_WEDGE;
        else if (nbd == 15) t = N15_TO_VTK_QUADRATIC_WEDGE;
        else if (nbd == 18) t = N18_TO_VTK_BIQUADRATIC_QUADRATIC_WEDGE;
        break;
      }
      GMM_ASSERT1(t != -1, "semi internal error. Could not map " <<
                  name_of_fem(pmf->fem_of_element(cv))
                << " to a VTK cell type");
      pmf_mapping_type[cv] = t;

      const std::vector<unsigned> &dmap = select_vtk_dof_mapping(t);
      //cout << "nbd = " << nbd << ", t = " << t << ", dmap = "<<dmap << "\n";
      GMM_ASSERT1(dmap.size() <= pmf->nb_basic_dof_of_element(cv),
                "inconsistency in vtk_dof_mapping");
      for (unsigned i=0; i < dmap.size(); ++i)
        pmf_dof_used.add(pmf->ind_basic_dof_of_element(cv)[dmap[i]]);
    }
    // cout << "mf.nb_dof = " << mf.nb_dof() << ", pmf->nb_dof="
    //      << pmf->nb_dof() << ", dof_used = " << pmf_dof_used.card() << "\n";
  }

  void vtu_export::check_header() {
    if (state >= HEADER_WRITTEN) return;
    os << "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"BigEndian\">\n";
    state = HEADER_WRITTEN;
  }

  void vtu_export::check_footer() {
    if (state >= FOOTER_WRITTEN) return;
    os << "</VTKFile>\n";
    state = FOOTER_WRITTEN;
  }

  void vtu_export::write_separ()
  { if (ascii) os << "\n"; }

  void vtu_export::write_mesh() {
    write_mesh_structure_from_mesh_fem();
  }

  void vtu_export::write_mesh_structure_from_mesh_fem() {
    if (state >= STRUCTURE_WRITTEN) return;
    check_header();
    os << "<UnstructuredGrid>\n";
    os << "<Piece \"NumberOfPoints=\"" << pmf_dof_used.card() << "\"NumberOfCells=\"" << pmf->convex_index().card() << "\n";
    os << "<Points>\n";
    os << "<DataArray type=\"Float32\" NumberOfComponents=\"" << dim_<< "\" Format=\"ascii\">\n"; 
    std::vector<int> dofmap(pmf->nb_dof());
    int cnt = 0;
    for (dal::bv_visitor d(pmf_dof_used); !d.finished(); ++d) {
      dofmap[d] = cnt++;
      base_node P = pmf->point_of_basic_dof(d);
      write_vec(P.const_begin(),P.size());
      write_separ();
    }
    size_type nb_cell_values = 0;
    for (dal::bv_visitor cv(pmf->convex_index()); !cv.finished(); ++cv)
      nb_cell_values += (1 + select_vtk_dof_mapping(pmf_mapping_type[cv]).size());
    os << "</DataArray>\n";
    os << "</Points>\n";
    os << "<Cells>\n";
    os << "<DataArray type=\"Int32\" Name=\"connectivity\" Format=\"ascii\">\n";
    for (dal::bv_visitor cv(pmf->convex_index()); !cv.finished(); ++cv) {
      const std::vector<unsigned> &dmap = select_vtk_dof_mapping(pmf_mapping_type[cv]);
      for (size_type i=0; i < dmap.size(); ++i)
        write_val(int(dofmap[pmf->ind_basic_dof_of_element(cv)[dmap[i]]]));
      write_separ();
    }
    os << "</DataArray>\n";
    os << "<DataArray type=\"Int32\" Name=\"types\" Format=\"ascii\">\n";
    for (dal::bv_visitor cv(pmf->convex_index()); !cv.finished(); ++cv) {
      write_val(select_vtk_type(pmf_mapping_type[cv]));
      write_separ();
    }
    os << "</DataArray>\n";
    os << "</Cells>\n";
    os << "</Piece>\n";
    os << "</UnstructuredGrid>\n";
    check_footer();

    state = STRUCTURE_WRITTEN;
  }

  /* -------------------------------------------------------------
   * OPENDX export
   * ------------------------------------------------------------- */

  dx_export::dx_export(std::ostream &os_, bool ascii_)
    : os(os_), ascii(ascii_) { init(); }

  dx_export::dx_export(const std::string& fname, bool ascii_, bool append_)
    : os(real_os), ascii(ascii_) {
    real_os.open(fname.c_str(),
                 std::ios_base::openmode(std::ios_base::in |
                                    std::ios_base::out |
                  (append_ ? std::ios_base::ate : std::ios_base::trunc)));
    GMM_ASSERT1(real_os.good(), "impossible to write to dx file '"
              << fname << "'");
    init();
    if (append_) { reread_metadata(); header_written = true; }
  }

  dx_export::~dx_export() {
    std::ios::pos_type p = os.tellp();
    write_series();
    os << "\n# --end of getfem export\nend\n";
    update_metadata(p);
  }

  void dx_export::init() {
    strcpy(header, "Exported by getfem++");
    psl = 0; dim_ = dim_type(-1); connections_dim = dim_type(-1);
    psl_use_merged = false;
    header_written = false;
  }

  void dx_export::write_separ()
  { if (ascii) os << "\n"; }

  template<typename T> static typename std::list<T>::iterator
  get_from_name(std::list<T> &c,
              const std::string& name, bool raise_error) {
    for (typename std::list<T>::iterator it = c.begin();
        it != c.end(); ++it) {
      if (it->name == name) return it;
    }
    GMM_ASSERT1(!raise_error, "object not found in dx file: " << name);
    return c.end();
  }

  std::list<dx_export::dxMesh>::iterator
  dx_export::get_mesh(const std::string& name, bool raise_error) {
    return get_from_name(meshes,name,raise_error);
  }
  std::list<dx_export::dxObject>::iterator
  dx_export::get_object(const std::string& name, bool raise_error) {
    return get_from_name(objects,name,raise_error);
  }


  bool dx_export::new_mesh(std::string &name) {
    name = default_name(name, int(meshes.size()), "mesh");
    std::list<dxMesh>::iterator it = get_mesh(name, false);
    if (it != meshes.end()) {
      if (&(*it) != &current_mesh())
       std::swap(current_mesh(),*it);
      return false;
    } else {
      meshes.push_back(dxMesh()); meshes.back().name = name;
      return true;
    }
  }

  void dx_export::exporting(const stored_mesh_slice& sl, bool merge_points,
                         std::string name) {
    if (!new_mesh(name)) return;
    psl_use_merged = merge_points;
    if (merge_points) sl.merge_nodes();
    psl = &sl; dim_ = dim_type(sl.dim());
    GMM_ASSERT1(psl->dim() <= 3, "4D slices and more are not supported");
    for (dim_type d = 0; d <= psl->dim(); ++d) {
      if (psl->nb_simplexes(d)) {
        if (connections_dim == dim_type(-1)) connections_dim = d;
        else GMM_ASSERT1(false, "Cannot export a slice containing "
                      "simplexes of different dimensions");
      }
    }
    GMM_ASSERT1(connections_dim != dim_type(-1), "empty slice!");
  }


  void dx_export::exporting(const mesh_fem& mf, std::string name) {
    name = default_name(name, int(meshes.size()), "mesh");
    if (!new_mesh(name)) return;
    const mesh &m = mf.linked_mesh();
    GMM_ASSERT1(mf.linked_mesh().convex_index().card() != 0,
              "won't export an empty mesh");

    dim_ = m.dim();
    GMM_ASSERT1(dim_ <= 3, "4D meshes and more are not supported");
    if (&mf != pmf.get())
      pmf = std::make_unique<mesh_fem>(const_cast<mesh&>(m), dim_type(1));
    bgeot::pgeometric_trans pgt = m.trans_of_convex(m.convex_index().first_true());
    GMM_ASSERT1(dxname_of_convex_structure
              (basic_structure(pgt->structure())) != 0,
              "DX Cannot handle " <<
              bgeot::name_of_geometric_trans(pgt) << ", use slices");
    /* initialize pmf with finite elements suitable for OpenDX */
    for (dal::bv_visitor cv(mf.convex_index()); !cv.finished(); ++cv) {
      bgeot::pgeometric_trans pgt2 = mf.linked_mesh().trans_of_convex(cv);
      GMM_ASSERT1(basic_structure(pgt->structure()) ==
                  basic_structure(pgt2->structure()),
                  "Cannot export this mesh to opendx, it contains "
                  "different convex types. Slice it first.");
      pfem pf = mf.fem_of_element(cv);
      bool discontinuous = false;
      for (unsigned i=0; i < pf->nb_dof(cv); ++i) {
        /* could be a better test for discontinuity .. */
        if (!dof_linkable(pf->dof_types()[i])) { discontinuous = true; break; }
      }
      pfem classical_pf1 = discontinuous ? classical_discontinuous_fem(pgt, 1)
                                         : classical_fem(pgt, 1);
      pmf->set_finite_element(cv, classical_pf1);
    }
    pmf_dof_used.add(0, pmf->nb_basic_dof());
    connections_dim = dim_type(pmf->nb_basic_dof_of_element(m.convex_index().first_true()));
  }

  void dx_export::exporting(const mesh& m, std::string name) {
    dim_ = m.dim();
    GMM_ASSERT1(dim_ <= 3, "4D meshes and more are not supported");
    pmf = std::make_unique<mesh_fem>(const_cast<mesh&>(m), dim_type(1));
    pmf->set_classical_finite_element(1);
    exporting(*pmf, name);
  }

  void dx_export::write_series() {
    for (std::list<dxSeries>::const_iterator it = series.begin();
        it != series.end(); ++it) {
      if (it->members.size() == 0) continue;
      size_type count = 0;
      os << "\nobject  \"" << it->name << "\" class series\n";
      for (std::list<std::string>::const_iterator ito = it->members.begin();
          ito != it->members.end(); ++ito, ++count) {
       os << "  member  " << count << " \"" << (*ito) << "\"\n";
      }
    }
  }

  void dx_export::serie_add_object_(const std::string &serie_name,
                               const std::string &object_name) {
    std::list<dxSeries>::iterator it = series.begin();
    while (it != series.end() && it->name != serie_name) ++it;
    if (it == series.end()) {
      series.push_back(dxSeries()); it = series.end(); --it;
      it->name = serie_name;
    }
    it->members.push_back(object_name);
  }

  void dx_export::serie_add_object(const std::string &serie_name,
                               const std::string &object_name) {
    /* create a series for edge data if possible (the cost is null
     and it may be useful) */
    std::list<dxObject>::iterator ito = get_object(object_name, false);
    if (ito != objects.end()) {
      std::list<dxMesh>::iterator itm = get_mesh(ito->mesh);
      if (itm != meshes.end() && (itm->flags & dxMesh::WITH_EDGES)) {
       serie_add_object_(serie_name + "_edges",
                      object_name + "_edges");
      }
    }
    /* fill the real serie */
    serie_add_object_(serie_name, object_name);
  }

  void dx_export::set_header(const std::string& s)
  { strncpy(header, s.c_str(), 256); header[255] = 0; }

  void dx_export::check_header() {
    if (header_written) return;
    header_written = true;
    os << "# data file for IBM OpenDX, generated by GetFem++ v "
       << GETFEM_VERSION << "\n";
    os << "# " << header << "\n";
  }

  void dx_export::update_metadata(std::ios::pos_type pos_series) {
    os.seekp(0,std::ios::end);
    os << "# This file contains the following objects\n";
    std::ios::pos_type pos_end = os.tellp();
    for (std::list<dxSeries>::const_iterator it = series.begin();
        it != series.end(); ++it) {
      os << "#S \"" << it->name << "\" which contains:\n";
      for (std::list<std::string>::const_iterator its = it->members.begin();
           its != it->members.end(); ++its)
        os << "#+   \"" << *its << "\"\n";
    }
    for (std::list<dxObject>::const_iterator it = objects.begin();
        it != objects.end(); ++it) {
      os << "#O \"" << it->name << "\" \"" << it->mesh << "\"\n";
    }
    for (std::list<dxMesh>::const_iterator it = meshes.begin();
        it != meshes.end(); ++it) {
      os << "#M \"" << it->name << "\" " << it->flags << "\n";
    }
    os << "#E \"THE_END\" " << std::setw(20) << pos_series << std::setw(20) << pos_end << "\n";
  }

  void dx_export::reread_metadata() {
    char line[512];
    real_os.seekg(0, std::ios::end);
    int count=0; char c;
    unsigned long lu_end, lu_series;
    do {
      real_os.seekg(-1, std::ios::cur);
      c = char(real_os.peek());
    } while (++count < 512 && c != '#');
    real_os.getline(line, sizeof line);
    if (sscanf(line, "#E \"THE_END\" %lu %lu", &lu_series, &lu_end) != 2)
      GMM_ASSERT1(false, "this file was not generated by getfem, "
                "cannot append data to it!\n");
    real_os.seekg(lu_end, std::ios::beg);
    do {
      char name[512]; unsigned n;
      int pos;
      real_os.getline(line, sizeof line);
      if (sscanf(line, "#%c \"%512[^\"]\"%n", &c, name, &pos) < 1)
        GMM_ASSERT1(false, "corrupted file! your .dx file is broken\n");
      if (c == 'S') {
        series.push_back(dxSeries()); series.back().name = name;
      } else if (c == '+') {
        series.back().members.push_back(name);
      } else if (c == 'O') {
        objects.push_back(dxObject()); objects.back().name = name;
        sscanf(line+pos, " \"%512[^\"]\"", name); objects.back().mesh = name;
      } else if (c == 'M') {
        meshes.push_back(dxMesh()); meshes.back().name = name;
        sscanf(line+pos, "%u", &n); meshes.back().flags = n;
      } else if (c == 'E') {
        break;
      } else GMM_ASSERT1(false, "corrupted file! your .dx file is broken\n");
    } while (1);
    real_os.seekp(lu_series, std::ios::beg);
  }

  void dx_export::write_convex_attributes(bgeot::pconvex_structure cvs) {
    const char *s_elem_type = dxname_of_convex_structure(cvs);
    if (!s_elem_type)
      GMM_WARNING1("OpenDX won't handle this kind of convexes");
    os << "\n  attribute \"element type\" string \"" << s_elem_type << "\"\n"
       << "  attribute \"ref\" string \"positions\"\n\n";
  }

  const char *dx_export::dxname_of_convex_structure(bgeot::pconvex_structure cvs) {
    const char *s_elem_type = 0;
    switch (cvs->dim()) {
      /* TODO: do something for point data */
      case 1: s_elem_type = "lines"; break;
      case 2:
       if (cvs->nb_points() == 3)
         s_elem_type = "triangles";
       else if (cvs->nb_points() == 4)
         s_elem_type = "quads";
       break;
      case 3:
       if (cvs->nb_points() == 4)
         s_elem_type = "tetrahedra";
       else if (cvs->nb_points() == 8)
         s_elem_type = "cubes";
       break;
    }
    return s_elem_type;
  }

  void dx_export::write_mesh() {
    check_header();
    if (current_mesh().flags & dxMesh::STRUCTURE_WRITTEN) return;
    if (psl) write_mesh_structure_from_slice();
    else write_mesh_structure_from_mesh_fem();

    os << "\nobject \"" << current_mesh_name() << "\" class field\n"
       << "  component \"positions\" value \""
       << name_of_pts_array(current_mesh_name()) << "\"\n"
       << "  component \"connections\" value \""
       << name_of_conn_array(current_mesh_name()) << "\"\n";
    current_mesh().flags |= dxMesh::STRUCTURE_WRITTEN;
  }

  /* export the slice data as an unstructured mesh composed of simplexes */
  void dx_export::write_mesh_structure_from_slice() {
    os << "\nobject \"" << name_of_pts_array(current_mesh_name())
       << "\" class array type float rank 1 shape "
      << int(psl->dim())
       << " items " << (psl_use_merged ? psl->nb_merged_nodes() : psl->nb_points());
    if (!ascii) os << " " << endianness() << " binary";
    os << " data follows\n";
    if (psl_use_merged) {
      for (size_type i=0; i < psl->nb_merged_nodes(); ++i) {
       for (size_type k=0; k < psl->dim(); ++k)
         write_val(float(psl->merged_point(i)[k]));
       write_separ();
      }
    } else {
      for (size_type ic=0; ic < psl->nb_convex(); ++ic) {
       for (size_type i=0; i < psl->nodes(ic).size(); ++i)
         for (size_type k=0; k < psl->dim(); ++k)
           write_val(float(psl->nodes(ic)[i].pt[k]));
       write_separ();
      }
    }

    os << "\nobject \"" << name_of_conn_array(current_mesh_name())
       << "\" class array type int rank 1 shape "
       << int(connections_dim+1)
       << " items " << psl->nb_simplexes(connections_dim);
    if (!ascii) os << " " << endianness() << " binary";
    os << " data follows\n";

    size_type nodes_cnt = 0; /* <- a virer , global_index le remplace */
    for (size_type ic=0; ic < psl->nb_convex(); ++ic) {
      const getfem::mesh_slicer::cs_simplexes_ct& s = psl->simplexes(ic);
      for (size_type i=0; i < s.size(); ++i) {
        if (s[i].dim() == connections_dim) {
          for (size_type j=0; j < s[i].dim()+1; ++j) {
           size_type k;
           if (psl_use_merged)
             k = psl->merged_index(ic, s[i].inodes[j]);
           else k = psl->global_index(ic, s[i].inodes[j]);
            write_val(int(k));
         }
          write_separ();
        }
      }
      nodes_cnt += psl->nodes(ic).size();
    }

    write_convex_attributes(bgeot::simplex_structure(connections_dim));
    assert(nodes_cnt == psl->nb_points()); // sanity check
  }

  void dx_export::write_mesh_structure_from_mesh_fem() {
    os << "\nobject \"" << name_of_pts_array(current_mesh_name())
       << "\" class array type float rank 1 shape "
       << int(pmf->linked_mesh().dim())
       << " items " << pmf->nb_dof();
    if (!ascii) os << " " << endianness() << " binary";
    os << " data follows\n";

    /* possible improvement: detect structured grids */
    for (size_type d = 0; d < pmf->nb_basic_dof(); ++d) {
      const base_node P = pmf->point_of_basic_dof(d);
      for (size_type k=0; k < dim_; ++k)
       write_val(float(P[k]));
      write_separ();
    }

    os << "\nobject \"" << name_of_conn_array(current_mesh_name())
       << "\" class array type int rank 1 shape "
       << int(connections_dim)
       << " items " << pmf->convex_index().card();
    if (!ascii) os << " " << endianness() << " binary";
    os << " data follows\n";

    for (dal::bv_visitor cv(pmf->convex_index()); !cv.finished(); ++cv) {
      for (size_type i=0; i < connections_dim; ++i)
        write_val(int(pmf->ind_basic_dof_of_element(cv)[i]));
      write_separ();
    }
    write_convex_attributes(basic_structure(pmf->linked_mesh().structure_of_convex(pmf->convex_index().first_true())));
  }

  void dx_export::exporting_mesh_edges(bool with_slice_edges) {
    write_mesh();
    if (current_mesh().flags & dxMesh::WITH_EDGES) return;
    if (psl) write_mesh_edges_from_slice(with_slice_edges);
    else write_mesh_edges_from_mesh();
    current_mesh().flags |= dxMesh::WITH_EDGES;
    os << "\nobject \"" << name_of_edges_array(current_mesh_name())
       << "\" class field\n"
       << "  component \"positions\" value \""
       << name_of_pts_array(current_mesh_name()) << "\"\n"
       << "  component \"connections\" value \""
       << name_of_conn_array(name_of_edges_array(current_mesh_name()))
       << "\"\n";
  }

  void dx_export::write_mesh_edges_from_slice(bool with_slice_edges) {
    std::vector<size_type> edges;
    dal::bit_vector slice_edges;
    psl->get_edges(edges, slice_edges, psl_use_merged);
    if (with_slice_edges) slice_edges.clear();
    os << "\nobject \""
       << name_of_conn_array(name_of_edges_array(current_mesh_name()))
       << "\" class array type int rank 1 shape 2"
       << " items " << edges.size()/2 - slice_edges.card();
    if (!ascii) os << " " << endianness() << " binary";
    os << " data follows\n";
    for (size_type i=0; i < edges.size()/2; ++i) {
      if (!slice_edges.is_in(i)) {
       write_val(int(edges[2*i]));
       write_val(int(edges[2*i+1]));
      }
      if ((i+1)%10 == 0) write_separ();
    }
    write_separ();
    write_convex_attributes(bgeot::simplex_structure(1));
  }

  void dx_export::write_mesh_edges_from_mesh() {
    bgeot::mesh_structure ms(pmf->linked_mesh()); ms.to_edges();
    os << "\nobject \""
       << name_of_conn_array(name_of_edges_array(current_mesh_name()))
       << "\" class array type int rank 1 shape 2"
       << " items " << ms.convex_index().card();
    if (!ascii) os << " " << endianness() << " binary";
    os << " data follows\n";
    for (dal::bv_visitor cv(ms.convex_index()); !cv.finished(); ++cv) {
      write_val(int(ms.ind_points_of_convex(cv)[0]));
      write_val(int(ms.ind_points_of_convex(cv)[1]));
      if ((cv+1)%20 == 0) write_separ();
    }
    write_separ();
    write_convex_attributes(bgeot::simplex_structure(1));
  }


  /* -------------------------------------------------------------
   * POS export (Gmsh post-processing format)
   * ------------------------------------------------------------- */
  struct gf2pos_dof_mapping : public std::vector<std::vector<unsigned> > {};

  static const std::vector<unsigned>& getfem_to_pos_dof_mapping(int t) {
    gf2pos_dof_mapping &dm = dal::singleton<gf2pos_dof_mapping>::instance();
    if (dm.size() == 0) {
      dm.resize(8);
      dm[pos_export::POS_PT] = {0};
      dm[pos_export::POS_LN] = {0, 1};
      dm[pos_export::POS_TR] = {0, 1, 2};
      dm[pos_export::POS_QU] = {0, 1, 3, 2};
      dm[pos_export::POS_SI] = {0, 1, 2, 3};
      dm[pos_export::POS_HE] = {0, 1, 3, 2, 4, 5, 7, 6};
      dm[pos_export::POS_PR] = {0, 1, 2, 3, 4, 5};
      dm[pos_export::POS_PY] = {0, 1, 3, 2, 4};
    }
    return dm[t];
  }

  pos_export::pos_export(std::ostream& osname): os(osname) {
    init();
  }

  pos_export::pos_export(const std::string& fname)
    : os(real_os), real_os(fname.c_str()) {
    GMM_ASSERT1(real_os, "impossible to write to pos file '" << fname << "'");
    init();
  }

  void pos_export::init() {
    strcpy(header, "Exported by GetFEM");
    state = EMPTY;
    view  = 0;
  }

  void pos_export::set_header(const std::string& s){
    strncpy(header, s.c_str(), 256);
    header[255] = 0;
  }

  void pos_export::check_header() {
    if (state >= HEADER_WRITTEN) return;
    os << "/* " << header << " */\n";
    os << "General.FastRedraw = 0;\n";
    os << "General.ColorScheme = 1;\n";
    state = HEADER_WRITTEN;
  }

  void pos_export::exporting(const mesh& m) {
    if (state >= STRUCTURE_WRITTEN) return;
    dim = dim_type(m.dim());
    GMM_ASSERT1(int(dim) <= 3, "attempt to export a "
                << int(dim) << "D mesh (not supported)");
    pmf = std::make_unique<mesh_fem>(const_cast<mesh&>(m), dim_type(1));
    pmf->set_classical_finite_element(1);
    exporting(*pmf);
    state = STRUCTURE_WRITTEN;
  }

  void pos_export::exporting(const mesh_fem& mf) {
    if (state >= STRUCTURE_WRITTEN) return;
    dim = dim_type(mf.linked_mesh().dim());
    GMM_ASSERT1(int(dim) <= 3, "attempt to export a "
                << int(dim) << "D mesh_fem (not supported)");
    if (&mf != pmf.get())
      pmf = std::make_unique<mesh_fem>(mf.linked_mesh(), dim_type(1));

    /* initialize pmf with finite elements suitable for Gmsh */
    for (dal::bv_visitor cv(mf.convex_index()); !cv.finished(); ++cv) {
      bgeot::pgeometric_trans pgt = mf.linked_mesh().trans_of_convex(cv);
      pfem pf = mf.fem_of_element(cv);

      bool discontinuous = false;
      for (unsigned i=0; i < pf->nb_dof(cv); ++i) {
        // could be a better test for discontinuity ...
        if (!dof_linkable(pf->dof_types()[i])) { discontinuous = true; break; }
      }
      pfem classical_pf1 = discontinuous ?
        classical_discontinuous_fem(pgt, 1) : classical_fem(pgt, 1);
      pmf->set_finite_element(cv, classical_pf1);
    }
    psl = NULL;

    /* find out which dof will be exported to Gmsh */
    for (dal::bv_visitor cv(pmf->convex_index()); !cv.finished(); ++cv) {
      int t = -1;
      switch (pmf->fem_of_element(cv)->nb_dof(cv)){
        case 1: t = POS_PT; break;
        case 2: t = POS_LN; break;
        case 3: t = POS_TR; break;
        case 4:
          if ( 2 == pmf->fem_of_element(cv)->dim() ) t = POS_QU;
          else if (3 == pmf->fem_of_element(cv)->dim()) t = POS_SI;
          break;
        case 6: t = POS_PR; break;
        case 8: t = POS_HE; break;
        case 5: t = POS_PY; break;
      }
      GMM_ASSERT1(t != -1, "semi internal error. Could not map "
                           << name_of_fem(pmf->fem_of_element(cv))
                           << " to a POS primitive type");
      pos_cell_type.push_back(unsigned(t));

      const std::vector<unsigned>& dmap = getfem_to_pos_dof_mapping(t);
      GMM_ASSERT1(dmap.size() <= pmf->nb_basic_dof_of_element(cv),
                  "inconsistency in pos_dof_mapping");
      std::vector<unsigned> cell_dof;
      cell_dof.resize(dmap.size(),unsigned(-1));
      for (size_type i=0; i < dmap.size(); ++i)
        cell_dof[i] = unsigned(pmf->ind_basic_dof_of_element(cv)[dmap[i]]);
      pos_cell_dof.push_back(cell_dof);
    }
    for (size_type i=0; i< pmf->nb_basic_dof(); ++i){
      std::vector<float> pt;
      pt.resize(dim,float(0));
      for (size_type j=0; j<dim; ++j)
        pt[j] = float(pmf->point_of_basic_dof(i)[j]);
      pos_pts.push_back(pt);
    }
    state = STRUCTURE_WRITTEN;
  }

  void pos_export::exporting(const stored_mesh_slice& sl) {
    if (state >= STRUCTURE_WRITTEN) return;
    psl = &sl;
    dim = dim_type(sl.dim());
    GMM_ASSERT1(int(dim) <= 3, "attempt to export a "
                << int(dim) << "D slice (not supported)");

    for (size_type ic=0, pcnt=0; ic < psl->nb_convex(); ++ic) {
      for (getfem::mesh_slicer::cs_simplexes_ct::const_iterator it=psl->simplexes(ic).begin();
           it != psl->simplexes(ic).end(); ++it) {
        int t = -1;
        switch ((*it).dim()){
          case 0: t = POS_PT; break;
          case 1: t = POS_LN; break;
          case 2: t = POS_TR; break;
          case 3: t = POS_SI; break;
        }
        GMM_ASSERT1(t != -1, "semi internal error.. could not map to a POS primitive type");
        pos_cell_type.push_back(unsigned(t));

        const std::vector<unsigned>& dmap = getfem_to_pos_dof_mapping(t);
        GMM_ASSERT1(dmap.size() <= size_type(t+1), "inconsistency in pos_dof_mapping");

        std::vector<unsigned> cell_dof;
        cell_dof.resize(dmap.size(),unsigned(-1));
        for (size_type i=0; i < dmap.size(); ++i)
          cell_dof[i] = unsigned(it->inodes[dmap[i]] + pcnt);
        pos_cell_dof.push_back(cell_dof);
      }
      for(getfem::mesh_slicer::cs_nodes_ct::const_iterator it=psl->nodes(ic).begin();
          it != psl->nodes(ic).end(); ++it) {
        std::vector<float> pt;
        pt.resize(dim,float(0));
        for (size_type i=0; i<dim; ++i)
          pt[i] = float(it->pt[i]);
        pos_pts.push_back(pt);
      }
      pcnt += psl->nodes(ic).size();
    }
    state = STRUCTURE_WRITTEN;
  }

  void pos_export::write(const mesh& m, const std::string &name){
    if (state >= IN_CELL_DATA) return;
    GMM_ASSERT1(int(m.dim()) <= 3, "attempt to export a "
                << int(m.dim()) << "D mesh (not supported)");
    pmf = std::make_unique<mesh_fem>(const_cast<mesh&>(m), dim_type(1));
    pmf->set_classical_finite_element(1);
    write(*pmf,name);
    state = IN_CELL_DATA;
  }

  void pos_export::write(const mesh_fem& mf, const std::string &name){
    if (state >= IN_CELL_DATA) return;
    check_header();
    exporting(mf);

    if (""==name) os << "View \"mesh " << view <<"\" {\n";
    else os << "View \"" << name <<"\" {\n";

    int t;
    std::vector<unsigned> cell_dof;
    std::vector<float> cell_dof_val;
    for (size_type cell = 0; cell < pos_cell_type.size(); ++cell) {
      t = pos_cell_type[cell];
      cell_dof = pos_cell_dof[cell];
      cell_dof_val.resize(cell_dof.size(),float(0));
      write_cell(t,cell_dof,cell_dof_val);
    }

    os << "};\n";
    os << "View[" << view << "].ShowScale = 0;\n";
    os << "View[" << view << "].ShowElement = 1;\n";
    os << "View[" << view << "].DrawScalars = 0;\n";
    os << "View[" << view << "].DrawVectors = 0;\n";
    os << "View[" << view++ << "].DrawTensors = 0;\n";
    state = IN_CELL_DATA;
  }

  void pos_export::write(const stored_mesh_slice& sl, const std::string &name){
    if (state >= IN_CELL_DATA) return;
    check_header();
    exporting(sl);

    if (""==name) os << "View \"mesh " << view <<"\" {\n";
    else os << "View \"" << name <<"\" {\n";

    int t;
    std::vector<unsigned> cell_dof;
    std::vector<float> cell_dof_val;
    for (size_type cell = 0; cell < pos_cell_type.size(); ++cell) {
      t = pos_cell_type[cell];
      cell_dof = pos_cell_dof[cell];
      cell_dof_val.resize(cell_dof.size(),float(0));
      write_cell(t,cell_dof,cell_dof_val);
    }

    os << "};\n";
    os << "View[" << view << "].ShowScale = 0;\n";
    os << "View[" << view << "].ShowElement = 1;\n";
    os << "View[" << view << "].DrawScalars = 0;\n";
    os << "View[" << view << "].DrawVectors = 0;\n";
    os << "View[" << view++ << "].DrawTensors = 0;\n";
    state = IN_CELL_DATA;
  }
}  /* end of namespace getfem. */
