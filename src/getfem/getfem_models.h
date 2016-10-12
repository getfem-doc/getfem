/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================

 Copyright (C) 2009-2016 Yves Renard

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
   @file getfem_models.h
   @author  Yves Renard <Yves.Renard@insa-lyon.fr>
   @date March 21, 2009.
   @brief Model representation in Getfem.
*/

#ifndef GETFEM_MODELS_H__
#define GETFEM_MODELS_H__

#include "getfem_assembling.h"
#include "getfem_partial_mesh_fem.h"
#include "getfem_im_data.h"

namespace getfem {

  class virtual_brick;
  /** type of pointer on a brick */
  typedef std::shared_ptr<const virtual_brick> pbrick;

  class virtual_dispatcher;
  typedef std::shared_ptr<const virtual_dispatcher> pdispatcher;

  class virtual_time_scheme;
  typedef std::shared_ptr<const virtual_time_scheme> ptime_scheme;


  class Neumann_elem_term;
  typedef std::shared_ptr<const Neumann_elem_term> pNeumann_elem_term;

  class virtual_interpolate_transformation;
  typedef std::shared_ptr<const virtual_interpolate_transformation>
  pinterpolate_transformation;

  class virtual_elementary_transformation;
  typedef std::shared_ptr<const virtual_elementary_transformation>
  pelementary_transformation;
  

  class ga_workspace;

  // Event management : The model has to react when something has changed in
  //    the context and ask for corresponding (linear) bricks to recompute
  //    some terms.
  //    For the moment two events are taken into account
  //      - Change in a mesh_fem
  //      - Change in the data of a variable
  //    For this, a brick has to declare on which variable it depends and
  //    on which data. When a linear brick depend on a variable, the
  //    recomputation is done when the eventual corresponding mesh_fem
  //    is changed (or the size of the variable for a fixed size variable).
  //    When a linear brick depend on a data, the recomputation is done
  //    when the corresponding vector value is changed. If a variable is used
  //    as a data, it has to be declared as a data by the brick.
  //    A nonlinear brick is recomputed at each assembly of the tangent system.
  //    Remember this behavior when some changed are done on the variable
  //    and/or data.
  //    The change on a mesh_im is not taken into account for the moment.
  //    The different versions of the variables is not taken into account
  //    separately.




  //=========================================================================
  //
  //  Model object.
  //
  //=========================================================================


  typedef gmm::rsvector<scalar_type> model_real_sparse_vector;
  typedef gmm::rsvector<complex_type> model_complex_sparse_vector;
  typedef std::vector<scalar_type> model_real_plain_vector;
  typedef std::vector<complex_type> model_complex_plain_vector;

  typedef gmm::col_matrix<model_real_sparse_vector> model_real_sparse_matrix;
  typedef gmm::col_matrix<model_complex_sparse_vector>
  model_complex_sparse_matrix;

  typedef gmm::row_matrix<model_real_sparse_vector>
  model_real_row_sparse_matrix;
  typedef gmm::row_matrix<model_complex_sparse_vector>
  model_complex_row_sparse_matrix;
  
  // For backward compatibility with version 3.0
  typedef model_real_plain_vector modeling_standard_plain_vector;
  typedef model_real_sparse_vector modeling_standard_sparse_vector;
  typedef model_real_sparse_matrix modeling_standard_sparse_matrix;
  typedef model_complex_plain_vector modeling_standard_complex_plain_vector;
  typedef model_complex_sparse_vector modeling_standard_complex_sparse_vector;
  typedef model_complex_sparse_matrix modeling_standard_complex_sparse_matrix;

  inline std::string sup_previous_and_dot_to_varname(std::string v) {
    if (!(v.compare(0, 8, "Previous")) && (v[8] == '_' || v[9] == '_')) {
      v = v.substr((v[8] == '_') ? 9 : 10);
    }
    if (!(v.compare(0, 3, "Dot")) && (v[3] == '_' || v[4] == '_')) {
      v = v.substr((v[3] == '_') ? 4 : 5);
    }
    if (!(v.compare(0, 9, "Old_"))) v = v.substr(9);
    return v;
  }

