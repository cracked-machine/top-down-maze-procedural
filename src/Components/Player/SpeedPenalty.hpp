#ifndef SRC_COMPONENTS_PLAYER_SPEEDPENALTY_HPP__
#define SRC_COMPONENTS_PLAYER_SPEEDPENALTY_HPP__

namespace Game::Cmp::Player
{

//! @brief  A percentage multipler is applied to the Player/Npc Lerp speed.
//!         e.g. 0.25 will make the player move at quarter of the Cmp::Persist::PlayerLerpSpeed
//!         This component should be added to the Player/Npc entity.
class SpeedPenalty
{
public:
  SpeedPenalty( float penalty )
      : m_penalty( penalty )
  {
  }
  float m_penalty;
};

} // namespace Game::Cmp::Player

#endif // SRC_COMPONENTS_PLAYER_SPEEDPENALTY_HPP__
