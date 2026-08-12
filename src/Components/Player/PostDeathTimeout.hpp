#ifndef SRC_CMPS_PLAYER_PLAYERPOST_DEATH_TIMEOUT_HPP__
#define SRC_CMPS_PLAYER_PLAYERPOST_DEATH_TIMEOUT_HPP__

namespace Game::Cmp::Player
{

//! @brief Added to the player entity after death occurs.
//!        Used as a cooldown timer to allow death animation to complete.
struct PostDeathTimeout : public sf::Clock
{
};

} // namespace Game::Cmp::Player

#endif // SRC_CMPS_PLAYER_PLAYERPOST_DEATH_TIMEOUT_HPP__