#ifndef SRC_CMPS_PLAYER_MOVEMENTSUPPRESSCOOLDOWN_HPP__
#define SRC_CMPS_PLAYER_MOVEMENTSUPPRESSCOOLDOWN_HPP__

namespace Game::Cmp::Player
{

//! @brief Added/refreshed on the player entity after a push/pull obstacle move to briefly suppress
//!        further player movement, animation and push/pull checks.
struct MovementSuppressCooldown : public sf::Clock
{
};

} // namespace Game::Cmp::Player

#endif // SRC_CMPS_PLAYER_MOVEMENTSUPPRESSCOOLDOWN_HPP__
