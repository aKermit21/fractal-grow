// Copyright (c) 2026 Robert Gajewski (pcc21.com)
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "fluctuate.h"
#include "pause_core.h"
#include "basics.h"
#include <SFML/Window/Keyboard.hpp>
#include <cstdlib>
#include <algorithm>
#include <iostream>

// animation related keys handling
bool MovFluctuate::key_decodation(sf::Keyboard::Key key) {
  switch (key) {
  // Tilde start windy wobbling
  case sf::Keyboard::Key::Grave: 
    if (!fluctuateState.windActive) {
      // copy Algo per level
      algo_data_fluctuate = conv_to_fluctuate(algo_data);
      srand( time(NULL));
      // start wind (wobbling) modification
      fluctuateState.windActive = true;
    } else {
      fluctuateState.windActive = false;
      stop_wind();
    }
    return true; // my key
  case sf::Keyboard::Key::Space:
    // Reset/Restore Live transformation algo from more primitive one
    algo_data_fluctuate = conv_to_fluctuate(algo_data);
    return true;
  case sf::Keyboard::Key::R:
    // Reset/Restore Live transformation algo from more primitive one
    algo_data_fluctuate = conv_to_fluctuate(algo_data);
    return true;
  // case sf::Keyboard::Key::P:
  //   // TODO: Possible more keys needed
  //   // Refresh final transformation algo with optional growing animation
  //   refreshWithRestartGrowing();
  //   return true;
  default:
    // Not my key
    return false;
  }
}


T_Fluctuate_Algo_Arr MovFluctuate::conv_to_fluctuate(T_Algo_Arr assym_algo){
  T_Fluctuate_Algo_Arr temp_algo;
  for (size_t level {0}; level <= cFrac::NrOfOrders; ++level) {
    for (size_t elem {0}; elem < cFrac::NrOfElements; ++elem) {
      temp_algo[level][elem].angle = assym_algo[elem].angle;
      temp_algo[level][elem].angle_down = assym_algo[elem].angle_down;
      // Those below two shall not be varing
      temp_algo[level][elem].repos = assym_algo[elem].repos;
      // primary element has no initial transformation so assume scale 1.0
      if (level == 0) {
        temp_algo[level][elem].scale = 1.0;
      } else {
        temp_algo[level][elem].scale = assym_algo[elem].scale;
      }
    }
  }
  return temp_algo;
}

void MovFluctuate::stop_wind() {
  // stop wind (wobbling) modification if it was running
  fluctuateState.windActive = false;
  // restore non-modified transformation algo
  algo_data_fluctuate = conv_to_fluctuate(algo_data);
  // Clear by filling with zeros
  windVelocity = {};
  // reset flash algo done by higher aggreg class
}

void MovFluctuate::pauseWind() {
  // stop wind (wobbling) modification if it was running
  fluctuateState.windActive = false;
}

void MovFluctuate::resumeWind() {
  // stop wind (wobbling) modification if it was running
  fluctuateState.windActive = true;
}


// called once in a display loop
void MovFluctuate::one_step_cfg_change() {
  // Growing or Wind (shaky)
  if (fluctuateState.growingActive) {
    oneStepGrowingChange();
  } else if (fluctuateState.windActive) {
    oneStepWindChange();
  }
}

void MovFluctuate::oneStepWindChange() {
  // Wind (shaky)
  // 0th (primary element) is always fixed
  for (size_t level {1}; level <= cFrac::NrOfOrders; ++level) {
    // enable random play
    int step = 6;
    // higher level bigger trembling
    // step *= level+1;
    for (size_t elem {0}; elem < cFrac::NrOfElements; ++elem) {
      auto delta = algo_data_fluctuate[level][elem].angle - algo_data[elem].angle;
      auto delta_down = algo_data_fluctuate[level][elem].angle_down
                     - algo_data[elem].angle_down;

      // Assymetric random changes for too big deviations
      if (delta > cWindTolerance) {
        windVelocity[level][elem].up += ((rand() % step) - step) * cWindFactor;
      } else if (delta < - cWindTolerance) {
        windVelocity[level][elem].up += ((rand() % step) + step) * cWindFactor;
      } else {
        windVelocity[level][elem].up += ((rand() % (2*step)) - step) * cWindFactor;
      }
      // down
      if (delta_down > cWindTolerance) {
        windVelocity[level][elem].down += ((rand() % step) - step) * cWindFactor;
      } else if (delta_down < - cWindTolerance) {
        windVelocity[level][elem].down += ((rand() % step) + step) * cWindFactor;
      } else {
        windVelocity[level][elem].down += ((rand() % (2*step)) - step) * cWindFactor;
      }

      // Simulate constant friction
      windVelocity[level][elem].down *= cWindFriction;
      windVelocity[level][elem].up *= cWindFriction;

      // Add velocity to an angle
      algo_data_fluctuate[level][elem].angle += windVelocity[level][elem].up;
      algo_data_fluctuate[level][elem].angle_down += windVelocity[level][elem].down;
      
    }
  }
}

