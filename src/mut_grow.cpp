// Copyright (c) 2026 Robert Gajewski
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "mut_grow.h"
#include "dbg_report.h"
#include "match_col.h"
#include "transform.h"
#include "light.h"
#include <SFML/Graphics/Color.hpp>
#include <cassert>

// Mutational Grow

// Static data placement
bool MutGrow::mMutEnabled {true}; // Enable Mutational Grow
int MutGrow::mGlobalExcited {0};
int MutGrow::growingCntr = 0;
unsigned long MutGrow::growPtrsCnt; 
std::vector<std::unique_ptr<MutGrow>> MutGrow::allMutGrowPtrs {};


bool MutGrow::possibleInitGrowMutation(Element * const ptrElement,
                                        const short level) {
  // this is to trace are we inside growing branch
  static MutGrow * lastMutGrowPtr = nullptr;
  static long int lastUsedlevel = 0;
  static bool withinLastBranch = false;

  if (level <= lastUsedlevel) {
    withinLastBranch = false;
  }
  
  if (ptrElement->mutationGrowPtr) {
    // within branch, check if within last branch
    assert(lastMutGrowPtr);
    if (lastMutGrowPtr == ptrElement->mutationGrowPtr) {
      // again within last branch
      withinLastBranch = true;
    }
    // do not consider already used element
    return false;
  }
  
  if (level==0) return false;
  if (!mMutEnabled) return false;
  
  // limit # of Mutation Nodes
  if (((growingCntr < cMaxGrowMutations) and (withinLastBranch))
      or (growingCntr == 0)) {

    // Start from relatively lower nodes (to have proper size)
    // if ((level > growingCntr*3) and (level <= 3+ growingCntr*3 )) {
    if (level <= 3+ growingCntr*3 ) {
    
      // Take approx vector length : |dx| + |dy| ~ sqrt(dx2 + dy2)
      auto approx_vec =  std::abs(ptrElement->stem_xy.vec_xy.dx) + 
                         std::abs(ptrElement->stem_xy.vec_xy.dy); 
      // Consider minimal size of element
      if (approx_vec > 8* TranAlg::s_SmallVect) {
    
        // consider only higher half of window
        if (ptrElement->stem_xy.vec_xy.y < cYmid) {
          // consider only elements pointing UP (-y poining up; +y down)
          if (ptrElement->stem_xy.vec_xy.dy
              < -1.8* std::abs(ptrElement->stem_xy.vec_xy.dx)) {
            // beside choose random
            if ((rand() % cMutationProbability) == 0) {
              // (Another) Mutation Growing Node selected
              ++growingCntr;
              lastUsedlevel = level;
              withinLastBranch = true;
              // Create new object and add pointer
              auto u_ptr_temp { std::make_unique<MutGrow>() };
              ptrElement->mutationGrowPtr = u_ptr_temp.get(); // ordinary ptr to a structure
              auto growPtr = ptrElement->mutationGrowPtr;
              lastMutGrowPtr = growPtr;
              growPtr->backlink = ptrElement;
              // Move Unique Ptr ownership to dedicated (garbage) Collection
              collectGrowPtr(std::move(u_ptr_temp));
              // Calculate Max growing fraction to reach given absolute size
              growPtr->growingFractionMax = 
                (3.f/4.f) * static_cast<float>(cYmid) / approx_vec;
              // Calculate growing step
              growPtr->growingFractionStep = growPtr->growingFractionMax / 150.f;
              // Update whole list of Core Nodes chain
              (void)traverseCoreElements();
              // match colors of the stem
              growPtr->calcDominatingColors();
              // Confirm New Core Element assigned
              return true;
            }
          }
        }  
      }
    }
  }
  // No new element assigned
  return false;
}

float MutGrow::getGrowMutationFraction(void) const {
  return growingFraction;
}


int MutGrow::getMaxLevel(void) {
  return growingCntr;
}


bool MutGrow::isGrowingState(void) const {
  return growingState == MutGrow::growingOngoing;
}
  

