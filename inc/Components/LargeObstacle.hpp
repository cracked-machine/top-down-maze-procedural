#ifndef __COMPONENTS_LARGE_OBSTACLE_HPP__
#define __COMPONENTS_LARGE_OBSTACLE_HPP__

#include <Components/Obstacle.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace ProceduralMaze::Cmp {

struct LargeObstacle : public sf::FloatRect
{
  LargeObstacle( Sprites::SpriteMetaType type, const sf::Vector2f &position, const sf::Vector2f &size,
                 bool is_shrine = false )
      : sf::FloatRect( position, size ),
        m_type( type ),
        m_is_shrine( is_shrine )
  {
  }

  // The type of sprite to use from the SpriteFactory
  Sprites::SpriteMetaType m_type;

  // active powers are in effect. This is a one-shot, time-limited effect.
  bool m_powers_active{ false };

  // all powers have been used up / expired. Cannot be re-activated.
  bool m_powers_extinct{ false };

  bool is_shrine() const { return m_is_shrine; }

private:
  bool m_is_shrine{ false };
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_LARGE_OBSTACLE_HPP__