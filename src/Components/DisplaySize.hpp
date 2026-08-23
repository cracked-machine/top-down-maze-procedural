#ifndef SRC_COMPONENTS_DISPLAYSIZE_HPP__
#define SRC_COMPONENTS_DISPLAYSIZE_HPP__

#include <SFML/System/Vector2.hpp>

namespace Game::Cmp
{

//! @brief Holds the current display/window size in pixels.
struct DisplaySize
{
  //! @brief Width and height of the display/window, in pixels.
  sf::Vector2u m_size;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_DISPLAYSIZE_HPP__
