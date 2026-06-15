// Copyright (c) 2026 Robert Gajewski (pcc21.com)
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include "basics.h"
#include "match_col.h"
#include "screen_size.h"
#include <cassert>


// Mutational Grow
// Mutational--that is spontaneously (not Initial Growing) initiated--
// growing of arbitrary element. Once started continues until Finished.
// In the meantime it can be Excited if matched by Color Light,
// this enhances grow

struct MutGrow {
  MutGrow() 
  : mColorExcite { false }
  , growingState { growingStarting }
  , growingFraction { 1.0f }
  , growingFractionMax { 10.0f }
  , growingFractionStep { 0.1f }
  , mDominatingColor { ColorMatch::NotFound }
  , backlink {nullptr}
  {
  }
  
  // # of Mut Growing elements
  constexpr static int cMaxGrowMutations { 1000 };

  // # of Mut Growing elements
  constexpr static int cMutationProbability { 5000 };

  static_assert(cFrac::NrOfOrders > 2.5f* cMaxGrowMutations,
                "Proportions between those parameter must be kept");
  // Above asserts relates to lines of code from possibleInitGrowMutation() function:
  // if (level <= 3+ growingCntr*3 ) {
  
  // Max # of Draw Levels from last Mutation (Core Element)
  constexpr static int cMaxDrawLevels { 9 };

  // Verify proportions
  static_assert((cFrac::NrOfOrders > cMaxDrawLevels) and
    "Wrong Parameters dependecy: NrOfOrders >> cMaxDrawLevels" );
  
  constexpr static int cGrowingSteps { 100 }; // assumed transitions steps

  enum MutGrowingState { growingStarting, growingOngoing, growingOngoingPause,
                        growingFinished };
  
  // common for all instances

  // Initialization which cannot be done with class/instance init
  static void postInit(ScreenM & screen);
  
  static bool possibleInitGrowMutation(Element * const ptr, const short level);
  // void handleSecGrow(SecGrow * const ptr);
  static void handleGrowMutationStep(void);

  // Relase all dynamic resources
  static void releaseAll();
  
  // Complete Reset
  static void resetAlgo();

  // Stop Algo for End of Game (permanent) or Pause
  static void stopAlgo(bool permanent=false);
  
  static void resumeAlgo();
  
  static void collectGrowPtr(std::unique_ptr<MutGrow> ptr);

  float getGrowMutationFraction(void) const;

  // traverse by backlink all Elements from last mutation till primary one
  static long int traverseCoreElements(void);

  // return Number Excited (core) Elements
  static int getExciteCounter(void);
  
  // Additionally optionally draw Color flags on Active mutation
  static void colorFlagsDraw(sf::RenderWindow & win);

  static int getMaxLevel(void);
  
  // Get best matching colors of last mutation
  static ColorMatch getBestMatchingColor(void);

  // Regards current element (instance)
  bool isGrowingState(void) const;

  // Regards last (top) element
  static bool isTopGrowingState(void);
  
  // returns if there is currently growing of Excited Element (globaly)
  static bool isGlobalExcited(void);

  bool mColorExcite;

private:

  // Middle of the vertical window
  static int cYmid;
  
  void calcDominatingColors(void);
  
  // return true if excited this cycle
  bool checkColorExcite(void);
  
  static int growingCntr; // counter for all
  static long growPtrsCnt; // counting pointers

  static bool mMutEnabled;

  // # of globally excited element
  static int mGlobalExcited;
    
  // State of single instance (non-static)
  MutGrowingState growingState;
  float growingFraction; // of an initial size

  // calculate at init per each element to reach given absolute size
  float growingFractionMax; 
  // calculate at init per each element
  float growingFractionStep; 

  // Dominating color best maching to
  // one of the LColors_Init except first (black)
  ColorMatch mDominatingColor;
  
  Element * backlink;

  // Collection of Mutational Grow pointers ownership
  static std::vector<std::unique_ptr<MutGrow>> allMutGrowPtrs;
};
