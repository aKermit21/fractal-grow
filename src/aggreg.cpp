// Copyright (c) 2026 Robert Gajewski
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "aggreg.h"
#include "autoscale.h"
#include "dbg_report.h"
#include "pause_core.h"
#include "colors.h"
#include "basics.h"
#include "light.h"
#include "mut_grow.h"
#include "demo_func.h"
#include "fluctuate.h"
#include "garbage_coll.h"
#include "text_draw.h"
#include "progress.h"
#include <SFML/Window/Keyboard.hpp>
#include <sstream>
#include <string>
#include <string_view>
#include <sys/types.h>

// Main program aggregat (collection of) structs
// containing MovAnim(animation), Color Palette,
// and Light Source

 
// Post Construction (Initialization) Init, sync
void MainProgAggr::postInitSync() {
  // Synchronize flash color pallete with (just initialized) current light color
  colorPal.calc_flash_color_pallet(LightS::s_lightColor);
}

// One step per display loop - to be disapthed to subordinate classes/struct
void MainProgAggr::oneStepCfgChange() {
  float size = logtxt.textDraw.getSizeCm();
  // Check progress, phase of game base on structure size
  SizePres progress = sizeCheckAndFormat(size);
  // Check for End of Game
  if (!progress.endOfGame) {
    // initial growing has finished
    if ((!movFluctuate.fluctuateState.growingActive) and
        (!movFluctuate.isPauseActive())) {
      ++m_frame_cnt;
      // Store real start time after Initial grow has finished if not already started
      movFluctuate.startTimeCounting();
      MutGrow::handleGrowMutationStep();
    }
  } else {
    // End of Game: Stop Mutations permanently
    MutGrow::stopAlgo(true);
    // Reset only once to allow some manual actions after End of Game
    if (!m_EndOfGame) {
      m_EndOfGame = true;
      // Reset light to middle position
      lightS.reset_light();
    }
  }
  // Still dislay some animation like wind
  movFluctuate.one_step_cfg_change();
  colorPal.one_step_flash_reset();  
  lightS.one_step_light_resume(m_EndOfGame );
}

 
void MainProgAggr::resetConfig(bool keyAction) {
  movFluctuate.resumeTimeFlow(); 
  // Mutational grow algo reset
  MutGrow::resetAlgo();
  // Release Memory resources
  MemAndDebug::release_all();
  // if done by keyboard
  if (keyAction) { 
    // Reset light position
    lightS.reset_light();
    }
  // Refresh flash color pallete
  colorPal.calc_flash_color_pallet(LightS::s_lightColor);
  colorPal.reset_flash_algo();
  // Stop display Config Info
  logtxt.stopSnapshotDraw();
  movFluctuate.restartTimeCounting();
  m_frame_cnt = 0L;
  m_EndOfGame = false;
  m_TimeStarted = false;
}
  
// (Re)Draw some possible artefacts on top of fractal structure
void MainProgAggr::drawTopArtefacts(sf::RenderWindow & win,
                    const AutoScale & rescale, const Element & prim_element) {

  // Draw rescaling and optionally Lights structure
  float scale = rescale.getShrinkCumulativeFactor();
  logtxt.rescale_draw(win, scale, rescale.ifRescaleActive());

  // Draw light complete or partially
  bool lightPartialDraw = rescale.ifRescaleActive();
  lightS.light_draw(win, lightPartialDraw);  

  // Draw config (snapshot) info if requested
  logtxt.snapshot_draw(win);

  // Draw Color Flags of latest active mutation
  if (!m_EndOfGame and !m_PauseActive) {
    MutGrow::colorFlagsDraw(win);
  }
  
  // Draw Speed if requested
  auto speed = movFluctuate.get_speedScale();
  logtxt.speed_draw(win, speed);
  
  // Welcome Draw at the beginning
  if (!m_demoActive) {
    logtxt.welcome_draw(win, speed);
  }
  
  // Draw Help if requested
  logtxt.help_draw(win);

  // Active Pause info
  if (movFluctuate.isPauseActive()) {
    logtxt.pauseDraw(win);
  }
  
  // Draw Option Info
  // Provide additional info
  TextDraw::DevData data;
  data.primVec.dx = prim_element.stem_xy.vec_xy.dx;
  data.primVec.dy = prim_element.stem_xy.vec_xy.dy;
  data.coreLevels = MutGrow::getMaxLevel();
  data.ElPtrsCnt = MemAndDebug::getElPtrsCnt();
  data.excitedCnt = MutGrow::getExciteCounter();
  data.excitedAct = MutGrow::isGlobalExcited();
  data.frames = m_frame_cnt;
  if (!m_EndOfGame) {
    // update game time as long as game is running
    m_GameTime = movFluctuate.getTimeOfTheGame();
  }
  data.time = m_GameTime;
  logtxt.developer_draw(win, data);

  // Draw End of Game info
  if (m_EndOfGame) {
    logtxt.textDraw.endOfGame_draw(win, data, m_demoActive);
  }

}

