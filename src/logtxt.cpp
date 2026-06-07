// Copyright (c) 2026 Robert Gajewski
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "logtxt.h"
#include "cfg_toml.h"
#include "config.h"
#include "dbg_report.h"
#include "basics.h"
#include "text_draw.h"
#include "transform.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <filesystem>
#include <fstream>
#include <stdlib.h>
#include <string_view>
#include <time.h>


// Request for drawing Help per # of frames
void LogText::startHelpDraw(void) {
  help_draw_cnt = cHelpDrawFrames;
}

// Request for drawing Speed per # of frames
void LogText::startSpeedDraw(void) {
  speed_scale_draw_cnt = cPrintSpeedFrames; 
}

// Request for drawing Snapshot info
void LogText::startSnapshotDraw(void) {
  m_snapshot_info_active = true; 
}

void LogText::stopSnapshotDraw(void) {
  m_snapshot_info_active = false; 
}

void LogText::switchDevDraw(void) {
  if (m_developer_draw_active) {
    m_developer_draw_active = false;
  } else {
    m_developer_draw_active = true;
  }
}

// Draw Help if requested (counter per frame)
void LogText::help_draw(sf::RenderWindow & win) {
  if (help_draw_cnt > 0) {
    textDraw.help_draw(win);
    --help_draw_cnt;
  }
}

// Draw Speed if requested (counter per frame)
void LogText::speed_draw(sf::RenderWindow & win, int speed) {
  if (speed_scale_draw_cnt > 0) {
    --speed_scale_draw_cnt;
    textDraw.speed_draw(win, speed);
  }
}

// Draw loaded (by F3) snapshot (config) info
void LogText::snapshot_draw(sf::RenderWindow & win) {
  if (m_snapshot_info_active and !loaded_snapshot_info_str.empty()) {
    textDraw.snapshot_draw(win, loaded_snapshot_info_str);
  }
}

// Present Optional info
void LogText::developer_draw(sf::RenderWindow & win, TextDraw::DevData & devData) {
  if (m_developer_draw_active) {
    devData.scale = m_scale;
    devData.size_cm = textDraw.getSizeCm();
    textDraw.developer_draw(win, devData);
  }
}

// Welcome 
void LogText::welcome_draw(sf::RenderWindow & win, int speed) const {
  static int frames { 0 };

  // Present welcome text for double time as Help would be (after F1)
  if (frames < 3* cHelpDrawFrames) {
    // Do not print welcome together with help
    if (help_draw_cnt == 0) {
      textDraw.welcome_draw(win, speed);
    }
    ++frames;
  }
}

void LogText::rescale_draw(sf::RenderWindow & win, float scale, bool active) {
  m_scale = scale;
  textDraw.rescale_draw(win, scale, active);
} 

float LogText::getScale(void) const {
  return m_scale;
}

// Establish log file path
std::string LogText::search_file_path(void) {
  static std::string dirpath_str;
  static std::string filepath_str {};
  
  if (log_subdir_state != sDNotChecked) {
    assert( !filepath_str.empty() );
    return filepath_str;
    }
  
  const size_t cSubdirsNum { cPath::cLogSubdirs.size() };
  assert( cSubdirsNum > 0);
  size_t dir_index = 0;

  do {
    dirpath_str = cPath::cLogSubdirs[dir_index];
    (void)TextDraw::replace_home_alias(dirpath_str);
    std::filesystem::directory_entry log_path{dirpath_str};
    // Check if subdirectory exists
    if (log_path.exists()) {
      log_subdir_state = sDExists;
      filepath_str = dirpath_str + snapshot_file_str;
      Dbg::report_info("Snapshot file location found/set to: " + filepath_str);
    } else {
      log_subdir_state = sDNotExists;
      Dbg::report_info("Possible Log (sub)directory Not found: " + dirpath_str);
      // if (sub)directory not found, use current directory
      filepath_str = snapshot_file_str;
    }
    dir_index++;
  } while ((sDNotExists == log_subdir_state) and (dir_index < cSubdirsNum));
  
  assert(log_subdir_state != sDNotChecked);
  assert(dir_index <= cSubdirsNum);

  return filepath_str;
}

void LogText::load_next_snapshot(Element& prim_element, T_Algo_Arr & transform_algo,
                                 T_Col_Palet & colors) {
  // File shall be immediately closed after any operation
  assert(!file_opened);

  loaded_snapshot_info_str =
    cfgToml.loadNextConfig(search_file_path(), prim_element, transform_algo, colors);
}

  
void LogText::log_close() noexcept {
  if (file_opened) {
    fout << std::endl; // flash data
    fout.close();
    file_opened = false;
  }
}

