#ifndef SRC_COMPONENTS_FONT_HPP__
#define SRC_COMPONENTS_FONT_HPP__

#include <SFML/Graphics/Font.hpp>

namespace Game::Cmp
{

//! @brief Component wrapping sf::Font, loaded from a file at construction.
class Font : public sf::Font
{
public:
  //! @brief Construct by loading a font from disk.
  //! @param font_path Path to the font file to load.
  Font( std::string font_path );
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_FONT_HPP__
