// Copyright (c) 2026 Robert Gajewski (pcc21.com)
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include <optional>
#include <string>

// Progress, Size handling and presentation
// including End of Game check

struct ImageRec {
  bool found {false};
  std::string file;
  float scale {1.f};  
  std::string text;
};

struct SizePres {
  // format to print
  std::string print {};
  bool endOfGame {false};
  float inCm {}; // cm
  std::optional<float> inM {};  // m
  std::optional<float> inKm {}; // km
  std::optional<float> inMm {}; // Mm (thous. km)
  std::optional<float> inLD {}; // LD (Lunar Distance) 384'000 km
  std::optional<float> inAU {}; // AU (Astronomical Unit) Earth-Sun
  std::optional<float> inLy {}; // ly (lights-years)
  std::optional<float> inPc {}; // pc (parsecs)
  std::optional<float> inkPc {}; // kpc (kilo parsecs)
};

// Present Size in flexible human readable format
SizePres sizeCheckAndFormat(const float sizeInCm);

// Find appropriate image for give sc
ImageRec findImageScale(const float size); 
