// Copyright (c) 2026 Robert Gajewski
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include "dbg_report.h"
#include "fractal.h"
#include "assert.h"
#include <SFML/Graphics/Color.hpp>
#include <array>

// Color definition in sf::Color:
// Color(Uint8 red, Uint8 green, Uint8 blue, Uint8 alpha = 255);

struct StemColor {
  sf::Color begin_c;
  sf::Color end_c;
};

// Colors for 0-NrOfrders+1 stems
// 0th order is primary element, following are counted:
//  1..NrOfOrders; thus '+1'
// If NrOfOrders+1 > NrOfColorsPaletes the later set is reuse in circular maner
// see getCircularColors();
using T_Col_Palet = std::array<StemColor, cFrac::NrOfColorPaletes>;

struct ColorPal {

  // Ordinary color set vs Flash color set (modifed colors flashed by light beam)
  enum ColorType { normalColors, flashColors };

  // maximum range (Uint8) of Color component used in sf::Color
  constexpr static int cRGB_ColorMax {255};

  ColorPal() 
    : m_indexPre { 0 }
  {
    Dbg::report_info("Init: ColorPal  ", preCalcColorPaletes.size());

    assert(preCalcColorPaletes.size() > 0);
    // default palette first from collection
    s_col_palet = preCalcColorPaletes[0];
    
    // Temporary initialize flash palette;
    // shall be initialize with right color by
    // derived LogKey class calling function: calc_flash_color_pallet()
    s_flash_col_palet = s_col_palet;
  }

  virtual ~ColorPal(){}

  // this is used by draw_stem() procedure (semi-global data)
  static T_Col_Palet s_col_palet;
  // used in flash effect
  static T_Col_Palet s_flash_col_palet;
  
  // switching global flash effect - used by draw_stem()
  static bool s_global_flash;
  static bool s_reset_flash_algo;
  
  // Helper function to obtain colors for any level event level exceeds Color Sets
  static StemColor getCircularColors(const ColorType type, const int level);

// called once in a display loop
// to switch off possible global control flags
// after signle frame (cycle)
  void one_step_flash_reset() {
    s_global_flash = false;
    s_reset_flash_algo = false;
  }
  
  // Restore unmodified pallet
  void reset_cur_color_pallet() { 
    s_col_palet = preCalcColorPaletes[0];
  
    // Start flash algo from beginning (AngleUnknown)
    s_reset_flash_algo = true;
  }
  
  // Reset only flash_algo
  void reset_flash_algo() {
      s_reset_flash_algo = true;
  }
  
  // key change pallette
  bool key_decodation(sf::Keyboard::Key key);
  
  // Calculate modified pallete used for flash effect
  void calc_flash_color_pallet(sf::Color active_light_col);

private:
  // Init collection of pre-calculated Color Palletes
  void initPreCalcColorPaletes();

  // diffrent random colors algo's
  T_Col_Palet just_random_colors();
  T_Col_Palet special_random_colors_v1();

  // Dimm or Reinforce colors
  void calc_updown_color_pallet(bool up);
  
  int m_indexPre;

  // Default hardcoded Colors Palette(s)
  // used before toml config file is loaded with F3
  // (or if config file is missing)
  const std::array<T_Col_Palet, 1U> preCalcColorPaletes = {{

  // 'Rainbow' style - see config/fractal-grow-cfg.toml '[config.colors]' section
   {{{ sf::Color( 163, 0, 0, 255), sf::Color( 128, 128, 0, 255)}, // Red -> Yellow
   { sf::Color( 128, 128, 0, 255), sf::Color( 0, 163, 0, 255)},  // Yellow -> Green
   { sf::Color( 0, 163, 0, 255), sf::Color( 0, 0, 180, 255)}, // Green -> Blue
   { sf::Color( 0, 0, 180, 255), sf::Color( 150, 0, 128, 255)}, // Blue -> Magenta
   { sf::Color( 163, 0, 0, 255), sf::Color( 128, 128, 0, 255)}, // Red -> Yellow
   { sf::Color( 128, 128, 0, 255), sf::Color( 0, 163, 0, 255)},  // Yellow -> Green
   { sf::Color( 0, 0, 180, 255), sf::Color( 150, 0, 128, 255)}, // Blue -> Magenta
   { sf::Color( 163, 0, 0, 255), sf::Color( 128, 128, 0, 255)}}} // Red -> Yellow

  // The Array can be extended or elements replaced taking data from config toml
 }};

};