  /** ``Model'' variables store the variables, the data and the
      description of a model. This includes the global tangent matrix, the
      right hand side and the constraints. There are two kinds of models, the
      ``real'' and the ``complex'' models.
  */
  class APIDECL model : public context_dependencies,
                        virtual public dal::static_stored_object {

  protected:

    // State variables of the model
    bool complex_version;
    bool is_linear_;
    bool is_symmetric_;
    bool is_coercive_;
    mutable model_real_sparse_matrix rTM;    // tangent matrix, real version
    mutable model_complex_sparse_matrix cTM; // tangent matrix, complex version
    mutable model_real_plain_vector rrhs;
    mutable model_complex_plain_vector crhs;
    mutable bool act_size_to_be_done;
    dim_type leading_dim;
    getfem::lock_factory locks_;

    // Variables and parameters of the model

    enum  var_description_filter {
      VDESCRFILTER_NO = 0, // Variable being directly the dofs of a given fem
      VDESCRFILTER_REGION = 1, /* Variable being the dofs of a fem on a mesh
                            *  region (uses mf.dof_on_region). */
      VDESCRFILTER_INFSUP = 2, /* Variable being the dofs of a fem on a mesh
                            * region with an additional filter on a mass
                            * matrix with respect to another fem. */
      VDESCRFILTER_CTERM = 4, /* Variable being the dofs of a fem on a mesh
                            * region with an additional filter with the
                            * coupling term with respect to another variable.*/
      VDESCRFILTER_REGION_CTERM = 5,  /* both region and cterm. */
    }; // INFSUP and CTERM are incompatible

    struct var_description {

      bool is_variable;  // This is a variable or a parameter.
      bool is_disabled;  // For a variable, to be solved or not
      bool is_complex;   // The variable is complex numbers
      bool is_affine_dependent;   // The variable depends in an affine way 
                                  // to another variable. 
      bool is_fem_dofs;  // The variable is the dofs of a fem
      var_description_filter filter; // A filter on the dofs is applied or not.
      size_type n_iter; //  Number of versions of the variable stored.
      size_type n_temp_iter; // Number of additional temporary versions
      size_type default_iter; // default iteration number.

      ptime_scheme ptsc; // For optional time integration scheme

      // fem description of the variable
      const mesh_fem *mf;        // Main fem of the variable.
      size_type m_region;        // Optional region for the filter
      const mesh_im *mim;        // Optional integration method for the filter
      ppartial_mesh_fem partial_mf; // Filter with respect to mf.
      std::string filter_var;       // Optional variable name for the filter

      bgeot::multi_index qdims;  // For data having a qdim != of the fem
                                 // (dim per dof for dof data)
                                 // and for constant variables.
      gmm::uint64_type v_num;
      std::vector<gmm::uint64_type> v_num_data;

      gmm::sub_interval I; // For a variable : indices on the whole system.
      // For an affine dependent variable, should be the same than the
      // orgininal variable
      std::vector<model_real_plain_vector> real_value;
      std::vector<model_complex_plain_vector> complex_value;
      std::vector<gmm::uint64_type> v_num_var_iter;
      std::vector<gmm::uint64_type> v_num_iter;

      // For affine dependent variables
      model_real_plain_vector affine_real_value;
      model_complex_plain_vector affine_complex_value;
      scalar_type alpha;    // Factor for the affine dependent variables
      std::string org_name; // Name of the original variable for affine
                            //  dependent variables

      // im data description
      const im_data *pim_data;

      size_type qdim() const { return qdims.total_size(); }

      var_description(bool is_var = false, bool is_com = false,
                      bool is_fem = false, size_type n_it = 1,
                      var_description_filter fil = VDESCRFILTER_NO,
                      const mesh_fem *mmf = 0,
                      size_type m_reg = size_type(-1),
                      bgeot::multi_index qdims_ = bgeot::multi_index(),
                      const std::string &filter_v = std::string(""),
                      const mesh_im *mim_ = 0, const im_data *pimd = 0)
        : is_variable(is_var), is_disabled(false), is_complex(is_com),
          is_affine_dependent(false), is_fem_dofs(is_fem), filter(fil),
          n_iter(std::max(size_type(1), n_it)), n_temp_iter(0),
          default_iter(0), ptsc(0), mf(mmf), m_region(m_reg), mim(mim_),
          filter_var(filter_v), qdims(qdims_), v_num(0),
          v_num_data(n_iter, act_counter()), I(0,0),
          alpha(1), pim_data(pimd) {
        
        if (filter != VDESCRFILTER_NO && mf != 0)
          partial_mf = std::make_shared<partial_mesh_fem>(*mf);
        // v_num_data = v_num;
        if (qdims.size() == 0) qdims.push_back(1);
        GMM_ASSERT1(qdim(), "Attempt to create a null size variable");
      }

      // add a temporary version for time integration schemes. Automatically
      // set the default iter to it. id_num is an identifier. Do not add
      // the version if a temporary already exist with this identifier.
      size_type add_temporary(gmm::uint64_type id_num);

      void clear_temporaries();

      const mesh_fem &associated_mf() const {
        GMM_ASSERT1(is_fem_dofs, "This variable is not linked to a fem");
        return (filter == VDESCRFILTER_NO) ? *mf : *partial_mf;
      }

      const mesh_fem *passociated_mf() const {
        if (!is_fem_dofs)
          return 0;
        return (filter == VDESCRFILTER_NO || partial_mf.get() == 0)
               ? mf : partial_mf.get();
      }

      size_type size() const // Should control that the variable is
      // indeed initialized by actualize_sizes() ...
      { return is_complex ? complex_value[0].size() : real_value[0].size(); }

      void set_size();
    };

  public:

    typedef std::vector<std::string> varnamelist;
    typedef std::vector<const mesh_im *> mimlist;
    typedef std::vector<model_real_sparse_matrix> real_matlist;
    typedef std::vector<model_complex_sparse_matrix> complex_matlist;
    typedef std::vector<model_real_plain_vector> real_veclist;
    typedef std::vector<model_complex_plain_vector> complex_veclist;

    struct term_description {
      bool is_matrix_term; // tangent matrix term or rhs term.
      bool is_symmetric;   // Term have to be symmetrized.
      bool is_global;      // Specific global term for highly coupling bricks
      std::string var1, var2;
      
      term_description(const std::string &v)
        : is_matrix_term(false), is_symmetric(false),
          is_global(false), var1(sup_previous_and_dot_to_varname(v)) {}
      term_description(const std::string &v1, const std::string &v2,
                       bool issym)
        : is_matrix_term(true), is_symmetric(issym), is_global(false),
          var1(sup_previous_and_dot_to_varname(v1)), var2(v2) {}
      term_description(bool ism, bool issym)
        : is_matrix_term(ism), is_symmetric(issym), is_global(true) {}
    };

    typedef std::vector<term_description> termlist;

    enum build_version { BUILD_RHS = 1,
                         BUILD_MATRIX = 2,
                         BUILD_ALL = 3,
                         BUILD_ON_DATA_CHANGE = 4,
                         BUILD_WITH_COMPLETE_RHS = 8,
                         BUILD_COMPLETE_RHS = 9,
    };

  protected:

    // rmatlist and cmatlist could be csc_matrix vectors to reduced the
    // amount of memory (but this should add a supplementary copy).
    struct brick_description {
      mutable bool terms_to_be_computed;
      mutable gmm::uint64_type v_num;
      pbrick pbr;                // brick pointer
      pdispatcher pdispatch;     // Optional dispatcher
      size_type nbrhs;           // Additional rhs for dispatcher.
      varnamelist vlist;         // List of variables used by the brick.
      varnamelist dlist;         // List of data used by the brick.
      termlist tlist;            // List of terms build by the brick
      mimlist mims;              // List of integration methods.
      size_type region;          // Optional region size_type(-1) for all.
      bool is_update_brick;      // Flag for declaring special type of brick
      // with no contributions.
      mutable scalar_type external_load; // External load computed in assembly

      mutable model_real_plain_vector coeffs;
      mutable scalar_type matrix_coeff = scalar_type(0);
      mutable real_matlist rmatlist;    // Matrices the brick have to fill in
      // (real version).
      mutable std::vector<real_veclist> rveclist; // Rhs the brick have to
      // fill in (real version).
      mutable std::vector<real_veclist> rveclist_sym; // additional rhs for
      //  symmetric terms (real version).
      mutable complex_matlist cmatlist; // Matrices the brick have to fill in
      // (complex version).
      mutable std::vector<complex_veclist> cveclist; // Rhs the brick have to
      // fill in (complex version).
      mutable std::vector<complex_veclist> cveclist_sym;  // additional rhs
      // for symmetric terms (real version).

      brick_description() : v_num(0) {}

      brick_description(pbrick p, const varnamelist &vl,
                        const varnamelist &dl, const termlist &tl,
                        const mimlist &mms, size_type reg)
        : terms_to_be_computed(true), v_num(0), pbr(p), pdispatch(0), nbrhs(1),
          vlist(vl), dlist(dl), tlist(tl), mims(mms), region(reg),
          is_update_brick(false), external_load(0),
          rveclist(1), rveclist_sym(1), cveclist(1),
          cveclist_sym(1)  { }
    };

    typedef std::map<std::string, var_description> VAR_SET;
    mutable VAR_SET variables;             // Variables list of the model
    std::vector<brick_description> bricks; // Bricks list of the model
    dal::bit_vector valid_bricks, active_bricks;
    typedef std::pair<std::string, size_type> Neumann_pair;
    typedef std::map<Neumann_pair, pNeumann_elem_term> Neumann_SET;
    mutable Neumann_SET Neumann_term_list; // Neumann terms list (mainly for
                                           // Nitsche's method)
    mutable std::map<std::string, std::vector<std::string> >
      Neumann_terms_auxilliary_variables;
    std::map<std::string, pinterpolate_transformation> transformations;
    std::map<std::string, pelementary_transformation> elem_transformations;

    // Structure dealing with time integration scheme
    int time_integration; // 0 : no, 1 : time step, 2 : init
    bool init_step;
    scalar_type time_step; // Time step (dt) for time integration schemes
    scalar_type init_time_step; // Time step for initiaisation of derivatives
    
    // Structure dealing with simple dof constraints
    typedef std::map<size_type, scalar_type> real_dof_constraints_var;
    typedef std::map<size_type, complex_type> complex_dof_constraints_var;
    mutable std::map<std::string, real_dof_constraints_var>
      real_dof_constraints;
    mutable std::map<std::string, complex_dof_constraints_var>
      complex_dof_constraints;
    void clear_dof_constraints()
    { real_dof_constraints.clear(); complex_dof_constraints.clear(); }

    // Structure dealing with nonlinear expressions
    struct gen_expr {
      std::string expr;
      const mesh_im &mim;
      size_type region;
      gen_expr(const std::string &expr_, const mesh_im &mim_,
               size_type region_) : expr(expr_), mim(mim_), region(region_) {}
    };

    mutable std::list<gen_expr> generic_expressions;

    // Groups of variables for interpolation on different meshes
    // generic assembly
    std::map<std::string, std::vector<std::string> > variable_groups;

    std::map<std::string, std::string> macros;


    virtual void actualize_sizes() const;
    bool check_name_validity(const std::string &name, bool assert=true) const;
    void brick_init(size_type ib, build_version version,
                    size_type rhs_ind = 0) const;

    void init() { complex_version = false; act_size_to_be_done = false; }

    void resize_global_system() const;

    //to be performed after to_variables is called.
    virtual void post_to_variables_step();

    scalar_type approx_external_load_; // Computed by assembly procedure
                                       // with BUILD_RHS option.

    VAR_SET::const_iterator find_variable(const std::string &name) const;

  public:

    void add_generic_expression(const std::string &expr, const mesh_im &mim,
                                size_type region) const
    { generic_expressions.push_back(gen_expr(expr, mim, region)); }
    void add_external_load(size_type ib, scalar_type e) const
    { bricks[ib].external_load = e; }
    scalar_type approx_external_load() { return approx_external_load_; }
    // call the brick if necessary
    void update_brick(size_type ib, build_version version) const;
    void linear_brick_add_to_rhs(size_type ib, size_type ind_data,
                                 size_type n_iter) const;
    void update_affine_dependent_variables();
    void brick_call(size_type ib, build_version version,
                    size_type rhs_ind = 0) const;
    model_real_plain_vector &rhs_coeffs_of_brick(size_type ib) const
    { return bricks[ib].coeffs; }
    scalar_type &matrix_coeff_of_brick(size_type ib) const
    { return bricks[ib].matrix_coeff; }
    bool is_var_newer_than_brick(const std::string &varname,
                                 size_type ib, size_type niter = size_type(-1)) const;
    bool is_var_mf_newer_than_brick(const std::string &varname,
                                    size_type ib) const;
    bool is_mim_newer_than_brick(const mesh_im &mim,
                                 size_type ib) const;

    pbrick brick_pointer(size_type ib) const {
      GMM_ASSERT1(valid_bricks[ib], "Inexistent brick");
      return bricks[ib].pbr;
    }

    void variable_list(varnamelist &vl) const
    { for (const auto &v : variables) vl.push_back(v.first); }

    void define_variable_group(const std::string &group_name,
                               const std::vector<std::string> &nl);
    bool variable_group_exists(const std::string &group_name) const
    { return variable_groups.count(group_name) > 0; }

    const std::vector<std::string> &
    variable_group(const std::string &group_name) const {
      GMM_ASSERT1(variable_group_exists(group_name),
                  "Undefined variable group " << group_name);
      return (variable_groups.find(group_name))->second;
    }

    void add_Neumann_term(pNeumann_elem_term p,
                          const std::string &varname,
                          size_type brick_num) const
    { Neumann_term_list[Neumann_pair(varname, brick_num)] = p; }

    size_type check_Neumann_terms_consistency(const std::string &varname)const;

    bool check_Neumann_terms_linearity(const std::string &varname) const;

    void auxilliary_variables_of_Neumann_terms
    (const std::string &varname, std::vector<std::string> &aux_var) const;

    void add_auxilliary_variables_of_Neumann_terms
    (const std::string &varname, const std::vector<std::string> &aux_vars) const;

    void add_auxilliary_variables_of_Neumann_terms
    (const std::string &varname, const std::string &aux_var) const;

    /* Compute the approximation of the Neumann condition for a variable
        with the declared terms.
        The output tensor has to have the right size. No verification.
    */
    void compute_Neumann_terms(int version, const std::string &varname,
                               const mesh_fem &mfvar,
                               const model_real_plain_vector &var,
                               fem_interpolation_context &ctx,
                               base_small_vector &n,
                               bgeot::base_tensor &output) const;

    void compute_auxilliary_Neumann_terms
    (int version, const std::string &varname,
     const mesh_fem &mfvar, const model_real_plain_vector &var,
     const std::string &aux_varname,
     fem_interpolation_context &ctx, base_small_vector &n,
     bgeot::base_tensor &output) const;

    /* function to be called by Dirichlet bricks */
    void add_real_dof_constraint(const std::string &varname, size_type dof,
                                 scalar_type val) const
    { real_dof_constraints[varname][dof] = val; }
    /* function to be called by Dirichlet bricks */
    void add_complex_dof_constraint(const std::string &varname, size_type dof,
                                    complex_type val) const
    { complex_dof_constraints[varname][dof] = val; }


    void add_temporaries(const varnamelist &vl, gmm::uint64_type id_num) const;

    const mimlist &mimlist_of_brick(size_type ib) const
    { return bricks[ib].mims; }

    const varnamelist &varnamelist_of_brick(size_type ib) const
    { return bricks[ib].vlist; }

    const varnamelist &datanamelist_of_brick(size_type ib) const
    { return bricks[ib].dlist; }

    size_type region_of_brick(size_type ib) const
    { return bricks[ib].region; }

    bool temporary_uptodate(const std::string &varname,
                            gmm::uint64_type id_num, size_type &ind) const;

    size_type n_iter_of_variable(const std::string &name) const {
      return variables.count(name) == 0 ? size_type(0)
                                        : variables[name].n_iter;
    }

    void set_default_iter_of_variable(const std::string &varname,
                                      size_type ind) const;
    void reset_default_iter_of_variables(const varnamelist &vl) const;

    void update_from_context() const { act_size_to_be_done = true; }

    const model_real_sparse_matrix &linear_real_matrix_term
    (size_type ib, size_type iterm);

    const model_complex_sparse_matrix &linear_complex_matrix_term
    (size_type ib, size_type iterm);

    /** Disable a brick.  */
    void disable_brick(size_type ib) {
      GMM_ASSERT1(valid_bricks[ib], "Inexistent brick");
      active_bricks.del(ib);
    }

    /** Enable a brick.  */
    void enable_brick(size_type ib) {
      GMM_ASSERT1(valid_bricks[ib], "Inexistent brick");
      active_bricks.add(ib);
    }

    /** Disable a variable (and its attached mutlipliers).  */
    void disable_variable(const std::string &name);

    /** Enable a variable (and its attached mutlipliers).  */
    void enable_variable(const std::string &name);

    /** Says if a name corresponds to a declared variable.  */
    bool variable_exists(const std::string &name) const {
      if (!(name.compare(0, 9, "Old_")))
	return variables.count(name.substr(9)) > 0;
      else
	return variables.count(name) > 0;
    }

    bool is_disabled_variable(const std::string &name) const;

    /** Says if a name corresponds to a declared data or disabled variable.  */
    bool is_data(const std::string &name) const;

    /** Says if a name corresponds to a declared data.  */
    bool is_true_data(const std::string &name) const;

    bool is_affine_dependent_variable(const std::string &name) const;

    const std::string &org_variable(const std::string &name) const;

    const scalar_type &factor_of_variable(const std::string &name) const;

    void set_factor_of_variable(const std::string &name, scalar_type a);

    bool is_im_data(const std::string &name) const;

    const im_data *pim_data_of_variable(const std::string &name) const;

    const gmm::uint64_type &
    version_number_of_data_variable(const std::string &varname,
                                    size_type niter = size_type(-1)) const;

    /** Boolean which says if the model deals with real or complex unknowns
        and data. */
    bool is_complex() const { return complex_version; }

    /** Return true if all the model terms do not affect the coercivity of
        the whole tangent system. */
    bool is_coercive() const { return is_coercive_; }

    /** Return true if all the model terms do not affect the coercivity of
        the whole tangent system. */
    bool is_symmetric() const { return is_symmetric_; }

    /** Return true if all the model terms are linear. */
    bool is_linear() const { return is_linear_; }

    /** Total number of degrees of freedom in the model. */
    size_type nb_dof() const;

    /** Leading dimension of the meshes used in the model. */
    dim_type leading_dimension() const { return leading_dim; }

    /** Gives a non already existing variable name begining by `name`. */
    std::string new_name(const std::string &name);

    const gmm::sub_interval &
    interval_of_variable(const std::string &name) const;

    /** Gives the access to the vector value of a variable. For the real
        version. */
    const model_real_plain_vector &
    real_variable(const std::string &name,
                  size_type niter = size_type(-1)) const;

    /** Gives the access to the vector value of a variable. For the complex
        version. */
    const model_complex_plain_vector &
    complex_variable(const std::string &name,
                     size_type niter = size_type(-1)) const;

    /** Gives the write access to the vector value of a variable. Make a
        change flag of the variable set. For the real version. */
    model_real_plain_vector &
    set_real_variable(const std::string &name,
                      size_type niter = size_type(-1)) const;

    /** Gives the write access to the vector value of a variable. Make a
        change flag of the variable set. For the complex version. */
    model_complex_plain_vector &
    set_complex_variable(const std::string &name,
                         size_type niter = size_type(-1)) const;

    model_real_plain_vector &
    set_real_constant_part(const std::string &name) const;

    model_complex_plain_vector &
    set_complex_constant_part(const std::string &name) const;

    template<typename VECTOR, typename T>
    void from_variables(VECTOR &V, T) const {
      for (const auto &v : variables)
        if (v.second.is_variable && !(v.second.is_affine_dependent)
            && !(v.second.is_disabled))
          gmm::copy(v.second.real_value[0],
                    gmm::sub_vector(V, v.second.I));
    }

    template<typename VECTOR, typename T>
    void from_variables(VECTOR &V, std::complex<T>) const {
      for (const auto &v : variables)
        if (v.second.is_variable && !(v.second.is_affine_dependent)
            && !(v.second.is_disabled))
          gmm::copy(v.second.complex_value[0],
                    gmm::sub_vector(V, v.second.I));
    }

    template<typename VECTOR> void from_variables(VECTOR &V) const {
      typedef typename gmm::linalg_traits<VECTOR>::value_type T;
      context_check(); if (act_size_to_be_done) actualize_sizes();
      from_variables(V, T());
    }

    template<typename VECTOR, typename T>
    void to_variables(const VECTOR &V, T) {
      for (auto &&v : variables)
        if (v.second.is_variable && !(v.second.is_affine_dependent)
            && !(v.second.is_disabled)) {
          gmm::copy(gmm::sub_vector(V, v.second.I),
                    v.second.real_value[0]);
          v.second.v_num_data[0] = act_counter();
        }
      update_affine_dependent_variables();
      this->post_to_variables_step();
    }

    template<typename VECTOR, typename T>
    void to_variables(const VECTOR &V, std::complex<T>) {
      for (auto &&v : variables)
        if (v.second.is_variable && !(v.second.is_affine_dependent)
            && !(v.second.is_disabled)) {
          gmm::copy(gmm::sub_vector(V, v.second.I),
                    v.second.complex_value[0]);
          v.second.v_num_data[0] = act_counter();
        }
      update_affine_dependent_variables();
      this->post_to_variables_step();
    }

    template<typename VECTOR> void to_variables(const VECTOR &V) {
      typedef typename gmm::linalg_traits<VECTOR>::value_type T;
      context_check(); if (act_size_to_be_done) actualize_sizes();
      to_variables(V, T());
    }

    /** Add a fixed size variable to the model assumed to be a vector.
        niter is the number of version of the variable stored. */
    void add_fixed_size_variable(const std::string &name, size_type size,
                                 size_type niter = 1);
    
    /** Add a fixed size variable to the model whith given tensor dimensions.
        niter is the number of version of the variable stored. */
    void add_fixed_size_variable(const std::string &name,
                                 const bgeot::multi_index &sizes,
                                 size_type niter = 1);

    /** Add a fixed size data to the model. niter is the number of version
        of the data stored, for time integration schemes. */
    void add_fixed_size_data(const std::string &name, size_type size,
                             size_type niter = 1);

    /** Add a fixed size data to the model. niter is the number of version
        of the data stored, for time integration schemes. */
    void add_fixed_size_data(const std::string &name,
                             const bgeot::multi_index &sizes,
                             size_type niter = 1);

    /** Resize a fixed size variable (or data) of the model. */
    void resize_fixed_size_variable(const std::string &name, size_type size);

    /** Resize a fixed size variable (or data) of the model. */
    void resize_fixed_size_variable(const std::string &name,
                                    const bgeot::multi_index &sizes);

    /** Add a fixed size data (assumed to be a vector) to the model and
        initialized with v. */
    template <typename VECT>
    void add_initialized_fixed_size_data(const std::string &name,
                                         const VECT &v) {
      this->add_fixed_size_data(name, gmm::vect_size(v));
      if (this->is_complex())
        gmm::copy(v, this->set_complex_variable(name));
      else
        gmm::copy(gmm::real_part(v), this->set_real_variable(name));
    }

    /** Add a fixed size data (assumed to be a vector) to the model and
        initialized with v. */
    template <typename VECT>
    void add_initialized_fixed_size_data(const std::string &name,
                                         const VECT &v,
                                         const bgeot::multi_index &sizes) {
      this->add_fixed_size_data(name, sizes);
      if (this->is_complex())
        gmm::copy(v, this->set_complex_variable(name));
      else
        gmm::copy(gmm::real_part(v), this->set_real_variable(name));
    }

    /** Add a fixed size data (assumed to be a matrix) to the model and
        initialized with M. */
    void add_initialized_matrix_data(const std::string &name,
                                     const base_matrix &M);
    void add_initialized_matrix_data(const std::string &name,
                                     const base_complex_matrix &M);

    /** Add a fixed size data (assumed to be a tensor) to the model and
        initialized with t. */
    void add_initialized_tensor_data(const std::string &name,
                                     const base_tensor &t);
    void add_initialized_tensor_data(const std::string &name,
                                     const base_complex_tensor &t);


    /** Add a scalar data (i.e. of size 1) to the model initialized with e. */
    template <typename T>
    void add_initialized_scalar_data(const std::string &name, T e) {
      this->add_fixed_size_data(name, 1, 1);
      if (this->is_complex())
        this->set_complex_variable(name)[0] = e;
      else
        this->set_real_variable(name)[0] = gmm::real(e);
    }


    /** Add data, defined at integration points.*/
    void add_im_data(const std::string &name, const im_data &im_data, size_type niter = 1);

    /** Add a variable being the dofs of a finite element method to the model.
        niter is the number of version of the variable stored, for time
        integration schemes. */
    void add_fem_variable(const std::string &name, const mesh_fem &mf,
                          size_type niter = 1);

    /** Add a variable linked to a fem with the dof filtered with respect
        to a mesh region. Only the dof returned by the dof_on_region
        method of `mf` will be kept. niter is the number of version
        of the data stored, for time integration schemes. */
    void add_filtered_fem_variable(const std::string &name, const mesh_fem &mf,
                                   size_type region, size_type niter = 1);


    /** Add a "virtual" variable be an affine depedent variable with respect
        to another variable. Mainly used for time integration scheme for
        instance to represent time derivative of variables.
        `alpha` is the multiplicative scalar of the dependency. */
    void add_affine_dependent_variable(const std::string &name,
                                       const std::string &org_name,
                                       scalar_type alpha = scalar_type(1));

    /** Add a data being the dofs of a finite element method to the model.*/
    void add_fem_data(const std::string &name, const mesh_fem &mf,
                      dim_type qdim = 1, size_type niter = 1);

    /** Add a data being the dofs of a finite element method to the model.*/
    void add_fem_data(const std::string &name, const mesh_fem &mf,
                      const bgeot::multi_index &sizes, size_type niter = 1);

    /** Add an initialized fixed size data to the model, assumed to be a
        vector field if the size of the vector is a multiple of the dof
        number. */
    template <typename VECT>
    void add_initialized_fem_data(const std::string &name, const mesh_fem &mf,
                                  const VECT &v) {
      this->add_fem_data(name, mf,
                         dim_type(gmm::vect_size(v) / mf.nb_dof()), 1);
      if (this->is_complex())
        gmm::copy(v, this->set_complex_variable(name));
      else
        gmm::copy(gmm::real_part(v), this->set_real_variable(name));
    }

    /** Add a fixed size data to the model. The data is a tensor of given
        sizes on each dof of the finite element method. */
    template <typename VECT>
    void add_initialized_fem_data(const std::string &name, const mesh_fem &mf,
                                  const VECT &v, 
                                  const bgeot::multi_index &sizes) {
      this->add_fem_data(name, mf, sizes, 1);
      if (this->is_complex())
        gmm::copy(v, this->set_complex_variable(name));
      else
        gmm::copy(gmm::real_part(v), this->set_real_variable(name));
    }

    /** Add a particular variable linked to a fem being a multiplier with
        respect to a primal variable. The dof will be filtered with the
        gmm::range_basis function applied on the terms of the model which
        link the multiplier and the primal variable. Optimized for boundary
        multipliers. niter is the number of version of the data stored,
        for time integration schemes. */
    void add_multiplier(const std::string &name, const mesh_fem &mf,
                        const std::string &primal_name,
                        size_type niter = 1);

    /** Add a particular variable linked to a fem being a multiplier with
        respect to a primal variable and a region. The dof will be filtered
        both with the gmm::range_basis function applied on the terms of
        the model which link the multiplier and the primal variable and on
        the dof on the given region. Optimized for boundary
        multipliers. niter is the number of version of the data stored,
        for time integration schemes. */
    void add_multiplier(const std::string &name, const mesh_fem &mf,
                        size_type region, const std::string &primal_name,
                        size_type niter = 1);

    /** Add a particular variable linked to a fem being a multiplier with
        respect to a primal variable. The dof will be filtered with the
        gmm::range_basis function applied on the mass matrix between the fem
        of the multiplier and the one of the primal variable.
        Optimized for boundary multipliers. niter is the number of version
        of the data stored, for time integration schemes. */
    void add_multiplier(const std::string &name, const mesh_fem &mf,
                        const std::string &primal_name, const mesh_im &mim,
                        size_type region, size_type niter = 1);


    /** Add a macro definition for the high generic assembly langage.
        This macro can be used for the definition of generic assembly bricks.
        The name of a macro cannot coincide with a variable name. */
    void add_macro(const std::string &name, const std::string &expr);

    /** Says if a macro of that name has been defined. */
    bool macro_exists(const std::string &name) const;

    /** Gives the exression string of a macro. */
    const std::string& get_macro(const std::string &name) const;

    /** Delete a variable or data of the model. */
    void delete_variable(const std::string &varname);

    /** Gives the access to the mesh_fem of a variable if any. Throw an
        exception otherwise. */
    const mesh_fem &mesh_fem_of_variable(const std::string &name) const;

    /** Gives a pointer to the mesh_fem of a variable if any. 0 otherwise.*/
    const mesh_fem *pmesh_fem_of_variable(const std::string &name) const;

    
    bgeot::multi_index qdims_of_variable(const std::string &name) const;
    size_type qdim_of_variable(const std::string &name) const;

    /** Gives the access to the tangent matrix. For the real version. */
    const model_real_sparse_matrix &real_tangent_matrix() const {
      GMM_ASSERT1(!complex_version, "This model is a complex one");
      context_check(); if (act_size_to_be_done) actualize_sizes();
      return rTM;
    }

    /** Gives the access to the tangent matrix. For the complex version. */
    const model_complex_sparse_matrix &complex_tangent_matrix() const {
      GMM_ASSERT1(complex_version, "This model is a real one");
      context_check(); if (act_size_to_be_done) actualize_sizes();
      return cTM;
    }

    /** Gives the access to the right hand side of the tangent linear system.
        For the real version. An assembly of the rhs has to be done first. */
    const model_real_plain_vector &real_rhs() const {
      GMM_ASSERT1(!complex_version, "This model is a complex one");
      context_check(); if (act_size_to_be_done) actualize_sizes();
      return rrhs;
    }

    /** Gives the access to the part of the right hand side of a term of a particular nonlinear brick. Does not account of the eventual time dispatcher. An assembly of the rhs has to be done first. For the real version. */
    const model_real_plain_vector &real_brick_term_rhs(size_type ib, size_type ind_term = 0, bool sym = false, size_type ind_iter = 0) const {
      GMM_ASSERT1(!complex_version, "This model is a complex one");
      context_check(); if (act_size_to_be_done) actualize_sizes();
      GMM_ASSERT1(valid_bricks[ib], "Inexistent brick");
      GMM_ASSERT1(ind_term < bricks[ib].tlist.size(), "Inexistent term");
      GMM_ASSERT1(ind_iter < bricks[ib].nbrhs, "Inexistent iter");
      GMM_ASSERT1(!sym || bricks[ib].tlist[ind_term].is_symmetric,
                  "Term is not symmetric");

      if (sym)
        return bricks[ib].rveclist_sym[ind_iter][ind_term];
      else
        return bricks[ib].rveclist[ind_iter][ind_term];
    }

    /** Gives access to the right hand side of the tangent linear system.
        For the complex version. */
    const model_complex_plain_vector &complex_rhs() const {
      GMM_ASSERT1(complex_version, "This model is a real one");
      context_check(); if (act_size_to_be_done) actualize_sizes();
      return crhs;
    }

    /** Gives access to the part of the right hand side of a term of a particular nonlinear brick. Does not account of the eventual time dispatcher. An assembly of the rhs has to be done first. For the real version. */
    const model_complex_plain_vector &complex_brick_term_rhs(size_type ib, size_type ind_term = 0, bool sym = false, size_type ind_iter = 0) const {
      GMM_ASSERT1(!complex_version, "This model is a complex one");
      context_check(); if (act_size_to_be_done) actualize_sizes();
      GMM_ASSERT1(valid_bricks[ib], "Inexistent brick");
      GMM_ASSERT1(ind_term < bricks[ib].tlist.size(), "Inexistent term");
      GMM_ASSERT1(ind_iter < bricks[ib].nbrhs, "Inexistent iter");
      GMM_ASSERT1(!sym || bricks[ib].tlist[ind_term].is_symmetric,
                  "Term is not symmetric");

      if (sym)
        return bricks[ib].cveclist_sym[ind_iter][ind_term];
      else
        return bricks[ib].cveclist[ind_iter][ind_term];
    }

    /** List the model variables and constant. */
    void listvar(std::ostream &ost) const;

    void listresiduals(std::ostream &ost) const;

    /** List the model bricks. */
    void listbricks(std::ostream &ost, size_type base_id = 0) const;

    /** Return the model brick ids. */
    const dal::bit_vector& get_active_bricks() const {
      return active_bricks;
    }

    /** Force the re-computation of a brick for the next assembly. */
    void touch_brick(size_type ib) {
      GMM_ASSERT1(valid_bricks[ib], "Inexistent brick");
      bricks[ib].terms_to_be_computed = true;
    }

    /** Add a brick to the model. varname is the list of variable used
        and datanames the data used. If a variable is used as a data, it
        should be declared in the datanames (it will depend on the value of
        the variable not only on the fem). Returns the brick index. */
    size_type add_brick(pbrick pbr, const varnamelist &varnames,
                        const varnamelist &datanames,
                        const termlist &terms, const mimlist &mims,
                        size_type region);

    /** Delete the brick of index ib from the model. */
    void delete_brick(size_type ib);

    /** Add an integration method to a brick. */
    void add_mim_to_brick(size_type ib, const mesh_im &mim);

    /** Change the term list of a brick. Used for very special bricks only. */
    void change_terms_of_brick(size_type ib, const termlist &terms);

    /** Change the variable list of a brick. Used for very special bricks only.
     */
    void change_variables_of_brick(size_type ib, const varnamelist &vl);

    /** Change the data list of a brick. Used for very special bricks only.
     */
    void change_data_of_brick(size_type ib, const varnamelist &vl);

    /** Change the mim list of a brick. Used for very special bricks only.
     */
    void change_mims_of_brick(size_type ib, const mimlist &ml);

    /** Change the update flag of a brick. Used for very special bricks only.
      */
    void change_update_flag_of_brick(size_type ib, bool flag);

    void set_time(scalar_type t = scalar_type(0), bool to_init = true);

    scalar_type get_time();

    void set_time_step(scalar_type dt) { time_step = dt; }
    scalar_type get_time_step() const { return time_step; }
    scalar_type get_init_time_step() const { return init_time_step; }
    int is_time_integration() const { return time_integration; }
    void set_time_integration(int ti) { time_integration = ti; }
    bool is_init_step() const { return init_step; }
    void cancel_init_step() { init_step = false; }
    void call_init_affine_dependent_variables(int version);
    void shift_variables_for_time_integration();
    void copy_init_time_derivative();
    void add_time_integration_scheme(const std::string &varname,
                                     ptime_scheme ptsc);
    void perform_init_time_derivative(scalar_type ddt)
    { init_step = true; init_time_step = ddt; }
    

    /** Add a time dispacther to a brick. */
    void add_time_dispatcher(size_type ibrick, pdispatcher pdispatch);

    void set_dispatch_coeff();

    /** For transient problems. Initialisation of iterations. */
    virtual void first_iter();

    /** For transient problems. Prepare the next iterations. In particular
        shift the version of the variables.
    */
    virtual void next_iter();

    /** Add a interpolate transformation to the model to be used with the
        generic assembly.
    */
    void add_interpolate_transformation(const std::string &name,
                                        pinterpolate_transformation ptrans) {
      if (transformations.count(name) > 0)
        GMM_ASSERT1(name.compare("neighbour_elt"), "neighbour_elt is a "
                    "reserved interpolate transformation name");
       transformations[name] = ptrans;
    }

    /** Get a pointer to the interpolate transformation `name`.
    */
    pinterpolate_transformation
    interpolate_transformation(const std::string &name) const {
      std::map<std::string, pinterpolate_transformation>::const_iterator
        it = transformations.find(name);
      GMM_ASSERT1(it != transformations.end(), "Inexistent transformation " << name);
      return it->second;
    }

    /** Tests if `name` correpsonds to an interpolate transformation.
    */
    bool interpolate_transformation_exists(const std::string &name) const
    { return transformations.count(name) > 0; }

    /** Add an elementary transformation to the model to be used with the
        generic assembly.
    */
    void add_elementary_transformation(const std::string &name,
                                       pelementary_transformation ptrans) {
       elem_transformations[name] = ptrans;
    }

    /** Get a pointer to the elementary transformation `name`.
    */
    pelementary_transformation
    elementary_transformation(const std::string &name) const {
      std::map<std::string, pelementary_transformation>::const_iterator
        it = elem_transformations.find(name);
      GMM_ASSERT1(it != elem_transformations.end(),
                  "Inexistent elementary transformation " << name);
      return it->second;
    }

    /** Tests if `name` correpsonds to an elementary transformation.
    */
    bool elementary_transformation_exists(const std::string &name) const
    { return elem_transformations.count(name) > 0; }

    /** Gives the name of the variable of index `ind_var` of the brick
        of index `ind_brick`. */
    const std::string &varname_of_brick(size_type ind_brick,
                                        size_type ind_var);

    /** Gives the name of the data of index `ind_data` of the brick
        of index `ind_brick`. */
    const std::string &dataname_of_brick(size_type ind_brick,
                                         size_type ind_data);

    /** Assembly of the tangent system taking into account the terms
        from all bricks. */
    virtual void assembly(build_version version);

    /** Gives the assembly string corresponding to the Neumann term of
        the fem variable `varname` on `region`. It is deduced from the
        assembly string declared by the model bricks.
        `region` should be the index of a boundary region
        on the mesh where `varname` is defined. Care to call this function
        only after all the volumic bricks have been declared.
        Complains, if a brick  omit to declare an assembly string.
    */
    std::string Neumann_term(const std::string &varname, size_type region);

    virtual void clear();

    explicit model(bool comp_version = false);

    /** check consistency of RHS and Stiffness matrix for brick with
        @param  ind_brick  - index of the brick
    */
    void check_brick_stiffness_rhs(size_type ind_brick) const;


  };

