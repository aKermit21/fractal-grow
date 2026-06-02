// Copyright (c) 2026 Robert Gajewski
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "autoscale.h"
#include "mut_grow.h"
#include "fractal.h"
#include "dbg_report.h"
#include "garbage_coll.h"
#include "transform.h"
#include "fluctuate.h"
#include <SFML/Graphics/PrimitiveType.hpp>
#include <chrono>
#include <optional>
#include <thread>

// Allocate subordinate elements/branches
// and initialize with structural data
bool new_elements_creation(Element * const parent_ptr, const long level)
{
  static long recur_funct_cnt { 0 };

  if (level > cFrac::NrOfOrders) { 
    assert((level <= cFrac::NrOfOrders) and
           "shall be prevented by parent function recurance_elements_redraw()");
    //Stop further branches
    parent_ptr->children_down = nullptr;
    parent_ptr->children_up = nullptr;
    return false; // nothing more to create
  }

  if (level == 1) {
    recur_funct_cnt = 0; // reset recurrance counter so it will count per cycle
    Dbg::report_info("Element size(of): ", sizeof(Element) );
  } else { ++recur_funct_cnt; }

  // Warn if too much elemnts created
  if (recur_funct_cnt >= Dbg::cCreateWarningThreshold) {
    Dbg::report_mltpl_warning(Dbg::mltplElementsCreate, recur_funct_cnt);
  }
  
  //Setup DOWN branch
  auto u_ptr_down_temp { std::make_unique<std::array<Element, cFrac::NrOfElements>>() };
  parent_ptr->children_down = u_ptr_down_temp.get(); // ordinary ptr to a structure
  // Move Unique Ptr ownership to dedicated (garbage) Collection
  MemAndDebug::collectElementPtr(std::move(u_ptr_down_temp));
  Dbg::count_elements(cFrac::NrOfElements);
  short ind;
  ind = 0;
  for(auto it = parent_ptr->children_down->begin(); it != parent_ptr->children_down->end(); ++it ) {
    it->order = level;
    it->b_type = downBranch;
    it->index = ++ind;  // 1..cFrac::NrOfElements
    it->stem_xy.prev_l_angle = lAngleUnknown;
    it->parent_ptr = parent_ptr; // link to already existing parent
  }
  
  // Setup UP branch
  auto&& ptr_up_temp = std::make_unique<std::array<Element, cFrac::NrOfElements>>();
  parent_ptr->children_up = ptr_up_temp.get();  // ordinary ptr to a structure
  // Move Unique Ptr ownership to dedicated (garbage) Collection
  MemAndDebug::collectElementPtr(std::move(ptr_up_temp));
  Dbg::count_elements(cFrac::NrOfElements);
  ind = 0;
  for(auto it = parent_ptr->children_up->begin(); it != parent_ptr->children_up->end(); ++it ) {
    it->order = level;
    it->b_type = upBranch;
    it->index = ++ind;  // 1..cFrac::NrOfElements
    it->stem_xy.prev_l_angle = lAngleUnknown;
    it->parent_ptr = parent_ptr; // link to already existing parent
  }
  
  return true; // something created
 
}


