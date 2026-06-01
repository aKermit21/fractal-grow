// Copyright (c) 2026 Robert Gajewski
// (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "match_col.h"
#include <SFML/Graphics/Color.hpp>
#include <cmath>
#include <string>
#include <unordered_map>
#include <algorithm>

// Matching Colors
// Written mostly by AI

// Konwersja do sf::Color
sf::Color getColorFromMatch(ColorMatch match)
{
    switch (match)
    {
        case ColorMatch::Yellow:  return sf::Color::Yellow;
        case ColorMatch::Green:   return sf::Color::Green;
        case ColorMatch::Cyan:    return sf::Color::Cyan;
        case ColorMatch::Blue:    return sf::Color::Blue;
        case ColorMatch::Magenta: return sf::Color::Magenta;
        case ColorMatch::Red:     return sf::Color::Red;
        case ColorMatch::NotFound:  return sf::Color::Black;
    }
    return sf::Color::White;
}

// Conversion to string (for debugging)
std::string colorMatchToString(ColorMatch match)
{
    switch (match)
    {
        case ColorMatch::Yellow:  return "Yellow";
        case ColorMatch::Green:   return "Green";
        case ColorMatch::Cyan:    return "Cyan";
        case ColorMatch::Blue:    return "Blue";
        case ColorMatch::Magenta: return "Magenta";
        case ColorMatch::Red:     return "Red";
        case ColorMatch::NotFound:  return "NotFound";
    }
    return "Unknown";
}

// Metoda 1: Dopasowanie oparte na dominujących składowych
ColorMatch matchColorVectorBased(const sf::Color& color)
{
    int r = color.r;
    int g = color.g;
    int b = color.b;
    
    int maxComponent = std::max({r, g, b});
    int minComponent = std::min({r, g, b});
    
    // Check if color is achromatic - white/gray
    if (maxComponent - minComponent < 30)  // Thredhold for white/gray
    {
        // White/Gray - take default
        return ColorMatch::Yellow;
    }
    
    // Normalisation to 0-1 (not used)
    // float rf = r / 255.0f;
    // float gf = g / 255.0f;
    // float bf = b / 255.0f;
    
    // Sprawdź dominację czerwonego vs zielonego dla żółtego
    // Verify domination of red vs green for yellow
    if (r > 150 && g > 150 && b < 100)  // Znaczna ilość R i G, mało B
    {
        // Yellow (mixture R and G)
        // Check for possible dominance
        if (r > g * 1.5f)  // Red clear dominance
            return ColorMatch::Red;
        else if (g > r * 1.5f)  //  Green clear dominance
            return ColorMatch::Green;
        else
            return ColorMatch::Yellow;
    }
    
    // Green (dominance of G)
    if (g > r && g > b)
    {
        if (g > r * 1.3f && g > b * 1.3f)
            return ColorMatch::Green;
    }
    
    // Blue (dominanace of B)
    if (b > r && b > g)
    {
        if (b > r * 1.3f && b > g * 1.3f)
            return ColorMatch::Blue;
    }
    
    // Cyan (dominance of G and B)
    if (g > 150 && b > 150 && r < 100)
    {
        if (std::abs(g - b) < 50)  // G i B zbalansowane
            return ColorMatch::Cyan;
        else if (g > b)
            return ColorMatch::Green;
        else
            return ColorMatch::Blue;
    }
    
    // Magenta (dominance of R and B)
    if (r > 150 && b > 150 && g < 100)
    {
        if (std::abs(r - b) < 50)  // R i B zbalansowane
            return ColorMatch::Magenta;
        else if (r > b)
            return ColorMatch::Red;
        else
            return ColorMatch::Blue;
    }
    
    // Red (dominance of R)
    if (r > g && r > b)
    {
        if (r > g * 1.3f && r > b * 1.3f)
            return ColorMatch::Red;
    }
    
    // Default matching otherwise - less distance method
    return matchColorByDistance(color);
}

