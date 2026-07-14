// Copyright (c) 2026 Robert Gajewski (pcc21.com)
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include "dbg_report.h"

//
// Counting frames for diffrent purposes

struct Frames {
  Frames()
    : mFrameCnt { 0 }
    , mFrameStamp { 0 }
  {
    Dbg::report_info("Init: Frames ", mFrameCnt);
  }

  // cTime1 frames after which time is assumed too long
  constexpr static int cTime1 { 500 };
  constexpr static int cTime2 { static_cast<int>(1.5 *cTime1) };
  
  void reset(void);
  void increment(void);
  
  // remember frames
  void putStamp(void);
  
  // Delay in frames from latest stamp
  long getFramesDelay(void);
  // Get Total Frames
  long getTotalFrames(void);

  // Derivatives of get FramesDelay
  bool isFrameTime1TooLong(void);
  bool isFrameTime2TooLong(void);
  
private:
  
  long mFrameCnt;

  // latest Stamp in frames
  long mFrameStamp;
  
};