  //=========================================================================
  //
  //  Time integration scheme object.
  //
  //=========================================================================

  /** The time integration scheme object provides the necessary methods
      for the model object to apply a time integration scheme to an
      evolutionnary problem.
  **/
  class APIDECL virtual_time_scheme {

  public:

    virtual void init_affine_dependent_variables(model &md) const = 0;
    virtual void init_affine_dependent_variables_precomputation(model &md)
      const = 0;
    virtual void time_derivative_to_be_intialized
      (std::string &name_v, std::string &name_previous_v) const = 0;
    virtual void shift_variables(model &md) const = 0;
    virtual ~virtual_time_scheme() {}
  };

  void add_theta_method_for_first_order(model &md, const std::string &varname,
                                        scalar_type theta);

  void add_theta_method_for_second_order(model &md, const std::string &varname,
                                         scalar_type theta);
  
  void add_Newmark_scheme(model &md, const std::string &varname,
                          scalar_type beta, scalar_type gamma);



  //=========================================================================
  //
  //  Time dispatcher object.
  //
  //=========================================================================

  /** The time dispatcher object modify the result of a brick in order to
      apply a time integration scheme.
  **/
  class APIDECL virtual_dispatcher {

  protected:

    size_type nbrhs_;
    std::vector<std::string> param_names;

