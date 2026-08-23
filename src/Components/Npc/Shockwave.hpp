#ifndef SRC_COMPONENTS_NPC_SHOCKWAVE_HPP__
#define SRC_COMPONENTS_NPC_SHOCKWAVE_HPP__

#include <Sprites/Shockwave.hpp>

namespace Game::Cmp::Npc
{

//! @brief Wrapper component for Sprites::Shockwave
class Shockwave
{
public:
  //! @brief Construct the wrapped shockwave sprite.
  //! @param pos World-space origin of the shockwave.
  //! @param points_per_segment Resolution of the shockwave ring geometry.
  Shockwave( sf::Vector2f pos, int points_per_segment )
      : sprite( pos, points_per_segment )
  {
  }

  //! @brief The underlying shockwave sprite/animation.
  Sprites::Shockwave sprite;
};

} // namespace Game::Cmp::Npc

#endif // SRC_COMPONENTS_NPC_SHOCKWAVE_HPP__
