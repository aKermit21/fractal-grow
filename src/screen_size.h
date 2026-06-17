// Copyright (c) 2026 Robert Gajewski (pcc21.com)
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <string_view>
#include "basics.h"
#include "dbg_report.h"

//
// Coordinates recalculation based on Screen Mode -
// full screen vs. fixed, hardcoded.
// Slightly slower methods (27-31ms) than direct constants (26-29ms)

struct ScreenM {
  explicit ScreenM(bool fullscreen)
    : mFullscreen {fullscreen}
    , RightShiftToCenter {0}
    {
      Dbg::report_info("Init: Screen (full=) ", fullscreen);

      // copy all coordinates (they are prepared for hardcoded size window)
      copyFixedScreen();

      if (fullscreen) {
        // recalculates all coordinates to full screen mode
        initiallyRecalculateFullScreen();
      } 

      // Calculates derivative values
      calcDerivatives();
    }
  
  sf::RenderWindow initWindow(std::string_view name, bool fullscreen);

  void resizingHandler(sf::RenderWindow & win, sf::Vector2u newSize);

  bool isFullScreen() const;
  
  int getWindowXsize() const;
  int getWindowYsize() const;

  Vec2D getPrim() const;

  float getPrimStemWidth() const;

  int getXcenterM() const;
  int getYcenterM() const;
  
  int getRightShiftToCenter() const;

  Vec2D recalculatePrimProportion(const Vec2D & inVec,
                                  float proportion = mProportions)  const;
  
  int getDesktopXsize() const;
  int getDesktopYsize() const;

private:
  // Original fixed frame x:y ratio: 6/5 (1200/1000)
  // Maximum full screen x:y ratio <= 4/3
  constexpr static float maxXYratio { 4.f/3 };
  
  const bool mFullscreen;

  // Primary Coordinates of frames to be used throughout code
  int WindowXsize;
  int WindowYsize;
  
  // Primary Element Coordinates considering possible Full Screen Mode
  Vec2D PrimModified;
  float PrimStemWidth;
  
  // Derivatives
  // Window size centre in accuracy units
  float XcenterM;
  float YcenterM;

  // Fullscreen sizes
  int DesktopXsize;
  int DesktopYsize;

  int RightShiftToCenter;

  static float mProportions;

  // recalculates all coordinates Initially
  void initiallyRecalculateFullScreen();

  // just copy all coordinates
  void copyFixedScreen();

  void calcDerivatives();
};