  public:

    size_type nbrhs() const { return nbrhs_; }

    typedef model::build_version build_version;

    virtual void set_dispatch_coeff(const model &, size_type) const
    { GMM_ASSERT1(false, "Time dispatcher with not set_dispatch_coeff !"); }

    virtual void next_real_iter
    (const model &, size_type, const model::varnamelist &,
     const model::varnamelist &,
     model::real_matlist &,
     std::vector<model::real_veclist> &,
     std::vector<model::real_veclist> &,
     bool) const {
      GMM_ASSERT1(false, "Time dispatcher with not defined first real iter !");
    }

    virtual void next_complex_iter
    (const model &, size_type, const model::varnamelist &,
     const model::varnamelist &,
     model::complex_matlist &,
     std::vector<model::complex_veclist> &,
     std::vector<model::complex_veclist> &,
     bool) const{
      GMM_ASSERT1(false,"Time dispatcher with not defined first comples iter");
    }

    virtual void asm_real_tangent_terms
    (const model &, size_type,
     model::real_matlist &, std::vector<model::real_veclist> &,
     std::vector<model::real_veclist> &,
     build_version) const {
      GMM_ASSERT1(false, "Time dispatcher with not defined real tangent "
                  "terms !");
    }

    virtual void asm_complex_tangent_terms
    (const model &, size_type,
     model::complex_matlist &, std::vector<model::complex_veclist> &,
     std::vector<model::complex_veclist> &,
     build_version) const {
      GMM_ASSERT1(false, "Time dispatcher with not defined complex tangent "
                  "terms !");
    }

    virtual_dispatcher(size_type _nbrhs) : nbrhs_(_nbrhs) {
      GMM_ASSERT1(_nbrhs > 0, "Time dispatcher with no rhs");
    }
    virtual ~virtual_dispatcher() {}

  };

  // ----------------------------------------------------------------------
  //
  // theta-method dispatcher
  //
  // ----------------------------------------------------------------------

  class APIDECL theta_method_dispatcher : public virtual_dispatcher {

  public:

    typedef model::build_version build_version;

    void set_dispatch_coeff(const model &md, size_type ib) const;

    template <typename MATLIST, typename VECTLIST>
    inline void next_iter(const model &md, size_type ib,
                          const model::varnamelist &/* vl */,
                          const model::varnamelist &/* dl */,
                          MATLIST &/* matl */,
                          VECTLIST &vectl, VECTLIST &vectl_sym,
                          bool first_iter) const {
      if (first_iter) md.update_brick(ib, model::BUILD_RHS);

      // shift the rhs
      for (size_type i = 0; i < vectl[0].size(); ++i)
        gmm::copy(vectl[0][i], vectl[1][i]);
      for (size_type i = 0; i < vectl_sym[0].size(); ++i)
        gmm::copy(vectl_sym[0][i], vectl_sym[1][i]);

      // add the component represented by the linear matrix terms to the
      // supplementary rhs
      md.linear_brick_add_to_rhs(ib, 1, 0);
    }

    void next_real_iter
    (const model &md, size_type ib, const model::varnamelist &vl,
     const model::varnamelist &dl, model::real_matlist &matl,
     std::vector<model::real_veclist> &vectl,
     std::vector<model::real_veclist> &vectl_sym, bool first_iter) const;

    void next_complex_iter
    (const model &md, size_type ib, const model::varnamelist &vl,
     const model::varnamelist &dl,
     model::complex_matlist &matl,
     std::vector<model::complex_veclist> &vectl,
     std::vector<model::complex_veclist> &vectl_sym,
     bool first_iter) const;

    void asm_real_tangent_terms
    (const model &md, size_type ib, model::real_matlist &/* matl */,
     std::vector<model::real_veclist> &/* vectl */,
     std::vector<model::real_veclist> &/* vectl_sym */,
     build_version version) const;