bool recurance_elements_redraw(Element * const parent_ptr, const long level, 
           sf::RenderWindow &win, const MovFluctuate &algo_anim,
           AutoScale & autoscale)
{
  static long recur_funct_cnt { 0L };

  // needed calculation of time between frames
  static auto prev_time = std::chrono::high_resolution_clock::now();

  // last allocated Core Element level
  static long lastCoreLevel { 0L };
  
  if (level == 0) {
    // Possible actions per every cycle

    // Smart report - Show # elemnts drawn per cycle if value is >10% change from previous
    Dbg::report_info_by_type(Dbg::infoTypeElementsDrawnPerCycle, recur_funct_cnt); //report o
    recur_funct_cnt = 0; // reset recurrance counter so it will count per cycle

    // time between frames
    auto next_time = std::chrono::high_resolution_clock::now();
    double elapsed_time_ms = 
      std::chrono::duration<double, std::milli>(next_time - prev_time).count();
    // Smart report - time perf frame in ms if value is >10% change from previous
    Dbg::report_info_by_type(Dbg::infoTypeTimePerFrame, elapsed_time_ms);

    // Ensure minimal time between consecutive frame drawing
    long correctionTime { 0 };
    if (elapsed_time_ms < cFrac::MinTimePerFrame) {
      correctionTime = cFrac::MinTimePerFrame - elapsed_time_ms;
      std::this_thread::sleep_for(std::chrono::milliseconds(correctionTime));
    }
    // Omit obove delay for inter frame time calculation
    prev_time = std::chrono::high_resolution_clock::now();
  
  } else {
    // action on non-first call
    ++recur_funct_cnt;
  }

  // Warn if too much elemnts drawed per cycle
  if (recur_funct_cnt >= Dbg::cDrawWarningThreshold) {
    Dbg::report_mltpl_warning(Dbg::mltplElementsDraw, recur_funct_cnt);
    // return false; // if aborted this distorts shape
  }

  // Check for Grow Mutation
  std::optional<float> excGrowScale {std::nullopt};
  if (parent_ptr->mutationGrowPtr) {
    excGrowScale = parent_ptr->mutationGrowPtr->getGrowMutationFraction();
  }
  
  // Tranform this vector (base on settings copied from parent) to the new one 
  parent_ptr->transform_vec_stem(algo_anim, excGrowScale);

  // Take approx vector length : |dx| + |dy| ~ sqrt(dx2 + dy2)
  auto approx_vec =  std::abs(parent_ptr->stem_xy.vec_xy.dx) + 
                     std::abs(parent_ptr->stem_xy.vec_xy.dy); 
  
  // Find place for Grow Mutation if initial growing has finished
  if (!algo_anim.fluctuateState.growingActive) {
    if (MutGrow::possibleInitGrowMutation(parent_ptr, level)) {
      // If allocated update last level
      lastCoreLevel = level; 
    }
  }

  autoscale.findMinMax(parent_ptr->stem_xy.vec_xy);

  // Draw the element
  parent_ptr->draw_stem(win, level, algo_anim.isCoreElemDisplay());

  if (level > cFrac::NrOfOrders) { 
    Dbg::report_once(Dbg::onceLevelsTotal, level);
    return false; // no more branches to scan
  }
  // Check also more then # levels/backlinks from
  // Primary element or from last Core Element
  if (level > lastCoreLevel + MutGrow::cMaxDrawLevels) {
    Dbg::report_once(Dbg::onceLevelsFromCore, level);
    return false;
  }
  
  // Consider element size limits on going to deeper branch
  // If size below threshold do not continue with children,
  // except it is a Core Element (than it must be followed)
  if ((approx_vec < TranAlg::s_SmallVect) and
      (!parent_ptr->coreElement)) {
      return false; // do not go deeper 
  }

  // Either both branches created or both null
  assert((parent_ptr->children_down != nullptr and
           parent_ptr->children_up != nullptr )
      or  (parent_ptr->children_down == nullptr and
           parent_ptr->children_up == nullptr ));
  
  // if needed - create next subordinate braches level starting from current branch
  if (parent_ptr->children_down == nullptr) {
    auto success = new_elements_creation(parent_ptr, level +1);
    if (!success) {
      return false;
    }
  }

  // Follow DOWN branch
  for(auto it = parent_ptr->children_down->begin(); it != parent_ptr->children_down->end(); ++it ) {
    // Traverse next level
    // Propagate (copy) parent position/vector to child (vec_xy is overriten!)
    it->stem_xy.vec_xy = parent_ptr->stem_xy.vec_xy; 
    recurance_elements_redraw(it, level+1, win, algo_anim, autoscale);
  }
  
  // Follow UP branch
  for(auto it = parent_ptr->children_up->begin(); it != parent_ptr->children_up->end(); ++it ) {
    // Traverse next level
    // Propagate (copy) parent position/vector to child (vec_xy is overriten!)
    it->stem_xy.vec_xy = parent_ptr->stem_xy.vec_xy; 
    recurance_elements_redraw(it, level+1, win, algo_anim, autoscale);
  }
  
  return true; // recurance continue
 
}
