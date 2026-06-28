// Copyright (c) 2026 Robert Gajewski (pcc21.com)
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "pause_core.h"
#include "demo_func.h"
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

//
// Pause animation, Core Elements display


bool PauseAni::key_decodation(sf::Keyboard::Key key) {
  switch (key) {
  // Core Elements are linking from primary element to top mutation
  case sf::Keyboard::Key::C: 
    if (mCoreElementDisplayRequested) {
      // switch off
      mCoreElementDisplayRequested = false;
    } else {
      // switch on
      mCoreElementDisplayRequested = true;
    }
    return true;

  default:
    // Not an error: other keys can be handled by another key_decodation 
    return false;
  }
}

bool PauseAni::isPauseActive() const {
  return mPauseActive;
}


bool PauseAni::isCoreElemDisplay() const {
  return mCoreElementDisplayRequested;
}

// Stop (pause) animation
void PauseAni::stopAnimation() {
  if (!mPauseActive) {
    mPauseActive = true;
    mCumulatedTime += std::chrono::steady_clock::now() - mLastStartedTime;
  }
}

// After Pause
void PauseAni::resumeTimeFlow() {
  if (mPauseActive) {
    mPauseActive = false;
    mLastStartedTime = std::chrono::steady_clock::now();
  }
}

// Time of Game start
void PauseAni::startTimeCounting() {
  if (!mTimeStarted) {
    mTimeStarted = true;
    mLastStartedTime = std::chrono::steady_clock::now();
    mCumulatedTime = std::chrono::duration<double>(0.0);
  }
}

void PauseAni::restartTimeCounting() {
    mTimeStarted = true;
    mLastStartedTime = std::chrono::steady_clock::now();
    mCumulatedTime = std::chrono::duration<double>(0.0);
}

std::chrono::duration<double> PauseAni::getTimeOfTheGame() {
  if (!mTimeStarted) {
    return {};
  }

  if (mPauseActive) {
    return mCumulatedTime;
  } else {
    return (std::chrono::steady_clock::now() - mLastStartedTime + mCumulatedTime);
  }
}
