/*===========================================================================

 Copyright (C) 2000-2020 Julien Pommier

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

#include "getfem/bgeot_rtree.h"

namespace bgeot {

  struct rtree_node : public rtree_elt_base {
    std::unique_ptr<rtree_elt_base> left, right;
    rtree_node(const base_node& bmin, const base_node& bmax,
               std::unique_ptr<rtree_elt_base> &&left_,
               std::unique_ptr<rtree_elt_base> &&right_)
      : rtree_elt_base(false, bmin, bmax), left(std::move(left_)),
        right( std::move(right_)) { }
  };

  struct rtree_leaf : public rtree_elt_base {
    rtree::pbox_cont lst;
    rtree_leaf(const base_node& bmin, const base_node& bmax,
               rtree::pbox_cont& lst_)
      : rtree_elt_base(true, bmin, bmax) { lst.swap(lst_); }
  };

  /* enlarge box to hold [a..b] */
  static void update_box(base_node& bmin, base_node& bmax,
                         const base_node& a, const base_node& b) {
    base_node::iterator itmin=bmin.begin(), itmax=bmax.begin();
    for (size_type i=0; i < a.size(); ++i) {
      itmin[i] = std::min(itmin[i], a.at(i));
      itmax[i] = std::max(itmax[i], b.at(i));
    }
  }

  inline static bool r1_ge_r2(const base_node& min1, const base_node& max1,
                       const base_node& min2, const base_node& max2,
                       scalar_type EPS) {
    for (size_type i=0; i < min1.size(); ++i)
      if ((min1[i] > min2[i]+EPS) || (max1[i] < max2[i]-EPS)) return false;
    return true;
  }

  inline static bool r1_inter_r2(const base_node& min1, const base_node& max1,
                          const base_node& min2, const base_node& max2,
                          scalar_type EPS) {
    for (size_type i=0; i < min1.size(); ++i)
      if ((max1[i] < min2[i]-EPS) || (min1[i] > max2[i]+EPS)) return false;
    return true;
  }

  /* some predicates for searches */
  struct intersection_p {
    const base_node &min, &max;
    const scalar_type EPS;
    intersection_p(const base_node& min_, const base_node& max_, scalar_type EPS_)
      : min(min_), max(max_), EPS(EPS_) {}
    bool operator()(const base_node& min2, const base_node& max2) const
    { return r1_inter_r2(min,max,min2,max2,EPS); }
    bool accept(const base_node& min2, const base_node& max2) const
    { return operator()(min2,max2); }
  };

  /* match boxes containing [min..max] */
  struct contains_p {
    const base_node &min, &max;
    const scalar_type EPS;
    contains_p(const base_node& min_, const base_node& max_, scalar_type EPS_)
      : min(min_), max(max_), EPS(EPS_) {}
    bool operator()(const base_node& min2, const base_node& max2) const
    { return r1_ge_r2(min2,max2,min,max,EPS); }
    bool accept(const base_node& min2, const base_node& max2) const
    { return r1_inter_r2(min,max,min2,max2,EPS); }
  };

  /* match boxes contained in [min..max] */
  struct contained_p {
    const base_node &min, &max;
    const scalar_type EPS;
    contained_p(const base_node& min_, const base_node& max_, scalar_type EPS_)
      : min(min_), max(max_), EPS(EPS_) {}
    bool accept(const base_node& min2, const base_node& max2) const
    { return r1_inter_r2(min,max,min2,max2,EPS); }
    bool operator()(const base_node& min2, const base_node& max2) const
    { return r1_ge_r2(min,max,min2,max2,EPS); }
  };

  /* match boxes containing P */
  struct has_point_p {
    const base_node &P;
    const scalar_type EPS;
    has_point_p(const base_node& P_, scalar_type EPS_) : P(P_), EPS(EPS_) {}
    bool operator()(const base_node& min2, const base_node& max2) const {
      for (size_type i = 0; i < P.size(); ++i) {
        if (P[i] < min2[i]-EPS) return false;
        if (P[i] > max2[i]+EPS) return false;
      }
      return true;
    }
    bool accept(const base_node& min2, const base_node& max2) const
    { return operator()(min2,max2); }
  };

  /* match boxes intersecting the line passing through org and of
     direction vector dirv.*/
  struct intersect_line {
    const base_node org;
    const base_small_vector dirv;
    intersect_line(const base_node& org_, const base_small_vector &dirv_)
      : org(org_), dirv(dirv_) {}
    bool operator()(const base_node& min2, const base_node& max2) const {
      size_type N = org.size();
      GMM_ASSERT1(N == min2.size(), "Dimensions mismatch");
      for (size_type i = 0; i < N; ++i)
        if (dirv[i] != scalar_type(0)) {
          scalar_type a1=(min2[i]-org[i])/dirv[i], a2=(max2[i]-org[i])/dirv[i];
          bool interf1 = true, interf2 = true;
          for (size_type j = 0; j < N; ++j)
            if (j != i) {
              scalar_type y1 = org[j] + a1*dirv[j], y2 = org[j] + a2*dirv[j];
              if (y1 < min2[j] || y1 > max2[j]) interf1 = false;
              if (y2 < min2[j] || y2 > max2[j]) interf2 = false;
            }
          if (interf1 || interf2) return true;
        }
      return false;
    }
    bool accept(const base_node& min2, const base_node& max2) const
    { return operator()(min2,max2); }
  };

  /* match boxes intersecting the line passing through org and of
     direction vector dirv.*/
  struct intersect_line_and_box {
    const base_node org;
    const base_small_vector dirv;
    const base_node min,max;
    const scalar_type EPS;
    intersect_line_and_box(const base_node& org_,
                           const base_small_vector &dirv_,
                           const base_node& min_, const base_node& max_,
                           scalar_type EPS_)
      : org(org_), dirv(dirv_), min(min_), max(max_), EPS(EPS_) {}
    bool operator()(const base_node& min2, const base_node& max2) const {
      size_type N = org.size();
      GMM_ASSERT1(N == min2.size(), "Dimensions mismatch");
      if (!(r1_inter_r2(min,max,min2,max2,EPS))) return false;
      for (size_type i = 0; i < N; ++i)
        if (dirv[i] != scalar_type(0)) {
          scalar_type a1=(min2[i]-org[i])/dirv[i], a2=(max2[i]-org[i])/dirv[i];
          bool interf1 = true, interf2 = true;
          for (size_type j = 0; j < N; ++j)
            if (j != i) {
              scalar_type y1 = org[j] + a1*dirv[j], y2 = org[j] + a2*dirv[j];
              if (y1 < min2[j] || y1 > max2[j]) interf1 = false;
              if (y2 < min2[j] || y2 > max2[j]) interf2 = false;
            }
          if (interf1 || interf2) return true;
        }
      return false;
    }
    bool accept(const base_node& min2, const base_node& max2) const
    { return operator()(min2,max2); }
  };

  size_type rtree::add_box(const base_node &min, const base_node &max,
                           size_type id) {
    box_index bi;
    if (tree_built) {
      GMM_WARNING3("Add a box when the tree is already built cancel the tree. "
                   "Unefficient operation.");
      tree_built = false; root = std::unique_ptr<rtree_elt_base>();
    }
    bi.min = &nodes[nodes.add_node(min, EPS)];
    bi.max = &nodes[nodes.add_node(max, EPS)];
    bi.id = (id + 1) ? id : boxes.size();
    return boxes.emplace(std::move(bi)).first->id;
  }
  
  rtree::rtree(scalar_type EPS_)
    : EPS(EPS_), boxes(box_index_topology_compare(EPS_)), tree_built(false)
  {}

  void rtree::clear() {
    root = std::unique_ptr<rtree_elt_base>();
    boxes.clear();
    nodes.clear();
    tree_built = false;
  }

  template <typename Predicate>
  static void find_matching_boxes_(rtree_elt_base *n, rtree::pbox_set& boxlst,
                                   const Predicate &p) {
    if (n->isleaf()) {
      const rtree_leaf *rl = static_cast<rtree_leaf*>(n);
      for (rtree::pbox_cont::const_iterator it = rl->lst.begin();
           it != rl->lst.end(); ++it) {
        if (p(*(*it)->min, *(*it)->max)) { boxlst.insert(*it); }
      }
    } else {
      const rtree_node *rn = static_cast<rtree_node*>(n);
      if (p.accept(rn->left->rmin,rn->left->rmax))
        bgeot::find_matching_boxes_(rn->left.get(), boxlst, p);
      if (p.accept(rn->right->rmin,rn->right->rmax))
        bgeot::find_matching_boxes_(rn->right.get(), boxlst, p);
    }
  }

  void rtree::find_intersecting_boxes(const base_node& bmin,
                                      const base_node& bmax,
                                      pbox_set& boxlst) const {
    
    boxlst.clear();
    GMM_ASSERT2(tree_built, "Boxtree not initialised.");
    if (root)
      find_matching_boxes_(root.get(),boxlst,intersection_p(bmin,bmax, EPS));
  }

  void rtree::find_containing_boxes(const base_node& bmin,
                                    const base_node& bmax,
                                    pbox_set& boxlst) const {
    boxlst.clear();
    GMM_ASSERT2( tree_built, "Boxtree not initialised.");
    if (root)
      find_matching_boxes_(root.get(), boxlst, contains_p(bmin,bmax, EPS));
  }

  void rtree::find_contained_boxes(const base_node& bmin,
                                   const base_node& bmax,
                                   pbox_set& boxlst) const {
    boxlst.clear();
    GMM_ASSERT2(tree_built, "Boxtree not initialised.");
    if (root)
      find_matching_boxes_(root.get(), boxlst, contained_p(bmin,bmax, EPS));
  }

  void rtree::find_boxes_at_point(const base_node& P, pbox_set& boxlst) const {
    boxlst.clear();
    GMM_ASSERT2(tree_built, "Boxtree not initialised.");
    if (root)
      find_matching_boxes_(root.get(), boxlst, has_point_p(P, EPS));
  }

  void rtree::find_line_intersecting_boxes(const base_node& org,
                                           const base_small_vector& dirv,
                                           pbox_set& boxlst) const {
    boxlst.clear();
    GMM_ASSERT2(tree_built, "Boxtree not initialised.");
    if (root)
      find_matching_boxes_(root.get(),boxlst,intersect_line(org, dirv));
  }

  void rtree::find_line_intersecting_boxes(const base_node& org,
                                           const base_small_vector& dirv,
                                           const base_node& bmin,
                                           const base_node& bmax,
                                           pbox_set& boxlst) const {
    boxlst.clear();
    GMM_ASSERT2(tree_built, "Boxtree not initialised.");
    if (root)
      find_matching_boxes_(root.get(), boxlst,
                           intersect_line_and_box(org, dirv, bmin, bmax, EPS));
  }

  /*
     try to split at the approximate center of the box. Could be much more
     sophisticated
  */
  static bool split_test(const rtree::pbox_cont& b,
                         const base_node& bmin, const base_node& bmax,
                         unsigned dir, scalar_type& split_v) {
    scalar_type v = bmin[dir] + (bmax[dir] - bmin[dir])/2; split_v = v;
    size_type cnt = 0;
    for (rtree::pbox_cont::const_iterator it = b.begin(); it!=b.end(); ++it) {
      if ((*it)->max->at(dir) < v) {
        if (cnt == 0) split_v = (*it)->max->at(dir);
        else split_v = std::max((*it)->max->at(dir),split_v);
        cnt++;
      }
    }
    return (cnt > 0 && cnt < b.size());
  }

  /*
     there are many flavors of rtree ... this one is more or less a quadtree
     where splitting does not occurs at predefined positions (hence the
     split_test function above).
     Regions of the tree do not overlap (box are splitted).
  */
  static std::unique_ptr<rtree_elt_base> build_tree_(rtree::pbox_cont b,
                                                     const base_node& bmin,
                                                     const base_node& bmax,
                                                     unsigned last_dir) {
    size_type N=bmin.size();
    scalar_type split_v(0);
    unsigned split_dir = unsigned((last_dir+1)%N);
    bool split_ok = false;
    if (b.size() > rtree_elt_base::RECTS_PER_LEAF) {
      for (size_type itry=0; itry < N; ++itry) {
        if (split_test(b, bmin, bmax, split_dir, split_v))
          { split_ok = true; break; }
        split_dir = unsigned((split_dir+1)%N);
      }
    }
    if (split_ok) {
      size_type cnt1=0,cnt2=0;
      for (rtree::pbox_cont::const_iterator it = b.begin();
           it != b.end(); ++it) {
        if ((*it)->min->at(split_dir) < split_v) cnt1++;
        if ((*it)->max->at(split_dir) > split_v) cnt2++;
      }
      assert(cnt1); assert(cnt2);
      GMM_ASSERT1(cnt1+cnt2 >= b.size(), "internal error");
      rtree::pbox_cont v1(cnt1), v2(cnt2);
      base_node bmin1(bmax), bmax1(bmin);
      base_node bmin2(bmax), bmax2(bmin);
      cnt1 = cnt2 = 0;
      for (rtree::pbox_cont::const_iterator it = b.begin();
           it != b.end(); ++it) {
        if ((*it)->min->at(split_dir) < split_v) {
          v1[cnt1++] = *it;
          update_box(bmin1,bmax1,*(*it)->min,*(*it)->max);
        }
        if ((*it)->max->at(split_dir) > split_v) {
          v2[cnt2++] = *it;
          update_box(bmin2,bmax2,*(*it)->min,*(*it)->max);
        }
      }
      for (size_type k=0; k < N; ++k) {
        bmin1[k] = std::max(bmin1[k],bmin[k]);
        bmax1[k] = std::min(bmax1[k],bmax[k]);
        bmin2[k] = std::max(bmin2[k],bmin[k]);
        bmax2[k] = std::min(bmax2[k],bmax[k]);
      }
      bmax1[split_dir] = std::min(bmax1[split_dir], split_v);
      bmin2[split_dir] = std::max(bmin2[split_dir], split_v);
      assert(cnt1 == v1.size()); assert(cnt2 == v2.size());
      return std::make_unique<rtree_node>
        (bmin, bmax,
         build_tree_(v1, bmin1, bmax1, split_dir),
         build_tree_(v2, bmin2, bmax2, split_dir));
    } else {
      return std::make_unique<rtree_leaf>(bmin, bmax, b);
    }
  }

  void rtree::build_tree() {
    if (!tree_built) {
      if (boxes.size() == 0) { tree_built = true; return; }
      getfem::local_guard lock = locks_.get_lock();
      assert(root == 0);
      pbox_cont b(boxes.size());
      pbox_cont::iterator b_it = b.begin();
      base_node bmin(*boxes.begin()->min), bmax(*boxes.begin()->max);
      for (box_cont::const_iterator it=boxes.begin(); it != boxes.end(); ++it) {
        update_box(bmin,bmax,*(*it).min,*(*it).max);
        *b_it++ = &(*it);
      }
      root = build_tree_(b, bmin, bmax, 0);
      tree_built = true;
    }
  }

  static void dump_tree_(rtree_elt_base *p, int level, size_type& count) {
    if (!p) return;
    for (int i=0; i < level; ++i) cout << "  ";
    cout << "span=" << p->rmin << ".." << p->rmax << " ";
    if (p->isleaf()) {
      rtree_leaf *rl = static_cast<rtree_leaf*>(p);
      cout << "Leaf [" << rl->lst.size() << " elts] = ";
      for (size_type i=0; i < rl->lst.size(); ++i)
        cout << " " << rl->lst[i]->id;
      cout << "\n";
      count += rl->lst.size();
    } else {
      cout << "Node\n";
      const rtree_node *rn = static_cast<rtree_node*>(p);
      if (rn->left) { dump_tree_(rn->left.get(), level+1, count); }
      if (rn->right) { dump_tree_(rn->right.get(), level+1, count); }
    }
  }

  void rtree::dump() {
    cout << "tree dump follows\n";
    if (!root) build_tree();
    size_type count = 0;
    dump_tree_(root.get(), 0, count);
    cout << " --- end of tree dump, nb of rectangles: " << boxes.size()
         << ", rectangle ref in tree: " << count << "\n";
  }
}
