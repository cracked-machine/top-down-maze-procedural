#ifndef __COMPONENTS_FONT_HPP__
#define __COMPONENTS_FONT_HPP__

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Font.hpp>
#include <base.hpp>

namespace ProceduralMaze::Cmp {

class Font : public sf::Font, Cmp::Base {
public:
    Font(std::string font_path)
    { 
        SPDLOG_DEBUG("Font()"); 
        if (!openFromFile(font_path)) { SPDLOG_ERROR("Unable to load {}", font_path); }
        
    }
    ~Font() { SPDLOG_DEBUG("~Font()"); }

};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_FONT_HPP__