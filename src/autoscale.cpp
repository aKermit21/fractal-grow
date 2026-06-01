// Copyright (c) 2026 Robert Gajewski
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "autoscale.h"
#include "dbg_report.h"
#include "fractal.h"
#include "transform.h"

using VecMinMax = Dbg::VecMinMax;
  
// initialize data on new frame/cycle
void AutoScale::cycleStart() {
  m_minmax.minX = cFrac::WindowXsize; // end of window 
  m_minmax.minY = cFrac::WindowYsize; // end of window 
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
        auto shrinkStep = cShrinkStepSmall;
        if (bigRescaleRequired(m_minmax)) {
          shrinkStep = cShrinkStepHuge;
        }
        m_cumulativeFactor *= shrinkStep;
        prim.stem_xy.shrinkStemCenter(shrinkStep, m_cumulativeFactor,
                                      winUsable_x_center, winUsable_y_center);
      }
    }
  }
}


// Calculate delta vector to center picure (in window) in multiple small steps
// uses accuracy units
AutoScale::VecDelta AutoScale::centerPicture(bool fastMode) {
  
  static_assert(cAcceptedDiff > cSmallStep, 
    "Accepted diffrence must be > than small change; otherwise algo will not conclude");

  static_assert(winUsable_x_center < cTran::cXcenterM, 
              "Usable horizontal center move to left (X axis)");

  // Calculate center position of latest frame picture
  int pic_x_center = (m_minmax.minX + m_minmax.maxX) /2;
  int pic_y_center = (m_minmax.minY + m_minmax.maxY) /2;
  
  // calculate complete move required
  VecDelta delta {};
  delta.dx = winUsable_x_center - pic_x_center;
  delta.dy = winUsable_y_center - pic_y_center;

  if ((abs(delta.dx) <= cAcceptedDiff) and
      (abs(delta.dy) <= cAcceptedDiff)) {\
    return cDeltaNoMove; // no movement needed anymore if very small diffrence
  }

  // For very big changes move without animation
  if ((std::abs(delta.dx) > cFrac::WindowXsize/2.f) or
      (std::abs(delta.dy) > cFrac::WindowYsize/2.f)) {
    return delta;
  }
      
  // Consider FastMode
  if (fastMode) {
    // move to center by big or small step
    VecDelta stepDelta { 0, 0 };
    if (delta.dx > cBigStep)  stepDelta.dx = cBigStep; 
    else if (delta.dx > cSmallStep) stepDelta.dx = cSmallStep;
    
    if (delta.dy > cBigStep) stepDelta.dy = cBigStep;
    else if (delta.dy > cSmallStep) stepDelta.dy = cSmallStep;
    
    if (delta.dx < -cBigStep) stepDelta.dx = -cBigStep;
    else if (delta.dx < -cSmallStep) stepDelta.dx = -cSmallStep;
  
    if (delta.dy < -cBigStep) stepDelta.dy = -cBigStep;
    else if (delta.dy < -cSmallStep) stepDelta.dy = -cSmallStep;

    return stepDelta;
  }
  
  // move to center only by small steps
  VecDelta stepDelta { 0, 0 };
  if (delta.dx > cSmallStep) stepDelta.dx = cSmallStep;
  if (delta.dy > cSmallStep) stepDelta.dy = cSmallStep;
  if (delta.dx < -cSmallStep) stepDelta.dx = -cSmallStep;
  if (delta.dy < -cSmallStep) stepDelta.dy = -cSmallStep;

  return stepDelta;
}

// Check if One Step rescale is prefered
bool AutoScale::bigRescaleRequired(const VecMinMax vec) const {

  auto xSpan = std::abs(vec.maxX - vec.minX);
  auto ySpan = std::abs(vec.maxY - vec.minY);
  if ((xSpan > 2.5*cFrac::WindowXsize) or (ySpan > 2.5*cFrac::WindowYsize)) {
    return true; 
  }
  else { 
    return false;
  }
}


// Below two functions must be correlated
// oherwise rescaling oscillations can occure

// Check if rescale has to be started 
bool AutoScale::rescaleRequired(const VecMinMax vec) const {

  if ((vec.minX < cMargin) or (vec.minY < cMargin) or
      (vec.maxX > (cFrac::WindowXsize - cLightMargin)) or
      (vec.maxY > (cFrac::WindowYsize - cMargin))) {
    return true; 
  }
  else { 
    return false;
  }
}
  
// Check if rescale has to be stopped 
bool AutoScale::rescaleFinished(const VecMinMax vec) const {

  if ((vec.minX >= cMargin + cHistMargin) and (vec.minY >= cMargin + cHistMargin) and
      (vec.maxX < (cFrac::WindowXsize - (2*cMargin) - cLightMargin)) and
      // Push image rather to bottom
      (vec.maxY < (cFrac::WindowYsize - (5*cMargin) ))) {
    return true; 
  }
  else { 
    return false;
  }  
}
