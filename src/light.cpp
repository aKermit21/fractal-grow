// Copyright (c) 2026 Robert Gajewski
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "light.h"
#include "basics.h"
#include "demo_func.h"
#include "cstdlib"
#include "match_col.h"
#include "mut_grow.h"
#include <SFML/Config.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstdint>
#include <cassert>
#include <cmath>

// Static variables placement

// Active color, to be overwritten by initialization; used also outside the class
sf::Color LightS::s_lightColor {sf::Color::White};

// Light vector
sf::Vector2i LightS::s_lightVec; // used also outside this class

// Light thus (individual) flash effect is active
bool LightS::s_lightActive { true };

// command light color and fast position move
LightS::RetResult LightS::key_decodation(sf::Keyboard::Key key){
  RetResult retResult { false, false };
  
  if (key == sf::Keyboard::Key::L) {
    // Light on/off toggle
    if (s_lightActive) {
      s_lightActive = false;
    } else {
      s_lightActive = true;
    }
  }
  else if ((key == sf::Keyboard::Key::A) or
           (key == sf::Keyboard::Key::Left)) {
    rotate_rgb_color_updown(rotateUpColor);
    retResult.colorChanged = true;
  }
  else if ((key == sf::Keyboard::Key::D) or
           (key == sf::Keyboard::Key::Right)) {
    rotate_rgb_color_updown(rotateDownColor);
    retResult.colorChanged = true;
  } 
  // Optional light jump options
  // else  if (key == sf::Keyboard::Key::Q) {
  //   move_light_position_by(-25);   // move fast to left
  //   retResult.lightMoved = true;
  // }
  // else if (key == sf::Keyboard::Key::E) {
  //   move_light_position_by(25);   // move fast to right
  //   retResult.lightMoved = true;
  // }
  else if ((key == sf::Keyboard::Key::S) or (key == sf::Keyboard::Key::W) or
           ((key == sf::Keyboard::Key::Up) or (key == sf::Keyboard::Key::Down))) {
    // Light smooth moving realized by light_draw()
    retResult.lightMoved = true;
  }
  else if (key == sf::Keyboard::Key::G) {
    // cycle through modes of rays grid visualization
    auto temp_int = static_cast<int>(rays_mode);
    ++temp_int;
    if (temp_int >= raysEndOfModes) {
      rays_mode = static_cast<RaysMode>(0);
    } else {
      rays_mode = static_cast<RaysMode>(temp_int);
    }
  } else {
    // Not an error: other keys can be handled by another key_decodation 
  }

  return retResult;
}


// Light redraw per cycle
void LightS::light_draw(sf::RenderWindow &win, bool tempDeactive){
  
  if (tempDeactive) {
    m_lightMoving = noMove;
  } else {
    // Move realization
    // Reposition smoothly light while key is being pressed or if demo
    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) or 
        (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))) {
      move_light_position_by(cMoveSmooth);   // Down
      m_lightMoving = downMove;
    } else if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) or
              (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))) {
      move_light_position_by(-cMoveSmooth);  // Up
      m_lightMoving = upMove;
    } else {
      if (m_demoMode) {
        // In demo mode light modes are changed auto by demoGenerator
        if (downMove == m_lightMoving) {
          move_light_position_by(cMoveSmoothDemo);
        } else if (upMove == m_lightMoving) {
          move_light_position_by(-cMoveSmoothDemo);
        }
      }
    }
  }
  
  // Draw rainbow of all possible colors
  win.draw(lrainbow);
  
  // Take position of light from vector (light has opposite y than vector y)
  float x_pos = (X_MID*2) - (CIRCLE_R *2) - (MAIN_SPOT_R *2);
  float y_pos = static_cast<float>(Y_MID - s_lightVec.y - MAIN_SPOT_R);
  sf::Vector2f position{x_pos, y_pos};

  // Draw Main Light if active
  if (s_lightActive and !tempDeactive) {
    sf::CircleShape shape(MAIN_SPOT_R);
    shape.setFillColor(s_lightColor);
    shape.setPosition(position);
    win.draw(shape);
  }

  // Draw optionally light rays grid
  // if ((rays_on or m_lightMoving) and s_lightActive) { 
  if (s_lightActive and 
       ((raysOn == rays_mode) or 
        ((m_lightMoving != noMove) and (raysMoving == rays_mode)))) { 
    // Put postion at centre of light
    position.x += MAIN_SPOT_R; 
    position.y += MAIN_SPOT_R; 
    lrays_grid = create_rays_grid(position);
    win.draw(lrays_grid);
  }
}