// Matching by euclidesian distance in RGB space
ColorMatch matchColorByDistance(const sf::Color& color)
{
    std::vector<std::pair<ColorMatch, sf::Color>> colors = {
        {ColorMatch::Yellow, sf::Color::Yellow},
        {ColorMatch::Green, sf::Color::Green},
        {ColorMatch::Cyan, sf::Color::Cyan},
        {ColorMatch::Blue, sf::Color::Blue},
        {ColorMatch::Magenta, sf::Color::Magenta},
        {ColorMatch::Red, sf::Color::Red}
    };
    
    ColorMatch bestMatch = ColorMatch::Yellow;
    float minDistance = std::numeric_limits<float>::max();
    
    for (const auto& [match, targetColor] : colors)
    {
        // Oblicz odległość euklidesową w przestrzeni RGB
        float dr = color.r - targetColor.r;
        float dg = color.g - targetColor.g;
        float db = color.b - targetColor.b;
        float distance = std::sqrt(dr*dr + dg*dg + db*db);
        
        if (distance < minDistance)
        {
            minDistance = distance;
            bestMatch = match;
        }
    }
    
    return bestMatch;
}

// Advance matching with wages for dominant colors
ColorMatch matchColorAdvanced(const sf::Color& color)
{
    int r = color.r;
    int g = color.g;
    int b = color.b;
    
    // Get normalized values
    float total = r + g + b;
    if (total == 0) total = 1;  // exclude division by zero
    
    float rNorm = r / total;
    float gNorm = g / total;
    float bNorm = b / total;
    
    // Defining ideal proportions for each color
    struct ColorProportion
    {
        ColorMatch match;
        float rWeight;
        float gWeight;
        float bWeight;
    };
    
    std::vector<ColorProportion> proportions = {
        {ColorMatch::Yellow, 0.5f, 0.5f, 0.0f},
        {ColorMatch::Green,  0.0f, 1.0f, 0.0f},
        {ColorMatch::Cyan,   0.0f, 0.5f, 0.5f},
        {ColorMatch::Blue,   0.0f, 0.0f, 1.0f},
        {ColorMatch::Magenta, 0.5f, 0.0f, 0.5f},
        {ColorMatch::Red,    1.0f, 0.0f, 0.0f}
    };
    
    ColorMatch bestMatch = ColorMatch::Yellow;
    float bestScore = -1.0f;
    
    for (const auto& prop : proportions)
    {
        // Oblicz podobieństwo do wzorca
        float diffR = std::abs(rNorm - prop.rWeight);
        float diffG = std::abs(gNorm - prop.gWeight);
        float diffB = std::abs(bNorm - prop.bWeight);
        
        // Score: the smaller difference the better
        float score = 1.0f - (diffR + diffG + diffB) / 2.0f;
        
        // Additional bonus if dominant component  is aligned
        if (prop.match == ColorMatch::Red && r > g && r > b)
            score += 0.2f;
        if (prop.match == ColorMatch::Green && g > r && g > b)
            score += 0.2f;
        if (prop.match == ColorMatch::Blue && b > r && b > g)
            score += 0.2f;
        if (prop.match == ColorMatch::Yellow && r > 100 && g > 100)
            score += 0.2f;
        if (prop.match == ColorMatch::Cyan && g > 100 && b > 100)
            score += 0.2f;
        if (prop.match == ColorMatch::Magenta && r > 100 && b > 100)
            score += 0.2f;
        
        if (score > bestScore)
        {
            bestScore = score;
            bestMatch = prop.match;
        }
    }
    
    return bestMatch;
}

// Main color matching function (choose method)
// ColorMatch matchColor(const sf::Color& color, bool useAdvanced = true)
ColorMatch matchColor(const sf::Color& color, bool useAdvanced )
{
    if (useAdvanced)
        return matchColorAdvanced(color);
    else
        return matchColorVectorBased(color);
}

// Example of usage
// Find matching color
// ColorMatch match = matchColor(colors[i], true);


sf::Color mediumColor(const sf::Color& color1, const sf::Color& color2)
{
    return sf::Color(
        (color1.r + color2.r) / 2,
        (color1.g + color2.g) / 2,
        (color1.b + color2.b) / 2,
        (color1.a + color2.a) / 2
    );
}


