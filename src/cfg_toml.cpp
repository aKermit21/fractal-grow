// Copyright (c) 2026 Robert Gajewski (pcc21.com)
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "cfg_toml.h"
// #include "toml++/impl/forward_declarations.hpp"
#include "aux_func.h"
#include "colors.h"
#include "dbg_report.h"
#include "basics.h"
#include "toml++/impl/parse_error.hpp"
#include "toml++/impl/parser.hpp"
#include "transform.h"
#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>
#include <filesystem>



// Inteface - common error handling, return string to display info
std::string CfgToml::loadNextConfig(std::string filePath, Element & prim_element,
                             T_Algo_Arr & transform_algo, T_Col_Palet & colors) {
  std::string info {};
  auto success = loadNextConfigInternal(filePath, info, prim_element, transform_algo, colors);
  if (!success) {
    if (m_fileconfigState != cFileDoesNotExist) {
      Dbg::report_warning("Error parsing config file " + filePath +
                          "\n          correct file or delete it!");
      
      m_fileconfigState = cFileConfigCorrupted; 
    }
  }
  return info;
}


// next config from loaded file
bool CfgToml::loadNextConfigInternal(std::string filePath, std::string & info,
                                     Element & prim_element,  T_Algo_Arr & tranform_algo,
                                     T_Col_Palet & colors) {

 
  if (m_fileconfigState != cFileConfigLoaded) {
  
    // Try to load configs from file
    auto success = loadTomlConfig(filePath);
    if (!success) {
      if (m_fileconfigState != cFileDoesNotExist) {
        Dbg::report_warning("Error parsing config file " + filePath);
        m_fileconfigState = cFileConfigCorrupted; 
      }
      return false;
    }
  }

  assert(m_configMaxNumber > -1);

  // m_fileconfigState = cFileConfigCorrupted; // after succesfull parsing change this

  // Iterate configs from list starting from end
  if (m_currentConfig == 0) {
    m_currentConfig = m_configMaxNumber;
  } else {
    --m_currentConfig;
  }

   
  toml::table * table_ptr = allConfig["config"][m_currentConfig].as_table();
  if (!table_ptr) return false; // error
  
  auto thisConfig = *table_ptr;
  std::stringstream ss;
  ss << thisConfig << '\n';
  Dbg::report_trace(ss.str());

  T_Algo_Arr tmp_tran_algo {};
  T_Col_Palet tmp_colors {};
  Element tmp_prim {};

  // Handling possible warnings
  T_Result_Flags resultFlags { 0 };
  
  // Retrieving transform part
  // 
  for (size_t i=0; i < cFrac::NrOfElements; ++i) {

    // Reposition reading, accepted both integer (old format) and float (new)
    if (thisConfig["transform"]["element"][i]["reposition"].is_integer()) {
      std::optional<int> tmpint = thisConfig["transform"]["element"][i]
        ["reposition"].value<int>();
      if (!tmpint) return false;
      tmp_tran_algo[i].repos = (*tmpint) / 1000.0; // Integer assumed in promile
      resultFlags.set(cFlagIntInsteadOfFloat);
    } else if (thisConfig["transform"]["element"][i]["reposition"].is_floating_point()) {
      std::optional<float> tmpfloat = thisConfig["transform"]["element"][i]
        ["reposition"].value<float>();
      if (!tmpfloat) return false;
      tmp_tran_algo[i].repos = *tmpfloat;
    } else {
      return false;
    }
  
    // Angle reading
    if (!thisConfig["transform"]["element"][i]["angle"].is_integer()) return false;
    std::optional<int> tmpint = thisConfig["transform"]["element"][i]["angle"].value<int>();
    if (!tmpint) return false;

    // See log_trans_config() and conv_to_assym() transformation
    tmp_tran_algo[i].angle =
      myAux::zeroOneDegreesToRadians(static_cast<float>(*tmpint));
    tmp_tran_algo[i].angle_down = -tmp_tran_algo[i].angle;
  
    // Scale reading
    if (!thisConfig["transform"]["element"][i]["scale"].is_floating_point()) return false;
    std::optional<float> tmpfloat = thisConfig["transform"]["element"][i]
      ["scale"].value<float>();
    if (!tmpfloat) return false;
    tmp_tran_algo[i].scale = *tmpfloat;

    std::stringstream ss;
    ss << "repos " << tmp_tran_algo[i].repos << "; "
       << "angle " << tmp_tran_algo[i].angle << "; "
       << "angle-down " << tmp_tran_algo[i].angle_down << "; "
       << "scale " << tmp_tran_algo[i].scale; 
    Dbg::report_trace(ss.str());
  }

  // Copy this to live config/algo
  tranform_algo = tmp_tran_algo;
  
  // Retrieving Color part
  // For this part it is acceptable partial faulty config read 
  StemColor prev_colors {};
  
  for (size_t i=0; i < cFrac::NrOfColorPaletes; ++i) {
    bool typeFault = false;
    
    // Begin stem color
    // Red
    typeFault = false;
    if(!thisConfig["colors"]["level"][i]["begin"]["red"].is_integer()) {
      typeFault = true;
    }
    std::optional<int> tmpint = thisConfig["colors"]["level"][i]["begin"]["red"].value<int>();
    if (tmpint and !typeFault) {
      tmp_colors[i].begin_c.r = *tmpint;
      prev_colors.begin_c.r = *tmpint;
    } else {
      // Cannot decode
      if (i == 0) {
        ColorFatalError(resultFlags, m_currentConfig);
        return false;
      } else {
        // Use previous order color and continue
        tmp_colors[i].begin_c.r = prev_colors.begin_c.r;
        resultFlags.set(cFlagMissingColorWarning);
      }
    }   

    // Green
    typeFault = false;
    if(!thisConfig["colors"]["level"][i]["begin"]["green"].is_integer()) {
      typeFault = true;
    }
    tmpint = thisConfig["colors"]["level"][i]["begin"]["green"].value<int>();
    if (tmpint and !typeFault) {
      tmp_colors[i].begin_c.g = *tmpint;
      prev_colors.begin_c.g = *tmpint;
    } else {
      // Cannot decode
      if (i == 0) {
        ColorFatalError(resultFlags, m_currentConfig);
        return false;
      } else {
        // Use previous order color and continue
        tmp_colors[i].begin_c.g = prev_colors.begin_c.g;
        resultFlags.set(cFlagMissingColorWarning);
      }
    }   
   
    // Blue
    typeFault = false;
    if(!thisConfig["colors"]["level"][i]["begin"]["blue"].is_integer()) {
      typeFault = true;
    }
    tmpint = thisConfig["colors"]["level"][i]["begin"]["blue"].value<int>();
    if (tmpint and !typeFault) {
      tmp_colors[i].begin_c.b = *tmpint;
      prev_colors.begin_c.b = *tmpint;
    } else {
      // Cannot decode
      if (i == 0) {
        ColorFatalError(resultFlags, m_currentConfig);
        return false;
      } else {
        // Use previous order color and continue
        tmp_colors[i].begin_c.b = prev_colors.begin_c.b;
        resultFlags.set(cFlagMissingColorWarning);
      }
    }   
  
    // End stem color
    // Red
    typeFault = false;
    if(!thisConfig["colors"]["level"][i]["end"]["red"].is_integer()) {
      typeFault = true;
    }
    tmpint = thisConfig["colors"]["level"][i]["end"]["red"].value<int>();
    if (tmpint and !typeFault) {
      tmp_colors[i].end_c.r = *tmpint;
      prev_colors.end_c.r = *tmpint;
    } else {
      // Cannot decode
      if (i == 0) {
        ColorFatalError(resultFlags, m_currentConfig);
        return false;
      } else {
        // Use previous order color and continue
        tmp_colors[i].end_c.r = prev_colors.end_c.r;
        resultFlags.set(cFlagMissingColorWarning);
      }
    }   

    // Green
    typeFault = false;
    if(!thisConfig["colors"]["level"][i]["end"]["green"].is_integer()) {
      typeFault = true;
    }
    tmpint = thisConfig["colors"]["level"][i]["end"]["green"].value<int>();
    if (tmpint and !typeFault) {
      tmp_colors[i].end_c.g = *tmpint;
      prev_colors.end_c.g = *tmpint;
    } else {
      // Cannot decode
      if (i == 0) {
        ColorFatalError(resultFlags, m_currentConfig);
        return false;
      } else {
        // Use previous order color and continue
        tmp_colors[i].end_c.g = prev_colors.end_c.g;
        resultFlags.set(cFlagMissingColorWarning);
      }
    }   
   
    // Blue
    typeFault = false;
    if(!thisConfig["colors"]["level"][i]["end"]["blue"].is_integer()) {
      typeFault = true;
    }
    tmpint = thisConfig["colors"]["level"][i]["end"]["blue"].value<int>();
    if (tmpint and !typeFault) {
      tmp_colors[i].end_c.b = *tmpint;
      prev_colors.end_c.b = *tmpint;
    } else {
      // Cannot decode
      if (i == 0) {
        ColorFatalError(resultFlags, m_currentConfig);
        return false;
      } else {
        // Use previous order color and continue
        tmp_colors[i].end_c.b = prev_colors.end_c.b;
        resultFlags.set(cFlagMissingColorWarning);
      }
    }   
  }

  // Copy this to live color palete
  colors = tmp_colors;

  // Retrieving primary element
  // 
  if (!thisConfig["primary"]["x"].is_integer()) return false;
  std::optional<int> tmpint = thisConfig["primary"]["x"].value<int>();
  if (!tmpint) return false;
  tmp_prim.stem_xy.vec_xy.x = *tmpint;
  
  if (!thisConfig["primary"]["y"].is_integer()) return false;
  tmpint = thisConfig["primary"]["y"].value<int>();
  if (!tmpint) return false;
  tmp_prim.stem_xy.vec_xy.y = *tmpint;
  
  if (!thisConfig["primary"]["dx"].is_integer()) return false;
  tmpint = thisConfig["primary"]["dx"].value<int>();
  if (!tmpint) return false;
  tmp_prim.stem_xy.vec_xy.dx = *tmpint;
  tmp_prim.stem_xy.vec_xy.originalDx = tmp_prim.stem_xy.vec_xy.dx;
  
  if (!thisConfig["primary"]["dy"].is_integer()) return false;
  tmpint = thisConfig["primary"]["dy"].value<int>();
  if (!tmpint) return false;
  tmp_prim.stem_xy.vec_xy.dy = *tmpint;
  tmp_prim.stem_xy.vec_xy.originalDy = tmp_prim.stem_xy.vec_xy.dy;
  
  // if (!thisConfig["primary"]["width"].is_number()) return false;
  // auto tmpwidth = thisConfig["primary"]["width"].value<float>();
  // if (!tmpwidth) return false;
  // // Calculate coordinates of stem taking given width
  // tmp_prim.stem_xy.width = *tmpwidth;
  tmp_prim.stem_xy.recalculateStemWidthCoordinates(1.0); // No adjustment
  
  // Copy this to live Element
  prim_element.stem_xy = tmp_prim.stem_xy;

  // Prepare info text
  bool description_success = false;
  std::stringstream ss_info {};
  std::optional<std::string> tmpstr;
  ss_info << '[' << m_configMaxNumber-m_currentConfig+1 << '/'
          << m_configMaxNumber+1 << "] ";
  if (thisConfig["description"].is_string()) {
    tmpstr = thisConfig["description"].value<std::string>();
    if (tmpstr) {
      if (!(*tmpstr).empty()) {
        ss_info << *tmpstr;
        description_success = true;
      }
    }
  }
  if (!description_success) {
    if (thisConfig["time-date"].is_string()) {
      tmpstr = thisConfig["time-date"].value<std::string>();
      if (tmpstr) {
        if (!(*tmpstr).empty()) {
          ss_info << *tmpstr;
        } else {
          Dbg::report_warning("neither description nor time-date field can be displayed");
        }
      }
    } else {
      Dbg::report_warning("neither description nor time-date field can be displayed");
    }
  }
  info = ss_info.str();
  Dbg::report_info(info);

  if (resultFlags.test(cFlagError)) {
    return false;
  }
  if (resultFlags.test(cFlagMissingColorWarning)) {
    Dbg::report_info("TOML config: Some color orders are missing; copying from previous orders");
  }
  if (resultFlags.test(cFlagIntInsteadOfFloat)) {
    Dbg::report_warning("TOML config: Reposition number integer (expected float); assuming in promile");
  } 

  m_fileconfigState = cFileConfigLoaded; 
  return true; // success
}

  
// Parsing and loading Toml snapshot/config
bool CfgToml::loadTomlConfig(std::string filePath){
  Dbg::report_info("Loading config file " + filePath);

  // Check if cfg (snapshot) file exists in given directory
  std::filesystem::path logFile{ filePath };
  if (!exists(logFile)) {
    m_fileconfigState = cFileDoesNotExist;
    Dbg::report_warning("File - " + filePath + " - Do NOT exists.");
    return false; // error
  }

  toml::array allConfigArr;
  
  try {
    allConfig = toml::parse_file(filePath);
    auto ptr = allConfig.get_as<toml::array>("config");
    // toml::array* ptr = allConfig.get_as<toml::array>("config");
    // std::cout << "ptr: " << ptr << '\n';
    if (!ptr) return false;
    allConfigArr = *ptr;
    Dbg::report_info("Loaded configurations: ", allConfigArr.size());
  }
  catch (const toml::parse_error & err){
    std::stringstream ss;
    ss << "Error parsing config file '" << *err.source().path
       << "':\n" << err.description()
       << "\n (" << err.source().begin << ")\n"
       << "  Correct config or delete/restore the file";
    Dbg::report_warning(ss.str());
    m_fileconfigState = cFileConfigCorrupted;
    return false; // error
  }

  // Shall be non-empty array
  if (!allConfigArr.size()) return false; 
  m_configMaxNumber = allConfigArr.size() -1;

  m_fileconfigState = cFileConfigLoaded;
  return true; // success
}


// Report Color decodation Error on first stem - thus cannot assume previous color
void CfgToml::ColorFatalError(T_Result_Flags & result, int number){
  Dbg::report_warning("Error parsing colors; cannot continue with configuration ",
                      m_configMaxNumber - number +1); // Numbering from list end
  result.set(cFlagColorError);
}
 
