#ifndef SRC_COMPONENTS_PLAYER_PENDINGNOPATH_HPP__
#define SRC_COMPONENTS_PLAYER_PENDINGNOPATH_HPP__

namespace Game::Cmp::Player
{

//! @brief Temporary stand-in for Cmp::Player::NoPath on a segment created directly under the player
//! (e.g. a plant replanted at the player's own position), so it doesn't immediately block their own
//! movement. See PlayerSystem::promote_pending_no_path, which swaps this for a real Cmp::Player::NoPath
//! (and Cmp::Particle::BlockParticle) once the player is no longer standing on the entity.
struct PendingNoPath
{
};

} // namespace Game::Cmp::Player

#endif // SRC_COMPONENTS_PLAYER_PENDINGNOPATH_HPP__
