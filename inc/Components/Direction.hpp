#ifndef __COMPONENTS_DIRECTION_HPP__
#define __COMPONENTS_DIRECTION_HPP__

#include <Base.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Cmp {

// Coordinate
class Direction : public sf::Vector2f, Cmp::Base
{
public:
  Direction( const sf::Vector2f &dir ) : sf::Vector2f( dir ) { SPDLOG_DEBUG( "Direction()" ); }

  ~Direction() { SPDLOG_DEBUG( "~Direction()" ); }

  // used for sprite flipping:
  // 1.f for moving right, -1.f for moving left
  // default: 1.f otherwise we get no sprite until we move
  float x_scale = 1.f;
  // used for sprite positioning after flip:
  // 0 for moving right, m_sprite_factory->DEFAULT_SPRITE_SIZE.x for moving left
  float x_offset = 0.f;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_DIRECTION_HPP__