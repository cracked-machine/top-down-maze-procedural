#ifndef __COMPONENTS_LARGE_OBSTACLE_HPP__
#define __COMPONENTS_LARGE_OBSTACLE_HPP__

#include <SFML/Graphics/RectangleShape.hpp>

#include <Components/Obstacle.hpp>

namespace ProceduralMaze::Cmp {

// LargeObstacle component represents a multi-block sprite defined geometrically as a rectangle
// It also tracks activation of its constituent sprites and overall activation state
class LargeObstacle : public sf::FloatRect
{
public:
  LargeObstacle( Sprites::SpriteMetaType type, const sf::Vector2f &position, const sf::Vector2f &size )
      : sf::FloatRect( position, size ),
        m_type( type )
  {
  }

  Sprites::SpriteMetaType getType() const { return m_type; }

  void increment_activated_sprite_count() { ++m_activated_sprite_count; }
  unsigned int get_activated_sprite_count() const { return m_activated_sprite_count; }

  void set_powers_active( bool activate = true ) { m_powers_active = activate; }
  bool are_powers_active() const { return m_powers_active; }

private:
  // The type of sprite to use from the SpriteFactory
  Sprites::SpriteMetaType m_type;

  // Track the number of activated sprites in this large obstacle
  // Callees are responsible for managing max thresholds
  unsigned int m_activated_sprite_count{ 0 };

  // Overall large obstacle activation. Has multiple meanings depending on context.
  // The callee is responsible for knowing when to set this. Usually when m_activated_sprite_count reaches some
  // threshold value.
  bool m_powers_active{ false };
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_LARGE_OBSTACLE_HPP__