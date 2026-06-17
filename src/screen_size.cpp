// Copyright (c) 2026 Robert Gajewski (pcc21.com)
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "screen_size.h"
#include "basics.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <string_view>

float ScreenM::mProportions {1.f};
  
// just copy all coordinates
void ScreenM::copyFixedScreen() {
  WindowXsize = cFrac::WindowXsizeFixed;
  WindowYsize = cFrac::WindowYsizeFixed;
  
  // Primary element size, position - assuming window 1200x1000
  PrimModified.x = cFrac::PrimStartXfixed;
  PrimModified.y = cFrac::PrimStartYfixed;
  PrimModified.dx = cFrac::PrimVecXfixed;
  PrimModified.dy = cFrac::PrimVecYfixed;
  PrimStemWidth = cFrac::PrimStemWidthFixed;
}

// recalculates all coordinates
void ScreenM::initiallyRecalculateFullScreen() {

  // Get the current desktop video mode
  sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
  
  // Access the dimensions (width and height)
  int screenWidth = desktop.size.x;
  int screenHeight = desktop.size.y;
  
  Dbg::report_info("Screen X size: ",screenWidth);
  Dbg::report_info("Screen Y size: ",screenHeight);

  // Take correction for cheated screen sizes
  int screenUsableHeight = screenHeight - 30;
  int screenUsableWidth = screenWidth - 30;
  // Store it
  DesktopXsize = screenUsableWidth;
  DesktopYsize = screenUsableHeight;

  // Consider Maximum full screen x:y ratio 
  if (DesktopXsize > DesktopYsize * maxXYratio) {
    screenUsableWidth = DesktopYsize * maxXYratio;
    // Horizontal centering
    RightShiftToCenter = (DesktopXsize - screenUsableWidth) / 2; 
  }

  // Get Hardcoded Primary Element
  Vec2D initVec;
  initVec.x = cFrac::PrimStartXfixed; 
  initVec.y = cFrac::PrimStartYfixed; 
  initVec.dx = cFrac::PrimVecXfixed; 
  initVec.dy = cFrac::PrimVecYfixed; 
  // Make a copy of dx and dy
  initVec.originalDx = cFrac::PrimVecXfixed; 
  initVec.originalDy = cFrac::PrimVecYfixed; 

  // Proportional Shrink or Enlargement
  float proportions;
  if ((DesktopXsize < WindowXsize) or (DesktopYsize < WindowYsize)) {
    // Rather shrink than enlarge
    if ((static_cast<float>(screenUsableWidth) / WindowXsize) >
        (static_cast<float>(DesktopYsize) / WindowYsize)) {
      // Shrink according to Y axis (Height)
      proportions = static_cast<float>(DesktopYsize) / WindowYsize;
    } else {
      // Shrink according to X axis (Width)
      proportions = static_cast<float>(screenUsableWidth) / WindowXsize;
    }
  } else {
    // Enlarge
    if ((static_cast<float>(screenUsableWidth) / WindowXsize) >
        (static_cast<float>(DesktopYsize) / WindowYsize)) {
      // Enlarge according to Y axis (Height)
      proportions = static_cast<float>(DesktopYsize) / WindowYsize;
    } else {
      // Enlarge according to X axis (Width)
      proportions = static_cast<float>(screenUsableWidth) / WindowXsize;
    }
  }

  WindowXsize = screenUsableWidth;
  // Use whole Y dimension
  WindowYsize = DesktopYsize;
  // Store proportion
  mProportions = proportions; 
  PrimModified = recalculatePrimProportion(initVec, proportions);
}


Vec2D ScreenM::recalculatePrimProportion(const Vec2D & input,
                                         float proportion) const {

  Vec2D outVec {input};

  outVec.x = proportion * input.x;
  outVec.x += RightShiftToCenter;

  outVec.y = proportion * input.y;

  outVec.dx = proportion * input.dx;
  outVec.dy = proportion * input.dy;
  // Make a copy of dx and dy
  outVec.originalDx = outVec.dx;
  outVec.originalDy = outVec.dy;

  return outVec;
}

  
void ScreenM::calcDerivatives() {
  XcenterM = static_cast<int>(WindowXsize / 2.0);
  YcenterM = static_cast<int>(WindowYsize / 2.0);
}

sf::RenderWindow ScreenM::initWindow(std::string_view name, bool fullscreen) {
  if (fullscreen) {
        auto desktop = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(sf::VideoMode(desktop.size), name,
                            sf::State::Fullscreen);
    return window;
  } else {
    sf::RenderWindow window(sf::VideoMode({static_cast<unsigned int>(WindowXsize), static_cast<unsigned int>(WindowYsize)}),  name);
    return window;
  }
}
  
void ScreenM::resizingHandler(sf::RenderWindow & win, sf::Vector2u newSize) {
  WindowXsize = newSize.x;
  
  // Consider Maximum full screen x:y ratio 
  if (static_cast<float>(newSize.x)/newSize.y > maxXYratio) {
    // Limit X-axis size
    WindowXsize = static_cast<unsigned int>(newSize.y * maxXYratio); 
  } 

  // Utilize whole Y-axis size
  WindowYsize = newSize.y;

  calcDerivatives();
  
  // Do not distort picture
  sf::View newView(sf::FloatRect({0.f, 0.f}, sf::Vector2f(newSize)));
  win.setView(newView);

}


bool ScreenM::isFullScreen() const {
  return mFullscreen;
}

int ScreenM::getWindowXsize() const {
return WindowXsize;
}

int ScreenM::getWindowYsize() const {
return WindowYsize;
}

int ScreenM::getDesktopXsize() const {
  return DesktopXsize;
}

int ScreenM::getDesktopYsize() const {
  return DesktopYsize;
}

Vec2D ScreenM::getPrim() const {
  return PrimModified;
}

float ScreenM::getPrimStemWidth() const {
  return PrimStemWidth;
}

int ScreenM::getXcenterM() const {
  return XcenterM;
}

int ScreenM::getYcenterM() const {
  return YcenterM;
}

int ScreenM::getRightShiftToCenter() const {
  return RightShiftToCenter;
}

