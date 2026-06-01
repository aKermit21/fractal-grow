// Copyright (c) 2026 Robert Gajewski
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "demo_func.h"
#include "assert.h"
#include <cstdlib>

// Demo common functions

// Modified random occurence every about # of frames
// so most of events happen around frames time;
// additionally it resets counter if event is to be realized
bool demoRand(int frames, long int & lastActionDistance) {

  if (lastActionDistanceIgnore == lastActionDistance) {
    // just normal random without condensing
    if ((rand() % frames) == 0) {
      return true;
    }
    return false;
  }

  // (nearly) forbid event threshold
  int forbidThreshold = static_cast<int>(((frames * 1) / 3));
  // discourage event threshold
  int earlyThreshold = static_cast<int>(((frames * 2) / 3));
  // encourage event threshold
  int lateThreshold = static_cast<int>(((frames * 4) / 3));

  if (lastActionDistance < forbidThreshold) return false;
    
  if (lastActionDistance < earlyThreshold) {
    // discourage
    if ((rand() % (frames * 4)) == 0) {
      // reset last action distance
      lastActionDistance = lastActionDistanceIgnore +1;
      // set event
      return true;
    }
    // return false;
  } else if (lastActionDistance < lateThreshold) {
    // normal
    if ((rand() % frames) == 0) {
      // reset last action distance
      lastActionDistance = lastActionDistanceIgnore +1;
      // set event
      return true;
    }
  } else {
    // encourage
    if ((rand() % (frames / 3)) == 0) {
      // reset last action distance
      lastActionDistance = lastActionDistanceIgnore +1;
      // set event
      return true;
    }
  }

  return false;
}


// Testing above algo
// TESTS
/*
constexpr size_t cNCases { 30 };

static std::array<int, cNCases +1> cases;
// (0)0-10%, (1)10-20%, etc., 30 - above 300% 

void presentRand() {
  for (size_t i = 0; i <= cNCases; ++i) {
    if (i < 10) std::cout << ' ';
    std::cout << i << "0% - " << cases[i] << '\n';
  }
  std::cout << std::endl;
}

void storeRand(int frames, long int distance) {
  double relativeDistance = static_cast<double>(distance)/frames;
  relativeDistance *= 10; // in 10%
  auto indexDistance = static_cast<size_t>(relativeDistance);
  if (indexDistance >= cNCases) {
    ++cases[cNCases];
  } else {
    ++cases[indexDistance];
    // assert(indexDistance > 0);
  }
}

int main() {
  int framesRand = 100;

  srand(time(NULL));

  long int distance = 1;
  long int trials = 0;
  do {
    ++trials;
    auto copyDistance = distance;
    if (demoRand(framesRand, distance)) {
      storeRand(framesRand, copyDistance);
      // distance is cleared by demoRand
    } else {
      ++distance;
    }

  } while ( trials <= 100000);

  presentRand();
}
*/
