// Copyright (c) 2026 Robert Gajewski (pcc21.com)
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "real_time.h"

//
// Real time related feature(s)

  
void RealTime::reportRealTimePerFrame() {
  auto nowTime = std::chrono::high_resolution_clock::now();

  if (mTimeStarted) {
    double elapsed_time_ms = 
      std::chrono::duration<double, std::milli>(nowTime - mTimeStamp).count();

    // Smart report - time per frame in ms if value is >10% change from previous
    Dbg::report_info_by_type(Dbg::infoTypeTimePerFrame, elapsed_time_ms);
  }

  // store new time stamp
  mTimeStamp = std::chrono::high_resolution_clock::now();
  mTimeStarted = true;
}


int RealTime::getFPS() {
  return mFPS;
} 
