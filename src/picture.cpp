// Copyright (c) 2026 Robert Gajewski
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "picture.h"
#include "text_draw.h"
#include "config.h"
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>
#include <unordered_map>
#include <string>
#include <SFML/Graphics/RenderWindow.hpp>
#include <cassert>
#include <string_view>

// Load Pictures and scale it


void PicPres::endOfGamePic(sf::RenderWindow & win) {
  std::string str {GalaxyImageVIEW};
  anyPicScaled(win, str, 1.0f);
}


void PicPres::anyPicScaled(sf::RenderWindow & win,
                           std::string picFile, float scale) {
  
  // find full image path
  std::string filepath = searchImageFilePath(picFile);
  // Load image from file or Texture Manager cache
  sf::Texture * texturePtr = texManager.load(picFile, filepath);
  if (!texturePtr) return; // abort - file cannot be loaded

  sf::Sprite sprite(*texturePtr);
  // semi-transparent
  sprite.setColor(sf::Color(255, 255, 255, 120));
  if (scale != 1.0f) {
    sprite.setScale({scale, scale}); // absolute scale factor
  }

  // Put to the right-bottom of frame
  sf::FloatRect bounds = sprite.getGlobalBounds();
  float x = static_cast<float>(win.getSize().x) - bounds.size.x;
  float y = static_cast<float>(win.getSize().y) - bounds.size.y;
  sprite.setPosition({x, y});
  win.draw(sprite);
}


// Establish full image file path
std::string PicPres::searchImageFilePath(const std::string_view imageFileStr) {
  // once established shall be the same
  static std::string dirpath_str {};
  
  std::string filepath_str {};
  
  if (mImageSubdirState == sDExists) {
    assert( !dirpath_str.empty() );
    // directory found
    filepath_str = dirpath_str; 
    filepath_str.append(imageFileStr);
    return filepath_str;
  } else if (mImageSubdirState == sDNotExists) {
    // If directory Not found try file name alone (that is use current directory)
    filepath_str = imageFileStr;
    return filepath_str;
  }

  assert(mImageSubdirState == sDNotChecked);
  // Try to find directory from list of possible locations
  const size_t cSubdirsNum { cPath::cImageSubdirs.size() };
  assert( cSubdirsNum > 0);
  size_t dir_index = 0;

  do {
    dirpath_str = cPath::cImageSubdirs[dir_index];
    (void)TextDraw::replace_home_alias(dirpath_str);
    std::filesystem::directory_entry imagePath{dirpath_str};
    // Check if subdirectory exists
    if (imagePath.exists()) {
      mImageSubdirState = sDExists;
      filepath_str = dirpath_str; 
      filepath_str.append(imageFileStr);
      Dbg::report_info("Image file location found/set to: " + filepath_str);
    } else {
      mImageSubdirState = sDNotExists;
      Dbg::report_info("Possible Image (sub)directory Not found: " + dirpath_str);
      // if (sub)directory not found, use current directory
      filepath_str = imageFileStr;
    }
    dir_index++;
  } while ((sDNotExists == mImageSubdirState) and (dir_index < cSubdirsNum));
  
  assert(mImageSubdirState != sDNotChecked);
  assert(dir_index <= cSubdirsNum);

  return filepath_str;
}
  

// Takes care to load texture/picture once only
sf::Texture * TextureManager::load(const std::string& name,
                                  const std::string& filename) {
  // Check if already loaded
  auto it = textures.find(name);
  if (it != textures.end()) {
    // Dbg::report_info("Texture \"" + name + "\" already loaded, returning cached version");
    return &it->second;
  }

  // Check if already has been try to load
  if (failedLoads.find(filename) != failedLoads.end())
  {
      // Skipping previously failed texture
      return nullptr;
  }
  
  // Try to Load new texture
  sf::Texture texture;
  if (!texture.loadFromFile(filename)) {
    // Remember this failed load
    failedLoads.insert(filename);
    Dbg::report_warning("Unsuccessfull texture load: " + name + " from " + filename);
    return nullptr;
  }
  
  // Load successfull - store it in local cache
  textures[name] = std::move(texture);
  Dbg::report_info("Loaded texture \"" + name + "\" from " + filename);
  return &textures[name];
}

    
bool TextureManager::isLoaded(const std::string& name) const {
  return textures.find(name) != textures.end();
}
    
void TextureManager::unload(const std::string& name) {
  textures.erase(name);
}

