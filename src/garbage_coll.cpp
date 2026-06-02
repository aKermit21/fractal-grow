// Copyright (c) 2026 Robert Gajewski
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "garbage_coll.h"
#include "dbg_report.h"
#include "mut_grow.h"
#include <iterator>
#include <memory>

// Garbage collector static variables
long MemAndDebug::elementPtrsCnt; 
// Collection of pointers ownership
std::vector<std::unique_ptr<std::array<Element, cFrac::NrOfElements>>> 
    MemAndDebug::allElementPtrs {};
// std::vector<std::unique_ptr<ExcGrow>> MemAndDebug::allExcGrowPtrs {};


// Add ownership pointer to the collection
void MemAndDebug::collectElementPtr(std::unique_ptr<std::array<Element, cFrac::NrOfElements>> ptr) {
  allElementPtrs.push_back(std::move(ptr));
  ++elementPtrsCnt;

  if (Dbg::cReportWarning) {
    if (elementPtrsCnt >= Dbg::cCreatePointersThreshold) {
      Dbg::report_mltpl_warning( Dbg::mltplPointers, elementPtrsCnt);
    }
  }

  // Check some very Big Number - # of pointers
  // *5(elements in branch) *120(Element size in B) = memory needed
  // up to 30GB
  if (Dbg::cReportError and elementPtrsCnt > 50'000'000) {
    Dbg::report_once(oncePointers, elementPtrsCnt);
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
  elementPtrsCnt = 0;
  Dbg::report_info("Garbage collector manually clean-up\n");
}

 long MemAndDebug::getElPtrsCnt(void) {
   return elementPtrsCnt;
 }
