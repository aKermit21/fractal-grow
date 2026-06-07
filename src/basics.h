// Copyright (c) 2026 Robert Gajewski
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include <array>
#include "config.h"
#include <SFML/Graphics.hpp>
#include <optional>

// Basic CONFIG Constants
namespace cFrac {
  inline const std::string ProgramName { cFullProgramName }; // from config.h
  inline const std::string DemoProgramName { "DEMO -- " +ProgramName+ " -- DEMO" };
  inline const std::string Version { VERSION_STR }; // from config.h
  
  inline constexpr int NrOfElements { 5 }; // number of elements in one child branch  
  // if > 5, requires definition of "Transformation Data" in transform.h

  inline constexpr int NrOfColorPaletes { 8 }; 

  // Maximal absolute possible nesting, down generations, counting from 0 (primary).
  // Minimum 7 because of fixed colors definitions in colors.h
  // (see also NrOfDrawOrders)
  inline constexpr int NrOfOrders { 3000 }; 

  // Graphic visible window  
  inline constexpr int WindowXsize { 1200 }; 
  inline constexpr int WindowYsize { 1000 }; 
  
  // Primary element size, position - assuming window 1200x1000
  inline constexpr float PrimStartX { 30.0 };
  inline constexpr float PrimStartY { 530.0 };
  inline constexpr float PrimVecX { 1000.0 };
  inline constexpr float PrimVecY { -80.0 };
  inline constexpr float PrimStemWidth { 10.0 };
  
  // Assumed initial Size in cm
  inline constexpr float InitialSize { 24 };
  
  // counter mark demo initialization
  inline constexpr int DemoInitCnt { 1 };

  // Minimal time per frame drawing in ms
  inline constexpr double MinTimePerFrame { 15.0 }; // 67Hz
}

enum BranchType { upBranch, downBranch, firstBranch };
enum LightAngleCase {lAngleUnknown = 0, lAngleAbove90, lAngleBelow90};

// other Config Constants in tranform.h

// Types used throughout project

// 2D vector and its current position - x1,y1 -> x2,y2 aka x,y dx,dy
// Warning: values are initially populated (overwritten) 
//          from parent then recalculated
struct Vec2D {
  float x;
  float y;
  float dx;
  float dy;
  // Original dx/dy used as base for Initial Growing transformation
  // used only for primary element recalculation
  // Can be optimize do use `original(s)` only in Primary Element
  float originalDx;
  float originalDy;
  // move along x,y and x+dx,y+dy line in fraction of original line
  // affects x,y - shall be done before rotation
  [[deprecated("Use Stem::repositionStem() instead")]]
  void reposition(const float fraction);
  // rotate and scale - affects dx,dy
  // angle in radians
  void rotateAndRescale(const float angle, const float scale);

  // had angle between light rays and (this) vec changed (<90 vs >90 deg)
  bool light_vec_angle_flip();

  // to small vector size to draw (compared to given threshols)
  bool vecTooSmall(float lengthThreshold);

private:
  template<typename T> // typically float or double
  void rotation_matrix(const T sin_val,
                       const T cos_val); // rotation matrix calculations
};

// Additional points used for drawing stem with thickness
// Warning: x#,y# values are recalculated each frame
struct Stem{
  Vec2D vec_xy; // bare line along centre of stem
  float x1 {};  // additional pofloats at base of stem
  float y1 {};  // deciding about their drawn thickness
  float x2 {};
  float y2 {};
  // move along x,y and x+dx,y+dy line - 
  // affects x,y and x1,y1,x2,y2 - shall be done before rotation;
  // used for contructing child element based on parent
  void reposition_stem(const float fraction, const float scale);
  // move by given (absolute) dx dy
  void repositionStemAbsolute(const float dx, const float dy);
  // Shrink stem according to given (usable) window Center
  void shrinkStemCenter(float factor, float cumulativeFactor, int xCenter, int yCenter);
  // Calculate coordinates of stem with some possible adjustmement (due to autoscale)
  void recalculateStemWidthCoordinates(float cumulativeFactor);

  // // to be used by Flash Light version
  // virtual bool light_vec_angle_flip() = 0;
};

// Stem with additional Flash Light handling
// values remain from previous frame unless explicitelly changed
struct StemFlash : Stem {
  // light angle from previous frame / cycle
  LightAngleCase prev_l_angle;
  // active light flash of stem for # of frames  
  int flash_cnt { 0 }; // no light flash
  // had angle between light rays and stem/vec changed (<90 vs >90 deg)
  bool light_vec_angle_flip();
private:
  LightAngleCase light_vec_angle(float vx, float vy);
};

struct FluctuateState {
  bool windActive;
  bool growingActive;
};

// Common transformation basics - see tranform.h
// live transformation version used for calculations
struct DRec {
  float repos;  // move (reposition) in fraction of original stem
  float angle;  // of a rotation for up branch in radians
  float angle_down;  // of a rotation for down branch (if symmetrical then just -angle)
  float scale; 
};

// compact transformation version used for storing data
struct DRecSymm {
  float repos;  // move (reposition) in fraction of original stem
  int angle;  // of a rotation in 0.1 deg
  float scale; 
};

using T_Algo_Arr = std::array<DRec, cFrac::NrOfElements>;
using T_Algo_Arr_Symm = std::array<DRecSymm, cFrac::NrOfElements>;

// More specifc rules enabling additional fluctuaction imposed
// on top of basic Algo defined above.
// Each level has diffrent angle for wind/wobble
// or modified scale for progressive growing.
// 0th order is primary element, then following orders 1..NrOfOrders - thus +1
using T_Fluctuate_Algo_Arr = std::array<T_Algo_Arr, cFrac::NrOfOrders +1>;

// Forward declaration to enable cross-reference
// between Element and ExcGrow (see mut_grow.h)
struct MutGrow;
// Another Forward declaration (see fluctuate.h)
struct MovFluctuate;
// Another Forward declaration - internal
struct ChildrenElementsCluster ;
  
/* Single Element of Fractal */
struct Element {
  int order { 0 }; // nesting level
  long int index {};     //  position within current branch - 1..cTran::NrOfElements
  BranchType b_type = firstBranch; // First branch valid only for first element
  // vetor / delta coordinates / stem thickness / Flash Light
  StemFlash stem_xy;   
  // Attached up/downside child elements (of next order)
  // in form of link list
  ChildrenElementsCluster * children_down {};
  ChildrenElementsCluster * children_up {};
  // Link to parent (single one)
  Element * parent_ptr {}; // pointer to previous already existing object
  MutGrow * mutationGrowPtr {};
  // Element belong to chain from Primary to (through all) Mutation Grows
  bool coreElement {false};
  // Tranform vec/stem from parent using special transformation array
  // - method for static (single frame) drawing
  void transform_vec_stem(const MovFluctuate & algo_fluct,
                          const std::optional<float> overrideScale);
  virtual void draw_stem(sf::RenderWindow & win, long level, const bool coreElemt);
  void initPrimary();   // Init data for first element 
};

// Cluster of Children - either up or down
struct ChildrenElementsCluster {
  // Parent is common for all Elements in Cluster
  bool toBePruned {false}; // marked to be deallocated by pruning method
  std::array<Element, cFrac::NrOfElements> elements;
};

