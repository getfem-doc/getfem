/*===========================================================================

 Copyright (C) 2004-2020 Julien Pommier

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

===========================================================================*/

#include "getfem/bgeot_small_vector.h"
namespace bgeot { 
  block_allocator *static_block_allocator::palloc = 0;

  block_allocator::block_allocator() {
    for (size_type i=0; i < OBJ_SIZE_LIMIT; ++i) 
      first_unfilled[i] = i ? size_type(-1) : 0; 
    /* bloc 0 is reserved for objects of size 0 -- it won't grow */
    blocks.push_back(block(0)); blocks.front().init();
  }
  block_allocator::~block_allocator() {
    for (size_type i=0; i < blocks.size(); ++i) 
      if (!blocks[i].empty()) blocks[i].clear();
    static_block_allocator::palloc = 0;
  }
  block_allocator::node_id block_allocator::allocate(block_allocator::size_type n) {
    if (n == 0) return 0;
    GMM_ASSERT1(n < OBJ_SIZE_LIMIT,
		"attempt to allocate a supposedly \"small\" object of " 
		<< n << " bytes\n");
    if (first_unfilled[n] == size_type(-1)) {
      blocks.push_back(block(n)); blocks.back().init();
      insert_block_into_unfilled(gmm::uint32_type(blocks.size()-1));
      GMM_ASSERT1(first_unfilled[n] <
		  (node_id(1)<<(sizeof(node_id)*CHAR_BIT - p2_BLOCKSZ)),
		  "allocation slots exhausted for objects of size " << n
		  << " (" << first_unfilled[n] << " allocated!),\n" << "either"
		  " increase the limit or check for a leak in your code.");
    }
    block &b = blocks[first_unfilled[n]]; SVEC_ASSERT(b.objsz == n);
    if (b.empty()) b.init(); /* realloc memory if needed */
    size_type vid = b.first_unused_chunk; SVEC_ASSERT(vid < BLOCKSZ); 
    size_type id = vid + first_unfilled[n]*BLOCKSZ;
    SVEC_ASSERT(b.refcnt(b.first_unused_chunk)==0);
    b.refcnt(vid) = 1; b.count_unused_chunk--;
    if (b.count_unused_chunk) {
      do b.first_unused_chunk++; while (b.refcnt(b.first_unused_chunk));
    } else {
      b.first_unused_chunk = BLOCKSZ;
      remove_block_from_unfilled(first_unfilled[n]);
    }
    //cout << "allocated " << first_unfilled[n] << ", " << vid << " @" << obj_data(id) << "\n";
    SVEC_ASSERT(obj_data(id));
    memset(obj_data(id), 0, n);
    //SVEC_ASSERT(refcnt(id) == 0);
    return id;
  }
  void block_allocator::deallocate(block_allocator::node_id nid) {
    if (nid == 0) return;
    size_type bid = nid / BLOCKSZ;
    size_type vid = nid % BLOCKSZ;
    block &b = blocks[bid];      
    //cout << "deallocate " << bid << "/dim=" << b.dim << ", " << vid << ", unused=" << b.unused << "\n";
    SVEC_ASSERT(b.refcnt(vid) == 1);
    b.refcnt(vid) = 0;
    if (b.count_unused_chunk++ == 0) {
      insert_block_into_unfilled(bid); 
      b.first_unused_chunk = gmm::uint16_type(vid);
    } else {
      b.first_unused_chunk = std::min(b.first_unused_chunk,
				      gmm::uint16_type(vid));
      if (b.count_unused_chunk == BLOCKSZ) b.clear();
    }
  }
  void block_allocator::memstats()  {
    cout << "block_allocator memory statistics:\ntotal number of blocks: " 
	 << blocks.size() << ", each blocks stores " << BLOCKSZ 
	 << " chuncks; size of a block header is " << sizeof(block) << " bytes\n";
    for (size_type d = 0; d < OBJ_SIZE_LIMIT; ++d) {
      size_type total_cnt=0, used_cnt=0, mem_total = 0, bcnt = 0;
      for (size_type i=0; i < blocks.size(); ++i) {
	if (blocks[i].objsz != d) continue; else bcnt++;
	if (!blocks[i].empty()) {
	  total_cnt += BLOCKSZ;
	  used_cnt += BLOCKSZ - blocks[i].count_unused_chunk;
	  mem_total += (BLOCKSZ+1)*blocks[i].objsz;
	}
	mem_total = gmm::uint32_type(mem_total + sizeof(block));
      }
      if (mem_total)
	cout << " sz " << d << ", memory used = " << mem_total << " bytes for " 
	     << total_cnt << " nodes, unused space = " 
	     << (total_cnt == 0 ? 100. : 100. - 100.* used_cnt / total_cnt) 
	     << "%, bcnt=" << bcnt << "\n";
    }
  }
  void block_allocator::insert_block_into_unfilled(block_allocator::size_type bid) {
    SVEC_ASSERT(bid < blocks.size());
    dim_type dim = dim_type(blocks[bid].objsz);
    SVEC_ASSERT(bid != first_unfilled[dim]);
    SVEC_ASSERT(blocks[bid].prev_unfilled+1 == 0);
    SVEC_ASSERT(blocks[bid].next_unfilled+1 == 0);
    blocks[bid].prev_unfilled = size_type(-1);
    blocks[bid].next_unfilled = first_unfilled[dim];
    if (first_unfilled[dim] != size_type(-1)) {
      SVEC_ASSERT(blocks[first_unfilled[dim]].prev_unfilled+1 == 0);
      blocks[first_unfilled[dim]].prev_unfilled = bid;
    }
    first_unfilled[dim] = bid;
    //cout << "** bloc " << bid << " has been INSERTED in unfilled list, which is now"; show_unfilled(bid);
  }
  void block_allocator::remove_block_from_unfilled(block_allocator::size_type bid) {
    SVEC_ASSERT(bid < blocks.size());
    dim_type dim = dim_type(blocks[bid].objsz);
    //cout << "** bloc " << bid << " is going to be REMOVE unfilled list, which is now"; show_unfilled(bid);
    size_type p = blocks[bid].prev_unfilled; blocks[bid].prev_unfilled = size_type(-1);
    size_type n = blocks[bid].next_unfilled; blocks[bid].next_unfilled = size_type(-1);
    if (p != size_type(-1)) { blocks[p].next_unfilled = n; }
    if (n != size_type(-1)) { blocks[n].prev_unfilled = p; }
    if (first_unfilled[dim] == bid) { SVEC_ASSERT(p+1==0); first_unfilled[dim] = n; }
    //cout << "** bloc " << bid << " has been REMOVED in unfilled list, which is now"; show_unfilled(bid);
  }
}
