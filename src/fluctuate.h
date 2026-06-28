// Copyright (c) 2026 Robert Gajewski (pcc21.com)
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include "basics.h"
#include "pause_core.h"
#include "opt_lyra.h"

// Modification of Transformation Algorithm 
// by adding wiggling effect (like from wind) to angle component or
// by adding growing effect to scale component

struct MovFluctuate : PauseAni {

  MovFluctuate(OptParams opts) 
    : PauseAni { opts.optSpeed }
    , fluctuateState {true, false}
    , GrowingEnabled {false}
    , windVelocity {}
    , growingDynamic {0} // all zero's except first element - see next lines of code
  { 
    Dbg::report_info("Init: MovFluctuate (speed=)", opts.optSpeed); 
    // Growing configurable by CLI option
    if (!opts.optGrowingOff) {
      GrowingEnabled = true;
      fluctuateState.growingActive = true;
    }
    Dbg::report_info("Growing Activation: ", fluctuateState.growingActive); 
    // Initialize algo_data_fluctuate table for growing before first display
    // copy Algo per level - draw() uses this format
    algo_data_fluctuate = conv_to_fluctuate(algo_data);
    // Initialize algo_data_fluctuate table for growing before first display
    if (fluctuateState.growingActive) {
      refreshWithRestartGrowing();
    }
  }

  constexpr static int cInitGrowTillLevel {6};
  
  // Two possible fluctuation animations: shaking and (initial) growing
  // See FluctuateState defined in basics.h
  FluctuateState fluctuateState;
  
  // animation related keys handling
  bool key_decodation(sf::Keyboard::Key key);

  // called once in a display loop
  // to realize fluctuation animation
  void one_step_cfg_change();
  
  void stop_wind();
  void pauseWind();
  void resumeWind();

  // Transformation full LIVE algorithm data including windy, growing effect
  // More specific algo: per level
  T_Fluctuate_Algo_Arr algo_data_fluctuate;

  T_Fluctuate_Algo_Arr conv_to_fluctuate(T_Algo_Arr);

  // Enable restart Growing effect
  void refreshWithRestartGrowing(void);

private:

  // Assumed deviations in radians for wind algo
  constexpr static float cWindTolerance { 0.05 };
  // Step to radian velocity factor
  constexpr static float cWindFactor { 0.0001 };
  // Friction simulation factor
  constexpr static float cWindFriction { 0.995 };

  // General enable state (not necessary in given time)
  bool GrowingEnabled;
  
  // realize growing specific fluctuation change
  void oneStepGrowingChange();
  
  // Wind (shaky)
  void oneStepWindChange();

  // temporary angular Velocity 
  using T_UpDown = struct {float up; float down;};
  std::array<std::array<T_UpDown, cFrac::NrOfElements>, cFrac::NrOfOrders+1>
      windVelocity;

  // Specific growing dynamic (x0.1)
  std::array<int, cFrac::NrOfOrders+1> growingDynamic;
};
