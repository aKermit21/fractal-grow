// Copyright (c) 2026 Robert Gajewski (pcc21.com)
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "transform.h"
#include "dbg_report.h"
#include "fluctuate.h"
#include "basics.h"
#include "aux_func.h"
#include <SFML/Window/Keyboard.hpp>
#include <cstdlib>
#include <optional>
#include <assert.h>

// Small vector - below this size stop recursive search/draw of children
float TranAlg::s_SmallVect;
float TranAlg::s_SmallVectDefault;


void Element::initPrimary() {
  // Use multiplied values for better accuracy tranformation
  stem_xy.vec_xy.x = cFrac::PrimStartX; 
  stem_xy.vec_xy.y = cFrac::PrimStartY; 
  stem_xy.vec_xy.dx = cFrac::PrimVecX; 
  stem_xy.vec_xy.dy = cFrac::PrimVecY; 
  // Make a copy of dx and dy
  stem_xy.vec_xy.originalDx = cFrac::PrimVecX; 
  stem_xy.vec_xy.originalDy = cFrac::PrimVecY; 
  // stem width
  stem_xy.x1 = stem_xy.vec_xy.x;
  stem_xy.x2 = stem_xy.vec_xy.x;
  stem_xy.y1 = stem_xy.vec_xy.y - cFrac::PrimStemWidth; 
  stem_xy.y2 = stem_xy.vec_xy.y + cFrac::PrimStemWidth; 
  // prevent initial angle change
  stem_xy.prev_l_angle = lAngleUnknown;
  // Primary stem is a Core Element
  coreElement = true;

  // Reset initial Pointers
  children_down = nullptr;
  children_up = nullptr;
  parent_ptr = nullptr;
}


// Tranform parent vector (also stem data) to the child one 
// considering index and branch type
void Element::transform_vec_stem(const MovFluctuate & fluctuate,
                                 const std::optional<float> overrideScale) { 
  
  const T_Fluctuate_Algo_Arr & algo_fluct = fluctuate.algo_data_fluctuate;
  
  float fraction; // reposition begining of stem along parent line (in fraction)
  float angle;  // rotation
  float scale;  // usually shrinking but exceptionally may be growing
  
    
  // Only (possible) special limited transformation of primary object
  if (order == 0) {
    // Dbg::report_info("Primary element tranformation" );
    stem_xy.vec_xy.dx =
      algo_fluct[0][0].scale * stem_xy.vec_xy.originalDx;
    stem_xy.vec_xy.dy =
      algo_fluct[0][0].scale * stem_xy.vec_xy.originalDy;
  } else  {
    // Transformation depends on index (element #)
    // but our branch index has range 1..cFrac::NrOfElements
    auto arr_index = index -1;
    if (arr_index < 0) {
      assert(false and "negative index"); // always assert
      Dbg::report_error("Branch Index out of range: ", index);
      fraction = 0.0; angle = 0.0; scale = 1.0;
    } else {
      fraction = algo_fluct.at(order).at(arr_index).repos; 
      scale = algo_fluct.at(order).at(arr_index).scale;
      assert((scale < 1.0) and
             "scale>=1 will created immediately infinite fractal");
      // Exceptional scaling - additional grow
      if (overrideScale.has_value()) {
        scale *= *overrideScale;
        assert(coreElement and "override scale only to coreElement");
        // Dbg::report_info("Exceptional scaling: ", static_cast<long>(scale*1000L));
      }
      if (b_type == upBranch) {
        // For core elements take non-fluctuating angle
        if (coreElement) {
          angle = fluctuate.algo_data[arr_index].angle;
        } else {
          angle = algo_fluct.at(order).at(arr_index).angle;
        }
      }
      else {
        // For core elements take non-fluctuating angle
        if (coreElement) {
          angle = fluctuate.algo_data[arr_index].angle_down;
        } else {
          angle = algo_fluct.at(order).at(arr_index).angle_down;
        }
        // For symmetrical
        //   angle = -angle; // reverse angle
      }
    }

    stem_xy.reposition_stem(fraction, scale);

    // rotate and re-scale
    stem_xy.vec_xy.rotateAndRescale(angle, scale);
    // dbg.report_trace("  rotation at angle", angle);
  }
}



// rather obsolete as only one hardcoded configuration is used
void TranAlg::rotate_pre_cfg(){
  int maxElement = preCalcAlgoData.size();
  
  assert(maxElement >= 1);
  assert((m_indexPre >= 0) and (m_indexPre < maxElement));

  ++m_indexPre;
  if (m_indexPre >= maxElement) {
    m_indexPre = 0;
  }
  
  // Take next Predefined configuration
  // Working copy
  algo_data_init = conv_to_assym(preCalcAlgoData[m_indexPre]);
  // Initial setting
  algo_data = algo_data_init;
}


// controlling smallest vector size to be drawn
// controls speed vs accurracy drawing
void TranAlg::speedIncrement(void){
  // Increase size thus speed
  ++m_speedScale;
  speedScaleVerify();
}

void TranAlg::speedDecrement(void){
  // Decrease size thus speed (but increase drawing details)
  --m_speedScale;
  speedScaleVerify();
  assert(m_speedScale >= 0); // index
}

// speed to small vector size calculation
void TranAlg::speedScaleVerify(void) {
  // Verify allowed range
  if (m_speedScale < 0) { m_speedScale = 0;}
  if (m_speedScale > SpeedScaleDataSize -1 ) { m_speedScale = SpeedScaleDataSize -1; }

  // Initial Speed vs Detailed drawing scale table
  s_SmallVect = SpeedScalaData[m_speedScale];
}


int TranAlg::get_speedScale() const {
  return m_speedScale;
}
  

// Convert Symmetrical initial algo to (potentially) Asymmetrical
// also convert 0.1deg (for manual reference) to internal radians
T_Algo_Arr TranAlg::conv_to_assym(T_Algo_Arr_Symm symm_algo) {
  T_Algo_Arr assym_algo;
  
  for (size_t ind {0}; ind < cFrac::NrOfElements; ++ind) {
    assym_algo[ind].repos = symm_algo[ind].repos;
    // Use radians (float) from now on
    assym_algo[ind].angle =
      myAux::zeroOneDegreesToRadians(static_cast<float>(symm_algo[ind].angle));
    // initially just mirror angle
    assym_algo[ind].angle_down = - assym_algo[ind].angle;
    assym_algo[ind].scale = symm_algo[ind].scale;
  }
  return assym_algo;
}


// to small vector size to draw (compared to given threshols)
bool Vec2D::vecTooSmall(float lengthThreshold) {
  float vecLenght2 = (dx*dx) + (dy*dy);
  return (vecLenght2 < (lengthThreshold*lengthThreshold));
}


