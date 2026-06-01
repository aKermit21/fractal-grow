// Copyright (c) 2026 Robert Gajewski
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "mut_grow.h"
#include "fractal.h"
#include "dbg_report.h"
#include <iostream>
#include <memory>
#include <vector>

// Collects link pointers ownership - Garbage Collector
// Dbg: Collects and reports debug info from every other class
struct MemAndDebug : Dbg
{
  MemAndDebug(){
    elementPtrsCnt = 0;
  }
  // explicit destructor always virtual: learncpp 25.4
  virtual ~MemAndDebug(){
#ifndef NDEBUG
    std::cerr << "Garbage Collector (Memory management) auto clean-up Done." << std::endl;
#endif
  }

  // Add ownership pointer to the collection
  static void collectElementPtr(std::unique_ptr<std::array<Element, cFrac::NrOfElements>> ptr);
  
  // static void collectGrowPtr(std::unique_ptr<ExcGrow> ptr);

  // Explicit release all allocated data from the collection
  static void release_all(void);

  static long getElPtrsCnt(void);

  private:

  static unsigned long elementPtrsCnt; 
  // Collection of Element pointers ownership
  static std::vector<std::unique_ptr<std::array<Element, cFrac::NrOfElements>>> allElementPtrs;
};

// extern GarbColl gc;

