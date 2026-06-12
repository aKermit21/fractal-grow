// Copyright (c) 2026 Robert Gajewski (pcc21.com)
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "basics.h"
#include "transform.h"
#include <cmath>
#include <assert.h>

// rotation matrix calculations
template<typename T>
void Vec2D::rotation_matrix(const T sin_val, const T cos_val) {
  float dx_new = dx * cos_val - dy * sin_val;
  dy = dx * sin_val + dy * cos_val;
  dx = dx_new;
}

// move along x,y and x+dx,y+dy line - shall be done before rotation
// proportionaly to length size
void Vec2D::reposition(const float fraction) {
  x = x + (dx * fraction);
  y = y + (dy * fraction);
}


// move along x,y and x+dx,y+dy line - shall be done before rotation
void Stem::reposition_stem(const float fraction, float scale) {
  float thickness = 0.0;

  // Calculate thickness of stem from range 0.008 - 0 based on vector size
  // Take approx parent vector length : |dx| + |dy| ~ sqrt(dx2 + dy2)
  float approx_vec =  std::abs(vec_xy.dx) + 
                       std::abs(vec_xy.dy); 

  // Consider scaling just to be applied (by rotateAndRescale() in transform.cpp)
  approx_vec *= scale;

  if (approx_vec > static_cast<float>(cFrac::WindowYsize)/2.0f) {
    thickness = 0.008f; // this initially corresponds to level 1
  } else if (approx_vec < static_cast<float>(cFrac::WindowYsize)/10.0f) {
    // No thickness for too small elements (applies tomost elements)
    thickness = 0.0f; 
  } else {
    // sqrt for better tuned dependency curve
    // Warning: This is potencially Slowing down
    thickness = 0.008f *
        std::sqrt(approx_vec / (static_cast<float>(cFrac::WindowYsize)/2.0f));
  }
   
  // Consider special rule for scale > 1
  // if (scale > 1.0) {
  //   thickness = thickness*scale;
  // }

  x1 = vec_xy.x + vec_xy.dx * (fraction - thickness);
  y1 = vec_xy.y + vec_xy.dy * (fraction - thickness);
  x2 = vec_xy.x + vec_xy.dx * (fraction + thickness);
  y2 = vec_xy.y + vec_xy.dy * (fraction + thickness);

  // Central Line transformation - older Vec2D::reposition()
  vec_xy.x = vec_xy.x + (vec_xy.dx * fraction);
  vec_xy.y = vec_xy.y + (vec_xy.dy * fraction);
}


// Calculate coordinates of stem taking given width
void Stem::recalculateStemWidthCoordinates(float cumulativeFactor) {
  // Use multiplied values for better accuracy tranformation
  float stem_x, stem_y, length;
  // create perpendicular vector of given width
  stem_x = -vec_xy.dy;
  stem_y = vec_xy.dx;
  length = std::sqrt(stem_x*stem_x + stem_y*stem_y);
  auto adjustedStemWidth = cumulativeFactor * cFrac::PrimStemWidth;
  y1 =  (vec_xy.y - stem_y * (adjustedStemWidth/length) );
  y2 =  (vec_xy.y + stem_y * (adjustedStemWidth/length) );
  x1 =  (vec_xy.x - stem_x * (adjustedStemWidth/length) );
  x2 =  (vec_xy.x + stem_x * (adjustedStemWidth/length) );
}


// Shrink stem according to given (usable) window Center
// used for auto-scaling
void Stem::shrinkStemCenter(float factor, float cumulativeFactor, 
                            int xCenter, int yCenter) {

  assert(factor <= 1 and factor > 0 and "factor for shrinking expected to be 0..1");
  assert(xCenter > 0 and yCenter > 0 and "expected plus coordinates");
  
  // Change position to obtain shrinking in relation to (window) central point
  int x1_center = vec_xy.x - xCenter;
  x1_center = static_cast<int>(x1_center * factor);
  vec_xy.x = x1_center + xCenter;
  
  int y1_center = vec_xy.y - yCenter;
  y1_center = static_cast<int>(y1_center * factor);
  vec_xy.y = y1_center + yCenter;
  
  vec_xy.dx *= factor; 
  vec_xy.dy *= factor;
  // Copy used for primary element possible 'growing' transformation
  vec_xy.originalDx = vec_xy.dx; 
  vec_xy.originalDy = vec_xy.dy;

  // Calculate coordinates of stem taking given width
  recalculateStemWidthCoordinates(cumulativeFactor);
}

// move by given (absolute) dx dy
void Stem::repositionStemAbsolute(const float dx, const float dy) {
  // Move Stem axis (vector)
  vec_xy.x += dx;
  vec_xy.y += dy;
  // Move possible stem ply drawing
  x1 += dx;
  x2 += dx;
  y1 += dy;
  y2 += dy;
}

void Vec2D::rotateAndRescale(const float angle, const float scale = 1.0) {
  if (angle != 0.0) {
      // Primary sin/cos math calculations in radians

      float temp_sinus = sin(angle);
      float temp_cosin = cos(angle);
      rotation_matrix(temp_sinus, temp_cosin);
  }
  if (scale != 1.0) {
    dx = dx * scale;
    dy = dy * scale;
  }
}
