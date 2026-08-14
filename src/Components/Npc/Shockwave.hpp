#ifndef SRC_COMPONENTS_NPC_SHOCKWAVE_HPP__
#define SRC_COMPONENTS_NPC_SHOCKWAVE_HPP__

#include <Sprites/Shockwave.hpp>

namespace Game::Cmp::Npc
{

//! @brief Wrapper component for Sprites::Shockwave
class Shockwave
{
public:
  Shockwave( sf::Vector2f pos, int points_per_segment )
      : sprite( pos, points_per_segment )
  {
  }

  Sprites::Shockwave sprite;
};

} // namespace Game::Cmp::Npc

#endif // SRC_COMPONENTS_NPC_SHOCKWAVE_HPP__
