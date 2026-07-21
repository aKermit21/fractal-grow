// Copyright (c) 2026 Robert Gajewski (pcc21.com)
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "progress.h"
#include <optional>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cassert>


// Present Size in flexible human readable format
SizePres sizeCheckAndFormat(float sizeInCm) {
    assert(sizeInCm >= 0.0f and "Size cannot be negative");

    // Enables 1'000'123 format
    struct apostrophe_separator : std::numpunct<char> {
    protected:
      char do_thousands_sep() const override { return '\''; }
      std::string do_grouping() const override { return "\3"; }
    };
  
    SizePres rec {};
    
    // Less than 100cm -> show in cm (no fractions)
    if (sizeInCm < 100.0f) {
        std::ostringstream oss;
        oss << static_cast<int>(std::round(sizeInCm)) << "cm";
        rec.inCm = sizeInCm;
        rec.print = oss.str();
        return rec;
    }
    
    // Convert to meters
    float sizeInM = sizeInCm / 100.0f;
    // From now on use meters
    rec.inM = sizeInM;
    
    // Less than 10m -> show in meters with 1 decimal
    if (sizeInM < 10.0f) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << sizeInM << " m";
        rec.print = oss.str();
        return rec;
    }
    
    // Less than 1000m -> show in meters (no fractions)
    if (sizeInM < 1000.0f) {
        std::ostringstream oss;
        oss << static_cast<int>(std::round(sizeInM)) << " m";
        rec.print = oss.str();
        return rec;
    }
    
    // Convert to kilometers
    float sizeInKm = sizeInM / 1000.0f;
    // From now on use kms
    rec.inKm = sizeInKm;
    
    // Less than 10km -> show in km with 1 decimal
    if (sizeInKm < 10.0f) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << sizeInKm << " km";
        rec.print = oss.str();
        return rec;
    }

    // Less than 1000km -> show in km
    if (sizeInKm < 1000.0f) {
        std::ostringstream oss;
        oss << static_cast<int>(std::round(sizeInKm)) << " km";
        rec.print = oss.str();
        return rec;
    }
    
    // Convert to 1000x kilometers
    float sizeInMm = sizeInKm / 1000.0f;
    // From now on use Mms
    rec.inMm = sizeInMm;

    // Less than 10Mm -> show in Mm with 1 decimal
    if (sizeInMm < 10.0f) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << sizeInMm << " thous. km";
        rec.print = oss.str();
        return rec;
    }
    
    // Lunar Distance - 384'000 km, 384 Mm
    constexpr static float cLD {384.0f};
    
    // Less than 1LD -> show in Mm 
    if (sizeInMm < cLD) {
        std::ostringstream oss;
        oss << static_cast<int>(round(sizeInMm)) << " thous. km";
        rec.print = oss.str();
        return rec;
    }

    // Convert to LD (Lunar Distance)
    float sizeInLD = sizeInMm / cLD;
    // From now on use LD
    rec.inLD = sizeInLD;
    
    // Less than 10LU -> show in LU with 1 decimal
    if (sizeInLD < 10.0f) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << sizeInLD << " LD (Earth-Moon)";
        rec.print = oss.str();
        return rec;
    }
    
    // Astronomical Units ~ 389 LD
    constexpr static float cAU {389.0f};
    
    // Less than 1AU -> show in LD
    if (sizeInLD < cAU) {
        std::ostringstream oss;
        oss << static_cast<int>(round(sizeInLD)) << " LD (Earth-Moon)";
        rec.print = oss.str();
        return rec;
    }

    // Convert to AU (Astronomical Unit)
    float sizeInAU = sizeInLD / cAU;
    // From now on use AU
    rec.inAU = sizeInAU;

    // Less than 10AU -> show in AU with 1 decimal
    if (sizeInAU < 10.0f) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << sizeInAU << " AU (Earth-Sun)";
        rec.print = oss.str();
        return rec;
    }

    // Light-year (ly) 1 ly≈63,241 AU
    constexpr static float cLY {63241.0f};

    // Less than 1ly -> show in AU
    if (sizeInAU < cLY) {
        std::ostringstream oss;
        // Apostroph 1'000'123 formatting
        oss.imbue(std::locale(std::locale(), new apostrophe_separator));
        oss << static_cast<int>(round(sizeInAU)) << " AU (Earth-Sun)";
        rec.print = oss.str();
        return rec;
    }

    // Convert to ly (Light-year)
    float sizeInLY = sizeInAU / cLY;
    // From now on use ly
    rec.inLy = sizeInLY;
    
    // 1 pc (parsec) ≈ 3.26 ly
    constexpr static float cPC {3.26f};

    // Less than 3.26ly -> show in ly with 1 decimal
    if (sizeInLY < cPC) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << sizeInLY << " ly (Lights-year)";
        rec.print = oss.str();
        return rec;
    }

    // Convert to pc (parsec)
    float sizeInPC = sizeInLY / cPC;
    // From now on use pc
    rec.inPc = sizeInPC;

    // Less than 10pc -> show in PC with 1 decimal
    if (sizeInPC < 10.0f) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << sizeInPC << " pc (parsec)";
        rec.print = oss.str();
        return rec;
    }

    // More than 10pc but less than kpc
    if (sizeInPC < 1000.0f) {
        std::ostringstream oss;
        oss << static_cast<int>(round(sizeInPC)) << " pc (parsec)";
        rec.print = oss.str();
        return rec;
    }

    // Convert to kpc (kilo parsec)
    float sizeInKPC = sizeInPC / 1000.0f;
    // From now on use kpc
    rec.inkPc = sizeInKPC;

    // Less than 8kpc -> show in kPC with 1 decimal
    if (sizeInKPC < 8.0f) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << sizeInKPC << " kpc (kilo parsec)";
        rec.print = oss.str();
        return rec;
    }

    std::ostringstream oss;
    oss << "~8'000 parsec, ~26'000 light-years";
    rec.endOfGame = true;
    rec.print = oss.str();
    return rec;
}