// Move position of light
void LightS::move_light_position_by(int move) {

  // limit to vertical size of window
  if ((s_lightVec.y - move > -Y_MID) and (s_lightVec.y - move < Y_MID)) {

    // transform vector and move main light
    s_lightVec.y -= move;  // vector x move in opposite direction
  
    // move all points of auxiliary rainbow
    for ( size_t i{0}; i < cCOLORS_NR+1; ++i ) {  // first color repeated
      lrainbow[i].position.y += move;
    }
  }
}


// Rotate rainbow palette thus change active light
void LightS::rotate_rgb_color_updown(MoveColor updn){
  
  if (updn == rotateUpColor) {
    // move all colors of rainbow except first (which is black in center of palette)
    // simple rotate to the right
    for ( size_t i{1}; i < cCOLORS_NR; ++i ) { 
      lrainbow[i].color = lrainbow[i+1].color;
    }

    // last color repeated for palette continuum
    lrainbow[cCOLORS_NR].color = lrainbow[1].color;
  }
  else {
    // move all colors of rainbow except first (which is black in center of palette)
    for ( size_t i{cCOLORS_NR}; i > 1; --i ) { 
      lrainbow[i].color = lrainbow[i-1].color;
    }

    // last color repeated for continuum
    lrainbow[1].color = lrainbow[cCOLORS_NR].color;
  }
  
  // update active color
  s_lightColor = lrainbow[1].color;
}


// Resume state at end of the cycle
void LightS::one_step_light_resume(bool endOfGame) {
  // without keeping keypress next cycle will be without light move
  // This is to set noMove mode alfter releasing the key
  if ((!m_demoMode) or endOfGame) {
    // Additionally end of game enables (in demo mode) light control
    m_lightMoving = noMove;
  }
}


// Create signle ray line consisting of sections
void LightS::create_ray_line(sf::Vector2f current_line_pos, bool s_fill, 
                             sf::VertexArray &auxg) {
  
  bool even_el;

  sf::Vector2f lvecf = sf::Vector2f(s_lightVec);

  // position of light is beginning of line
  sf::Vertex line_element{};
  line_element.position = sf::Vector2f(current_line_pos);
  // use smaller chunks of original light vector
  lvecf /= 6.f;

  // Optionaly start from fill or brake
  if (s_fill) {
    line_element.position = sf::Vector2f(current_line_pos); // position of light is beginning of line
    line_element.color = dim_color(s_lightColor, 60);
    auxg.append(line_element);
    even_el = false;
  } else {
    even_el = true;
  }
  
  [[maybe_unused]] int assertCnt = 0;
  
  // Add grid sections from right to left
  do {
    ++assertCnt;  // discover infinite loop
    assert((assertCnt < 1000) and
           "Too much line elements. Possible Infinite loop.");
    current_line_pos += lvecf; // move line along the lightvector
    line_element.position = sf::Vector2f(current_line_pos); // position of light is beginning of line
    line_element.color = dim_color(s_lightColor, 60);
    auxg.append(line_element);
    // Switch even/odd element
    if (even_el) {
      even_el = false;
    } else {
      even_el = true;
    }
  } while (current_line_pos.x > 0); // line goes from right to left

  // Maske sure there are even # of elements (to complete last line)
  if (!even_el) {
    // add last element once again
    auxg.append(line_element);
  }
}


// Based on Light position draw rays
sf::VertexArray LightS::create_rays_grid(sf::Vector2f lpos) {
  
  sf::VertexArray auxg(sf::PrimitiveType::Lines);

  sf::Vector2f line_pos = lpos;
  bool start_fill = true;
  
  // Direct line from main light
  create_ray_line(lpos, true, auxg);

  [[maybe_unused]] int assertCnt = 0;
  // Lines/rays to the left
  start_fill = false;
  do {
    assertCnt++;
    line_pos.y -= 59;
    // lines start at irregular postion

    create_ray_line(line_pos, start_fill, auxg);

    // Switch even/odd line
    if (start_fill) {
      start_fill = false;
    } else {
      start_fill = true;
    }
    assert((assertCnt < 1000) and
           "Too much grid lines. Possible Infinite loop");
    
  // till top of window with extra margin
  } while (line_pos.y > -Y_MID); 
  
  // Restart Lines/rays position
  line_pos = lpos;
  start_fill = false;
  do {
    line_pos.y += 59;
    // lines start at irregular postion

    create_ray_line(line_pos, start_fill, auxg);

    // Switch even/odd line
    if (start_fill) {
      start_fill = false;
    } else {
      start_fill = true;
    }
    assert((assertCnt < 1000) and
           "Too much grid lines. Possible Infinite loop");
    
  // till bottom of window with extra margin
  } while (line_pos.y < Y_MID *3);

  return auxg;
}