    virtual void asm_complex_tangent_terms
    (const model &md, size_type ib, model::complex_matlist &/* matl */,
     std::vector<model::complex_veclist> &/* vectl */,
     std::vector<model::complex_veclist> &/* vectl_sym */,
     build_version version) const;

    theta_method_dispatcher(const std::string &THETA);
  };

  //=========================================================================
  //
  //  Functions adding standard time dispatchers.
  //
  //=========================================================================

  /** Add a theta-method time dispatcher to a list of bricks. For instance,
      a matrix term $K$ will be replaced by
      $\theta K U^{n+1} + (1-\theta) K U^{n}$.
  */
  void APIDECL add_theta_method_dispatcher(model &md, dal::bit_vector ibricks,
                                   const std::string &THETA);

  /** Function which udpate the velocity $v^{n+1}$ after the computation
      of the displacement $u^{n+1}$ and before the next iteration. Specific
      for theta-method and when the velocity is included in the data
      of the model.
  */
  void APIDECL velocity_update_for_order_two_theta_method
  (model &md, const std::string &U, const std::string &V,
   const std::string &pdt, const std::string &ptheta);


  /** Add a midpoint time dispatcher to a list of bricks. For instance,
      a nonlinear term $K(U)$ will be replaced by
      $K((U^{n+1} +  U^{n})/2)$.
  */
  void APIDECL add_midpoint_dispatcher(model &md, dal::bit_vector ibricks);

  /** Function which udpate the velocity $v^{n+1}$ after the computation
      of the displacement $u^{n+1}$ and before the next iteration. Specific
      for Newmark scheme and when the velocity is included in the data
      of the model. This version inverts the mass matrix by a conjugate
      gradient.
  */
  void APIDECL velocity_update_for_Newmark_scheme
  (model &md, size_type id2dt2b, const std::string &U, const std::string &V,
   const std::string &pdt, const std::string &ptwobeta,
   const std::string &pgamma);

  //=========================================================================
  //
  //  Brick object.
  //
  //=========================================================================

  /** The virtual brick has to be derived to describe real model bricks.
      The set_flags method has to be called by the derived class.
      The virtual methods asm_real_tangent_terms and/or
      asm_complex_tangent_terms have to be defined.
      The brick should not store data. The data have to be stored in the
      model object.
  **/
  class APIDECL virtual_brick {
  protected:
    bool islinear;    // The brick add a linear term or not.
    bool issymmetric; // The brick add a symmetric term or not.
    bool iscoercive;  // The brick add a potentialy coercive terms or not.
    //   (in particular, not a term involving a multiplier)
    bool isreal;      // The brick admits a real version or not.
    bool iscomplex;   // The brick admits a complex version or not.
    bool isinit;      // internal flag.
    bool compute_each_time; // The brick is linear but needs to be computed
    // each time it is evaluated.
    bool hasNeumannterm; // The brick declares at list a Neumann term.
    bool isUpdateBrick;  // The brick does not contribute any terms to the
    // system matrix or right-hand side, but only updates state variables.
    std::string name; // Name of the brick.

  public:

    typedef model::build_version build_version;

    virtual_brick() { isinit = false; }
    virtual ~virtual_brick() { }
    void set_flags(const std::string &bname, bool islin, bool issym,
                   bool iscoer, bool ire, bool isco, bool each_time = false,
                   bool hasNeumannt = true) {
      name = bname;
      islinear = islin; issymmetric = issym; iscoercive = iscoer;
      isreal = ire; iscomplex = isco; isinit = true;
      compute_each_time = each_time; hasNeumannterm = hasNeumannt;
    }

#   define BRICK_NOT_INIT GMM_ASSERT1(isinit, "Set brick flags !")
    bool is_linear()    const { BRICK_NOT_INIT; return islinear;    }
    bool is_symmetric() const { BRICK_NOT_INIT; return issymmetric; }
    bool is_coercive()  const { BRICK_NOT_INIT; return iscoercive;  }
    bool is_real()      const { BRICK_NOT_INIT; return isreal;      }
    bool is_complex()   const { BRICK_NOT_INIT; return iscomplex;   }
    bool has_Neumann_term() const {BRICK_NOT_INIT; return hasNeumannterm; }
    bool is_to_be_computed_each_time() const
    { BRICK_NOT_INIT; return compute_each_time; }
    const std::string &brick_name() const { BRICK_NOT_INIT; return name; }


    /** Assembly of bricks real tangent terms.
        In case of Getfem's compilation with OpenMP option,
        this method is executed on multiple threads.
        The parallelism is provided by distributing all tangent matrices and
        vectors and accumulating them later into the original. Additionally,
        by default, all mesh_region objects, participating in the assembly,
        are also partitioned. In order to avoid data race conditions, this
        method should not modify any data simultaneously accessible from
        multiple threads. In case this is unavoidable, the race can be
        prevented by distributing this data (of type T) between the threads
        via getfem::omp_distribute<T> (prefered method) or
        protected from concurrent access with mutexes (e.g. getfem::omp_lock)
        or OpenMP critical section. */
    virtual void asm_real_tangent_terms(const model &, size_type,
                                        const model::varnamelist &,
                                        const model::varnamelist &,
                                        const model::mimlist &,
                                        model::real_matlist &,
                                        model::real_veclist &,
                                        model::real_veclist &,
                                        size_type, build_version) const
    { /** doesn't have to be overriden if serial pre- post- assemblies are
          defined */
    }


    /** Assembly of bricks complex tangent terms.
        In case of Getfem's compilation with OpenMP option,
        this method is executed on multiple threads.
        The parallelism is provided by distributing all tangent matrices and
        vectors and accumulating them later into the original. Additionally,
        by default, all mesh_region objects, participating in the assembly,
        are also partitioned. In order to avoid data race conditions, this
        method should not modify any data simultaneously accessible from
        multiple threads. In case this is unavoidable, the race can be
        prevented by distributing this data (of type T) between the threads
        via getfem::omp_distribute<T> (prefered method) or
        protected from concurrent access with mutexes (e.g. getfem::omp_lock)
        or OpenMP critical section. */
    virtual void asm_complex_tangent_terms(const model &, size_type,
                                           const model::varnamelist &,
                                           const model::varnamelist &,
                                           const model::mimlist &,
                                           model::complex_matlist &,
                                           model::complex_veclist &,
                                           model::complex_veclist &,
                                           size_type, build_version) const
    { /** doesn't have to be overriden if serial pre- post- assemblies are
          defined*/
    }


    /** Peform any pre assembly action for real term assembly. The purpose of
        this method is to do any action that cannot be peformed in the main
        assembly routines in parallel.
        Possible action can be modification of the model object, cashing
        some data that cannot be distributed etc. */
    virtual void real_pre_assembly_in_serial(const model &, size_type,
                                        const model::varnamelist &,
                                        const model::varnamelist &,
                                        const model::mimlist &,
                                        model::real_matlist &,
                                        model::real_veclist &,
                                        model::real_veclist &,
                                        size_type, build_version) const { };

    /** Peform any pre assembly action for complex term assembly. The purpose
        of this method is to do any action that cannot be peformed in the
        main assembly routines in parallel.
        Possible action can be modification of the model object, cashing
        some data that cannot be distributed etc. */
    virtual void complex_pre_assembly_in_serial(const model &, size_type,
                                        const model::varnamelist &,
                                        const model::varnamelist &,
                                        const model::mimlist &,
                                        model::complex_matlist &,
                                        model::complex_veclist &,
                                        model::complex_veclist &,
                                        size_type, build_version) const { };

    /** Peform any post assembly action for real terms. The purpose of this
        method is to do any action that cannot be peformed in the main
        assembly routines in parallel.
        Possible action can be modification of the model object, cashing
        some data that cannot be distributed etc. */
    virtual void real_post_assembly_in_serial(const model &, size_type,
                                        const model::varnamelist &,
                                        const model::varnamelist &,
                                        const model::mimlist &,
                                        model::real_matlist &,
                                        model::real_veclist &,
                                        model::real_veclist &,
                                        size_type, build_version) const { };

    /** Peform any post assembly action for complex terms. The purpose of this
        method is to do any action that cannot be peformed in the main
        assembly routines in parallel.
        Possible action can be modification of the model object, cashing
        some data that cannot be distributed etc. */
    virtual void complex_post_assembly_in_serial(const model &, size_type,
                                        const model::varnamelist &,
                                        const model::varnamelist &,
                                        const model::mimlist &,
                                        model::complex_matlist &,
                                        model::complex_veclist &,
                                        model::complex_veclist &,
                                        size_type, build_version) const { };


    /** check consistency of stiffness matrix and rhs */
    void check_stiffness_matrix_and_rhs(const model &, size_type,
                                        const model::termlist& tlist,
                                        const model::varnamelist &,
                                        const model::varnamelist &,
                                        const model::mimlist &,
                                        model::real_matlist &,
                                        model::real_veclist &,
                                        model::real_veclist &, size_type rg,
                                        const scalar_type delta = 1e-8) const;
    /** The brick may declare an assembly string for the computation of the
        Neumann terms (in order to prescribe boundary conditions with
        Nitche's method). */
    virtual std::string declare_volume_assembly_string
    (const model &, size_type, const model::varnamelist &,
     const model::varnamelist &) const {
      GMM_ASSERT1(false, "No assemby string declared, computation of Neumann "
                  "term impossible for brick " << name);
    }

    private:
      /** simultaneous call to real_pre_assembly, real_assembly
          and real_post_assembly */
      void full_asm_real_tangent_terms_(const model &, size_type,
        const model::varnamelist &,
        const model::varnamelist &,
        const model::mimlist &,
        model::real_matlist &,
        model::real_veclist &,
        model::real_veclist &,
        size_type, build_version) const;
  };

  //=========================================================================
  //
  //  Virtual interpolate_transformation object.
  //
  //=========================================================================

  struct var_trans_pair {
    std::string varname, transname;
    bool operator <(const var_trans_pair &vt) const {
      return (varname < vt.varname) ||
             (!(varname > vt.varname) && transname < vt.transname);
    }
    var_trans_pair() : varname(), transname() {}
    var_trans_pair(const std::string &v, const std::string &t)
      : varname(v), transname(t) {}
  };

  class APIDECL virtual_interpolate_transformation {

  public:
    virtual void extract_variables
    (const ga_workspace &workspace, std::set<var_trans_pair> &vars,
     bool ignore_data, const mesh &m,
     const std::string &interpolate_name) const = 0;
    virtual void init(const ga_workspace &workspace) const = 0;
    virtual int transform
    (const ga_workspace &workspace, const mesh &m,
     fem_interpolation_context &ctx_x, const base_small_vector &Normal,
     const mesh **m_t, size_type &cv, short_type &face_num,
     base_node &P_ref, base_small_vector &N_y,
     std::map<var_trans_pair, base_tensor> &derivatives,
     bool compute_derivatives) const = 0;
    virtual void finalize() const = 0;

    virtual ~virtual_interpolate_transformation() {}
  };

  //=========================================================================
  //
  //  Virtual elementary_transformation object.
  //
  //=========================================================================

  class APIDECL virtual_elementary_transformation {

  public:
    
    virtual void give_transformation(const mesh_fem &mf, size_type cv,
                                     base_matrix &M) const = 0;
    virtual ~virtual_elementary_transformation() {}
  };

  //=========================================================================
  //
  //  Neumann term object.
  //
  //=========================================================================

  /* For a PDE in a weak form, the Neumann condition correspond to
     prescribe a certain derivative of the unkown (the normal derivative
     for the Poisson problem for instance). The Neumann term objects allows
     to compute the finite element approximation of this certain derivative.
     This allows, first ot have an estimate of this term (for instance, it can
     give an approximation of the stress at the boundary in a problem of
     linear elasticity) but also it allows to prescribe some boundary
     conditions with Nitsche's method (For dirichlet or contact boundary
     conditions for instance).
  */

  struct APIDECL Neumann_elem_term  {

    std::vector<std::string> auxilliary_variables;

    // The function should return the Neumann term when version = 1,
    // its derivative when version = 2 and its second derivative
    // when version = 3.
    // CAUTION : The output tensor has the right size and the reult has to
    //           be ADDED. previous additions of other term have not to be
    //           erased.