// Find appropriate image for given size
ImageRec findImageScale(const float size) {
    ImageRec imRec;
    imRec.found = false;

    SizePres sRec = sizeCheckAndFormat(size);

    if (sRec.inM.has_value()) {
        // Meters range
        // Man figure ?
    }
    
    if (sRec.inM.has_value()) {
        // m range
        float tSize = *sRec.inM;
        // Display range
        if ((tSize > 4) and (tSize < 30)) {
            // *** Testing - Begin ***
            // Only use/uncomment below section while testing picture insert
            // imRec.found = true;
            // imRec.file = "burj_khalifa.jpg";
            // imRec.text = "Testing!!! whatever image";
            // // Size of an object (100%)
            // imRec.scale = 5.f / tSize;
            // *** Testing - end ***
        }
        else if ((tSize > 50) and (tSize < 200)) {
            // *** Testing - Begin ***
            // Only use/uncomment below section while testing picture insert
            // imRec.found = true;
            // imRec.file = "not_found.jpg";
            // imRec.text = "Testing!!! not found image";
            // // Size of an object (100%)
            // imRec.scale = 100.f / tSize;
            // *** Testing - end ***
        }
        else if ((tSize > 300) and (tSize < 5000))  {
            imRec.found = true;
            imRec.file = "burj_khalifa.jpg";
            imRec.text = "of the the order of a skyscraper - Burj Khalifa";
            // Size of an object (100%)
            imRec.scale = 800.f / tSize;
        }
    }
    

    if (sRec.inKm.has_value()) {
        // Km range
        float tSize = *sRec.inKm;
        // Display range
        if ((tSize > 10) and (tSize < 100)) {
            imRec.found = true;
            imRec.file = "ny_city.jpg";
            imRec.text = "of the order of a Big City's (like New York) size";
            // Size of an object (100%)
            imRec.scale = 20.f / tSize;
        }
    }
    
    if (sRec.inMm.has_value()) {
        // MegaMeters / thous.km range
        float tSize = *sRec.inMm;
        if ((tSize > 14) and (tSize < 200)) {
            imRec.found = true;
            imRec.file = "Earth.jpg";
            imRec.text = "of the order of Earth's size";
            // if (tSize < 40.f) tSize = 40.f;
            imRec.scale = 40.f / tSize;
        } else if (tSize > 150 and (tSize < 1500))  {
            imRec.found = true;
            imRec.file = "Moon.jpg";
            imRec.text = "of the order of the Lunar Distance (LD)";
            // if (tSize < 300.f) tSize = 300.f;
            // *2 as result of small original picture
            imRec.scale = 300.f *2.f / tSize; 
        }
    }

    return imRec;
}
