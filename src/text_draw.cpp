// Copyright (c) 2026 Robert Gajewski
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "text_draw.h"
#include "dbg_report.h"
#include "basics.h"
#include "config.h"
#include "progress.h"
#include "mut_grow.h"
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <string>

const std::string TextDraw::home_alias { "$HOME" };     // alias
const std::string TextDraw::home_dir = getenv("HOME");  // real directory

bool TextDraw::init_font() {
  std::string dirpath_str;
  std::string filepath_str;
  std::string file_str { cPath::cFont_file_str };
  
  const size_t cSubdirsNum { cPath::cFontSubdirs.size() };
  assert( cSubdirsNum > 0);
  size_t dir_index = 0;

  do {
    dirpath_str = cPath::cFontSubdirs[dir_index];
    (void)replace_home_alias(dirpath_str);
    if (dirpath_str.find(home_alias) != std::string::npos) {
      dirpath_str.replace(dirpath_str.find(home_alias),home_alias.length(), home_dir);
    }
    filepath_str = dirpath_str + file_str;
    std::filesystem::path font_file{ filepath_str };
    // Check if font file exists in given directory
    if (exists(font_file)) {
      font_subdir_state = sDExists;
      Dbg::report_info("Font file found: " + filepath_str);
    } else {
      font_subdir_state = sDNotExists;
      Dbg::report_info("Font file Not found: " + filepath_str);
      // if (sub)directory not found, try to load from current directory
      filepath_str = cPath::cFont_file_str;
    }
    dir_index++;
  } while ((sDNotExists == font_subdir_state) and (dir_index < cSubdirsNum));
  
  assert(font_subdir_state != sDNotChecked);
  assert(dir_index <= cSubdirsNum);
  
  // Load fonts from a file
  if (!m_font.openFromFile(filepath_str)) {
    Dbg::report_warning("Font file " + filepath_str + " cannot be find/open");
    return false;
  } 
  return true; // fonts loaded
}


// Replace (possible) $HOME alias with explicit path
bool TextDraw::replace_home_alias(std::string & path) {
    if (path.find(home_alias) != std::string::npos) {
      Dbg::report_trace("path replaced from: " + path, 0);
      path.replace(path.find(home_alias),home_alias.length(), home_dir);
      Dbg::report_trace("               to: " + path, 0);
      return true; // replacement occured
    }
    return false;
}
  
void TextDraw::help_draw(sf::RenderWindow &win) const {
  const static std::string help_text { "<F1> - Help \n\n"
    " Arrows - to move/change light:\n"
    " Up or W (keep press) - Up light source move\n"
    " Down or S (keep press) - Down light move \n"
    " Left or A - Light color rotation\n"
    " Right or D - rotation reversed\n\n"
    " <F3> - Load next configuration from config file\n\n"
    " L - Light on/off toggle\n"
    " G - Switch Grid rays visualisation mode\n\n"
    " <Space> - Pause / Freeze Mutations\n"
    " <Enter> - Resume Mutations\n\n"
    " X - EXit\n"
    " R - Reset\n\n"
    " PageUp - Speed Up (less details)\n"
    " PageDown - Speed Down (more details)\n\n"
    " J - Dimm all colors (dJimm)\n"
    " U - Up all colors\n"
    " H - Global flash of active color\n"
    " ~ - Wind (wobbling) effect off/on" };
  
  if (m_font_loaded) {
    sf::Text text(m_font, help_text, 24);
    text.setStyle(sf::Text::Regular);
    text.setFillColor(sf::Color::White);
    sf::Vector2f myPostion(20, 45);
    text.setPosition(myPostion);
    // Draw it
    win.draw(text);
  }
}

void TextDraw::welcome_draw(sf::RenderWindow &win, int speed) const {
  if (m_font_loaded) {
    std::stringstream text_ss;;
    text_ss << "F1 for help\n";
    text_ss << "Speed scale - " << speed << '\n';
    sf::Text text(m_font, text_ss.str(), 30);
    text.setStyle(sf::Text::Regular);
    text.setFillColor(sf::Color::White);
    sf::Vector2f myPostion(30, 30);
    text.setPosition(myPostion);
    // Draw it
    win.draw(text);
  }
}

void TextDraw::speed_draw(sf::RenderWindow &win, int speed) const {
  if (m_font_loaded) {
    std::stringstream text_ss;;
    text_ss << "Speed scale - " << speed;
    sf::Text text(m_font, text_ss.str(), 30);
    text.setStyle(sf::Text::Regular);
    text.setFillColor(sf::Color::White);
    sf::Vector2f myPostion(30, 40);
    text.setPosition(myPostion);
    // Draw it
    win.draw(text);
  }
}


void TextDraw::snapshot_draw(sf::RenderWindow &win, std::string & info) const {
  if (m_font_loaded) {
    sf::Text text(m_font, info, 20);
    text.setStyle(sf::Text::Regular);
    text.setFillColor(sf::Color::Yellow);
    sf::Vector2f myPostion(20, 20);
    text.setPosition(myPostion);
    // Draw it
    win.draw(text);
  }
}


