#ifndef SRC_COMPONENTS_PLAYER_MOVEMENTDELTA_HPP__
#define SRC_COMPONENTS_PLAYER_MOVEMENTDELTA_HPP__

namespace Game::Cmp::Player
{

//! @brief The real (post-collision) distance in pixels the player moved during the most recent
//! PlayerSystem::update_player_position() call. Zero whenever the player didn't actually move
//! (no input, movement suppressed, or blocked by collision) even if a direction key is held.
//! Downstream systems (footstep sfx cadence, walk-cycle animation framerate) accumulate/consume
//! this value so both automatically track real movement speed, including Cmp::Player::SpeedPenalty.
class MovementDelta
{
public:
  MovementDelta( float distance = 0.f )
      : m_distance( distance )
  {
  }
  float m_distance;
};

} // namespace Game::Cmp::Player

#endif // SRC_COMPONENTS_PLAYER_MOVEMENTDELTA_HPP__
