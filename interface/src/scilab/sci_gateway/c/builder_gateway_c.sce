// ====================================================================
// Copyright 2009
// Yann COLLETTE
// This file is released into the public domain
// ====================================================================

getfem_path = get_absolute_file_path('builder_gateway_c.sce');

Table = ['gf_util',             'sci_gf_scilab'; ...
         'gf_cvstruct_get',     'sci_gf_scilab'; ...
         'gf_geotrans',         'sci_gf_scilab'; ...
         'gf_geotrans_get',     'sci_gf_scilab'; ...
         'gf_compute',          'sci_gf_scilab'; ...
         'gf_mesh_fem',         'sci_gf_scilab'; ...
         'gf_mesh_fem_set',     'sci_gf_scilab'; ...
         'gf_mesh_fem_get',     'sci_gf_scilab'; ...
         'gf_mesh_im',          'sci_gf_scilab'; ...
         'gf_mesh_im_set',      'sci_gf_scilab'; ...
         'gf_mesh_im_get',      'sci_gf_scilab'; ...
         'gf_eltm',             'sci_gf_scilab'; ...
         'gf_mesh',             'sci_gf_scilab'; ...
         'gf_mesh_set',         'sci_gf_scilab'; ...
         'gf_mesh_get',         'sci_gf_scilab'; ...
         'gf_mdbrick',          'sci_gf_scilab'; ...
         'gf_mdbrick_get',      'sci_gf_scilab'; ...
         'gf_mdbrick_set',      'sci_gf_scilab'; ...
         'gf_mdstate',          'sci_gf_scilab'; ...
         'gf_mdstate_get',      'sci_gf_scilab'; ...
         'gf_mdstate_set',      'sci_gf_scilab'; ...
         'gf_model',            'sci_gf_scilab'; ...
         'gf_model_get',        'sci_gf_scilab'; ...
         'gf_model_set',        'sci_gf_scilab'; ...
         'gf_slice',            'sci_gf_scilab'; ...
         'gf_slice_get',        'sci_gf_scilab'; ...
         'gf_slice_set',        'sci_gf_scilab'; ...
         'gf_levelset',         'sci_gf_scilab'; ...
         'gf_levelset_get',     'sci_gf_scilab'; ...
         'gf_levelset_set',     'sci_gf_scilab'; ...
         'gf_mesh_levelset',    'sci_gf_scilab'; ...
         'gf_mesh_levelset_get','sci_gf_scilab'; ...
         'gf_mesh_levelset_set','sci_gf_scilab'; ...
         'gf_precond',          'sci_gf_scilab'; ...
         'gf_precond_get',      'sci_gf_scilab'; ...
         'gf_linsolve',         'sci_gf_scilab'; ...
         'gf_spmat',            'sci_gf_scilab'; ...
         'gf_spmat_set',        'sci_gf_scilab'; ...
         'gf_spmat_get',        'sci_gf_scilab'; ...
         'gf_asm',              'sci_gf_scilab'; ...
         'gf_fem',              'sci_gf_scilab'; ...
         'gf_fem_get',          'sci_gf_scilab'; ...
         'gf_integ',            'sci_gf_scilab'; ...
         'gf_integ_get',        'sci_gf_scilab'; ...
         'gf_workspace',        'sci_gf_scilab'; ...
         'gf_delete',           'sci_gf_scilab'];

Files = ['gfm_common.c','gfm_scilab.c'];

Libraries = [];

cflags = '-g -I' + getfem_path + ' -I' + getfem_path + '../../../';
ldflags = getfem_path + '/../../../.libs/libgetfemint.a ' + getfem_path + '/../../../../../src/.libs/libgetfem.a';

tbx_build_gateway('scigetfem_c', Table, Files, getfem_path, Libraries, ldflags, cflags);

clear tbx_build_gateway;
