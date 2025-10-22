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

  // all powers have been used up / expired. Cannot be re-activated.
  bool m_powers_extinct{ false };

  bool is_shrine() const { return m_is_shrine; }

  void increment_active_count() { ++m_active_count; }
  unsigned int get_active_count() const { return m_active_count; }

  void set_powers_active() { m_powers_active = true; }
  bool are_powers_active() const { return m_powers_active; }

private:
  bool m_is_shrine{ false };
  unsigned int m_active_count{ 0 };
  // active powers are in effect. This is a one-shot, time-limited effect.
  bool m_powers_active{ false };
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_LARGE_OBSTACLE_HPP__