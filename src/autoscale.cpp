// Copyright (c) 2026 Robert Gajewski (pcc21.com)
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "autoscale.h"
#include "dbg_report.h"
#include "basics.h"
#include "screen_size.h"
#include <cassert>
#include <cmath>

using VecMinMax = Dbg::VecMinMax;
  
// initialize data on new frame/cycle
void AutoScale::cycleStart() {
  m_minmax.minX = m_screen.getWindowXsize(); // end of window 
  m_minmax.minY = m_screen.getWindowYsize(); // end of window 
  m_minmax.maxX = 0;
  m_minmax.maxY = 0;
}

// collect vector sizes to find drawing Min/Max per frame/cycle
void AutoScale::findMinMax(const Vec2D & vec) {
  if (m_minmax.minX > vec.x) { m_minmax.minX = vec.x;}
  // consider that vector can also go backward
  if (m_minmax.minX > vec.x + vec.dx) { m_minmax.minX = vec.x + vec.dx;}
  if (m_minmax.maxX < vec.x) { m_minmax.maxX = vec.x;}
  if (m_minmax.maxX < vec.x + vec.dx) { m_minmax.maxX = vec.x + vec.dx;}
  if (m_minmax.minY > vec.y) { m_minmax.minY = vec.y;}
  if (m_minmax.minY > vec.y + vec.dy) { m_minmax.minY = vec.y + vec.dy;}
  if (m_minmax.maxY < vec.y) { m_minmax.maxY = vec.y;}
  if (m_minmax.maxY < vec.y + vec.dy) { m_minmax.maxY = vec.y + vec.dy;}
}

// get Min,Max per frame in real size;
// performe single step of rescale if needed
void AutoScale::performAutoscaleCycle(Element & prim, bool fastMode) {

  // Collecting picture contour sizes for whole program duration
  Dbg::find_minmax(m_minmax);

  if(m_optionOn) {
  
    if(rescaleRequired(m_minmax) and !m_rescaleActive) {
      // Rescale Activation
      Dbg::report_info("Rescale Activation ", m_rescaleActive );
      m_rescaleActive = true;

    } else if(rescaleFinished(m_minmax) and m_rescaleActive) {
      // Rescale Deactivation
      Dbg::report_info("Rescale Deactivation ", m_rescaleActive );
      m_rescaleActive = false;
    } else {
      // No rescaling or continuation of rescaling
    }

    if (m_rescaleActive) {
      // Realize single step of rescale
      VecDelta delta = centerPicture(fastMode); // uses Usable (not absolute) center
      // first center picure within window
      if ((delta.dx != cDeltaNoMove.dx) or (delta.dy != cDeltaNoMove.dy)) {
        // move to center by calculated step
        prim.stem_xy.repositionStemAbsolute(delta.dx, delta.dy);
      } else {
        // then shrink if needed (by factor)
        m_cumulativeFactor *= cShrinkStep;
        prim.stem_xy.shrinkStemCenter(cShrinkStep, m_cumulativeFactor,
                                  winUsable_x_center, winUsable_y_center, m_screen);
      }
    }
  }
}


// Calculate delta vector to center picure (in window) in multiple small steps
// uses accuracy units
AutoScale::VecDelta AutoScale::centerPicture(bool fastMode) {
  
  static_assert(cAcceptedDiff > cSmallStep, 
    "Accepted diffrence must be > than small change; otherwise algo will not conclude");

  assert(winUsable_x_center < m_screen.getWindowXsize() and 
              "Usable horizontal center move to left (X axis)");

  // Calculate center position of latest frame picture
  int pic_x_center = (m_minmax.minX + m_minmax.maxX) /2;
  int pic_y_center = (m_minmax.minY + m_minmax.maxY) /2;
  
  // calculate complete move required
  VecDelta delta {};
  delta.dx = winUsable_x_center - pic_x_center;
  delta.dy = winUsable_y_center - pic_y_center;

  if ((std::abs(delta.dx) <= cAcceptedDiff) and
      (std::abs(delta.dy) <= cAcceptedDiff)) {\
    return cDeltaNoMove; // no movement needed anymore if very small diffrence
  }

  // For very big changes move without animation
  if ((std::abs(delta.dx) > m_screen.getWindowXsize()/2.f) or
      (std::abs(delta.dy) > m_screen.getWindowYsize()/2.f)) {
    Dbg::report_warning("Big AutoScale move - Shall Not happen (|delta|=)",
                        std::sqrt(delta.dx *delta.dx + delta.dy*delta.dy));
    return delta;
  }
      
  float myStep {cSmallStep};
  // Consider FastMode
  if (fastMode) {
    if ((std::abs(delta.dx) >= cBigStep) or (std::abs(delta.dy) >= cBigStep)) {
      myStep = cBigStep;
    } 
    // Fallback to Slow mode if delta is too small
  }

  // Move to center by number of consecutive steps
  VecDelta stepDelta { 0, 0 };
  // Make proportional move x vs y
  if (std::abs(delta.dx) > std::abs(delta.dy)) {
    if (delta.dx > 0.f) { stepDelta.dx = myStep; }
    else {stepDelta.dx = -myStep;}
    stepDelta.dy = stepDelta.dx * delta.dy / delta.dx;
  } else {
    if (delta.dy > 0.f) { stepDelta.dy = myStep; }
    else {stepDelta.dy = -myStep;}
    stepDelta.dx = stepDelta.dy * delta.dx / delta.dy;
  }

  return stepDelta;
}


// Below two functions must be correlated
// oherwise rescaling oscillations can occure

// Check if rescale has to be started 
bool AutoScale::rescaleRequired(const VecMinMax vec) const {
  // Possible Right shift to center drawing in full screen mode
  int shift = 0;
  if (m_screen.isFullScreen()) {
    shift = m_screen.getRightShiftToCenter();
  }

  if ((vec.minX < cMargin) or (vec.minY < cMargin) or
      (vec.maxX > (m_screen.getWindowXsize() - cLightMargin + shift)) or
      (vec.maxY > (m_screen.getWindowYsize() - cMargin))) {
    return true; 
  }
  else { 
    return false;
  }
}
  
// Check if rescale has to be stopped 
bool AutoScale::rescaleFinished(const VecMinMax vec) const {
  // Possible Right shift to center drawing in full screen mode
  int shift = 0;
  if (m_screen.isFullScreen()) {
    shift = m_screen.getRightShiftToCenter();
  }

  if ((vec.minX >= cMargin + cHistMargin) and (vec.minY >= cMargin + cHistMargin) and
      (vec.maxX < (m_screen.getWindowXsize() - (2*cMargin) - cLightMargin + shift)) and
      // Push image rather to bottom
      (vec.maxY < (m_screen.getWindowYsize() - (5*cMargin) ))) {
    return true; 
  }
  else { 
    return false;
  }  
}

// Usable center need to be recalculated (only)
// if Window Resize system event happened
void AutoScale::resizeHandler() {
  winUsable_x_center = m_screen.isFullScreen() ?
              m_screen.getXcenterM() + m_screen.getRightShiftToCenter()
                          - static_cast<int>(cLightMargin /2.0) :
              m_screen.getXcenterM() - static_cast<int>(cLightMargin /2.0);
  winUsable_y_center = m_screen.getYcenterM() + cHistMargin -2;
}
