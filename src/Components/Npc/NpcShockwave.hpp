#ifndef SRC_COMPONENTS_NPC_NPCSHOCKWAVE_HPP__
#define SRC_COMPONENTS_NPC_NPCSHOCKWAVE_HPP__

#include <Sprites/Shockwave.hpp>

namespace Game::Cmp
{

//! @brief Wrapper component for Sprites::Shockwave
class NpcShockwave
{
public:
  NpcShockwave( sf::Vector2f pos, int points_per_segment )
      : sprite( pos, points_per_segment )
  {
  }

  Sprites::Shockwave sprite;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_NPC_NPCSHOCKWAVE_HPP__