    virtual void compute_Neumann_term
    (int version, const mesh_fem &/*mfvar*/,
     const model_real_plain_vector &/*var*/,
     fem_interpolation_context& /*ctx*/,
     base_small_vector &/*n*/, base_tensor &/*output*/,
     size_type /*auxilliary_ind*/ = 0) const = 0;

    virtual ~Neumann_elem_term() {}

  };





  //=========================================================================
  //
  //  Functions adding standard bricks to the model.
  //
  //=========================================================================

  /** Add a matrix term given by the assembly string `expr` which will
      be assembled in region `region` and with the integration method `mim`.
      Only the matrix term will be taken into account, assuming that it is
      linear.
      The advantage of declaring a term linear instead of nonlinear is that
      it will be assembled only once and no assembly is necessary for the
      residual.
      Take care that if the expression contains some variables and if the
      expression is a potential or of first order (i.e. describe the weak
      form, not the derivative of the weak form), the expression will be
      derivated with respect to all variables.
      You can specify if the term is symmetric, coercive or not.
      If you are not sure, the better is to declare the term not symmetric
      and not coercive. But some solvers (conjugate gradient for instance)
      are not allowed for non-coercive problems.
      `brickname` is an otpional name for the brick.
  */
  size_type APIDECL add_linear_generic_assembly_brick
  (model &md, const mesh_im &mim, const std::string &expr,
   size_type region = size_type(-1), bool is_sym = false,
   bool is_coercive = false, std::string brickname = "",
   bool return_if_nonlin = false);

  /** Add a nonlinear term given by the assembly string `expr` which will
      be assembled in region `region` and with the integration method `mim`.
      The expression can describe a potential or a weak form. Second order
      terms (i.e. containing second order test functions, Test2) are not
      allowed.
      You can specify if the term is symmetric, coercive or not.
      If you are not sure, the better is to declare the term not symmetric
      and not coercive. But some solvers (conjugate gradient for instance)
      are not allowed for non-coercive problems.
      `brickname` is an otpional name for the brick.
  */
  size_type APIDECL add_nonlinear_generic_assembly_brick
  (model &md, const mesh_im &mim, const std::string &expr,
   size_type region = size_type(-1), bool is_sym = false,
   bool is_coercive = false, std::string brickname = "");


  /** Add a source term given by the assembly string `expr` which will
      be assembled in region `region` and with the integration method `mim`.
      Only the residual term will be taken into account.
      Take care that if the expression contains some variables and if the
      expression is a potential, the expression will be
      derivated with respect to all variables.
      `brickname` is an otpional name for the brick.
  */
  size_type APIDECL add_source_term_generic_assembly_brick
  (model &md, const mesh_im &mim, const std::string &expr,
   size_type region = size_type(-1),  std::string brickname = "",
   std::string directvarname = std::string(),
   const std::string &directdataname = std::string(),
   bool return_if_nonlin = false);

  /** Add a Laplacian term on the variable `varname` (in fact with a minus :
      :math:`-\text{div}(\nabla u)`). If it is a vector
      valued variable, the Laplacian term is componentwise. `region` is an
      optional mesh region on which the term is added. Return the brick index
      in the model.
  */
  size_type APIDECL add_Laplacian_brick
  (model &md, const mesh_im &mim, const std::string &varname,
   size_type region = size_type(-1));


  /** Add an elliptic term on the variable `varname`.
      The shape of the elliptic
      term depends both on the variable and the data. This corresponds to a
      term $-\text{div}(a\nabla u)$ where $a$ is the data and $u$ the variable.
      The data can be a scalar, a matrix or an order four tensor. The variable
      can be vector valued or not. If the data is a scalar or a matrix and
      the variable is vector valued then the term is added componentwise.
      An order four tensor data is allowed for vector valued variable only.
      The data can be constant or describbed on a fem. Of course, when
      the data is a tensor describe on a finite element method (a tensor
      field) the data can be a huge vector. The components of the
      matrix/tensor have to be stored with the fortran order (columnwise) in
      the data vector (compatibility with blas). The symmetry and coercivity
      of the given matrix/tensor is not verified (but assumed). `region` is an
      optional mesh region on which the term is added. Note that for the real
      version which uses the high-level generic assembly language, `dataexpr`
      can be any regular expression of the high-level generic assembly
      language (like "1", "sin(X[0])" or "Norm(u)" for instance) even
      depending on model variables.
      Return the brick index in the model.
  */
  size_type APIDECL add_generic_elliptic_brick
  (model &md, const mesh_im &mim, const std::string &varname,
   const std::string &dataexpr, size_type region = size_type(-1));


  /** Add a source term on the variable `varname`. The source term is
      represented by `dataexpr` which could be any regular expression of the
      high-level generic assembly language (except for the complex version
      where it has to be a declared data of the model). `region` is an
      optional mesh region on which the term is
      added. An additional optional data `directdataname` can be provided. The
      corresponding data vector will be directly added to the right hand
      side without assembly. Return the brick index in the model.
  */
  size_type APIDECL add_source_term_brick
  (model &md, const mesh_im &mim, const std::string &varname,
   const std::string &dataexpr, size_type region = size_type(-1),
   const std::string &directdataname = std::string());

  /** Add a source term on the variable `varname` on a boundary `region`.
      The source term is
      represented by the data `dataepxpr` which could be any regular
      expression of the high-level generic assembly language (except
      for the complex version where it has to be a declared data of
      the model). A scalar product with the outward normal unit vector to
      the boundary is performed. The main aim of this brick is to represent
      a Neumann condition with a vector data without performing the
      scalar product with the normal as a pre-processing.
  */
  size_type APIDECL add_normal_source_term_brick
  (model &md, const mesh_im &mim, const std::string &varname,
   const std::string &dataexpr, size_type region);


  /** Add a (simple) Dirichlet condition on the variable `varname` and
      the mesh region `region`. The Dirichlet condition is prescribed by
      a simple post-treatment of the final linear system (tangent system
      for nonlinear problems) consisting of modifying the lines corresponding
      to the degree of freedom of the variable on `region` (0 outside the
      diagonal, 1 on the diagonal of the matrix and the expected value on
      the right hand side).
      The symmetry of the linear system is kept if all other bricks are
      symmetric.
      This brick is to be reserved for simple Dirichlet conditions (only dof
      declared on the correspodning boundary are prescribed). The application
      of this brick on reduced f.e.m. may be problematic. Intrinsic vectorial
      finite element method are not supported.
      `dataname` is the optional right hand side of  the Dirichlet condition.
      It could be constant or (important) described on the same finite
      element method as `varname`.
      Returns the brick index in the model.
  */
  size_type APIDECL add_Dirichlet_condition_with_simplification
  (model &md, const std::string &varname, size_type region,
   const std::string &dataname = std::string());


  /** Add a Dirichlet condition on the variable `varname` and the mesh
      region `region`. This region should be a boundary. The Dirichlet
      condition is prescribed with a multiplier variable `multname` which
      should be first declared as a multiplier
      variable on the mesh region in the model. `dataname` is the optional
      right hand side of  the Dirichlet condition. It could be constant or
      described on a fem; scalar or vector valued, depending on the variable
      on which the Dirichlet condition is prescribed. Return the brick index
      in the model.
  */
  size_type APIDECL add_Dirichlet_condition_with_multipliers
  (model &md, const mesh_im &mim, const std::string &varname,
   const std::string &multname, size_type region,
   const std::string &dataname = std::string());

  /** Same function as the previous one but the multipliers variable will
      be declared to the brick by the function. `mf_mult` is the finite element
      method on which the multiplier will be build (it will be restricted to
      the mesh region `region` and eventually some conflicting dofs with some
      other multiplier variables will be suppressed).
  */
  size_type APIDECL add_Dirichlet_condition_with_multipliers
  (model &md, const mesh_im &mim, const std::string &varname,
   const mesh_fem &mf_mult, size_type region,
   const std::string &dataname = std::string());

  /** Same function as the previous one but the `mf_mult` parameter is
      replaced by `degree`. The multiplier will be described on a standard
      finite element method of the corresponding degree.
  */
  size_type APIDECL add_Dirichlet_condition_with_multipliers
  (model &md, const mesh_im &mim, const std::string &varname,
   dim_type degree, size_type region,
   const std::string &dataname = std::string());


  /** When `ind_brick` is the index of a Dirichlet brick with multiplier on
      the model `md`, the function return the name of the multiplier variable.
      Otherwise, it has an undefined behavior.
  */
  const APIDECL std::string &mult_varname_Dirichlet(model &md, size_type ind_brick);

  /** Add a Dirichlet condition on the variable `varname` and the mesh
      region `region`. This region should be a boundary. The Dirichlet
      condition is prescribed with penalization. The penalization coefficient
      is intially `penalization_coeff` and will be added to the data of
      the model. `dataname` is the optional
      right hand side of  the Dirichlet condition. It could be constant or
      described on a fem; scalar or vector valued, depending on the variable
      on which the Dirichlet condition is prescribed.
      `mf_mult` is an optional parameter which allows to weaken the
      Dirichlet condition specifying a multiplier space.
      Returns the brick index in the model.
  */
  size_type APIDECL add_Dirichlet_condition_with_penalization
  (model &md, const mesh_im &mim, const std::string &varname,
   scalar_type penalization_coeff, size_type region,
   const std::string &dataname = std::string(),
   const mesh_fem *mf_mult = 0);

  /** Add a Dirichlet condition on the variable `varname` and the mesh
      region `region`. This region should be a boundary. `Neumannterm`
      is the expression of the Neumann term (obtained by the Green formula)
      described as an expression of the high-level
      generic assembly language. This term can be obtained with
      md. Neumann_term(varname, region) once all volumic bricks have
      been added to the model. The Dirichlet
      condition is prescribed with Nitsche's method. `datag` is the optional
      right hand side of the Dirichlet condition. `datagamma0` is the
      Nitsche's method parameter. `theta` is a scalar value which can be
      positive or negative. `theta = 1` corresponds to the standard symmetric
      method which is conditionnaly coercive for  `gamma0` small.
      `theta = -1` corresponds to the skew-symmetric method which is
      inconditionnaly coercive. `theta = 0` is the simplest method
      for which the second derivative of the Neumann term is not necessary
      even for nonlinear problems. Return the brick index in the model.
  */
  size_type APIDECL add_Dirichlet_condition_with_Nitsche_method
  (model &md, const mesh_im &mim, const std::string &varname,
   const std::string &Neumannterm,
   const std::string &datagamma0, size_type region,
   scalar_type theta = scalar_type(0),
   const std::string &datag = std::string());


  /** Add a Dirichlet condition to the normal component of the vector
      (or tensor) valued variable `varname` and the mesh
      region `region`. This region should be a boundary. The Dirichlet
      condition is prescribed with a multiplier variable `multname` which
      should be first declared as a multiplier
      variable on the mesh region in the model. `dataname` is the optional
      right hand side of  the normal Dirichlet condition.
      It could be constant or
      described on a fem; scalar or vector valued, depending on the variable
      on which the Dirichlet condition is prescribed (scalar if the variable
      is vector valued, vector if the variable is tensor valued).
      Returns the brick index in the model.
  */
  size_type APIDECL add_normal_Dirichlet_condition_with_multipliers
  (model &md, const mesh_im &mim, const std::string &varname,
   const std::string &multname, size_type region,
   const std::string &dataname = std::string());

  /** Same function as the previous one but the multipliers variable will
      be declared to the brick by the function. `mf_mult` is the finite element
      method on which the multiplier will be build (it will be restricted to
      the mesh region `region` and eventually some conflicting dofs with some
      other multiplier variables will be suppressed).
  */
  size_type APIDECL add_normal_Dirichlet_condition_with_multipliers
  (model &md, const mesh_im &mim, const std::string &varname,
   const mesh_fem &mf_mult, size_type region,
   const std::string &dataname = std::string());