void TextDraw::rescale_draw(sf::RenderWindow & win, float scale, bool active) {
  constexpr static int cFontSize { 20 };
  if (m_font_loaded) {
    // Convert Scaling factor to Size (in cm)
    float size = cFrac::InitialSize / scale;
    // Remember original value for Development draw and finding game phase
    m_size_cm = size;
    std::stringstream text_ss;
    auto sizeRec = sizeCheckAndFormat(size);
    text_ss << "approx. Size: " << sizeRec.print;
    if (active) {
      text_ss << "    Auto-Rescale active";
    }
    sf::Text text(m_font, text_ss.str(), cFontSize);
    text.setStyle(sf::Text::Regular);
    text.setFillColor(sf::Color::Yellow);
    // Put text at the bottom of window
    sf::Vector2f myPostion(10, cFrac::WindowYsize - cFontSize *2);
    text.setPosition(myPostion);
    // Draw it
    win.draw(text);
  }
}


void TextDraw::developer_draw(sf::RenderWindow & win, const DevData & data) const {
  constexpr static int cFontSize { 18 };

  // Enables 1'000'123 format
  struct apostrophe_separator : std::numpunct<char> {
  protected:
      char do_thousands_sep() const override { return '\''; }
      std::string do_grouping() const override { return "\3"; }
  };

  if (m_font_loaded) {
    std::stringstream text_ss;
    text_ss << "Primary el. Size:\n";
    text_ss.imbue(std::locale(std::locale(), new apostrophe_separator));
    text_ss << "dx = " << data.primVec.dx << '\n';
    text_ss << "dy = " << data.primVec.dy << '\n';
    text_ss << "Mut Level: " << data.coreLevels << '\n';
    text_ss << "Ptrs: " << data.ElPtrsCnt << '\n';
    auto color = MutGrow::getBestMatchingColor();
    text_ss << "Match Col: " << colorMatchToString(color) << '\n';
    // text_ss << "Begin " << colorMatchToString(color.begin) << '\n';
    // text_ss << "End   " << colorMatchToString(color.end) << '\n';
    text_ss << "Tot Excited: " << data.excitedCnt << '\n';
    text_ss << "now Active: " << data.excitedAct << '\n';
    text_ss << "Frame (cm): " << data.size_cm << '\n';
    text_ss << "Scale frac.: " << data.scale << '\n';
    text_ss << "Time (s): " << std::fixed << std::setprecision(1)
      << data.time.count() << '\n';
    text_ss << "Frames: " << data.frames;
    sf::Text text(m_font, text_ss.str(), cFontSize);
    text.setStyle(sf::Text::Regular);
    text.setFillColor(sf::Color::Yellow);
    // Put text at the right top of window
    sf::Vector2f myPostion(cFrac::WindowXsize - cFontSize *11.0f, cFontSize/2.f);
    text.setPosition(myPostion);
    // Draw it
    win.draw(text);
  }
}

void TextDraw::endOfGame_draw(sf::RenderWindow & win, DevData & data,
                              bool demo) const {
  // Enables 1'000'123 format
  struct apostrophe_separator : std::numpunct<char> {
  protected:
      char do_thousands_sep() const override { return '\''; }
      std::string do_grouping() const override { return "\3"; }
  };

  std::stringstream text_ss;
  text_ss << "GALAXY Edge Reached!";
  if (demo) {
    text_ss << " in DEMO Mode";
  }

  int tMin = static_cast<int>(data.time.count()) / 60;
  int tSec = static_cast<int>(data.time.count()) % 60;
  std::stringstream text_ss2;
  text_ss2.imbue(std::locale(std::locale(), new apostrophe_separator));
  text_ss2 << "in " << tMin << " min " << tSec << " seconds\n";
  text_ss2 << "that is " << data.frames << " frames\n";
  text_ss2 << "with " << data.excitedCnt << " Excited mutations.";

  if (m_font_loaded) {
    sf::Text text(m_font, text_ss.str(), 30);
    text.setStyle(sf::Text::Regular);
    text.setFillColor(sf::Color::Yellow);
    text.setLineAlignment(sf::Text::LineAlignment::Center);
    sf::Vector2f myPostion(cFrac::WindowXsize / 2.0f, 50.0f);
    text.setPosition(myPostion);
    win.draw(text);

    text.setCharacterSize(25);
    text.setString(text_ss2.str());
    sf::Vector2f myPostion2(cFrac::WindowXsize / 2.0f, 100.0f);
    text.setPosition(myPostion2);
    win.draw(text);
  }

  // print also to terminal
  static bool printedOnce {false};
  if (!printedOnce) {
    printedOnce = true;
    std::cout << text_ss.str() << '\n';
    std::cout << text_ss2.str() << '\n';
  }
}


void TextDraw::pictureInfo_draw(sf::RenderWindow & win,
                                const std::string & info) const {
  constexpr static int cFontSize { 20 };

  sf::Text text(m_font, info, cFontSize);
  text.setStyle(sf::Text::Regular);
  text.setFillColor({100,100,100,255}); // Gray
  sf::Vector2f myPostion((2.f*cFrac::WindowXsize / 3.0f),
                         cFrac::WindowYsize - cFontSize *2);
  text.setPosition(myPostion);
  win.draw(text);
} 

  
float TextDraw::getSizeCm(void) const {
  return m_size_cm;
}