// (Re)Draw some possible artefacts (picture) as background
void MainProgAggr::drawBottomArtefacts(sf::RenderWindow & win) {
  // Consider global CLI option
  if (m_optPictureOff) return;
  
  // Put End of Game picture
  if (m_EndOfGame) {
    picPres.endOfGamePic(win);
  }

  float size = logtxt.textDraw.getSizeCm();
  // Find proper image for this size
  ImageRec imageRec = findImageScale(size); // from progress.h/.cpp
  if (imageRec.found) {
    // Present picture itself
    picPres.anyPicScaled(win, imageRec.file, imageRec.scale);
    // Add picture description
    logtxt.textDraw.pictureInfo_draw(win, imageRec.text);
  }
}


// General key decodation
// can be dispatched to subordinate classes/structs
void MainProgAggr::key_decodation(const sf::Keyboard::Key key,
                                  Element& prim_element) {
  if (key == sf::Keyboard::Key::Space) {
    // Stop both types of animation
    movFluctuate.stopAnimation(); // Pause time flow
    movFluctuate.pauseWind();
    // Stop mutations (temporary)
    MutGrow::stopAlgo(false);
    // print current speed scale for # of frames
    logtxt.startSpeedDraw();
    m_PauseActive = true;
  }
  else if (key == sf::Keyboard::Key::R) {
    resetConfig(true); // key action
  }
  else if (key == sf::Keyboard::Key::Enter) {
    // Resume mutations if stopped
    MutGrow::resumeAlgo();
    movFluctuate.resumeWind();
    movFluctuate.resumeTimeFlow();
    m_PauseActive = false;
  }
  else if ((key == sf::Keyboard::Key::F1) or
           (key == sf::Keyboard::Key::Escape)) {
    // Help text will be appearing for some time
    logtxt.startHelpDraw();
  } 
  else if (key == sf::Keyboard::Key::F3) {
    resetConfig(true); // key action
    // Retrieve fractal snapshot/configuration from file
    logtxt.load_next_snapshot( prim_element, movFluctuate.algo_data, ColorPal::s_col_palet);
    // Refresh final transformation algo with optional growing animation
    movFluctuate.refreshWithRestartGrowing();
    // Allow display snapshot description (or time)
    logtxt.startSnapshotDraw();
  } 
  else if (key == sf::Keyboard::Key::PageUp) {
    // Increase size thus speed
    movFluctuate.speedIncrement();
    // draw speed scale for next xx frames
    logtxt.startSpeedDraw();
  } 
  else if (key == sf::Keyboard::Key::PageDown) {
    // Decrease size thus speed
    movFluctuate.speedDecrement();
    // draw speed scale for next xx frames
    logtxt.startSpeedDraw();
  } 
  else if (key == sf::Keyboard::Key::E) {
    // Switch developer info drawing
    logtxt.switchDevDraw();
  } 
  else {
    if (key == sf::Keyboard::Key::Grave) {
      // Additional global action in some cases
      colorPal.reset_flash_algo();
    }
    // Scan for subordinate classes key actions 
    bool keyFound = false;
    // possible Colors Palletes related control
    keyFound |= colorPal.key_decodation(key);
    
    // possible animation move control
    keyFound |= movFluctuate.StopFlash::key_decodation(key);
    
    // possible wind (wobbling) or growing animation control
    keyFound |= movFluctuate.MovFluctuate::key_decodation(key);
    
    // possible Light control
    auto l_lightRet = lightS.key_decodation(key);
    if (l_lightRet.colorChanged) {
      // Refresh additionally flash color pallete
      colorPal.calc_flash_color_pallet(LightS::s_lightColor);
    }
    keyFound |= l_lightRet.lightMoved;

    // Key decodation successfull by some base functionality/class
    if (keyFound) {  }
  }
  
}

// Single demo step
void MainProgAggr::demoGenerator(void) {
  // total Counter of demos
  static long int allDemoCnt { cFrac::DemoInitCnt };
  static long int lastActionDistance { 0 };

  if ((!m_demoActive) or (m_EndOfGame)) return;
  
  // Count demo frames
  Dbg::demo_frames(allDemoCnt);
  if (allDemoCnt < 0) {
    Dbg::report_warning("Demo Counter Overflow ? : ", allDemoCnt);
    allDemoCnt = cFrac::DemoInitCnt;
  }
  assert(lastActionDistance >= 0 and "Demo algo counter failure");

  if (allDemoCnt <= cFrac::DemoInitCnt) {
    // start demo with displaying help text (on top of fractal)
    logtxt.startHelpDraw();
    // Initialize other subordinate generators
    (void)lightS.demoGenerator(cFrac::DemoInitCnt);
    // Further initialization like seed generation
    srand(time(NULL));
    allDemoCnt = cFrac::DemoInitCnt +1;
    }
  else {
    ++lastActionDistance;
    ++allDemoCnt;
  }

  // Do NOT perform action one after another
  // especially color rotation
  if (lastActionDistance < 3) return;
  
  bool actionDone = false;

  if (lightS.demoGenerator(allDemoCnt)) {
    // Refresh flash color pallete if some color reconfiguration being made
    colorPal.calc_flash_color_pallet(LightS::s_lightColor);
    actionDone = true;
  }
  
  if (actionDone) lastActionDistance = 0;
}


