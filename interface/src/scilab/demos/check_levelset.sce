// YC: object oriented
// must replace all the gfObject functions. gfObject has been removed
gf_workspace('clear all');
clf;
m=gf_mesh('regular_simplices', -1:.2:1, -1:.2:1, 'degree', 2, 'noised');
ls1 = gf_levelset(m, 2, 'x^2 + y^2 - 0.7^2', 'x-.4')
ls2 = gf_levelset(m, 2, '0.6*x^2 + (y-0.1)^2 - 0.6^2');
ls3 = gf_levelset(m, 4, 'x^2 + (y+.08)^2 - 0.05^2');

mls=gfMeshLevelset(m) 
set(mls, 'add', ls1);
if 1 then
  set(mls, 'del', ls1);
  set(mls, 'add', ls1);
  set(mls, 'add', ls2);
  set(mls, 'add', ls2);
  set(mls, 'add', ls2);
  set(mls, 'add', ls3);
end;
set(mls, 'adapt');

gfObject(get(mls, 'linked_mesh'))

lls = gf_mesh_levelset_get(mls, 'levelsets')

cm = gfObject(get(mls, 'cut_mesh'))

ctip = get(mls, 'crack_tip_convexes')

mf=gfMeshFem(m); set(mf, 'classical_fem', 1);
mfls=gfMeshFem('levelset',mf,mls);

//gf_workspace('stats');

nbd = get(mfls,'nbdof')
if 1 then
  sl=gfSlice(list('none'), mls, 2);
  //for i=1:nbd,
//  U=zeros(1,nbd);U(i)=1;
    U=rand(1,nbd);
    gf_plot(mfls,U,'refine',4,'zplot','on');
    //pause;
//end;
  //gf_plot_mesh(cm, 'curved', 'on','refine',8,'edges_width',2);
  gf_plot_mesh(m, 'curved', 'on','refine',8, 'edges_color', [0 0 0]);
  // colorbar; // YC: we must get the min / max value of something ... colorbar(minval, maxval)
else
  for i=1:nbd
    U=zeros(1,nbd); U(i)=1;
    gf_plot(mfls,U,'refine',16);
    gf_plot_mesh(cm, 'curved', 'on','refine',8);
    gf_plot_mesh(m, 'curved', 'on','refine',8, 'edges_color', [0 0 0]);
    pause
  end
end

