#ifndef __COMPONENTS_FONT_HPP__
#define __COMPONENTS_FONT_HPP__

#include <SFML/Graphics/Font.hpp>

namespace Game::Cmp
{

class Font : public sf::Font
{
public:
  Font( std::string font_path );
};

} // namespace Game::Cmp

#endif // __COMPONENTS_FONT_HPP__