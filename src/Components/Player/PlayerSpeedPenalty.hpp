#ifndef SRC_CMP_PLAYER_PLAYER_SPEEDPENALTY_HPP_
#define SRC_CMP_PLAYER_PLAYER_SPEEDPENALTY_HPP_

namespace ProceduralMaze::Cmp
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

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMP_PLAYER_PLAYER_SPEEDPENALTY_HPP_