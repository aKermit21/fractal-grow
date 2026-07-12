// Copyright (c) 2026 Robert Gajewski (pcc21.com)
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include "dbg_report.h"
#include "basics.h"
#include "screen_size.h"
#include <SFML/Window/Keyboard.hpp>

// Perform (optionally) autoscale by changing size and location
// of first element to fit whole drawing into window
struct AutoScale
{
  using VecMinMax = Dbg::VecMinMax;
  struct VecDelta {float dx; float dy;};

  explicit AutoScale(const ScreenM & screen, bool onOff = true)
  : m_screen {screen}
  , m_minmax{} // actual start values are set by cycleStart
  , m_optionOn{onOff}
  , m_rescaleActive{false}
  , m_cumulativeFactor{1.0}
  {
    // Initial autoscale centering, may change after Resize event
    resizeHandler();

    cycleStart();
    Dbg::report_info("Init: AutoScale (Xsize)  ", m_minmax.minX);
  }

  
  // Window margins considered in start or stop Rescale algo
  constexpr static int cMargin { 12 };
  // more room at the right expected because of light size
  constexpr static int cLightMargin { 80 }; 
  // margin top histeresis
  constexpr static int cHistMargin { 80 };
  // NO mevement, centering, needed
  constexpr static VecDelta cDeltaNoMove { 0, 0 };
  
  // Single step move (centering)
  constexpr static float cSmallStep { 3.0 }; // graphic points
  constexpr static float cBigStep { 5.0 }; // move in fastMode
  constexpr static float cAcceptedDiff { 4.0 }; // graphic points
  // Shrinking step
  constexpr static float cShrinkStep { 0.996f };

  // initialize data for new frame/cycle
  void cycleStart();

    // collect vector sizes to find drawing Min/Max per frame/cycle
  void findMinMax(const Vec2D & vec);
  
  // get Min,Max per frame in real size.
  // Do rescale if needed
  void performAutoscaleCycle(Element & prim, bool fastMode=false);

  bool ifRescaleActive() const {
    return m_rescaleActive;
  };

  float getShrinkCumulativeFactor() const {
    return m_cumulativeFactor;
  }

  void resetAutoScale() {
    m_rescaleActive = false;
    m_cumulativeFactor = 1.0f;
  }
  
  void resizeHandler();
  
  private:

  // Usable window center used throughout autoscale
  int winUsable_x_center;
  // Push figure rather to bottom as it grows up
  int winUsable_y_center;

  const ScreenM & m_screen;
  
  // finding edges (min/max) of all elements per last frame 
  VecMinMax m_minmax;
  
  // Maximum expected size of picture after autoscale shrinking
  float mShrunkPicSizeX;
  float mShrunkPicSizeY;

  // Rescale (option) is switched On
  bool m_optionOn;
  
  // Rescale is Currently active
  bool m_rescaleActive;

  float m_cumulativeFactor { 1 };
  
  VecMinMax getRealScale(void) const;
  
  VecDelta centerPicture(bool fastMode);
  
  bool rescaleRequired(const VecMinMax vec) const;
  bool rescaleFinished(const VecMinMax vec) const;
  bool initialResizingNeeded(const VecMinMax vec) const;
};