  /** Same function as the previous one but the `mf_mult` parameter is
      replaced by `degree`. The multiplier will be described on a standard
      finite element method of the corresponding degree.
  */
  size_type APIDECL add_normal_Dirichlet_condition_with_multipliers
  (model &md, const mesh_im &mim, const std::string &varname,
   dim_type degree, size_type region,
   const std::string &dataname = std::string());

  /** Add a Dirichlet condition to the normal component of the vector
      (or tensor) valued variable `varname` and the mesh
      region `region`. This region should be a boundary. The Dirichlet
      condition is prescribed with penalization. The penalization coefficient
      is intially `penalization_coeff` and will be added to the data of
      the model. `dataname` is the optional
      right hand side of  the Dirichlet condition. It could be constant or
      described on a fem; scalar or vector valued, depending on the variable
      on which the Dirichlet condition is prescribed (scalar if the variable
      is vector valued, vector if the variable is tensor valued).
      `mf_mult` is an optional parameter which allows to weaken the
      Dirichlet condition specifying a multiplier space.
      Return the brick index in the model.
  */
  size_type APIDECL add_normal_Dirichlet_condition_with_penalization
  (model &md, const mesh_im &mim, const std::string &varname,
   scalar_type penalization_coeff, size_type region,
   const std::string &dataname = std::string(),
   const mesh_fem *mf_mult = 0);



  /** Add a Dirichlet condition on the normal component of the variable
      `varname` and the mesh
      region `region`. This region should be a boundary. `Neumannterm`
      is the expression of the Neumann term (obtained by the Green formula)
      described as an expression of the high-level
      generic assembly language. This term can be obtained with
      md.Neumann_term(varname, region) once all volumic bricks have
      been added to the model.The Dirichlet
      condition is prescribed with Nitsche's method. `datag` is the optional
      scalar right hand side of the Dirichlet condition. `datagamma0` is the
      Nitsche's method parameter. `theta` is a scalar value which can be
      positive or negative. `theta = 1` corresponds to the standard symmetric
      method which is conditionnaly coercive for  `gamma0` small.
      `theta = -1` corresponds to the skew-symmetric method which is
      inconditionnaly coercive. `theta = 0` is the simplest method
      for which the second derivative of the Neumann term is not necessary
      even for nonlinear problems. Return the brick index in the model.
  */
  size_type APIDECL add_normal_Dirichlet_condition_with_Nitsche_method
  (model &md, const mesh_im &mim, const std::string &varname,
   const std::string &Neumannterm, const std::string &datagamma0,
   size_type region, scalar_type theta = scalar_type(0),
   const std::string &datag = std::string());


  /** Add some pointwise constraints on the variable `varname` thanks to
      a penalization. The penalization coefficient is initially
      `penalization_coeff` and will be added to the data of the model.
      The conditions are prescribed on a set of points given in the data
      `dataname_pt` whose dimension is the number of points times the dimension
      of the mesh. If the variable represents a vector field, the data
      `dataname_unitv` represents a vector of dimension the number of points
      times the dimension of the vector field which should store some
      unit vectors. In that case the prescribed constraint is the scalar
      product of the variable at the corresponding point with the corresponding
      unit vector.
      The optional data `dataname_val` is the vector of values to be prescribed
      at the different points.
      This brick is specifically designed to kill rigid displacement
      in a Neumann problem.
  */
  size_type APIDECL add_pointwise_constraints_with_penalization
  (model &md, const std::string &varname,
   scalar_type penalisation_coeff, const std::string &dataname_pt,
   const std::string &dataname_unitv = std::string(),
   const std::string &dataname_val = std::string());


  /** Add some pointwise constraints on the variable `varname` using a given
      multiplier `multname`.
      The conditions are prescribed on a set of points given in the data
      `dataname_pt` whose dimension is the number of points times the dimension
      of the mesh.
      The multiplier variable should be a fixed size variable of size the
      number of points.
      If the variable represents a vector field, the data
      `dataname_unitv` represents a vector of dimension the number of points
      times the dimension of the vector field which should store some
      unit vectors. In that case the prescribed constraint is the scalar
      product of the variable at the corresponding point with the corresponding
      unit vector.
      The optional data `dataname_val` is the vector of values to be prescribed
      at the different points.
      This brick is specifically designed to kill rigid displacement
      in a Neumann problem.
  */
  size_type APIDECL add_pointwise_constraints_with_given_multipliers
  (model &md, const std::string &varname,
   const std::string &multname, const std::string &dataname_pt,
   const std::string &dataname_unitv = std::string(),
   const std::string &dataname_val = std::string());

  /** Add some pointwise constraints on the variable `varname` using
      multiplier. The multiplier variable is automatically added to the model.
      The conditions are prescribed on a set of points given in the data
      `dataname_pt` whose dimension is the number of points times the dimension
      of the mesh.
      If the variable represents a vector field, the data
      `dataname_unitv` represents a vector of dimension the number of points
      times the dimension of the vector field which should store some
      unit vectors. In that case the prescribed constraint is the scalar
      product of the variable at the corresponding point with the corresponding
      unit vector.
      The optional data `dataname_val` is the vector of values to be prescribed
      at the different points.
      This brick is specifically designed to kill rigid displacement
      in a Neumann problem.
  */
  size_type APIDECL add_pointwise_constraints_with_multipliers
  (model &md, const std::string &varname, const std::string &dataname_pt,
   const std::string &dataname_unitv = std::string(),
   const std::string &dataname_val = std::string());


  /** Change the penalization coefficient of a Dirichlet condition with
      penalization brick. If the brick is not of this kind,
      this function has an undefined behavior.
  */
  void APIDECL change_penalization_coeff(model &md, size_type ind_brick,
                                         scalar_type penalisation_coeff);

  /** Add a generalized Dirichlet condition on the variable `varname` and
      the mesh region `region`. This version is for vector field.
      It prescribes a condition @f$ Hu = r @f$ where `H` is a matrix field.
      This region should be a boundary. The Dirichlet
      condition is prescribed with a multiplier variable `multname` which
      should be first declared as a multiplier
      variable on the mesh region in the model. `dataname` is the
      right hand side of  the Dirichlet condition. It could be constant or
      described on a fem; scalar or vector valued, depending on the variable
      on which the Dirichlet condition is prescribed. `Hname' is the data
      corresponding to the matrix field `H`. It has to be a constant matrix
      or described on a scalar fem. Return the brick index in the model.
  */
  size_type APIDECL add_generalized_Dirichlet_condition_with_multipliers
  (model &md, const mesh_im &mim, const std::string &varname,
   const std::string &multname, size_type region,
   const std::string &dataname, const std::string &Hname);

  /** Same function as the preceeding one but the multipliers variable will
      be declared to the brick by the function. `mf_mult` is the finite element
      method on which the multiplier will be build (it will be restricted to
      the mesh region `region` and eventually some conflicting dofs with some
      other multiplier variables will be suppressed).
  */
  size_type APIDECL add_generalized_Dirichlet_condition_with_multipliers
  (model &md, const mesh_im &mim, const std::string &varname,
   const mesh_fem &mf_mult, size_type region,
   const std::string &dataname, const std::string &Hname);

  /** Same function as the preceeding one but the `mf_mult` parameter is
      replaced by `degree`. The multiplier will be described on a standard
      finite element method of the corresponding degree.
  */
  size_type APIDECL add_generalized_Dirichlet_condition_with_multipliers
  (model &md, const mesh_im &mim, const std::string &varname,
   dim_type degree, size_type region,
   const std::string &dataname, const std::string &Hname);

  /** Add a Dirichlet condition on the variable `varname` and the mesh
      region `region`. This version is for vector field.
      It prescribes a condition @f$ Hu = r @f$ where `H` is a matrix field.
      This region should be a boundary. This region should be a boundary.
      The Dirichlet
      condition is prescribed with penalization. The penalization coefficient
      is intially `penalization_coeff` and will be added to the data of
      the model. `dataname` is the
      right hand side of  the Dirichlet condition. It could be constant or
      described on a fem; scalar or vector valued, depending on the variable
      on which the Dirichlet condition is prescribed. `Hname' is the data
      corresponding to the matrix field `H`. It has to be a constant matrix
      or described on a scalar fem. `mf_mult` is an optional parameter
      which allows to weaken the Dirichlet condition specifying a
      multiplier space. Return the brick index in the model.
  */
  size_type APIDECL add_generalized_Dirichlet_condition_with_penalization
  (model &md, const mesh_im &mim, const std::string &varname,
   scalar_type penalization_coeff, size_type region,
   const std::string &dataname, const std::string &Hname,
   const mesh_fem *mf_mult = 0);

  /** Add a Dirichlet condition on the variable `varname` and the mesh
      region `region`. This region should be a boundary. This version
      is for vector field. It prescribes a condition
      @f$ Hu = r @f$ where `H` is a matrix field. `Neumannterm`
      is the expression of the Neumann term (obtained by the Green formula)
      described as an expression of the high-level
      generic assembly language. of the high-level
      generic assembly language. This term can be obtained with
      md.Neumann_term(varname, region) once all volumic bricks have
      been added to the model. The Dirichlet
      condition is prescribed with Nitsche's method. `datag` is the optional
      right hand side of the Dirichlet condition. `datagamma0` is the
      Nitsche's method parameter. `theta` is a scalar value which can be
      positive or negative. `theta = 1` corresponds to the standard symmetric
      method which is conditionnaly coercive for  `gamma0` small.
      `theta = -1` corresponds to the skew-symmetric method which is
      inconditionnaly coercive. `theta = 0` is the simplest method
      for which the second derivative of the Neumann term is not necessary
      even for nonlinear problems. Return the brick index in the model.
  */
  size_type APIDECL add_generalized_Dirichlet_condition_with_Nitsche_method
  (model &md, const mesh_im &mim, const std::string &varname,
   const std::string &Neumannterm, const std::string &datagamma0,
   size_type region, scalar_type theta,
   const std::string &datag, const std::string &dataH);


  /** Add a Helmoltz brick to the model. This corresponds to the scalar
      equation (@f$\Delta u + k^2u = 0@f$, with @f$K=k^2@f$).
      The weak formulation is (@f$\int k^2 u.v - \nabla u.\nabla v@f$)

      `dataexpr` should contain the wave number $k$. It can be real or
      complex.
  */
  size_type APIDECL add_Helmholtz_brick(model &md, const mesh_im &mim,
                                        const std::string &varname,
                                        const std::string &dataexpr,
                                        size_type region = size_type(-1));


  /** Add a Fourier-Robin brick to the model. This correspond to the weak term
      (@f$\int (qu).v @f$) on a boundary. It is used to represent a
      Fourier-Robin boundary condition.

      `dataexpr` is the parameter $q$ which should be a
      (@f$N\times N@f$) matrix term, where $N$ is the dimension of the
      variable `varname`. It can be an arbitrary valid expression of the
      high-level generic assembly language (except for the complex version
      for which it should be a data of the model). Note that an additional
      right hand side can be added with a source term brick.
  */
  size_type APIDECL add_Fourier_Robin_brick(model &md, const mesh_im &mim,
                                            const std::string &varname,
                                            const std::string &dataexpr,
                                            size_type region);


  // Constraint brick.
  model_real_sparse_matrix APIDECL &set_private_data_brick_real_matrix
  (model &md, size_type indbrick);
  model_real_plain_vector APIDECL &set_private_data_brick_real_rhs
  (model &md, size_type indbrick);
  model_complex_sparse_matrix APIDECL &set_private_data_brick_complex_matrix
  (model &md, size_type indbrick);
  model_complex_plain_vector APIDECL &set_private_data_brick_complex_rhs
  (model &md, size_type indbrick);
  size_type APIDECL add_constraint_with_penalization
  (model &md, const std::string &varname, scalar_type penalisation_coeff);
  size_type APIDECL add_constraint_with_multipliers
  (model &md, const std::string &varname, const std::string &multname);

  void set_private_data_rhs
  (model &md, size_type indbrick, const std::string &varname);

