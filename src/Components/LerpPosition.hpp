#ifndef SRC_COMPONENTS_LERPPOSITION_HPP__
#define SRC_COMPONENTS_LERPPOSITION_HPP__

#include <SFML/System/Vector2.hpp>

namespace Game::Cmp
{

//! @brief Holds in-motion lerp data for a given entity (player, npc, etc..)
struct LerpPosition
{
  //! @brief Construct lerp data targeting a destination position.
  //! @param target The world-space position being lerped towards.
  //! @param speed Lerp completion speed - affects how fast the lerp completes.
  LerpPosition( sf::Vector2f target, float speed = 1.0f )
      : m_target( target ),
        m_lerp_speed( speed )
  {
  }

  //! @brief The world-space position being lerped towards.
  sf::Vector2f m_target;

  //! @brief Lerp progress: 0.0 = start position, 1.0 = target position.
  float m_lerp_factor{ 0.0f };

  //! @brief Lerp completion speed - affects how fast lerp completes.
  float m_lerp_speed{};

  //! @brief Store the initial position. This should be set after construction.
  sf::Vector2f m_start;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_LERPPOSITION_HPP__
