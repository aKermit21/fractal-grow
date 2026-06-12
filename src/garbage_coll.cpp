// Copyright (c) 2026 Robert Gajewski (pcc21.com)
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "garbage_coll.h"
#include "dbg_report.h"
#include "basics.h"
#include "transform.h"
#include "mut_grow.h"
#include <iterator>
#include <memory>

// Garbage collector static variables
long MemAndDebug::mElementPtrsCnt; 
// Collection of pointers ownership
std::vector<std::unique_ptr<ChildrenElementsCluster>> 
    MemAndDebug::allElementPtrs {};


// Add ownership pointer to the collection
void MemAndDebug::collectElementPtr(std::unique_ptr<ChildrenElementsCluster> ptr) {
  allElementPtrs.push_back(std::move(ptr));
  ++mElementPtrsCnt;

  if (Dbg::cReportWarning) {
    if (mElementPtrsCnt >= Dbg::cCreatePointersThreshold) {
      Dbg::report_mltpl_warning( Dbg::mltplPointers, mElementPtrsCnt);
    }
  }

  // Check some very Big Number - # of pointers
  // *5(elements in branch) *120(Element size in B) = memory needed
  // up to 18GB
  if (Dbg::cReportError and mElementPtrsCnt > 30'000'000) {
    Dbg::report_once(oncePointers, mElementPtrsCnt);
    // Fatal Error ???
    // release_all();
    // throw "Too large number of elements created.";
  }
}
  
// Explicit release all allocated data from the collection
void MemAndDebug::release_all(){
  // Release all Mutational Grow Nodes (manually) in reverse order
  MutGrow::releaseAll();
  
  // Release all Elements (manually) in reverse order
  for (auto rit { MemAndDebug::allElementPtrs.rbegin() };
    rit != MemAndDebug::allElementPtrs.rend(); ++rit)  {
      rit->reset();  // release allocated data
  }
  allElementPtrs.clear();  // clear vector itself
  mElementPtrsCnt = 0;
  Dbg::report_info("Garbage collector manually clean-up\n");
}

// Remove unused Elements to limit memory consumption
void MemAndDebug::pruneElementsClusterVector(int cycle) {
  static long counter {0};

  ++counter;
  if ((counter % cycle) == 0) {
    // 1st stage - only marking Clusters
    prunePreperationsCluster();
    // Actual Deallocation
    pruneDeallocateCluster();
  }
}
  
// Only marking Clusters to be removed
void MemAndDebug::prunePreperationsCluster(void) {
  long pruneCounter {0};
  
  // Scan all unique ptrs in vector
  for (auto& ptr : allElementPtrs) {
    if (!ptr) {
      // pointer may be null if related cluster was deallocated
      // by previous/next Pruning step.
      // Then go to next pointer
      continue;
    }
    bool cannotBeRemoved = false;

    // Check if parent (common to all elements) is CoreElement
    // WARNING: Parent can be already deallocated
    // that is processing must be done in right order or done in a single run.
    auto parent = ptr.get()->elements.at(0).parent_ptr;
    if (parent->coreElement) {
      cannotBeRemoved = true;
    }

    if (!cannotBeRemoved) {
      // Consider Parent size
      if (! parent->stem_xy.vec_xy.vecTooSmall(TranAlg::s_SmallVect)) {
        cannotBeRemoved = true;
      } else {
        // Means parent length is too small (< SmallVect).
        // Double Check Element in this Cluster (children)
        // shall be in the same category.
        [[maybe_unused]]
        auto firstChildVec = ptr.get()->elements.at(0).stem_xy.vec_xy;
        [[maybe_unused]]
        auto lastChildVec = ptr.get()->elements.at(cFrac::NrOfElements-1).stem_xy.vec_xy;
        assert((firstChildVec.vecTooSmall(TranAlg::s_SmallVect)) and
               (lastChildVec.vecTooSmall(TranAlg::s_SmallVect)));
      }
    }

    if (!cannotBeRemoved) {
      ptr.get()->toBePruned = true;
      if (ptr.get()->elements.at(0).parent_ptr) {
        // WARNING: parent_ptr is potencially dangling pointer;
        // that is processing must be done in right order
        // or done in a single run before actual allocation.
        ptr.get()->elements.at(0).parent_ptr->children_down = nullptr;
        ptr.get()->elements.at(0).parent_ptr->children_up = nullptr;
      }
      ++pruneCounter;
    }
  }
  Dbg::report_info("Pruning preperation: ", pruneCounter);
}


void MemAndDebug::pruneDeallocateCluster(void){
  long pruneCounter {0};
  
  // First pass: just prepare list of what shall be pruned
  std::vector<size_t> toPruneIndices;
  for (size_t i = 0; i < allElementPtrs.size(); ++i) {
      if (allElementPtrs[i] and allElementPtrs[i]->toBePruned) {
        toPruneIndices.push_back(i);
      }
  }

  // Second pass: reset them (Deallocate)
  for (auto idx : toPruneIndices) {
      allElementPtrs[idx].reset();
      ++pruneCounter;
  }

  // OPTIONALLY: As it takes considerable time
  // Erase not used ptr from vector
  // for (auto it = allElementPtrs.begin(); it != allElementPtrs.end(); ) {
  //     if (*it == nullptr) {
  //         it = allElementPtrs.erase(it); // erase returns next iterator
  //     } else {
  //         ++it;
  //     }
  // }
  
  // Update total counter
  mElementPtrsCnt -= pruneCounter;
  // [[maybe_unused]]
  // long temp = allElementPtrs.size();
  // assert(temp == mElementPtrsCnt);
  Dbg::report_info("Pruning Deallocation: ", pruneCounter);
}


long MemAndDebug::getElPtrsCnt(void) {
   return mElementPtrsCnt;
}
