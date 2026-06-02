// Copyright (c) 2026 Robert Gajewski
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "fractal.h"
#include "dbg_report.h"
#include "light.h"
#include "colors.h"
#include "mut_grow.h"
#include "assert.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>

void Element::draw_stem(sf::RenderWindow &win, long order, const bool displayCore) {

  // 90 deg transition vec / light vec
  if (stem_xy.light_vec_angle_flip()) {
    // show local (this stem) flash
    stem_xy.flash_cnt = LightS::cFlashLightCntMax;
  }
  
  // Further comtrol of flash effect 
  if (ColorPal::s_global_flash) {
    // Flash with all stems
    stem_xy.flash_cnt = LightS::cFlashGlobalCntMax;
  } else if (ColorPal::s_reset_flash_algo) {
    stem_xy.prev_l_angle = lAngleUnknown;
    stem_xy.flash_cnt = 0;
  // Time freeze option (stop type)
  } else if (stem_xy.flash_cnt > 0) {
    // keep flash effect for some time
    --stem_xy.flash_cnt;
  } else { }
  
  assert((order >= 0) and (order <= cFrac::NrOfOrders +1));
  if (stem_xy.x1==0 or stem_xy.x2==0 or stem_xy.y1==0 or stem_xy.y2==0) {
    Dbg::report_warning(" Suspected (0) stem data coordinate(s),"
                        " possible not initialized ", stem_xy.x1);
  }

  // Check if there is some width of stem to be drawn
  if ((std::abs(stem_xy.x1-stem_xy.x2) > 2.0)
      or std::abs(stem_xy.y1-stem_xy.y2) > 2.0) { 
      float fvdx = (stem_xy.vec_xy.x + stem_xy.vec_xy.dx);
      float fvdy = (stem_xy.vec_xy.y + stem_xy.vec_xy.dy);
      
    // Flash active or Excited (by light) active mutation
    if ((stem_xy.flash_cnt > 0 and LightS::s_lightActive) or
      ((mutationGrowPtr != nullptr) and
            mutationGrowPtr->mColorExcite))  {
      // Draw Flash version

      // Filled triangles
      sf::VertexArray triangle(sf::PrimitiveType::Triangles, 3);
      triangle[0].position = sf::Vector2f(stem_xy.x1,stem_xy.y1);
      triangle[1].position = sf::Vector2f(fvdx, fvdy);
      triangle[2].position = sf::Vector2f(stem_xy.x2,stem_xy.y2);
      
      if ((mutationGrowPtr != nullptr) and
            mutationGrowPtr->mColorExcite)  {
        // Excited version - white
        triangle[0].color = sf::Color::White;
        triangle[1].color = sf::Color::White;
        triangle[2].color = sf::Color::White;
        // Wider the triangle for Excited
        auto widerX1 = stem_xy.x1;
        auto widerX2 = stem_xy.x2;
        if (widerX1 >= widerX2) {
          widerX1 += 1.5f; widerX2 -= 1.5f;
        } else {
          widerX1 -= 1.5f; widerX2 += 1.5f;
        }
        triangle[0].position = sf::Vector2f(widerX1,stem_xy.y1);
        triangle[2].position = sf::Vector2f(widerX2,stem_xy.y2);
      } else {
        // Flash colors
        triangle[0].color = ColorPal::
          getCircularColors(ColorPal::flashColors, order).begin_c;
        triangle[2].color = ColorPal::
          getCircularColors(ColorPal::flashColors, order).begin_c;
        triangle[1].color = ColorPal::
          getCircularColors(ColorPal::flashColors, order).end_c;
      }
      win.draw(triangle);

    } else {
      // Draw ordinary version

      // Empty triangles
      sf::VertexArray two_lines(sf::PrimitiveType::Lines, 4);
      two_lines[0].position = sf::Vector2f(stem_xy.x1,stem_xy.y1);
      two_lines[1].position = sf::Vector2f(fvdx, fvdy);
      two_lines[2].position = sf::Vector2f(stem_xy.x2,stem_xy.y2);
      two_lines[3].position = sf::Vector2f(fvdx, fvdy);
      if (coreElement and displayCore) {
        two_lines[0].color = sf::Color( 250, 250, 250, 255);
        two_lines[1].color = sf::Color( 250, 250, 250, 255);
        two_lines[2].color = sf::Color( 250, 250, 250, 255);
        two_lines[3].color = sf::Color( 250, 250, 250, 255);
      } else {
        // Regular colors
        two_lines[0].color = ColorPal::
          getCircularColors(ColorPal::normalColors, order).begin_c;
        two_lines[2].color = ColorPal::
          getCircularColors(ColorPal::normalColors, order).begin_c;
        two_lines[1].color = ColorPal::
          getCircularColors(ColorPal::normalColors, order).end_c;
        two_lines[3].color = ColorPal::
          getCircularColors(ColorPal::normalColors, order).end_c;
      }
      win.draw(two_lines);
    }
    
  } else { // no triangle stem, too narrow:
           //simple or multiple (if flash) line(s)
    float fvx = stem_xy.vec_xy.x;
    float fvy = stem_xy.vec_xy.y;
    float fvdx = (stem_xy.vec_xy.x + stem_xy.vec_xy.dx);
    float fvdy = (stem_xy.vec_xy.y + stem_xy.vec_xy.dy);

    // Flash active or Excited (by light) active mutation
    if ((stem_xy.flash_cnt > 0 and LightS::s_lightActive) or
      ((mutationGrowPtr != nullptr) and
            mutationGrowPtr->mColorExcite))  {
      // Common Flash / Excited settings.
      // Double/Triple line thickness
      sf::VertexArray tri_line(sf::PrimitiveType::Lines, 6);
      tri_line[0].position = sf::Vector2f(fvx,fvy);
      tri_line[1].position = sf::Vector2f(fvdx, fvdy);
      tri_line[2].position = sf::Vector2f(fvx +1,fvy);
      tri_line[3].position = sf::Vector2f(fvdx +1, fvdy);

      if ((mutationGrowPtr != nullptr) and
            mutationGrowPtr->mColorExcite)  {
        // Excited version
        tri_line[4].position = sf::Vector2f(fvx -1,fvy);
        tri_line[5].position = sf::Vector2f(fvdx -1, fvdy);
        tri_line[0].color = sf::Color::White;
        tri_line[1].color = sf::Color::White;
        tri_line[2].color = sf::Color::White;
        tri_line[3].color = sf::Color::White;
        tri_line[4].color = sf::Color::White;
        tri_line[5].color = sf::Color::White;
      } else {
        // Draw Flash version
        tri_line[4].position = sf::Vector2f(fvx,fvy +1);
        tri_line[5].position = sf::Vector2f(fvdx, fvdy +1);
        // Flash version - Flash colors
        tri_line[0].color = ColorPal::
            getCircularColors(ColorPal::flashColors, order).begin_c;
        tri_line[1].color = ColorPal::
            getCircularColors(ColorPal::flashColors, order).end_c;
        tri_line[2].color = ColorPal::
            getCircularColors(ColorPal::flashColors, order).begin_c;
        tri_line[3].color = ColorPal::
            getCircularColors(ColorPal::flashColors, order).end_c;
        tri_line[4].color = ColorPal::
            getCircularColors(ColorPal::flashColors, order).begin_c;
        tri_line[5].color = ColorPal::
            getCircularColors(ColorPal::flashColors, order).end_c;
      }
      win.draw(tri_line);

    } else {
      // Draw ordinary version
      // Single line
      sf::VertexArray one_line(sf::PrimitiveType::Lines, 2);
      one_line[0].position = sf::Vector2f(fvx,fvy);
      one_line[1].position = sf::Vector2f(fvdx, fvdy);
      // Special colors for Core Elements
      if (coreElement and displayCore) {
        one_line[0].color = sf::Color( 250, 250, 250, 255);
        one_line[1].color = sf::Color( 250, 250, 250, 255);
      } else {
        // Regular colors
        one_line[0].color = ColorPal::
            getCircularColors(ColorPal::normalColors, order).begin_c;
        one_line[1].color = ColorPal::
            getCircularColors(ColorPal::normalColors, order).end_c;
      }

      win.draw(one_line);
    }

  }

}

