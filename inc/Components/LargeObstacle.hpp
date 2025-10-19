#ifndef __COMPONENTS_LARGE_OBSTACLE_HPP__
#define __COMPONENTS_LARGE_OBSTACLE_HPP__

#include <Components/Obstacle.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace ProceduralMaze::Cmp {

struct LargeObstacle : public sf::FloatRect
{
  LargeObstacle( Sprites::SpriteMetaType type, const sf::Vector2f &position, const sf::Vector2f &size )
      : sf::FloatRect( position, size ),
        m_type( type )
  {
  }
  Sprites::SpriteMetaType m_type;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_LARGE_OBSTACLE_HPP__