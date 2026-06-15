// Copyright (c) 2026 Robert Gajewski (pcc21.com)
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include "colors.h"
#include "basics.h"
#include "opt_lyra.h"
#include "screen_size.h"
#include "text_draw.h"
#include "cfg_toml.h"
#include <string>
#include <fstream>
#include <string_view>

// Logfile and Text Printing
// including help printing with F1
struct LogText {
  explicit LogText(OptParams opts, const ScreenM & screen) 
    : textDraw(screen)
    , file_opened { false } 
    , cHelpDrawFrames {110}
    , cPrintSpeedFrames {60}
    , snapshot_file_str { opts.optSnapshot }    
    , loaded_snapshot_info_str {}
    , help_draw_cnt { 0 }
    , speed_scale_draw_cnt { 0 }
    , m_snapshot_info_active { false }
    , m_developer_draw_active { false }
    , mSizing { screen }
    , m_scale { 1.0f }
    , log_subdir_state { sDNotChecked }
  {
    Dbg::report_info("Init: LogText (speed=) ", opts.optSpeed);
  } 

  virtual ~LogText() { log_close(); }

  // log/snapshot subdirectory
  enum SubDirState {sDNotChecked, sDExists, sDNotExists};

  // Font and text display related functions
  TextDraw textDraw;

  // Toml stored snapshots configurations
  CfgToml cfgToml;

  // retrieve (next) snapshot from file
  void load_next_snapshot(Element& prim_element, T_Algo_Arr & transf_arr,
                          T_Col_Palet & col_palet);
  
  // Set enabling counter
  void startHelpDraw(void);
  void startSpeedDraw(void);
  // after loading snapshot with F3
  void startSnapshotDraw(void);
  void stopSnapshotDraw(void);
  
  // Optional Info
  void switchDevDraw(void);

  // Conditional draws
  void help_draw(sf::RenderWindow & win); 
  void pauseDraw(sf::RenderWindow & win) const; 
  void speed_draw(sf::RenderWindow & win, int speed);
  void snapshot_draw(sf::RenderWindow & win); 
  void developer_draw(sf::RenderWindow & win, TextDraw::DevData & data) const ; 
  
  // Dispatch draw
  void welcome_draw(sf::RenderWindow & win, int speed) const; 
  void rescale_draw(sf::RenderWindow & win, float scale, bool active); 

  float getScale(void) const;
  
private:
  std::string search_file_path(void);

  bool file_opened;

  const int cHelpDrawFrames;
  const int cPrintSpeedFrames;
  const std::string snapshot_file_str;

  std::string loaded_snapshot_info_str;

  void log_close(void) noexcept;

  int help_draw_cnt;
  // To draw for several frames Speed after its change
  int speed_scale_draw_cnt;
  bool m_snapshot_info_active;
  bool m_developer_draw_active;
  const ScreenM & mSizing;
  
  // Original values use in rescaling
  float m_scale;
  
  SubDirState log_subdir_state;
  std::ofstream fout;
};
