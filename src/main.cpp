// Copyright (c) 2026 Robert Gajewski
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "basics.h"
#include "aggreg.h"
#include "autoscale.h"
#include "mut_grow.h"
#include "opt_lyra.h"
#include "garbage_coll.h"
#include "fluctuate.h"
#include <cassert>
#include <iostream>
#include <optional>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

bool recurance_elements_redraw(Element * const prim_ptr, const long level, 
                sf::RenderWindow & win, const MovFluctuate & algo_anim, AutoScale & autoScale);


int main(int argc, const char** argv)
{
  try {
    srand(time(NULL));
    
    // analyze command CLI options
    OptParams options { optParse(argc, argv ) };
    assert(options.parseResult >= OptParams::ok and "parse conclusion missing?"); 
    // Early return if problems with parsing or help/version only called
    if (options.parseResult == OptParams::help ) { return 0; }
    else if (options.parseResult == OptParams::error ) { return 2; } // cmd parsing error
    else { /* continue */ }
  
    // Collecting errors, warning, info (trace); also Garbage collector: memory management
    MemAndDebug memDbg;
    // Auto (re)scalling
    AutoScale autoScale;

    // Top aggregation starts: Drawing, animation, colors, logs, key decode
    MainProgAggr fractMain(options);
    fractMain.postInitSync();

    std::string windowName {cFrac::ProgramName};
    if (options.optDemo) windowName = cFrac::DemoProgramName;

    sf::RenderWindow window(sf::VideoMode({cFrac::WindowXsize, cFrac::WindowYsize}), 
                            windowName);

    // First fractal element (order 0)
    Element prim_element;
    prim_element.initPrimary();

    while (window.isOpen()) {

      while (const std::optional<sf::Event> event = window.pollEvent()) {
        assert(event and "shall be non-empty event here");
        // Window button close
        if (event->is<sf::Event::Closed>()) {
          window.close();
          }
        // Key pressed event
        else if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
          assert(keyEvent and "shall be non-empty keyEvent here");
          // Close on X and Escape
          if (keyEvent->code == sf::Keyboard::Key::X) {
            window.close();
          } else {
            if ((keyEvent->code == sf::Keyboard::Key::R) or 
                (keyEvent->code == sf::Keyboard::Key::F3)) {
              // Reset
              prim_element.initPrimary();
              autoScale.resetAutoScale();
            } // intentionaly lack of else, reset handling continued below
            // Further Key decodation dispatcher
            fractMain.key_decodation(keyEvent->code, prim_element);
          }
        }
        else {
          // Another event than window-close or keyboard, maybe mouse?
          // Anyway igone!
        }
      }

      window.clear();

      autoScale.cycleStart();

      // put some possible background
      fractMain.drawBottomArtefacts(window);

      // Reconfigurate elements according to current algo and Draw in recurrence
      (void)recurance_elements_redraw(&prim_element, 0, window, 
                                      fractMain.movFluctuate, autoScale); // 0 - start level

      autoScale.performAutoscaleCycle(prim_element, MutGrow::isGlobalExcited());

      // Light source and/or possible text info - on top of drawing
      fractMain.drawTopArtefacts(window, autoScale, prim_element);

      window.display();
    
      if (!autoScale.ifRescaleActive()) {
        // possible signle step change of algo due to animation,
        // growing or flash (light effect). Check also End of Game.
        fractMain.oneStepCfgChange();
        // also possible demo generation step
        fractMain.demoGenerator();
        // Remove not used memory
        memDbg.pruneElementsClusterVector(200);
      }
    }
  }
  catch (const char * exception) {
    std::cerr << "Exception (Fatal Error): " << exception << std::endl;
  }
#ifdef NDEBUG
  // For release mode catch all exceptions
  catch(const std::exception & exception) {
    std::cerr << "Error - Standard (std) Exception: " << exception.what()  << std::endl;
  }
  catch(...) {
    std::cerr << "Error - Unknown Exception" << std::endl;
  }
  // std::cout << " NDEBUG !! (release mode)" << std::endl;
#endif

  // gc.release_all(); //Manually (not really needed) dynamic data cleanup
  return 0;
}

