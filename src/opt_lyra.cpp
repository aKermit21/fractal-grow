// Copyright (c) 2026 Robert Gajewski (pcc21.com)
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "opt_lyra.h"
#include "config.h"
#include "dbg_report.h"
#include "basics.h"
#include <iostream>

OptParams optParse(int argc, const char** argv)
{
  OptParams myArgs;
  myArgs.parseResult = OptParams::ok;
  bool showVersion { false };
  bool showHelp { false };

  // The parser with the multiple option arguments. They are composed
  // together by the "|" operator.
  auto cli = lyra::help(showHelp)
             .description(cFrac::ProgramName + " (" + PROJECT_STR + ")")
      | lyra::opt(showVersion)
            ["-v"]["--version"]("Version")
      | lyra::opt(myArgs.optDemo)
            ["-d"]["--demo"]("Run demo")
      | lyra::opt(myArgs.optFullScreen)
            ["-f"]["--full"]("Full Screen mode (hardcoded size by default)")
      | lyra::opt(myArgs.optPictureOff)
            ["-p"]["--nopic"]("Do Not display Progress Pictures (On by default)")
      | lyra::opt(myArgs.optGrowingOff)
            ["-g"]["--nogrow"]("Off Initial Growing (On by default)")
      | lyra::opt(myArgs.optSpeed, "speed")
            ["-s"]["--speed"]("Initial Speed vs Detail draw [0-20]")
      | lyra::opt(myArgs.optSnapshot, "file")
            ["-c"]["--config"]("Config File"); 

  // Parse the program arguments:
  auto result = cli.parse({ argc, argv });

  // Check that the arguments where valid:
  if (!result) {
      std::cerr << "Error in command line: " << result.message() << std::endl;
      std::cerr << cli << '\n'; 
      myArgs.parseResult = OptParams::error;
      return myArgs;
  }

  if (showVersion) {
    std::cout << "Program: " << cFrac::ProgramName << " (" << PROJECT_STR << ')' << '\n';
    std::cout << "Version: " << cFrac::Version << "\n\n";
  }
  
  // Show help when asked
  if (showHelp or showVersion) {
      std::cout << cli << '\n'; 
      myArgs.parseResult = OptParams::help;
  }

  Dbg::report_info("Option demo : ", myArgs.optDemo);
  Dbg::report_info("Option initial speed : ", myArgs.optSpeed);
  Dbg::report_info("Option Snapshot file: " + myArgs.optSnapshot); 
  
  return myArgs;
}
