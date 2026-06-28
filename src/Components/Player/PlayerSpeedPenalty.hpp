#ifndef SRC_COMPONENTS_PLAYER_PLAYERSPEEDPENALTY_HPP__
#define SRC_COMPONENTS_PLAYER_PLAYERSPEEDPENALTY_HPP__

namespace Game::Cmp
{

//! @brief  A percentage multipler is applied to the Player/Npc Lerp speed.
//!         e.g. 0.25 will make the player move at quarter of the Cmp::Persist::PlayerLerpSpeed
//!         This component should be added to the Player/Npc entity.
class PlayerSpeedPenalty
{
public:
  PlayerSpeedPenalty( float penalty )
      : m_penalty( penalty )
  {
  }
  float m_penalty;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_PLAYER_PLAYERSPEEDPENALTY_HPP__