void MovFluctuate::refreshWithRestartGrowing(void) {
  // copy Algo per level - draw() uses this format
  algo_data_fluctuate = conv_to_fluctuate(algo_data);

  if (GrowingEnabled) {
    fluctuateState.growingActive = true;
    // Populate all with 0's
    growingDynamic = {0};
    // except first item
    growingDynamic[0] = 3; 
    // start final algo modification before first desplay
    oneStepGrowingChange();
  }
}
 
void MovFluctuate::oneStepGrowingChange() {
  // Growing scheme
  // next column starts (0->1) if previous reaches PRIMING_NUMBER (here 5)
  // grow element by +1 if already non-zero until it reaches LAST_NUMBER (here 10)
  // start with 1 0 0 ...
  // 1, 0, 0
  // 2, 0, 0
  // 3, 0, 0
  // 4, 0, 0
  // 5, 1, 0
  // 6, 2, 0
  // 7, 3, 0
  // 8, 4, 0
  // 9, 5, 1
  //10, 6, 2
  //10, 7, 3
  //10, 8, 4, 0
  //10, 9, 5, 1
  //10,10, 6, 2

  // Iteration change

  constexpr int PRIMING_NUMBER = 10;
  constexpr int LAST_NUMBER = 30;

  bool startNextColumn {false};
  for (size_t index {0}; index<=cFrac::NrOfOrders; ++index) {
    if (growingDynamic[index] > 0) {
      if (growingDynamic[index] < LAST_NUMBER) ++growingDynamic[index];
      if (growingDynamic[index] > PRIMING_NUMBER) {
        startNextColumn = true;
      } else {
        startNextColumn = false;
      }
    } else {
      // zero value
      if (startNextColumn) {
        growingDynamic[index] = 1;
        startNextColumn = false;
      }
    }
  }

  // Iteration finished
  // Finish on 5th level as further levels growing are not visible
  // and may slowdown animation start
  int limit = std::min(cInitGrowTillLevel, cFrac::NrOfOrders);
  if (growingDynamic[limit] == LAST_NUMBER) {
    fluctuateState.growingActive = false;
    growingDynamic.fill(LAST_NUMBER); // 100%
    // Revert original Algo data
    // (actually not needed but verifies that growing algo did not go wild)
    algo_data_fluctuate = conv_to_fluctuate(algo_data);
    Dbg::report_info("Initial Growing finished. ", 1); 
  }

  // Testing growing iteration algo

  // static int counter {0};
  // ++counter;
  // if (fluctuateState.growingActive) {
  //   std::cout << " Growing (" << counter << "): ";
  //   for (size_t index {0}; index<=cFrac::NrOfOrders; ++index) {
  //     std::cout << growingDynamic[index] << ", ";
  //   }
  //   std::cout << '\n';
  // }

  // Do actual modification

  // 0th order is primary element, then following orders 1..NrOfOrders - thus '<='
  for (size_t level {0}; level <= cFrac::NrOfOrders; ++level) {
    for (size_t elem {0}; elem < cFrac::NrOfElements; ++elem) {
      // Only scale component modified
      if (level == 0) {
        // primary element has no initial transformation so assume scale 1.0
        algo_data_fluctuate[level][elem].scale = static_cast<float>(growingDynamic[level]) / LAST_NUMBER;
      } else {
        algo_data_fluctuate[level][elem].scale = 
          algo_data[elem].scale * (static_cast<float>(growingDynamic[level]) / LAST_NUMBER);
      }
      algo_data_fluctuate[level][elem].angle = algo_data[elem].angle;
      algo_data_fluctuate[level][elem].angle_down = algo_data[elem].angle_down;
      algo_data_fluctuate[level][elem].repos = algo_data[elem].repos;
    }
  }
  
}