bool MutGrow::isTopGrowingState(void) {
  if (allMutGrowPtrs.empty()) return false;
  
  // Get last mutational Node
  MutGrow * lastNode = allMutGrowPtrs.back().get();
  assert(lastNode);
  
  return lastNode->growingState == MutGrow::growingOngoing;
}
  

void MutGrow::handleGrowMutationStep(void) {
  if (!mMutEnabled) return;
  
  // Scan all unique ptrs in vector
  for (auto& ptr : allMutGrowPtrs) {
    if (!ptr) {
      assert(false);
      return;
    }
    switch (ptr->growingState) {
      case growingStarting:
        ptr->growingState = growingOngoing;
        ptr->growingFraction = 1.0;
        break;
      case growingOngoing:
        // Check Light Color Catch consition.
        // Mutation excite by proper light color (at proper angle)
        if (ptr->checkColorExcite()) {
          // Excited this cycle - modify growing parameters
          // Fixed increase
          ptr->growingFractionMax *= 2.f;
          ptr->growingFractionStep *= 1.5f;
          ++mGlobalExcited; // this will make autoscale faster
          assert((mGlobalExcited < 1000) and "Too much active excite stems");
        } 
        // Keep growing
        if (ptr->growingFraction < ptr->growingFractionMax) {
          ptr->growingFraction += ptr->growingFractionStep;
        } else {
          // End of Grow
          ptr->growingState = growingFinished;
          if (mGlobalExcited > 0) {
            --mGlobalExcited;
          }
        }
        break;
      case growingFinished:
      default:
        // do nothing
        break;
    }
  }
}


// Stop Algo for End of Game or Pause
void MutGrow::stopAlgo() {
  mMutEnabled = false;  

  // Put all nodes to growing finished (usually only last may be still growing)
  for (auto& ptr : allMutGrowPtrs) {
    if (!ptr) {
      assert(false);
      return;
    }
    // End of Grow
    ptr->growingState = growingFinished;
  }
}

void MutGrow::resumeAlgo() {
  mMutEnabled = true;
}


bool MutGrow::isGlobalExcited(void) {
  return (mGlobalExcited);
}


// Add ownership pointer to the collection
void MutGrow::collectGrowPtr(std::unique_ptr<MutGrow> ptr) {
  allMutGrowPtrs.push_back(std::move(ptr));
  ++growPtrsCnt;

  if (Dbg::cReportWarning) {
    if (growPtrsCnt >= cMaxGrowMutations) {
      Dbg::report_warning("Too much Grow Mutations: ", growPtrsCnt);
    }
  }
}


// Additionally optionally draw Color flags on Active mutation
void MutGrow::colorFlagsDraw(sf::RenderWindow & win) {
  if (allMutGrowPtrs.empty()) return;

  // rectangle flag parameters
  constexpr static float FlagWidth { 25.f };
  constexpr static float FlagLength { 55.f };
  // pointer flag parameters
  constexpr static float PointerProximity { 3.f };
  constexpr static float PointerLength { 20.f };
  constexpr static float PointerWidth { 16.f };
  
  // Get last mutational Node
  MutGrow * lastNode = allMutGrowPtrs.back().get();
  assert(lastNode);
  assert(lastNode->backlink);

  // Consider only active mutation
  if (lastNode->isGrowingState()) {
    // take last Element
    Element * el = lastNode->backlink;

    // takes coordinates of middle of related element
    float fvxMed = (el->stem_xy.vec_xy.x + (1.f/2.f *el->stem_xy.vec_xy.dx));
    float fvyMed = (el->stem_xy.vec_xy.y + (1.f/2.f *el->stem_xy.vec_xy.dy));
    
    // Medium Color
    sf::Color colorBegin = ColorPal::
        getCircularColors(ColorPal::normalColors, el->order).begin_c;
    sf::Color colorEnd = ColorPal::
        getCircularColors(ColorPal::normalColors, el->order).end_c;
    sf::Color medColor = mediumColor(colorBegin, colorEnd);

    // Rectangle part of the Flag
    sf::RectangleShape flagRectangle(sf::Vector2f(FlagLength, FlagWidth));
    // Place Flag to the left of stem Centre
    flagRectangle.setPosition({fvxMed -FlagLength -PointerLength,
                              fvyMed -FlagWidth -PointerWidth});
    flagRectangle.setOutlineColor(sf::Color::White);
    flagRectangle.setFillColor(medColor);
    win.draw(flagRectangle);

    // Pointer part of the Flag - consists of 2 adjacent triangles (4 points)
    sf::VertexArray flagPointer(sf::PrimitiveType::TriangleStrip, 4);
    flagPointer[0].position = sf::Vector2f(fvxMed - PointerLength,
                                           fvyMed - (FlagWidth/3) - PointerWidth); 
    flagPointer[1].position = sf::Vector2f(fvxMed - PointerLength, fvyMed - PointerWidth); 
    flagPointer[2].position = sf::Vector2f(fvxMed - PointerProximity,
                                           fvyMed - PointerProximity); 
    flagPointer[3].position = sf::Vector2f(fvxMed - PointerLength - (FlagWidth/3),
                                           fvyMed - PointerWidth); 
    for (int i = 0; i < 4; i++)
    {
        flagPointer[i].color = medColor;
    }
    win.draw(flagPointer);
  }
  
}


