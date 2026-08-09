#ifndef SRC_COMPONENTS_ABSOLUTERENDEROFFSET_HPP__
#define SRC_COMPONENTS_ABSOLUTERENDEROFFSET_HPP__

#include <SFML/System/Vector2.hpp>

namespace Game::Cmp
{

//! @brief Component to store an absolute world-space render offset (in pixels)
//! Used by RenderGameSystem to visually displace a sprite without affecting its logical/collision position, this should be set by other systems
class AbsoluteRenderOffset
{
public:
  AbsoluteRenderOffset( sf::Vector2f offset = { 0.f, 0.f } )
      : m_offset( offset )
  {
  }

  sf::Vector2f getOffset() const { return m_offset; }
  void setOffset( sf::Vector2f offset ) { m_offset = offset; }

private:
  sf::Vector2f m_offset;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_ABSOLUTERENDEROFFSET_HPP__
