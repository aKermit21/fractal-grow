// Copyright (c) 2026 Robert Gajewski (pcc21.com)
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "basics.h"
#include "dbg_report.h"
#include <unordered_map>
#include <unordered_set>
#include <string_view>

// 
// Load Pictures and scale it


// Galaxy - End Of Game - image
inline constexpr std::string_view GalaxyImageVIEW = "Galaxy.jpg";

// Takes care to load texture/picture once only
struct TextureManager
{
    sf::Texture * load(const std::string& name, const std::string& filename);
    bool isLoaded(const std::string& name) const;
    void unload(const std::string& name);

private:
    std::unordered_map<std::string, sf::Texture> textures;
    // Load file failure
    std::unordered_set<std::string> failedLoads;  // Track failed files
    
};


struct PicPres {
  PicPres()
    : mImageSubdirState { sDNotChecked }
  {
    Dbg::report_info("Init: PicPres");
  } 
  
  // image files subdirectory
  enum SubDirState {sDNotChecked, sDExists, sDNotExists};

  // Insert End of Game - Galaxy - picture from image subdirectory
  void endOfGamePic(sf::RenderWindow & win);

  // Present any picture from image subdirectory and scale it
  void anyPicScaled(sf::RenderWindow & win, std::string picFile, float scale);

private:
  
  // Establish full image file path
  std::string searchImageFilePath(const std::string_view imageFile);
  
  TextureManager texManager;
  
  SubDirState mImageSubdirState;
  std::string mFilepath {};
  sf::Texture mTexture;
  
};