// Relase all dynamic resources
void MutGrow::releaseAll() {
  // Release all Mutationl Grow (manually) in reverse order
  for (auto rit { allMutGrowPtrs.rbegin() }; rit != allMutGrowPtrs.rend(); ++rit)  {
    rit->reset();  // release allocated data
  }
  allMutGrowPtrs.clear();  // clear vector itself
}

// traverse by backlink all Elements from last mutation till primary one
// and mark them
long MutGrow::traverseCoreElements() {
  if (allMutGrowPtrs.empty()) return 0L;

  // Start from last mutational Node
  MutGrow * lastNode = allMutGrowPtrs.back().get();
  assert(lastNode);
  assert(lastNode->backlink);
  // last Element
  Element * el = lastNode->backlink;
  el->coreElement = true;

  // Follow Elements backlinks until last marked
  assert(el->parent_ptr); // at least one link expected
  long int counter {1L};
  while (el->parent_ptr) {
    el = el->parent_ptr;
    // Check of already marked as Core Element
    if (el->coreElement) {
      // Exit loop - we've reached an already-processed element
      break;
    }
    // Mark current element as core
    el->coreElement = true;
    ++counter;
  }; 
  
  return counter;
}

// return Number of Excited (core) Elements
int MutGrow::getExciteCounter() {
  if (allMutGrowPtrs.empty()) return 0;

  int counter {0};
  for (auto it { allMutGrowPtrs.begin() }; it != allMutGrowPtrs.end(); ++it)  {
    auto excite = it->get()->mColorExcite;
    if (excite) {
      ++counter;
    }
  }
  
  return counter;
}


void MutGrow::calcDominatingColors(void) {
  auto colors =  ColorPal::getCircularColors(
                       ColorPal::normalColors, backlink->order);
  
  sf::Color medColor = mediumColor( colors.begin_c, colors.end_c);
  mDominatingColor = matchColor(medColor);
}


bool MutGrow::checkColorExcite(void) {
  // Verify moment of fresh element flash activation
  if (backlink->stem_xy.flash_cnt > LightS::cFlashLightCntMax-3) {
    if (LightS::s_lightColor == getColorFromMatch(mDominatingColor))  {
      // Active color equivalent to stem (medium)
      if (!mColorExcite) {
        // report only once
        Dbg::report_info("Color Match begin: ", static_cast<long>(mDominatingColor));
        mColorExcite = true;
        // Excited this cycle
        return true;
      }
    } 
  }
  return false;
}


ColorMatch MutGrow::getBestMatchingColor(void) {
  if (allMutGrowPtrs.empty()) return ColorMatch::NotFound;
  // take from latest mutation
  return allMutGrowPtrs.back()->mDominatingColor;
}


void MutGrow::resetAlgo() {
  growingCntr = 0;
  growPtrsCnt = 0;
  releaseAll();
  mMutEnabled = true;
}
