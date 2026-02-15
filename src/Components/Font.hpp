#ifndef __COMPONENTS_FONT_HPP__
#define __COMPONENTS_FONT_HPP__

#include <SFML/Graphics/Font.hpp>

namespace ProceduralMaze::Cmp
{

class Font : public sf::Font
{
public:
  Font( std::string font_path );
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_FONT_HPP__