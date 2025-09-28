#ifndef __CMP_LERPPOSITION_HPP__
#define __CMP_LERPPOSITION_HPP__

#include <SFML/System/Vector2.hpp>
#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::Cmp {

// Set the m_start position after construction
struct LerpPosition
{
  LerpPosition( sf::Vector2f target, float speed = 1.0f )
      : m_target( target ),
        m_lerp_speed( speed )
  {
  }
  sf::Vector2f m_target;
  // 0.0 = start position, 1.0 = target position
  float m_lerp_factor{ 0.0f };
  // Lerp completion speed - affects how fast lerp completes
  float m_lerp_speed{};
  // Store the initial position
  sf::Vector2f m_start;
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP_LERPPOSITION_HPP__