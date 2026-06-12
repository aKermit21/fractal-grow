// Copyright (c) 2026 Robert Gajewski (pcc21.com)
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include "dbg_report.h"
#include "basics.h"
#include "assert.h"

// Transformation Algorithm Configuration data
// used to tranform parent to child
// and setup primary element and
namespace cTran {
  // Window size centre in accuracy units
  inline constexpr float cXcenterM = cFrac::WindowXsize / 2.0;
  inline constexpr float cYcenterM = cFrac::WindowYsize / 2.0;
}

struct TranAlg {

  TranAlg(int speed=8) 
    : m_speedScale { speed }
    , m_indexPre { 0 }
  {
    Dbg::report_info("Init: TranAlg (speed=) ", speed);
    Dbg::report_info("Init: TranAlg (algosize=) ", preCalcAlgoData.size());

    assert(preCalcAlgoData.size() > 0);
    algo_data = conv_to_assym(preCalcAlgoData[0]);
    algo_data_init = algo_data;

    // Initial Speed vs Detailed drawing scale table
    speedScaleVerify(); // input: m_speedScale
    s_SmallVectDefault = SpeedScalaData[m_speedScale];
  }

  // Transformation full algorithm data used by derived animation class(es)
  // Working copy
  T_Algo_Arr algo_data;
  // Initial Data
  T_Algo_Arr algo_data_init;


  // Small vector - below this size stop recursive search/draw of children
  static float s_SmallVect;
  // Default of the above
  static float s_SmallVectDefault;

  // rotate from one of the pre-calculated configuration
  // rather obsolete as only one hardcoded configuration is used
  void rotate_pre_cfg();

  // setting smallest vector size to be drawn
  // controls speed vs accurracy drawing
  void speedIncrement(void);
  void speedDecrement(void);

  int get_speedScale() const;

private:

  int m_speedScale;

  // Speed vs Detailed drawing scale
  // speed to small vector size calculation
  void speedScaleVerify(void);

  // Speed scale data - small vector sizes threshold (to be drawn)
  constexpr static int SpeedScaleDataSize = 21;
  constexpr static std::array<float, SpeedScaleDataSize> SpeedScalaData = {
    1.0,  // 0. - Single actual point
    1.1, 
    1.2,
    1.4,
    1.6,
    1.8, // 5
    2,
    2.2,
    2.5, // 8. Default 
    2.7,
    3, // 10. - 3 visible points
    3.3,
    4.1, 
    4.5,
    5,
    5.6,
    6.4,
    7,
    8,
    9,
    10 // 20.  - 10 points
  };

  // Convert Symmetrical algo to working copy of (potentialy) Asymmetrical
  // and with diffrent granularity
  T_Algo_Arr conv_to_assym(T_Algo_Arr_Symm symm_algo);
  
  // obsolete as there is only single hardcoded configuration
  int m_indexPre;
  
  // Collection of Proportional cgf data pre-calculated 
  // Transformation data Symmetrical (angle down branch = -angle up branch)
  //  { repos (fraction of previous), angle (0.1deg), scale (fraction) }
  constexpr static std::array<T_Algo_Arr_Symm, 1U> preCalcAlgoData {{
    // Hardcoded basic leaf if toml config file not found (or corrupted);
    // see config/fractal-grow-cfg.toml "Rainbow" config '[config.transform]';
    // see also colors.h config preCalcColorPaletes array
    {{{0.222f, 750, 0.38f},
    {0.445f, 686, 0.3225f},
    {0.634f, 625, 0.265f},
    {0.790f, 562, 0.2075f},
    {0.912f, 500, 0.15f}}}, 
  
  // The Array can be extended or elements replaced taking data from config toml
  }};
  
};