  template <typename VECT, typename T>
  void set_private_data_rhs(model &md, size_type ind,
                            const VECT &L, T) {
    model_real_plain_vector &LL = set_private_data_brick_real_rhs(md, ind);
    gmm::resize(LL, gmm::vect_size(L));
    gmm::copy(L, LL);
  }

  template <typename VECT, typename T>
  void set_private_data_rhs(model &md, size_type ind, const VECT &L,
                            std::complex<T>) {
    model_complex_plain_vector &LL = set_private_data_brick_complex_rhs(md, ind);
    gmm::resize(LL, gmm::vect_size(L));
    gmm::copy(L, LL);
  }

  /** For some specific bricks having an internal right hand side vector
      (explicit bricks: 'constraint brick' and 'explicit rhs brick'),
      set this rhs.
  */
  template <typename VECT>
  void set_private_data_rhs(model &md, size_type indbrick, const VECT &L) {
    typedef typename gmm::linalg_traits<VECT>::value_type T;
    set_private_data_rhs(md, indbrick, L, T());
  }

  template <typename MAT, typename T>
  void set_private_data_matrix(model &md, size_type ind,
                               const MAT &B, T) {
    model_real_sparse_matrix &BB = set_private_data_brick_real_matrix(md, ind);
    gmm::resize(BB, gmm::mat_nrows(B), gmm::mat_ncols(B));
    gmm::copy(B, BB);
  }

  template <typename MAT, typename T>
  void set_private_data_matrix(model &md, size_type ind, const MAT &B,
                               std::complex<T>) {
    model_complex_sparse_matrix &BB
      = set_private_data_brick_complex_matrix(md, ind);
    gmm::resize(BB, gmm::mat_nrows(B), gmm::mat_ncols(B));
    gmm::copy(B, BB);
  }

  /** For some specific bricks having an internal sparse matrix
      (explicit bricks: 'constraint brick' and 'explicit matrix brick'),
      set this matrix. @*/
  template <typename MAT>
  void set_private_data_matrix(model &md, size_type indbrick,
                               const MAT &B) {
    typedef typename gmm::linalg_traits<MAT>::value_type T;
    set_private_data_matrix(md, indbrick, B, T());
  }

  /** Add an additional explicit penalized constraint on the variable
      `varname`. The constraint is $BU=L$ with `B` being a rectangular
      sparse matrix.
      Be aware that `B` should not contain a plain row, otherwise the whole
      tangent matrix will be plain. It is possible to change the constraint
      at any time with the methods set_private_matrix and set_private_rhs.
      The method change_penalization_coeff can also be used.
  */
  template <typename MAT, typename VECT>
  size_type add_constraint_with_penalization
  (model &md, const std::string &varname, scalar_type penalisation_coeff,
   const MAT &B, const VECT &L) {
    size_type ind
      = add_constraint_with_penalization(md, varname, penalisation_coeff);
    size_type n = gmm::mat_nrows(B), m = gmm::mat_ncols(B);
    set_private_data_rhs(md, ind, L);
    set_private_data_matrix(md, ind, B);
    return ind;
  }

  /** Add an additional explicit constraint on the variable `varname` thank to
      a multiplier `multname` peviously added to the model (should be a fixed
      size variable).
      The constraint is $BU=L$ with `B` being a rectangular sparse matrix.
      It is possible to change the constraint
      at any time with the methods set_private_matrix
      and set_private_rhs.
  */
  template <typename MAT, typename VECT>
  size_type add_constraint_with_multipliers
  (model &md, const std::string &varname, const std::string &multname,
   const MAT &B, const VECT &L) {
    size_type ind = add_constraint_with_multipliers(md, varname, multname);
    set_private_data_rhs(md, ind, L);
    set_private_data_matrix(md, ind, B);
    return ind;
  }

  template <typename MAT>
  size_type add_constraint_with_multipliers
  (model &md, const std::string &varname, const std::string &multname,
   const MAT &B, const std::string &Lname) {
    size_type ind = add_constraint_with_multipliers(md, varname, multname);
    set_private_data_rhs(md, ind, Lname);
    set_private_data_matrix(md, ind, B);
    return ind;
  }

  size_type APIDECL add_explicit_matrix(model &md, const std::string &varname1,
                                        const std::string &varname2,
                                        bool issymmetric, bool iscoercive);
  size_type APIDECL add_explicit_rhs(model &md, const std::string &varname);

  /** Add a brick reprenting an explicit matrix to be added to the tangent
      linear system relatively to the variables 'varname1' and 'varname2'.
      The given matrix should have as many rows as the dimension of
      'varname1' and as many columns as the dimension of 'varname2'.
      If the two variables are different and if `issymmetric' is set to true
      then the transpose of the matrix is also added to the tangent system
      (default is false). set `iscoercive` to true if the term does not
      affect the coercivity of the tangent system (default is false).
      The matrix can be changed by the command set_private_matrix.
  */
  template <typename MAT>
  size_type add_explicit_matrix(model &md, const std::string &varname1,
                                const std::string &varname2, const MAT &B,
                                bool issymmetric = false,
                                bool iscoercive = false) {
    size_type ind = add_explicit_matrix(md, varname1, varname2,
                                        issymmetric, iscoercive);
    set_private_data_matrix(md, ind, B);
    return ind;
  }

  /**  Add a brick representing an explicit right hand side to be added to
       the right hand side of the tangent
       linear system relatively to the variable 'varname'.
       The given rhs should have the same size than the dimension of
       'varname'. The rhs can be changed by the command set_private_rhs.
  */
  template <typename VECT>
  size_type add_explicit_rhs(model &md, const std::string &varname,
                             const VECT &L) {
    size_type ind = add_explicit_rhs(md, varname);
    set_private_data_rhs(md, ind, L);
    return ind;
  }


  /** Linear elasticity brick ( @f$ \int \sigma(u):\varepsilon(v) @f$ ).
      for isotropic material. Parametrized by the Lam� coefficients
      lambda and mu.
  */
  size_type APIDECL add_isotropic_linearized_elasticity_brick
  (model &md, const mesh_im &mim, const std::string &varname,
   const std::string &dataname_lambda, const std::string &dataname_mu,
   size_type region = size_type(-1),
   const std::string &dataname_preconstraint = std::string());

  /**  Linear elasticity brick ( @f$ \int \sigma(u):\varepsilon(v) @f$ ).
      for isotropic material. Parametrized by Young modulus and Poisson ratio
      For two-dimensional problems, corresponds to the plain strain
      approximation
      ( @f$ \lambda = E\nu/((1+\nu)(1-2\nu)), \mu = E/(2(1+\nu)) @f$ ).
      Corresponds to the standard model for three-dimensional problems.
  */
  size_type APIDECL add_isotropic_linearized_elasticity_brick_pstrain
  (model &md, const mesh_im &mim, const std::string &varname,
   const std::string &data_E, const std::string &data_nu,
   size_type region);

  /** 
      Linear elasticity brick ( @f$ \int \sigma(u):\varepsilon(v) @f$ ).
      for isotropic material. Parametrized by Young modulus and Poisson ratio.
      For two-dimensional problems, corresponds to the plain stress
      approximation
      ( @f$ \lambda^* = E\nu/(1-\nu^2), \mu = E/(2(1+\nu)) @f$ ).
      Corresponds to the standard model for three-dimensional problems.
  */
  size_type APIDECL add_isotropic_linearized_elasticity_brick_pstress
  (model &md, const mesh_im &mim, const std::string &varname,
   const std::string &data_E, const std::string &data_nu,
   size_type region);

  void APIDECL compute_isotropic_linearized_Von_Mises_or_Tresca
  (model &md, const std::string &varname, const std::string &dataname_lambda,
   const std::string &dataname_mu, const mesh_fem &mf_vm,
   model_real_plain_vector &VM, bool tresca);

  /**
     Compute the Von-Mises stress or the Tresca stress of a field
     (only valid for isotropic linearized elasticity in 3D)
     Parametrized by Lame coefficients.
  */
  template <class VECTVM>
  void compute_isotropic_linearized_Von_Mises_or_Tresca
  (model &md, const std::string &varname, const std::string &dataname_lambda,
   const std::string &dataname_mu, const mesh_fem &mf_vm,
   VECTVM &VM, bool tresca) {
    model_real_plain_vector VMM(mf_vm.nb_dof());
    compute_isotropic_linearized_Von_Mises_or_Tresca
      (md, varname, dataname_lambda, dataname_mu, mf_vm, VMM, tresca);
    gmm::copy(VMM, VM);
  }

  /**
     Compute the Von-Mises stress  of a displacement field for isotropic
     linearized elasticity in 3D or in 2D with plane strain assumption.
     Parametrized by Young modulus and Poisson ratio.
  */
  void APIDECL compute_isotropic_linearized_Von_Mises_pstrain
  (model &md, const std::string &varname, const std::string &data_E,
   const std::string &data_nu, const mesh_fem &mf_vm,
   model_real_plain_vector &VM);

  /**
     Compute the Von-Mises stress  of a displacement field for isotropic
     linearized elasticity in 3D or in 2D with plane stress assumption.
     Parametrized by Young modulus and Poisson ratio.
  */
  void APIDECL compute_isotropic_linearized_Von_Mises_pstress
  (model &md, const std::string &varname, const std::string &data_E,
   const std::string &data_nu, const mesh_fem &mf_vm,
   model_real_plain_vector &VM);


  /**
     Mixed linear incompressibility condition brick.

     Update the tangent matrix with a pressure term:
     @f[
     T \longrightarrow
     \begin{array}{ll} T & B \\ B^t & M \end{array}
     @f]
     with @f$ B = - \int p.div u @f$ and
     @f$ M = \int \epsilon p.q @f$ ( @f$ \epsilon @f$ is an optional
     penalization coefficient).

     Be aware that an inf-sup condition between the finite element method
     describing the rpressure and the primal variable has to be satisfied.

     For nearly incompressible elasticity,
     @f[ p = -\lambda \textrm{div}~u @f]
     @f[ \sigma = 2 \mu \varepsilon(u) -p I @f]
  */
  size_type APIDECL add_linear_incompressibility
  (model &md, const mesh_im &mim, const std::string &varname,
   const std::string &multname_pressure, size_type region = size_type(-1),
   const std::string &dataexpr_penal_coeff = std::string());

  /** Mass brick ( @f$ \int \rho u.v @f$ ).
      Add a mass matix on a variable (eventually with a specified region).
      If the parameter $\rho$ is omitted it is assumed to be equal to 1.
  */
  size_type APIDECL add_mass_brick
  (model &md, const mesh_im &mim, const std::string &varname,
   const std::string &dataexpr_rho = std::string(),
   size_type region = size_type(-1));

  /** Basic d/dt brick ( @f$ \int \rho ((u^{n+1}-u^n)/dt).v @f$ ).
      Add the standard discretization of a first order time derivative. The
      parameter $rho$ is the density which could be omitted (the defaul value
      is 1). This brick should be used in addition to a time dispatcher for the
      other terms.
  */
  size_type APIDECL add_basic_d_on_dt_brick
  (model &md, const mesh_im &mim, const std::string &varname,
   const std::string &dataname_dt,
   const std::string &dataname_rho = std::string(),
   size_type region = size_type(-1));

  /** Basic d2/dt2 brick ( @f$ \int \rho ((u^{n+1}-u^n)/(\alpha dt^2) - v^n/(\alpha dt) ).w @f$ ).
      Add the standard discretization of a second order time derivative. The
      parameter $rho$ is the density which could be omitted (the defaul value
      is 1). This brick should be used in addition to a time dispatcher for the
      other terms. The time derivative $v$ of the variable $u$ is preferably
      computed as a post-traitement which depends on each scheme.
  */
  size_type APIDECL add_basic_d2_on_dt2_brick
  (model &md, const mesh_im &mim, const std::string &varnameU,
   const std::string &datanameV,
   const std::string &dataname_dt,
   const std::string &dataname_alpha,
   const std::string &dataname_rho = std::string(),
   size_type region = size_type(-1));


}  /* end of namespace getfem.                                             */


#endif /* GETFEM_MODELS_H_*/
