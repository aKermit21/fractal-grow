// Copyright (c) 2026 Robert Gajewski
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include <SFML/Graphics.hpp>
#include <string_view>
#include "dbg_report.h"
#include "match_col.h"

// Drawing Texts on top of Picture
struct TextDraw {
  TextDraw()
    : font_subdir_state { sDNotChecked }
    , m_font_loaded { false }
    , m_size_cm { 0.0 }
  {
    Dbg::report_info("Init: TextDraw ");
    m_font_loaded = init_font();
  }

  virtual ~TextDraw() { 
    // close font file ?
  }

  struct Vec {
    float dx;
    float dy;
  };
    
  struct DevData {
    Vec primVec;
    int coreLevels;
    long ElPtrsCnt;
    MatchingColors matchColors;
    int excitedCnt;  // excited caught number
    bool excitedAct; // excite mutation is active
    float scale;   // scaling in fractions
    float size_cm; // original size in cm
    long frames;
    std::chrono::duration<double> time; // in seconds
  };

  // Real draw
  void help_draw(sf::RenderWindow & win) const; 
  void pauseDraw(sf::RenderWindow & win) const; 
  void welcome_draw(sf::RenderWindow & win, int speed) const; 
  void speed_draw(sf::RenderWindow & win, int speed) const; 
  void snapshot_draw(sf::RenderWindow & win, std::string & info) const; 
  void rescale_draw(sf::RenderWindow & win, float scale, bool active); 
  void endOfGame_draw(sf::RenderWindow & win, DevData & data, bool demo) const; 
  void pictureInfo_draw(sf::RenderWindow & win, const std::string & text) const; 
  float getSizeCm(void) const;
  
  // Present developer info
  void developer_draw(sf::RenderWindow & win, const DevData & data) const; 

  // Helper
  // Replace (possible) $HOME alias with explicit path
  static bool replace_home_alias(std::string & path);

  const static std::string home_alias;     // alias
  const static std::string home_dir;       // real directory
  
private:
  // font subdirectory
  enum SubDirState {sDNotChecked, sDExists, sDNotExists};
  SubDirState font_subdir_state;
  bool m_font_loaded;
  sf::Font m_font;

  float m_size_cm;
  
  bool init_font(void);
};
