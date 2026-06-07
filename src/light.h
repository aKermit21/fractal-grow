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
#include "basics.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>

// Light source causing flash effect on stems

struct LightS {

  LightS() 
  : m_lightMoving { noMove }
  , rays_mode { raysMoving }
  , m_demoMode { false }
  {
    Dbg::report_info("Init: Lights  ", Y_MID);
    // Init light structures
    reset_light();
  }

  virtual ~LightS() {}

  // effect lasting # of frames
  constexpr static int cFlashGlobalCntMax { 5 };
  //flash efect from 90deg light rays
  constexpr static int cFlashLightCntMax { 10 };

  // Color rotation
  enum MoveColor { rotateDownColor, rotateUpColor };

  // Result of Key decodation
  struct RetResult {
    bool colorChanged;
    bool lightMoved;
  };

  // Rainbow structure colors (including black in center)
  constexpr static int cCOLORS_NR = 7;

  constexpr static std::array<sf::Color, cCOLORS_NR>
    LColors_Init = {sf::Color::Black, 
    sf::Color::Yellow, sf::Color::Green, sf::Color::Cyan, sf::Color::Blue, 
    sf::Color::Magenta, sf::Color::Red};
    
  // command light color and position
  RetResult key_decodation(sf::Keyboard::Key key);

  // Light redraw per cycle
  void light_draw(sf::RenderWindow &win, bool tempDeactive=false);

  void reset_light();

  // Resume state at end of the cycle
  void one_step_light_resume(bool endOfGame);

  bool demoGenerator(long int demoCnt);

  // Active color, to be overwritten by initialization; used also outside the class
  static sf::Color s_lightColor;

  // Light vector
  static sf::Vector2i s_lightVec; // used also outside this class
  
  // Light thus (individual) flash effect is active
  static bool s_lightActive;

private:
  // Rays grid visualization mode
  enum RaysMode {raysOff, raysOn, raysMoving, raysEndOfModes };

  // Current lights Moving mode
  enum LightMove {noMove, upMove, downMove};
  
  // calculate middle of window
  constexpr static int X_MID { cFrac::WindowXsize / 2 }; 
  constexpr static int Y_MID { cFrac::WindowYsize / 2 };

  // Main light radius
  constexpr static int MAIN_SPOT_R { 25 };

  // Rainbow structure consts
  constexpr static float CIRCLE_R { 45.f };

  // Steps per frame to realize smooth move
  constexpr static int cMoveSmooth { 5 };
  constexpr static int cMoveSmoothDemo { 3 };
  
  // Light is in motion
  LightMove m_lightMoving;

  // Backup colors circle
  sf::VertexArray lrainbow;

  // Light rays (visualisation) grid
  sf::VertexArray lrays_grid {};
  
  // (Mode of) Presence of rays grid visualization
  RaysMode rays_mode;

  bool m_demoMode;

  // Attenuate color
  sf::Color dim_color(sf::Color color, int percent); 
  
  // Rotate rainbow (RGB)
  void rotate_rgb_color_updown(MoveColor move);
  
  // Move position of light
  void move_light_position_by(int move);
  
  // Based on Light position draw rays (multi-line)
  sf::VertexArray create_rays_grid(sf::Vector2f l_pos);

  // Create signle ray line consisting of sections
  void create_ray_line(sf::Vector2f init_pos, bool start_fill, 
                       sf::VertexArray &auxg);
   
  // Init pick-up light color circle
  sf::VertexArray init_rainbow();
};
