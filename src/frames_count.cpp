// Copyright (c) 2026 Robert Gajewski (pcc21.com)
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "frames_count.h"
#include "dbg_report.h"
#include <cassert>

//
// Counting frames for diffrent purposes

void Frames::reset(void) {
  mFrameCnt = 0;
  mFrameStamp = 0;
}

void Frames::increment(void) {
  ++mFrameCnt;
  assert(mFrameCnt >= 0);
}

// remember frames
void Frames::putStamp(void) {
  mFrameStamp = mFrameCnt;
}

// Delay in frames from latest stamp
long Frames::getFramesDelay(void) {
  assert(mFrameCnt >= mFrameStamp);
  return mFrameStamp - mFrameCnt; 
}

// Get Total Frames
long Frames::getTotalFrames(void) {
  return mFrameCnt;
}

// Derivatives of get FramesDelay
bool Frames::isFrameTime1TooLong(void) {
  assert(mFrameCnt >= mFrameStamp);
  return (mFrameCnt - mFrameStamp) > cTime1; 
}

bool Frames::isFrameTime2TooLong(void) {
  long delay = mFrameCnt - mFrameStamp;
  assert(delay >= 0);
  if (delay > cTime2) {
    Dbg::report_once(Dbg::onceTooLongFrameTime, delay);
    return true;
  }
  return false;
}

  