// Attenuate color
sf::Color LightS::dim_color(sf::Color color, int percent) {
  sf::Color ret_color;

  ret_color.b = static_cast<std::uint8_t>((color.b * percent) / 100);
  ret_color.g = static_cast<std::uint8_t>((color.g * percent) / 100);
  ret_color.r = static_cast<std::uint8_t>((color.r * percent) / 100);
  
  return ret_color;
} 


// Init alternative collors circle
sf::VertexArray LightS::init_rainbow(){
  int angle;
  float angle_rad;
  sf::VertexArray auxl(sf::PrimitiveType::TriangleFan, cCOLORS_NR+1); // first color repeated

  // Take position of light from vector
  float y_pos = static_cast<float>(Y_MID - s_lightVec.y);
  
  // Central point - right side
  auxl[0].position = sf::Vector2f((X_MID*2) - CIRCLE_R - 5.f, y_pos);
  auxl[0].color = sf::Color::Black;

  // Fan of all colors
  angle = 0;
  for (size_t i{1}; i<cCOLORS_NR; ++i) {
    // Converting degress to radians
    angle_rad = angle * 3.14159 / 180;
    auxl[i].position = sf::Vector2f((X_MID*2) - CIRCLE_R - 5.f - (cos(angle_rad)* CIRCLE_R),
      y_pos + sin(angle_rad)*CIRCLE_R);
    auxl[i].color = LColors_Init[i];
    // angle for next step
    angle += 60;
    }

  // Last element with repeated first color to obtain continuum
  angle_rad = angle * 3.14159 / 180;
  auxl[cCOLORS_NR].position = sf::Vector2f((X_MID*2) - CIRCLE_R - 5.f
     - (cos(angle_rad)* CIRCLE_R), y_pos + sin(angle_rad)*CIRCLE_R); 
  auxl[cCOLORS_NR].color = LColors_Init[1];
  
  return auxl;
}

// Reset or init light structures
void LightS::reset_light() {
  // Ligth vector - horizontal as y=0 and x (from right to left)_adjusted to centre of window
  s_lightVec = { -X_MID + 2*static_cast<int>(CIRCLE_R) - MAIN_SPOT_R, 0};
  rays_mode = raysMoving;
  // create colors rainbow structure
  lrainbow = init_rainbow();
  // Active color synchronuous with color palette
  s_lightColor = lrainbow[1].color; 
  m_lightMoving = noMove;
}

bool LightS::demoGenerator(long int demoCnt) {

  static long int lastColorMoveDistance { 0 };

  if (demoCnt == cFrac::DemoInitCnt) {
    m_demoMode = true;
    // For demo use automatic grid display when ligth is moving
    rays_mode = raysMoving;
  }
  
  // Active event occured from this generator
  bool actionDone = false;
  ++lastColorMoveDistance;

  // Light move mode change every about 50 frames - more for Move than noMove
  if (noMove == m_lightMoving) {
    if (demoRand(70, lastColorMoveDistance)) {
      // Start moving light to opposite direction than occupied half
      actionDone = true;
      if (s_lightVec.y == 0) {
        if ((rand() % 2) == 0) {
          m_lightMoving = downMove;
        } else {
          m_lightMoving = upMove;
        }
      }
      else if (s_lightVec.y > 0) {
        m_lightMoving = downMove;
      } else {
        m_lightMoving = upMove;
      }
    } // NoMove
  } else {
    if ((rand() % 70) == 0) {
      m_lightMoving = noMove;
    }
  }
  
  // rotate color till Matching Color of Mutation
  static int rotationCnt {0};
  // Yellow is default color
  static sf::Color prevTargetColor {sf::Color::Yellow};
  
  // if Growing of last node is Not Active go to Yellow Color as it most bright
  sf::Color targetColor = sf::Color::Yellow;
  if (MutGrow::isTopGrowingState()) {
    targetColor = getColorFromMatch(MutGrow::getBestMatchingColor());
  }

  if ((targetColor != prevTargetColor) or
      (rotationCnt > 0)) {
    prevTargetColor = targetColor;
    
    if ((LightS::s_lightColor == targetColor)
         or (rotationCnt > LightS::cCOLORS_NR)) {
      // Matching color found - stay here
      rotationCnt = 0;
    } else {
      // rotate till match color or maximum changes
      // mark rotation in progress
      ++rotationCnt;
      // Move to next color one step per frame
      rotate_rgb_color_updown(rotateDownColor);
      // To Refresh flash color pallete
      actionDone = true;
    }
  }

  return actionDone;
}
